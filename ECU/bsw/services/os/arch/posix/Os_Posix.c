/*
 * =====================================================================================
 *  Project  : AUTOSAR-CP OS Shim (POSIX/Pthread)
 *  File     : Os_Posix.c
 *  Author   : HALA Academy
 *  Purpose  : Lớp giả lập (shim) cho OS AUTOSAR Classic chạy mô phỏng trên PC/macOS
 *             sử dụng pthread. Cung cấp các API tối thiểu:
 *               - Lifecycle  : StartOS, ShutdownOS
 *               - Task       : ActivateTask, TerminateTask
 *               - Event      : WaitEvent, SetEvent, GetEvent, ClearEvent
 *               - Alarm (ms) : SetRelAlarm
 *
 *  Thiết kế bám sát khái niệm AUTOSAR:
 *    - Task      ↔ pthread_t (mỗi ActivateTask tạo 1 thread chạy thân TASK())
 *    - Event     ↔ cặp mutex/condvar + bitmask (dành cho Extended Task, ví dụ Task_Com)
 *    - Counter   ↔ (bỏ qua), Alarm dùng thread ngủ theo ms để phát sinh chu kỳ
 *    - Schedule  ↔ (bỏ qua), đủ cho mô phỏng chu kỳ 10/100 ms
 *
 *  Ghi chú:
 *    - Đây là mã MÔ PHỎNG, KHÔNG phải OS RTOS thật; không có ưu tiên/preemptive.
 *    - API/kiểu dữ liệu công khai định nghĩa tại Os.h / Os_Types.h / Os_Cfg.h.
 *
 *  © HALA Academy. Dùng cho mục đích học tập & mô phỏng.
 * =====================================================================================
 */

#include "Os.h" /* API & kiểu dữ liệu OS công khai */

#include <pthread.h>   /* pthread_t, mutex, condvar */
#include <stdatomic.h> /* atomic_int cho cờ “đang chạy” */
#include <unistd.h>    /* sleep/usleep/pause */
#include <time.h>      /* nanosleep */
#include <stdio.h>     /* printf, perror */
#include <string.h>    /* memset (nếu cần) */
#include <signal.h>    /* _exit trên POSIX */
#include <errno.h>     /* errno cho log lỗi */

/* ****************************************************************************************
 * TIỆN ÍCH THỜI GIAN
 * **************************************************************************************** */
static void OS_SleepMs(uint32_t ms)
{
    struct timespec ts;
    ts.tv_sec = (time_t)(ms / 1000u);
    ts.tv_nsec = (long)((ms % 1000u) * 1000000L);
    (void)nanosleep(&ts, NULL);
}

/* ****************************************************************************************
 * QUẢN LÝ TASK (Task Control)
 * **************************************************************************************** */
typedef struct
{
    pthread_t thread;     /* Thread POSIX đại diện cho Task */
    pthread_mutex_t mtx;  /* Mutex bảo vệ event mask */
    pthread_cond_t cv;    /* Condition variable để WaitEvent thức dậy */
    EventMaskType events; /* Bitmask Event đang được set */
    atomic_int running;   /* 0 = chưa chạy/đã kết thúc ; 1 = đang chạy */
    const char *name;     /* Tên để in log */
    void (*entry)(void);  /* Con trỏ vào thân TASK() do ứng dụng định nghĩa */
} OsTaskCtl;

/* Bảng ánh xạ ID → entry function (do Os_Cfg.h công bố ID) */
static OsTaskCtl s_task[TASK_COUNT] =
    {
        [InitTask_ID] = {.entry = InitTask, .name = "InitTask"},
        [Task_10ms_ID] = {.entry = Task_10ms, .name = "Task_10ms"},
        [Task_100ms_ID] = {.entry = Task_100ms, .name = "Task_100ms"},
        [Task_Com_ID] = {.entry = Task_Com, .name = "Task_Com"},
        [Task_Can_ID] = {.entry = Task_Can, .name = "Task_Can"},
};

/* Cleanup đảm bảo PostTaskHook + running=0 luôn được gọi kể cả khi pthread_exit() */
static void OS_TaskCleanup(void *arg)
{
    OsTaskCtl *const tc = (OsTaskCtl *)arg;
    PostTaskHook(); /* gọi hook hậu-task */
    pthread_cond_destroy(&tc->cv);
    pthread_mutex_destroy(&tc->mtx);
    atomic_store(&tc->running, 0); /* đánh dấu task đã kết thúc (cho phép kích lại) */
    tc->thread = (pthread_t)NULL;  /* reset địa chỉ của task Os cần chạy lại */
}

/* Trampoline: gọi PreTaskHook → entry → đảm bảo cleanup */
static void *OS_TaskTrampoline(void *arg)
{
    OsTaskCtl *tc = (OsTaskCtl *)arg; // con trỏ đến block lưu trữ thông tin cấu hình cho task hiện tại
    atomic_store(&tc->running, 1);    // cập nhật trạng thái running

    /* Đăng ký cleanup: chạy khi thread return HOẶC khi pthread_exit() được gọi */
    pthread_cleanup_push(OS_TaskCleanup, tc); // đăng ký hàm dọn dẹp, sẽ chạy khi thread kết thúc bất thường.

    // con trỏ hàm để gọi ra task sẽ khởi tạo (Init)
    PreTaskHook();
    tc->entry();            /* Thân TASK(...) do app định nghĩa */
    pthread_cleanup_pop(1); /* 1 = thực thi cleanup ngay lúc pop */
    return NULL;
}

/* ****************************************************************************************
 * QUẢN LÝ ALARM (mức ms)
 * **************************************************************************************** */
typedef struct
{
    uint8_t id;        /* Alarm ID (theo Os_Cfg.h) */
    TickType start_ms; /* Trễ khởi động (ms) */
    TickType cycle_ms; /* Chu kỳ (ms); 0 = one-shot */
    pthread_t thread;  /* Thread POSIX đại diện cho Alarm */
    atomic_int active; /* 1 = đang hoạt động; 0 = dừng */
} OsAlarmCtl;

static OsAlarmCtl s_alarm[ALARM_COUNT] =
    {
        [Alarm_10ms] = {.id = Alarm_10ms},
        [Alarm_100ms] = {.id = Alarm_100ms},
};

/* ****************************************************************************************
 * API: TASK
 * **************************************************************************************** */
StatusType ActivateTask(TaskType t) //
{
    // kiểm tra Task_ID có hợp lệ dựa trên enum trong Os_Cfg.h
    if (t >= TASK_COUNT)
    {
        return E_OS_ID;
    }
    // static int counter = 0; //đếm số lượng task được khởi tạo

    OsTaskCtl *tc = &s_task[t]; // con trỏ đến thông tin cáu hình của 1 task trong bảng ánh xạ

    /* Chỉ tạo thread khi task chưa chạy */
    if (!atomic_load(&tc->running))
    {
        /* Khởi tạo sync primitives mỗi activation (đủ cho mô phỏng) */
        (void)pthread_mutex_init(&tc->mtx, NULL);
        (void)pthread_cond_init(&tc->cv, NULL);
        tc->events = 0;
        // printf("task count: %d\n",++counter);
        const int rc = pthread_create(&tc->thread, NULL, OS_TaskTrampoline, tc);
        if (rc != 0)
        {
            perror("[OS] pthread_create (task)");
            return E_OS_STATE;
        }

        /* Không cần join; detach để hệ thống tự thu dọn khi task kết thúc */
        (void)pthread_detach(tc->thread);
        return E_OK;
    }

    /* Task đang chạy → bản shim coi là hết slot (không queue thêm instance) */
    return E_OS_LIMIT;
}

StatusType TerminateTask(void)
{
    pthread_exit(NULL); /* sẽ kích hoạt OS_TaskCleanup() */
    PostTaskHook();
    return E_OK; /* unreachable (giữ prototype) */
}

/* ****************************************************************************************
 * API: EVENT cho Extended Task (ví dụ Task_Com)
 * **************************************************************************************** */
StatusType WaitEvent(EventMaskType m)
{
    /* Bản tối giản: giả định WaitEvent dùng cho Task_Com */
    OsTaskCtl *const tc = &s_task[Task_Com_ID];

    (void)pthread_mutex_lock(&tc->mtx);
    while ((tc->events & m) == 0u)
    {
        (void)pthread_cond_wait(&tc->cv, &tc->mtx);
    }
    (void)pthread_mutex_unlock(&tc->mtx);
    return E_OK;
}

StatusType SetEvent(TaskType t, EventMaskType m)
{
    if (t >= TASK_COUNT)
    {
        return E_OS_ID;
    }

    OsTaskCtl *const tc = &s_task[t];

    (void)pthread_mutex_lock(&tc->mtx);
    tc->events |= m;                       /* set bit */
    (void)pthread_cond_broadcast(&tc->cv); /* đánh thức task đang WaitEvent */
    (void)pthread_mutex_unlock(&tc->mtx);

    return E_OK;
}

StatusType GetEvent(TaskType t, EventMaskType *m)
{
    if ((t >= TASK_COUNT) || (m == NULL))
    {
        return E_OS_ID;
    }

    OsTaskCtl *const tc = &s_task[t];

    (void)pthread_mutex_lock(&tc->mtx);
    *m = tc->events;
    (void)pthread_mutex_unlock(&tc->mtx);
    return E_OK;
}

StatusType ClearEvent(EventMaskType m)
{
    /* Giả định gọi từ Task_Com trong mô phỏng */
    OsTaskCtl *const tc = &s_task[Task_Com_ID];

    (void)pthread_mutex_lock(&tc->mtx);
    tc->events &= (EventMaskType)(~m);
    (void)pthread_mutex_unlock(&tc->mtx);
    return E_OK;
}

/* ****************************************************************************************
 * API: ALARM (đơn vị mili-giây)
 * **************************************************************************************** */
static void *OS_AlarmThread(void *arg)
{
    OsAlarmCtl *const a = (OsAlarmCtl *)arg; // con trỏ đến block chứa cấu hình báo thức cho task chạy theo chu kỳ
    atomic_store(&a->active, 1);             // set trạng thái bắt đầu chạy

    /* Trễ khởi động một lần (nếu có) */
    if (a->start_ms > 0u)
    {
        OS_SleepMs(a->start_ms); // nạp thời gian bắt đầu xử lý vào bộ đếm
    }

    /* Vòng kích hoạt chu kỳ */
    while (atomic_load(&a->active)) // kiểm tra trạng thái của báo thức liên tục
    {
        /* Ánh xạ alarm → task chu kỳ tương ứng */
        switch (a->id)
        {
        case Alarm_10ms:
            // printf("[Os] :Alarm thread 10ms\n");
            (void)ActivateTask(Task_10ms_ID);
            (void)ActivateTask(Task_Can_ID);
            break;
        case Alarm_100ms:
            // printf("[Os] :Alarm thread 100ms\n");
            (void)ActivateTask(Task_100ms_ID);
            break;
        default:
            /* Không hỗ trợ alarm khác trong bản tối giản */
            break;
        }

        /* One-shot: thoát vòng nếu cycle_ms == 0 */
        if (a->cycle_ms == 0u)
        {
            break;
        }
        OS_SleepMs(a->cycle_ms); // thời gian lặp
    }

    atomic_store(&a->active, 0); // reset trạng thái báo thức
    return NULL;
}

StatusType SetRelAlarm(uint8_t alarmId, TickType start_ms, TickType cycle_ms)
{
    // kiểm tra Alarm_ID có hợp lệ dựa trên enum trong Os_Cfg.h
    if (alarmId >= ALARM_COUNT)
    {
        return E_OS_ID;
    }

    // con trỏ tới thông tin cấu hình alarm trong bảng ánh xạ loại báo thức cho task chạy theo chu kỳ
    OsAlarmCtl *const a = &s_alarm[alarmId];

    // cấu hình thời gian lặp lại báo thức cho task (chỉ gán 1 lần)
    a->start_ms = start_ms;
    a->cycle_ms = cycle_ms;

    // khởi tạo luồng chạy cho task chạy theo báo thức
    const int rc = pthread_create(&a->thread, NULL, OS_AlarmThread, a);
    if (rc != 0)
    {
        perror("[OS] pthread_create (alarm)");
        return E_OS_STATE;
    }

    /* Alarm cũng detach để tự thu dọn */
    (void)pthread_detach(a->thread); // Luồng Alarm chạy trên nền -> ko cần join để báo kết quả về hàm khởi tạo nó
    return E_OK;
}

/* ****************************************************************************************
 * LIFECYCLE OS
 * **************************************************************************************** */
StatusType StartOS(uint8_t appMode)
{
    (void)appMode;

    /* Reset trạng thái các Task */
    for (int i = 0; i < TASK_COUNT; ++i)
    {
        s_task[i].thread = (pthread_t)0;
        // mặc định các bit đều được reset -> chưa có task nào được set
        s_task[i].events = 0u;
        // Khai báo trạng thái ban đầu của từng task -> mỗi task sẽ sử dụng 1 biến running để xác định trạng thái đang chạy/dừng
        (void)atomic_exchange(&s_task[i].running, 0);
        /* Mutex/cond sẽ được init lại khi ActivateTask */
    }

    /* Reset trạng thái các Alarm */
    for (int i = 0; i < ALARM_COUNT; ++i)
    {
        s_alarm[i].thread = (pthread_t)0;
        s_alarm[i].start_ms = 0u;
        s_alarm[i].cycle_ms = 0u;
        (void)atomic_exchange(&s_alarm[i].active, 0);
    }

    /* Hook khởi động (app có thể in log) */
    StartupHook();

    /* Autostart: InitTask */
    (void)ActivateTask(InitTask_ID);

    /* Khởi tạo sẵn và đợi event để xử lý*/
    (void)ActivateTask(Task_Com_ID);

    /* Vòng “idle” giữ process sống trong mô phỏng */
    for (;;)
    {
        sleep(1);
    }
    /* Unreachable */
}

void ShutdownOS(StatusType e)
{
    (void)e;
    ShutdownHook(e);
    _exit(0); /* kết thúc tiến trình mô phỏng */
}

/* ****************************************************************************************
 * HOOKS MẶC ĐỊNH (weak) — Ứng dụng có thể định nghĩa lại trong app/hooks/Os_Hook.c
 * **************************************************************************************** */
__attribute__((weak)) void StartupHook(void) { /* no-op */ }
__attribute__((weak)) void ShutdownHook(StatusType error) { (void)error; }
__attribute__((weak)) void PreTaskHook(void) { /* no-op */ }
__attribute__((weak)) void PostTaskHook(void) { /* no-op */ }

#include "Os.h"
#include <stdio.h>
#include "Rte.h"
#include "Rte_batch.h"
/* Task theo sự kiện: chờ EV_RX / EV_TX từ “ISR mô phỏng” (Com feeder) */
TASK(Task_Com)
{
    EventMaskType ev;

    for(;;) {
        /* Đợi bất kỳ sự kiện RX/TX */
        (void)WaitEvent(EV_RX | EV_TX);

        /* Đọc mặt nạ sự kiện hiện tại của chính Task_Com */
        (void)GetEvent(Task_Com_ID, &ev);

        if (ev & EV_RX) {
            (void)ClearEvent(EV_RX);
            Rte_Com_RxBatch();   /* xử lý gói tin nhận: map vào RTE buffers */
        }
        if (ev & EV_TX) {
            (void)ClearEvent(EV_TX);
            //Rte_Com_TxBatch();   /* pack từ RTE buffers → BSW/COM gửi đi */
        }
    }

    //printf("[Task_Com] running...\n");
    /* Không TerminateTask vì đây là task vòng lặp sự kiện */
}

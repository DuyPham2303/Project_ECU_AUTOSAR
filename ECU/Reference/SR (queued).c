#include <stdint.h>
#include <stdbool.h>
#define RTE_E_NOT_OK    ((uint8_t)0)
#define RTE_E_OK        ((uint8_t)1)

typedef bool boolean;

typedef uint8_t Std_ReturnType;
//Khai báo chuẩn
/* Gửi (producer) */
//Std_ReturnType Rte_Send_<C>_<P>_<D>(IN <Type> value);

/* Nhận (consumer) -- rút từng phần tử khỏi hàng đợi */
//Std_ReturnType Rte_Receive_<C>_<R>_<D>(OUT <Type>* p);


/* 
- Mã lỗi thường gặp
    + Hàng đợi trống: RTE_E_NO_DATA (tên có thể khác tùy tool).
    + Hàng đợi đầy/giới hạn: RTE_E_LIMIT. 
*/

    //Ví dụ nhanh — Button edge
// /* Producer: Swc_ButtonIf phát hiện cạnh và gửi sự kiện */
// (void)Rte_Send_ButtonIf_ButtonEvt_Edge(BTN_EDGE_RISE);

// /* Consumer: Swc_Hmi "xả" queue đến khi hết sự kiện */
// BtnEdge_e e;
// while (Rte_Receive_Hmi_ButtonEvt_Edge(&e) == RTE_E_OK) {
//     if (e == BTN_EDGE_RISE) Ui_ToggleMenu();
//     if (e == BTN_EDGE_FALL) Ui_Beep();
// }

/* -------------------------------------VÍ DỤ CỤ THỂ---------------------------------------- */

/* 
    + queued: Button events → HmiMgr
*/

typedef enum { 
    BTN_EDGE_NONE=0, 
    BTN_EDGE_RISE=1, 
    BTN_EDGE_FALL=2 
}ButtonEvent_e;

#define BUTTON_PRESSED  1
#define BUTTON_RELEASED 2

/* Swc_ButtonIf_ISR or Runnable signaled by ISR */
void Swc_ButtonIf_OnEdge(boolean pressed)
{
    ButtonEvent_e ev = pressed ? BUTTON_PRESSED : BUTTON_RELEASED;
    (void)Rte_Send_ButtonIf_PPort_ButtonEvent(ev);  /* enqueue event */
}

/* Swc_HmiMgr_Run10ms: đọc hết hàng đợi trong mỗi tick */
void Swc_HmiMgr_Run10ms(void)
{
    ButtonEvent_e ev;
    while (Rte_Receive_HmiMgr_RPort_ButtonEvent(&ev) == RTE_E_OK) {
        Hmi_HandleButton(ev);
    }
}


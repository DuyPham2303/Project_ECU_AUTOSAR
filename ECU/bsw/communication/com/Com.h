#ifndef COM_H
#define COM_H

#include "Std_Types.h"
#include "CanIf.h"  /* để dùng PduIdType và PDU_ENGINE_SPEED */

<<<<<<< HEAD
void Com_Init();    //khởi tạo PduR và Can 
/* signal có dữ liệu mới và lưu vào buffer nội bộ COM */
void  Com_RxIndication(PduIdType pduId, const uint8* data, uint8 dlc); 

/* Gọi định kỳ trong SchM_MainFunction_10ms (task 10ms)
    - decode nội dung gói tin 
    - cập nhật dữ liệu cho tầng Application
*/
void Com_MainFunctionRx();

/* Gọi trong Rte_Com_RxBatch khi có RX event (task com) 
    - map dữ liệu cho rte buffer 
    - gọi Rte_Com_Update_EngineSpeedFromPdu()
*/
Std_ReturnType Com_EngineSpeed(uint16* rpm); 
=======
void Com_Init();    //dùng để khởi tạo PduR và Can 
void           Com_RxIndication(PduIdType pduId, const uint8* data, uint8 dlc); //gọi bởi PduR để gừi gói Pdu
Std_ReturnType Com_EngineSpeed(uint16* rpm);  /* đọc giá trị đã giải mã */
>>>>>>> 8802487a3d94ff76a59b87df7c7baeedf94db7c9
#endif /* COM_H */

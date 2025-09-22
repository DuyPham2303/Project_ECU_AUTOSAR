#include "Com.h"
#include "Rte.h"
#include <stdio.h>

/* Buffer & cờ trạng thái cho PDU_ENGINE_SPEED */
static volatile uint16  s_engineSpeedRpm     = 0u;
static volatile boolean s_engineSpeedUpdated = FALSE;

/* decode mã CAN thành giá trị thực tế */
static void Com_Unpack_EngineSpeed(const uint8* data, uint8 dlc)
{
    if (dlc < 2u) {
        printf("[Com] EngineSpeed: DLC=%u < 2 -> bỏ\n", (unsigned)dlc);
        return;
    }
    /* Big-endian (Motorola): rpm = (data[0] << 8) | data[1] */
    uint16 rpm = (uint16)((((uint16)data[0]) << 8) | ((uint16)data[1]));

    //cập nhật vào buffer sẵn sàng dể Rte sử dụng
    s_engineSpeedRpm = rpm;
    s_engineSpeedUpdated = TRUE;

    printf("[Com] EngineSpeed unpack: %u rpm (bytes %02X %02X)\n",
           (unsigned)rpm, (unsigned)data[0], (unsigned)data[1]);
}

/* Xác định CAN ID tương ứng với loại dữ liệu cần xử lý */
void Com_RxIndication(PduIdType pduId, const uint8* data, uint8 dlc)
{
    if (pduId == PDU_ENGINE_SPEED) {
        /* 1) Đẩy thẳng lên RTE (data[0]=MSB, data[1]=LSB) */
        Rte_Com_Update_EngineSpeedFromPdu(data, dlc);

        /* 2) (tuỳ chọn) vẫn giải mã nội bộ để Com_EngineSpeed() dùng */
        Com_Unpack_EngineSpeed(data, dlc);

        printf("[Com] RxIndication: PDU_ENGINE_SPEED → RTE + legacy buffer\n");
        /* (nếu có notification khác, gọi thêm ở đây) */
    } else {
        printf("[Com] RxIndication: pduId=%u không hỗ trợ\n", (unsigned)pduId);
    }
}

//cập nhật buffer đã decode
Std_ReturnType Com_EngineSpeed(uint16* rpm)
{
    if (rpm == NULL) return E_NOT_OK;
    if (s_engineSpeedUpdated == TRUE) {
        *rpm = s_engineSpeedRpm;
        s_engineSpeedUpdated = FALSE; /* consume flag (kiểu ON_CHANGE) */
        return E_OK;
    }
    return E_NOT_OK;
}

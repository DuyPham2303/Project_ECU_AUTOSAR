#include "Com.h"
#include "PduR.h"
#include <stdio.h>
#include <string.h>

//Biến thông số cục bộ COM để lưu trữ dữ liệu truyền/nhận giữa Rte và tầng dưới 
static uint8 s_engineSpeedFrame[8]; /* buffer lưu raw CAN frame tạm thời */
static uint16 s_engineSpeedRpm;     /* buffer lưu rpm giải mã */
static uint8 s_engineSpeedDlc = 0;  /* buffer lưu DLC */
static boolean s_engineSpeedUpdated = FALSE;    //flag cập nhật trạng thái 

static const uint16 MAX_RPM = 924;
static const uint8 MAX_KMH = 75;
// cập nhật buffer đã decode
Std_ReturnType Com_EngineSpeed(uint16 *rpm)
{
    if (rpm == NULL)
        return E_NOT_OK;
    if (s_engineSpeedUpdated == TRUE)
    {
        *rpm = s_engineSpeedRpm;
        s_engineSpeedUpdated = FALSE; /* consume flag (kiểu ON_CHANGE) */
        return E_OK;
    }
    return E_NOT_OK;
}

/* decode mã CAN thành giá trị thực tế */
void Com_Init()
{
    CanIf_Init();
    PduR_Init();    
    printf("[Com] Initialized (calling CanIf_Init & PduR_Init)\n");
}

static void Com_Unpack_EngineSpeed(const uint8 *data, uint8 dlc)
{
    if (dlc < 2u)
    {
        printf("[Com] EngineSpeed: DLC=%u < 2 -> bỏ\n", (unsigned)dlc);
        return;
    }

    // Little-engian -> decode raw speed từ mảng 8 byte thô từ Can
    uint16 raw_kmh = (uint16)((((uint16)data[2]) << 8) | ((uint16)data[1]));

    float kmh = raw_kmh / 100.0;

    if (kmh > MAX_KMH)
        kmh = MAX_KMH;

    uint16 rpm = (MAX_RPM * kmh) / MAX_KMH;

    s_engineSpeedRpm = rpm;         //cập nhật final rpm vào buffer nội bộ Com
    s_engineSpeedUpdated = TRUE;    //cập nhật flag đánh dấu đã lưu rpm mới 

    printf("[Com] EngineSpeed unpack: %u rpm(bytes %02X %02X)\t kmh = %.2f\n",
           (unsigned)rpm, (unsigned)data[0], (unsigned)data[1], kmh);
}

/* Xác định CAN ID tương ứng với loại dữ liệu cần xử lý */
void Com_RxIndication(PduIdType pduId, const uint8 *data, uint8 dlc)
{
    if (pduId == PDU_ENGINE_SPEED)
    {

        /* cập nhật dữ liệu đã decode vào buffer nội bộ*/
        memcpy(s_engineSpeedFrame, data, dlc);
        s_engineSpeedDlc = dlc;

        printf("[Com] RxIndication: PDU_ENGINE_SPEED → RTE + legacy buffer\n");
        /* (nếu có notification khác, gọi thêm ở đây) */
    }
    else
    {
        printf("[Com] RxIndication: pduId=%u không hỗ trợ\n", (unsigned)pduId);
    }
}

void Com_MainFunctionRx()
{

    printf("[Com] Com_MainFunctionRx : unpack EngineSpeed\n");

    /* Decode dữ liệu raw */
    Com_Unpack_EngineSpeed(s_engineSpeedFrame, s_engineSpeedDlc);
}
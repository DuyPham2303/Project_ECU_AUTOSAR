#include "Can.h"
#include "csv_io.h"
#include "Std_Types.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

/* CanIf API: driver sẽ gọi khi đọc xong 1 frame */
Std_ReturnType CanIf_Receive(uint32 canId, const uint8* data, uint8 dlc);

/* helper: skip spaces */
static char* skip_spaces(char* p) {
    while (p && *p && isspace((unsigned char)*p)) ++p;
    return p;
}

/* parse một dòng dạng: "can,0x3F0 8 03 80 25 00 00 00 00 00"
 * out: id, dlc, data[]
 * trả TRUE nếu parse OK
 */
static boolean parse_can_line(char* line, uint32* outId, uint8* outDlc, uint8 outData[8])
{
    if (!line || !outId || !outDlc || !outData) return FALSE;
    char* p = line;
    char* end = NULL;
    unsigned long id = strtoul(p, &end, 0);
    if (end == p) return FALSE; /* không parse được */
    //printf("parsed %ld OK\n",id);
    p = skip_spaces(end);

    /* parse dlc */
    unsigned long dlc_ul = strtoul(p, &end, 0);
    if (end == p) return FALSE;
    if (dlc_ul > 8ul) dlc_ul = 8ul; /* giới hạn */
    p = skip_spaces(end);
    //printf("parsed %ld OK\n",dlc_ul);

    /* parse dữ liệu theo dlc */
    uint8 dlc = (uint8)dlc_ul;
    for (uint8 i = 0; i < dlc; i++) {
        unsigned long b = strtoul(p, &end, 16);
        if (end == p) return FALSE;
        outData[i] = (uint8)b;
        //printf("parsed data[%d] : %ld OK\n",i,outData[i]);
        p = skip_spaces(end);
    }
    /* các byte còn lại (nếu dlc < 8) set 0 */
    for (uint8 i = dlc; i < 8u; i++) outData[i] = 0u;

    *outId  = (uint32)id;
    *outDlc = dlc;
    return TRUE;
}

void Can_Init(void)
{
    printf("[CAN] Init OK (CSV source, polling via Can_MainFunction_Read)\n");
}

void Can_MainFunction_Read(void)
{
    //buffer chứa chuỗi Can frame
    char frame_str[128];
    /* đọc tất cả dòng còn lại cho channel "can" */
    re_read:
    //đợi đến khi đọc xong 
    while(csv_getString("can", frame_str, sizeof(frame_str)) == TRUE);
    printf("[CAN] CSV: \"%s\"\n", frame_str);

    //nơi lưu trữ parsed Can frame -> 
    uint32 id = 0;
    uint8  dlc = 0;
    uint8  data[8];

    /* tạo bản sao có thể chỉnh sửa vì parse dùng strtok/ptr */
    char buf[128];
    size_t n = strlen(frame_str);
    if (n >= sizeof(buf)) n = sizeof(buf) - 1;
    memcpy(buf, frame_str, n);
    buf[n] = '\0';
    if (!parse_can_line(buf, &id, &dlc, data)) {
        printf("[CAN] Parse FAIL -> bỏ qua dòng\n");
        memset(frame_str,0,sizeof(frame_str));
        goto re_read; //label để quay lại nếu đọc frame Can thất bại 
    }

    printf("[CAN] RX: ID=0x%X DLC=%u Data=", (unsigned)id, (unsigned)dlc);

    for (uint8 i = 0; i < dlc; i++) printf("%02X ", data[i]);
    printf("\n");

    /* 
        - lớp phân phối, xác định frame hiện tại thuộc PDU nào
        - chịu trách nhiệm giao đúng gói dữ liệu cho tầng trên
    */
    (void)CanIf_Receive(id, data, dlc);  //xử lý chuyên biệt cho giao thức CAN
}

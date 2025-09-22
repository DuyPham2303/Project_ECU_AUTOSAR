#include "Os.h"
#include "EcuM.h"   /* Khai báo EcuM_Init() */
#include <stdio.h>
/* App mode mặc định đã định nghĩa trong Os_Cfg.h (được include qua Os.h) */

int main(void)
{
    /* Pre-OS init (tối thiểu) */
    EcuM_Init();

    /* Vào OS → autostart InitTask */
    (void)StartOS(OSDEFAULTAPPMODE);

    /* Không bao giờ quay lại đây trong mô phỏng */
    for(;;) { }
    return 0;
}

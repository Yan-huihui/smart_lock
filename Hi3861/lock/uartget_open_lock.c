#include <stdio.h>
#include "cmsis_os2.h"
#include "ohos_init.h"
#include "iot_gpio_ex.h"
#include "hi_errno.h"
#include "hi_types_base.h"
#include "lock.h"
#include "uartget_open_lock.h"

int FACE_FLAG = 0;

static void *findFaceOpenLock(void)
{
    
    while (1){
        if(FACE_FLAG == 1){
            setClock(Lock_Close_After_Open);
            TaskMsleep(3000);
            FACE_FLAG = 0;
        }
        TaskMsleep(20);
    }
    
}

hi_u32 findFaceOpenLockTack(hi_void)
{
    osThreadAttr_t hisignallingAttr = {0};

    hisignallingAttr.stack_size = 512;
    hisignallingAttr.priority = 28;
    hisignallingAttr.name = "find Face Open Lock Tack";

    IoTWatchDogDisable();

    if (osThreadNew((osThreadFunc_t)findFaceOpenLock, NULL, &hisignallingAttr) == NULL) {
        printf("Failed to create findFaceOpenLockTack\r\n");
        return HI_ERR_FAILURE;
    }
    return HI_ERR_SUCCESS;
}
SYS_RUN(findFaceOpenLockTack);

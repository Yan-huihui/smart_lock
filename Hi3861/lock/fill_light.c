#include <stdio.h>
#include <hi_io.h>
#include <hi_gpio.h>
#include "cmsis_os2.h"
#include "ohos_init.h"
#include "iot_gpio_ex.h"
#include "hi_errno.h"
#include "hi_types_base.h"
#include "iot_gpio.h"

#include "fill_light.h"

static void *fillLight(void)
{
    IoTGpioInit(LIGHT_GPIO);
    IoSetFunc(LIGHT_GPIO, IOT_IO_FUNC_GPIO_10_GPIO);          //设置引脚复用普通GPIO
    IoTGpioSetDir(LIGHT_GPIO, IOT_GPIO_DIR_IN);               //设置输入
    IoSetPull(LIGHT_GPIO, IOT_IO_PULL_NONE);                 

    IoTGpioInit(LED1_GPIO);
    IoSetFunc(LED1_GPIO, IOT_IO_FUNC_GPIO_8_GPIO);           //设置引脚复用普通GPIO
    IoTGpioSetDir(LED1_GPIO, IOT_GPIO_DIR_OUT);              //设置输出
    IoSetPull(LED1_GPIO, IOT_IO_PULL_DOWN);                  //设置下拉

    IoTGpioInit(LED2_GPIO);
    IoSetFunc(LED2_GPIO, IOT_IO_FUNC_GPIO_2_GPIO);           //设置引脚复用普通GPIO
    IoTGpioSetDir(LED2_GPIO, IOT_GPIO_DIR_OUT);              //设置输出
    IoSetPull(LED2_GPIO, IOT_IO_PULL_DOWN);                  //设置下拉
    
    IotGpioValue val = IOT_GPIO_VALUE0;
    while(1){
        IoTGpioGetInputVal(LIGHT_GPIO, &val);
        if(val == 1){
            TaskMsleep(500);
            IoTGpioGetInputVal(LIGHT_GPIO, &val);
            if (val == 1){
                IoTGpioSetOutputVal(LED1_GPIO, 1);
                IoTGpioSetOutputVal(LED2_GPIO, 1);
            }
        }else{
            TaskMsleep(500);
            IoTGpioGetInputVal(LIGHT_GPIO, &val);
            if (val == 0){
                IoTGpioSetOutputVal(LED1_GPIO, 0);
                IoTGpioSetOutputVal(LED2_GPIO, 0);
            }
        }
        TaskMsleep(20);
    }
}

hi_u32 fillLightTack(hi_void)
{
    osThreadAttr_t hisignallingAttr = {0};

    hisignallingAttr.stack_size = 512;
    hisignallingAttr.priority = 29;
    hisignallingAttr.name = "fillLightTack";

    IoTWatchDogDisable();

    if (osThreadNew((osThreadFunc_t)fillLight, NULL, &hisignallingAttr) == NULL) {
        printf("Failed to create fillLightTack\r\n");
        return HI_ERR_FAILURE;
    }
    return HI_ERR_SUCCESS;
}
SYS_RUN(fillLightTack);
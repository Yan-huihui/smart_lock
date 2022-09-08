#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "iot_gpio_ex.h"

#include "lock.h"

int clockInit(void)
{
    IoTGpioInit(LOCK_GPIO);
    IoSetFunc(LOCK_GPIO, IOT_IO_FUNC_GPIO_9_GPIO);           //设置引脚复用普通GPIO
    IoTGpioSetDir(LOCK_GPIO, IOT_GPIO_DIR_OUT);              //设置输出
    IoSetPull(LOCK_GPIO, IOT_IO_PULL_NONE);                  //设置下拉
}

int setClock(enum LockState g_lockStat)
{
    switch (g_lockStat){
        case Lock_ON:
            IoTGpioSetOutputVal(LOCK_GPIO, 1);
            TaskMsleep(LOCK_TIME_MS);
            break;
        case Lock_OFF:
            IoTGpioSetOutputVal(LOCK_GPIO, 0);
            TaskMsleep(LOCK_TIME_MS);
            break;
        case Lock_Close_After_Open:
            IoTGpioSetOutputVal(LOCK_GPIO, 1);
            TaskMsleep(LOCK_TIME_MS);
            IoTGpioSetOutputVal(LOCK_GPIO, 0);
            TaskMsleep(LOCK_TIME_MS);
            break;
        default:
            TaskMsleep(LOCK_TIME_MS);
            break;
    }
    return 0;
}


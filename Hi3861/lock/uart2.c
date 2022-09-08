#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <hi_io.h>
#include <hi_uart.h>
#include <hi_stdlib.h>
#include <hi_gpio.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio_ex.h"
#include "iot_uart.h"
#include "iot_gpio.h"
#include "hi_mux.h"

#include "uart2.h"
#include "lock.h"
   
unsigned char data[] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x08, 0x32, 0x01, 0xFF, 0xFF, 0x00, 0x04, 0x02, 0x3E};
unsigned char readbuff[64] = {0};

static int lockState = 0;
static int fingerState = 0;         //串口读取状态，是否进行读指纹模块返回来的数据
int time = 0;

static void ExtInt_Dely(unsigned int Counter)
{
    while(Counter--);
}

static void printReadbuff(void)
{
    printf("printf UART2 readbuff : ");
    int i;
    for(i = 0; i < 17; ++i){
        printf("%02x ", readbuff[i]);
    }
    printf("\n");
}

//串口2初始化
static void uart2Init(void)
{
    IoSetFunc(IOT_IO_NAME_GPIO_11, IOT_IO_FUNC_GPIO_11_UART2_TXD);
    IoSetFunc(IOT_IO_NAME_GPIO_12, IOT_IO_FUNC_GPIO_12_UART2_RXD); 

    IotUartAttribute uart_attr = {
        .baudRate = 57600,
        .dataBits = 8,
        .stopBits = 2,
        .parity = 0,
    };

    //UART 初始化
    IoTUartInit(DEMO_UART_NUM2, &uart_attr);
}

//中断服务程序
static void InterruptUartWrite(char *arg)
{
    IotGpioValue val = IOT_GPIO_VALUE0;
    (void)arg;
    printf("InterruptUartWrite\n"); 
    ExtInt_Dely(0x19999);
    unsigned int GPIO_val = IoTGpioGetInputVal(IOT_GPIO_Touch, &val);
    if(val == 1){
        fingerState = 1;          //置起进行串口与指纹模块通信标志位
    }
    printf("gpio_val = %d, IotGpioValue *p =%d\n",GPIO_val, val);
}

//初始化GPIO，设置为外部中断
static void GPIOinterruptInit(void)
{
    IoTGpioInit(IOT_GPIO_Touch);                      //初始化
    IoSetFunc(IOT_GPIO_Touch, 0);                     //设置引脚复用普通GPIO
    IoTGpioSetDir(IOT_GPIO_Touch, IOT_GPIO_DIR_IN);   //设置引脚输入
    IoSetPull(IOT_GPIO_Touch, IOT_IO_PULL_DOWN);      //设置下拉
    /*设置gpio中断，上升沿触发执行InterruptUartWrite函数*/
    IoTGpioRegisterIsrFunc(IOT_GPIO_Touch, IOT_INT_TYPE_EDGE, IOT_GPIO_EDGE_RISE_LEVEL_HIGH, InterruptUartWrite, NULL);
}

//进行串口读取数据
static void *UartReadTask(const char *arg)
{ 
    (void)arg;
    uart2Init();
    printf("end Init uart2\r\n");
    hi_u32 mutex;
    hi_mux_create (&mutex);
    printf("end set mutex\n");
    
    printf("UartReadTask\n"); 
    
    while(1){
        if(fingerState == 1){
            hi_mux_pend(mutex, HI_SYS_WAIT_FOREVER);
            IoTUartWrite(DEMO_UART_NUM2, data, 17);
            hi_mux_post(mutex);

            printf("in UartReadTask\n");
            int ret = 0;
            //读取指纹模块发回来的数据
            while(1){
                hi_mux_pend(mutex, HI_SYS_WAIT_FOREVER);
                ret = IoTUartRead(DEMO_UART_NUM2, readbuff, 17);
                hi_mux_post(mutex);

                if(ret > 0){
                    printf("break uartread\n");
                    time = 0;
                    break;
                }

                printf("wait uartread\n");
                time++;
                if(time > 50){
                    printf("uartread timeout");
                    time = 0;
                    break;
                }
                usleep(10000);
            }
            //打印读到的数据
            hi_mux_pend(mutex, HI_SYS_WAIT_FOREVER);
            printReadbuff(); 
            hi_mux_post(mutex); 
            //对读到数据进行判断执行相应操作
            if(readbuff[0] == 0xEF && readbuff[9] == 0x00 ){
                printf("open lock succeed\n");
                setClock(Lock_Close_After_Open);
                IoTUartRead(DEMO_UART_NUM2, readbuff, 17);
                memset(readbuff, 0, sizeof(readbuff));
                printReadbuff();
            }else if(readbuff[0] == 0xEF && readbuff[9] != 0x00){
                
                printf("open lock fail\n");
                if(readbuff[9] == 0x09){
                    printf("No prints came up\n");
                }else if(readbuff[9] == 0x26){
                    printf("Time out\n");
                }

                setClock(Lock_OFF);
                IoTUartRead(DEMO_UART_NUM2, readbuff, 17);
                memset(readbuff, 0, sizeof(readbuff));
                printReadbuff();
            }else if(readbuff[0] != 0xEF && readbuff[0] != 0x00){
                printf("other cmd\n");
                IoTUartRead(DEMO_UART_NUM2, readbuff, 17);
                memset(readbuff, 0, sizeof(readbuff));
                printReadbuff();
            }
            fingerState = 0;

        }else{
            usleep(10000);
        }
                      
    }
}

//创建一个任务线程，单独处理串口读取任务
static void UartRead(void)
{
    osThreadAttr_t attr;

    GPIOinterruptInit();
    printf("end Init GPIOinterrup\r\n");

    clockInit();
    setClock(Lock_OFF);
    printf("end init clock\n");

    IoTWatchDogDisable();

    attr.name = "UartReadTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 2048; /* 任务大小4096 */
    attr.priority = 26;
    //attr.priority = osPriorityNormal;
    if (osThreadNew((osThreadFunc_t)UartReadTask, NULL, &attr) == NULL) {
        printf("[UartReadTask] Falied to create UartReadTask!\n");
    }
}

SYS_RUN(UartRead);

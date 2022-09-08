#ifndef __UART2_H
#define __UART2_H

#define DEMO_UART_NUM2           HI_UART_IDX_2
#define IOT_GPIO_Touch           IOT_IO_NAME_GPIO_6

static void ExtInt_Dely(unsigned int Counter);
static void printReadbuff(void);
static void uart2Init(void);
static void InterruptUartWrite(char *arg);
static void GPIOinterruptInit(void);
static void *UartReadTask(const char *arg);
//static void *LockTask(const char *arg);

#endif
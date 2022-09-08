#ifndef __LOCK_H
#define __LOCK_H

//锁的状态
enum LockState {
    Lock_ON = 0,
    Lock_OFF,
    Lock_Close_After_Open,
};

#define LOCK_GPIO       IOT_IO_NAME_GPIO_7        //设置GPIO端口 for hispark_pegasus
#define LOCK_TIME_MS    3000                      //延时时间


int clockInit(void);
int setClock(enum LockState g_lockStat);

#endif
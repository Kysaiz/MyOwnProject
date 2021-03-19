#ifndef __USART3_H
#define __USART3_H
#include "stm32f4xx.h"
#include "sys.h" 


// 串口3初始化
extern void usart3_init(uint32_t baud);

// 发送数据到手机
extern void usart3_send_str(char *str);

// 接收来自手机蓝牙的指令
extern void USART3_IRQHandler(void);

// 蓝牙连接状态中断初始化
extern void usart3_EXTI4_init(void);


#endif

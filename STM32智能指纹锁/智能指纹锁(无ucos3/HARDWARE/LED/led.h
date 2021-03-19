#ifndef __LED_H__
#define __LED_H__

#include "stm32f4xx.h" 
#include "sys.h"

// 初始化LED
extern void LED_Init(void);

// 复用模式配置LED3
extern void led3_init(void);
extern void tim1_set_freq(uint32_t freq);
extern void tim1_ch4_set_duty(uint32_t duty);

// 复用模式配置LED1
extern void led1_init(void);
extern void tim14_set_freq(uint32_t freq);
extern void tim14_set_duty(uint32_t duty);

#endif

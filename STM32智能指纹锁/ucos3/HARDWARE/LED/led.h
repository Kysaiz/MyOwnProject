#ifndef __LED_H__
#define __LED_H__

#include "stm32f4xx.h" 
#include "sys.h"

// ��ʼ��LED
extern void LED_Init(void);

// ����ģʽ����LED3
extern void led3_init(void);
extern void tim1_set_freq(uint32_t freq);
extern void tim1_ch4_set_duty(uint32_t duty);

// ����ģʽ����LED1
extern void led1_init(void);
extern void tim14_set_freq(uint32_t freq);
extern void tim14_set_duty(uint32_t duty);

#endif

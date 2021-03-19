#ifndef __SR04_H__
#define __SR04_H__

#include "stm32f4xx.h" 
#include "sys.h"
#include "delay.h"

extern void sr04_init(void);
extern uint32_t sr04_get_distance(void);

#endif

#ifndef __DELAY_H
#define __DELAY_H 	

#include "sys.h"

extern void systick_init(void);
extern int32_t  delay_ms(uint32_t nms)
extern int32_t delay_us(uint32_t nus)

#endif

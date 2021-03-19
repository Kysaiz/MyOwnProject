#ifndef __BEEP_H__
#define __BEEP_H__

#include "stm32f4xx.h" 
#include "sys.h"

extern void BEEP_Init(void);//≥ı ºªØ

#define BEEP(x)		PFout(8)=(x)

#endif

#include "delay.h"

int32_t delay_us(uint32_t nus)
{
	uint32_t temp;

	SysTick->CTRL = 0; 					
	SysTick->LOAD = (nus*21)-1; 		
	SysTick->VAL = 0; 					
	SysTick->CTRL = 1; 					
	
	while(1)
	{
	
		temp=SysTick->CTRL;
		
		//检测count flag
		if(temp & 0x00010000)
			break;
		
		//检测系统定时器是否意外关闭	
		if((temp & 0x1)==0)
			return -1;		
	}
	
	SysTick->CTRL = 0; 					

	return 0;
}


int32_t  delay_ms(uint32_t nms)
{
	uint32_t t = nms;

	uint32_t temp;
	
	
	while(t--)
	{
		SysTick->CTRL = 0; 			
		SysTick->LOAD = 21000-1; 	
		SysTick->VAL = 0; 			
		SysTick->CTRL = 1; 			
		while(1)
		{

			temp=SysTick->CTRL;
			
			//检测count flag
			if(temp & 0x00010000)
				break;
			
			//检测系统定时器是否意外关闭	
			if((temp & 0x1)==0)
				return -1;		
		}
	}	
	
	SysTick->CTRL = 0; 	
	
	return 0;
}

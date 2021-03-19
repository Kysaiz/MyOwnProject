#include "tim.h"

volatile uint32_t g_time = 0;		// 系统运行时间
volatile uint32_t dht11_time = 5;	// dht11读取时间

void tim3_init(void)
{
	TIM_TimeBaseInitTypeDef     TIM_TimeBaseStructure;
	NVIC_InitTypeDef 			NVIC_InitStructure ;
	
	//打开TIM3的硬件时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	//配置TIM3的相关参数：计数值（决定定时时间）、分频值
	//TIM3的硬件时钟频率为10KHz
	//只要进行10000次计数，就是1秒时间的到达
	//只要进行10000/2次计数，就是1秒/2时间的到达
	TIM_TimeBaseStructure.TIM_Period = 10000-1;//计数值5000-1，决定了定时时间500ms
	TIM_TimeBaseStructure.TIM_Prescaler = 8400-1;//预分频，也就是第一次分频，当前8400分频，就是84MHz/8400
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;//时钟分频，也称之二次分频这个F407是不支持的，因此该参数是不会生效
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//就是从0开始计数，然后计数到TIM_Period
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	
	//配置TIM3的中断触发方式：时间更新中断
	TIM_ITConfig(TIM3,TIM_IT_Update , ENABLE);
	
	//配置TIM3的优先级
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	//使能定时器3工作
	TIM_Cmd(TIM3, ENABLE);
}

void TIM3_IRQHandler(void)
{
	//检测是否超时时间到达
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET)
	{
		g_time++;
		dht11_time++;
		IWDG_ReloadCounter();
		
		//清空标志位，告诉CPU当前数据接收完毕，可以响应新的中断请求
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
	
	}
}

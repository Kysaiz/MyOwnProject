#include "tim.h"

volatile uint32_t g_time = 0;		// ϵͳ����ʱ��
volatile uint32_t dht11_time = 5;	// dht11��ȡʱ��

void tim3_init(void)
{
	TIM_TimeBaseInitTypeDef     TIM_TimeBaseStructure;
	NVIC_InitTypeDef 			NVIC_InitStructure ;
	
	//��TIM3��Ӳ��ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	//����TIM3����ز���������ֵ��������ʱʱ�䣩����Ƶֵ
	//TIM3��Ӳ��ʱ��Ƶ��Ϊ10KHz
	//ֻҪ����10000�μ���������1��ʱ��ĵ���
	//ֻҪ����10000/2�μ���������1��/2ʱ��ĵ���
	TIM_TimeBaseStructure.TIM_Period = 10000-1;//����ֵ5000-1�������˶�ʱʱ��500ms
	TIM_TimeBaseStructure.TIM_Prescaler = 8400-1;//Ԥ��Ƶ��Ҳ���ǵ�һ�η�Ƶ����ǰ8400��Ƶ������84MHz/8400
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;//ʱ�ӷ�Ƶ��Ҳ��֮���η�Ƶ���F407�ǲ�֧�ֵģ���˸ò����ǲ�����Ч
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//���Ǵ�0��ʼ������Ȼ�������TIM_Period
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	
	//����TIM3���жϴ�����ʽ��ʱ������ж�
	TIM_ITConfig(TIM3,TIM_IT_Update , ENABLE);
	
	//����TIM3�����ȼ�
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	//ʹ�ܶ�ʱ��3����
	TIM_Cmd(TIM3, ENABLE);
}

void TIM3_IRQHandler(void)
{
	//����Ƿ�ʱʱ�䵽��
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET)
	{
		g_time++;
		dht11_time++;
		IWDG_ReloadCounter();
		
		//��ձ�־λ������CPU��ǰ���ݽ�����ϣ�������Ӧ�µ��ж�����
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
	
	}
}

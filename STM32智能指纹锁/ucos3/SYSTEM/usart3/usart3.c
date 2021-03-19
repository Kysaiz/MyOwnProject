#include "usart3.h"
#include "includes.h"

volatile uint8_t g_usart3_event = 0;
volatile uint8_t g_usart3_buf[128] = {0};
volatile uint32_t g_usart3_cnt = 0;

extern 			OS_Q		g_queue_usart;				//��Ϣ���еĶ���

static GPIO_InitTypeDef		GPIO_InitStructure;
static USART_InitTypeDef 		USART_InitStructure ;
static NVIC_InitTypeDef 		NVIC_InitStructure ;
static NVIC_InitTypeDef		NVIC_InitStruct;
static EXTI_InitTypeDef 		EXTI_InitStruct;

// ����3��ʼ��
void usart3_init(uint32_t baud)
{

	
	//�򿪶˿�BӲ��ʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE );

	//�򿪶˿�CӲ��ʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE );
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE );
	
	//�򿪴���3��Ӳ��ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE );	
	
	//����PB10��PB11���ţ�ΪAFģʽ�����ù���ģʽ��
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_10|GPIO_Pin_11;//ָ����10 11������ 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF ;//����Ϊ���ù���ģʽ
	GPIO_InitStructure.GPIO_Speed =GPIO_Speed_50MHz ;//�������ŵ���Ӧʱ��=1/100MHz .
	//�Ӹߵ�ƽ�л����͵�ƽ1/100MHz,�ٶ�Խ�죬���Ļ�Խ��
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP ;//��������ģʽ��������������͹����������
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL ;//��ʹ���ڲ�����������
	GPIO_Init(GPIOB ,&GPIO_InitStructure);


	//��PB10��PB11�Ĺ��ܽ���ָ��Ϊ����3
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource10,GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource11,GPIO_AF_USART3);	
	
	//���ô���1�Ĳ����������ʡ�����λ��У��λ��ֹͣλ��������
	USART_InitStructure.USART_BaudRate = baud;//������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//8λ����λ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//1��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//��У��
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ��������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//�����ڷ��ͺͽ�������
	USART_Init(USART3, &USART_InitStructure);
	
	
	//ʹ�ܴ���3�Ľ����ж�
	USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	
	
	//ʹ�ܴ���3����
	USART_Cmd(USART3, ENABLE);

	//����PE5�����ڽ������������ź�
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5;//ָ����5������ 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN ;//����Ϊ����ģʽ
	GPIO_InitStructure.GPIO_Speed =GPIO_Speed_50MHz ;//�������ŵ���Ӧʱ��=1/100MHz .
	//�Ӹߵ�ƽ�л����͵�ƽ1/100MHz,�ٶ�Խ�죬���Ļ�Խ��
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP ;//��������ģʽ��������������͹����������
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL ;//��ʹ���ڲ�����������
	GPIO_Init(GPIOE ,&GPIO_InitStructure);

}

// �������ݵ��ֻ�
void usart3_send_str(char *str)
{
	char *p = str;
	
	while(p && (*p!='\0'))
	{
	
		USART_SendData(USART3,*p);
		
		// ������δ����ʱ�������ѭ��
		while(USART_GetFlagStatus(USART3,USART_FLAG_TXE)==RESET);
		p++;
	
	}


}


// ���������ֻ�������ָ��
void USART3_IRQHandler(void)
{
	uint8_t d;
	OS_ERR err;
	
	OSIntEnter();
	//����Ƿ���յ�����
	if(USART_GetITStatus(USART3,USART_IT_RXNE)==SET)
	{
		d = USART_ReceiveData(USART3);
		
		g_usart3_buf[g_usart3_cnt] = d;
		
		g_usart3_cnt++;
		
		// ��'#'����
		if ((d == '#') || (g_usart3_cnt > sizeof(g_usart3_buf)))
		{
			g_usart3_event = 1;
		}
		
		//��ձ�־λ������CPU��ǰ���ݽ�����ϣ����Խ����µ�����
		USART_ClearITPendingBit(USART3,USART_IT_RXNE);
	
	}
	
	OSIntExit();
	
	//�Ѿ��������
	if(g_usart3_event)
	{
		//������Ϣ���Ƚ��ȳ�
		OSQPost(&g_queue_usart,&g_usart3_buf,g_usart3_cnt,OS_OPT_POST_FIFO,&err);
	
		//�����صı�־λ�ͼ���ֵ
		g_usart3_event=0;
		g_usart3_cnt=0;
	}	
}

void usart3_EXTI9_5_init(void)
{	
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource5);
	NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn; // ָ��ͨ��
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0; // ��ռ���ȼ�
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;// ��Ӧ���ȼ�
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; // ��ͨ��
	NVIC_Init(&NVIC_InitStruct);

	EXTI_InitStruct.EXTI_Line = EXTI_Line5; 
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;  
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling; 
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;  
	EXTI_Init(&EXTI_InitStruct);
}

void EXTI9_5_IRQHandler(void)
{
	OS_ERR err;
	
	OSIntEnter();
	
	//����Ƿ����ж�����
	if(EXTI_GetITStatus(EXTI_Line5) != RESET)
	{
		// �����ش���
		if (PEin(5)) 
		{
			PFout(9) = 0; // �������ӣ�LED0��
		}
		else
		{
			PFout(9) = 1; // �����Ͽ�
			OSQPost(&g_queue_usart, "quit", 4, OS_OPT_POST_FIFO, &err);
		}
		
		EXTI_ClearITPendingBit(EXTI_Line5);		
	}
	
	OSIntExit();
}

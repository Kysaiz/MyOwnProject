#include "usart3.h"
#include "includes.h"

volatile uint8_t g_usart3_event = 0;
volatile uint8_t g_usart3_buf[128] = {0};
volatile uint32_t g_usart3_cnt = 0;

extern 			OS_Q		g_queue_usart;				//消息队列的对象

static GPIO_InitTypeDef		GPIO_InitStructure;
static USART_InitTypeDef 		USART_InitStructure ;
static NVIC_InitTypeDef 		NVIC_InitStructure ;
static NVIC_InitTypeDef		NVIC_InitStruct;
static EXTI_InitTypeDef 		EXTI_InitStruct;

// 串口3初始化
void usart3_init(uint32_t baud)
{

	
	//打开端口B硬件时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE );

	//打开端口C硬件时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE );
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE );
	
	//打开串口3的硬件时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE );	
	
	//配置PB10和PB11引脚，为AF模式（复用功能模式）
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_10|GPIO_Pin_11;//指定第10 11根引脚 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF ;//配置为复用功能模式
	GPIO_InitStructure.GPIO_Speed =GPIO_Speed_50MHz ;//配置引脚的响应时间=1/100MHz .
	//从高电平切换到低电平1/100MHz,速度越快，功耗会越高
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP ;//推挽的输出模式，增加输出电流和灌电流的能力
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL ;//不使能内部上下拉电阻
	GPIO_Init(GPIOB ,&GPIO_InitStructure);


	//将PB10和PB11的功能进行指定为串口3
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource10,GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource11,GPIO_AF_USART3);	
	
	//配置串口1的参数：波特率、数据位、校验位、停止位、流控制
	USART_InitStructure.USART_BaudRate = baud;//波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//8位数据位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//1个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无校验
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//允许串口发送和接收数据
	USART_Init(USART3, &USART_InitStructure);
	
	
	//使能串口3的接收中断
	USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	
	
	//使能串口3工作
	USART_Cmd(USART3, ENABLE);

	//配置PE5，用于接收蓝牙连接信号
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5;//指定第5根引脚 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN ;//配置为输入模式
	GPIO_InitStructure.GPIO_Speed =GPIO_Speed_50MHz ;//配置引脚的响应时间=1/100MHz .
	//从高电平切换到低电平1/100MHz,速度越快，功耗会越高
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP ;//推挽的输出模式，增加输出电流和灌电流的能力
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL ;//不使能内部上下拉电阻
	GPIO_Init(GPIOE ,&GPIO_InitStructure);

}

// 发送数据到手机
void usart3_send_str(char *str)
{
	char *p = str;
	
	while(p && (*p!='\0'))
	{
	
		USART_SendData(USART3,*p);
		
		// 当蓝牙未连接时会做这个循环
		while(USART_GetFlagStatus(USART3,USART_FLAG_TXE)==RESET);
		p++;
	
	}


}


// 接收来自手机蓝牙的指令
void USART3_IRQHandler(void)
{
	uint8_t d;
	OS_ERR err;
	
	OSIntEnter();
	//检测是否接收到数据
	if(USART_GetITStatus(USART3,USART_IT_RXNE)==SET)
	{
		d = USART_ReceiveData(USART3);
		
		g_usart3_buf[g_usart3_cnt] = d;
		
		g_usart3_cnt++;
		
		// 以'#'结束
		if ((d == '#') || (g_usart3_cnt > sizeof(g_usart3_buf)))
		{
			g_usart3_event = 1;
		}
		
		//清空标志位，告诉CPU当前数据接收完毕，可以接收新的数据
		USART_ClearITPendingBit(USART3,USART_IT_RXNE);
	
	}
	
	OSIntExit();
	
	//已经接收完毕
	if(g_usart3_event)
	{
		//发送消息，先进先出
		OSQPost(&g_queue_usart,&g_usart3_buf,g_usart3_cnt,OS_OPT_POST_FIFO,&err);
	
		//清空相关的标志位和计数值
		g_usart3_event=0;
		g_usart3_cnt=0;
	}	
}

void usart3_EXTI9_5_init(void)
{	
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource5);
	NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn; // 指定通道
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0; // 抢占优先级
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;// 响应优先级
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE; // 打开通道
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
	
	//检查是否有中断请求
	if(EXTI_GetITStatus(EXTI_Line5) != RESET)
	{
		// 上升沿触发
		if (PEin(5)) 
		{
			PFout(9) = 0; // 蓝牙连接，LED0亮
		}
		else
		{
			PFout(9) = 1; // 蓝牙断开
			OSQPost(&g_queue_usart, "quit", 4, OS_OPT_POST_FIFO, &err);
		}
		
		EXTI_ClearITPendingBit(EXTI_Line5);		
	}
	
	OSIntExit();
}

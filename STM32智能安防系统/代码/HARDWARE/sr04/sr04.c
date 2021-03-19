#include "sr04.h"


void sr04_init(void)
{
	GPIO_InitTypeDef		GPIO_InitStructure;
	
	//使能端口B、端口E的硬件时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE );
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE );
	
	
	//PB6为输出模式，因为该引脚连接到Trig
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6;//指定第6根引脚 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT ;//配置为输出模式
	GPIO_InitStructure.GPIO_Speed =GPIO_Speed_50MHz ;//配置引脚的响应时间=1/100MHz .
	//从高电平切换到低电平1/100MHz,速度越快，功耗会越高
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP ;//推挽的输出模式，增加输出电流和灌电流的能力
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL ;//不使能内部上下拉电阻
	GPIO_Init(GPIOB ,&GPIO_InitStructure);	
	
	
	//PE6为输入模式，因为要检测ECHO输出高电平的持续时间
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6;//指定第6根引脚 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN ;//配置为输出模式
	GPIO_InitStructure.GPIO_Speed =GPIO_Speed_50MHz ;//配置引脚的响应时间=1/100MHz .
	//从高电平切换到低电平1/100MHz,速度越快，功耗会越高
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP ;//推挽的输出模式，增加输出电流和灌电流的能力
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL ;//不使能内部上下拉电阻
	GPIO_Init(GPIOE ,&GPIO_InitStructure);	
	
	
	//PB6引脚初始状态为低电平，根据时序图可以了解到
	PBout(6)=0;

}

uint32_t sr04_get_distance(void)
{
	uint32_t t=0;

	//PB6输出高电平
	PBout(6)=1;
	
	//延时10us
	delay_us(10);

	//PB6输出低电平
	PBout(6)=0;	
	
	//PE6要等待高电平出现
	while(PEin(6)==0);
	
	
	//测量高电平的持续时间
	while(PEin(6))
	{
	
		t++;
		delay_us(9);//超声波每传输9us时间，距离为3mm
	
	}
	
	//因为该时间是包含发射和返回的时间，需要除以2
	t/=2;
	
	return 3*t;
}

#include "dht11.h"

static GPIO_InitTypeDef		GPIO_InitStructure;

void dht11_init()
{
	//使能端口G硬件时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG,ENABLE);

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	
	//初始化
	GPIO_Init(GPIOG,&GPIO_InitStructure);
	
	
	//PG9输出高电平，这是引脚的初始电平状态
	PGout(9)=1;	
}

int32_t dht11_read(uint8_t *pbuf)
{
	uint32_t t = 0;
	int32_t i, j;
	uint8_t d = 0;
	uint8_t *p = pbuf;
	uint8_t check_sum = 0;
	
	// 配置PG9起始模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; // 设置为输出模式
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD; // 开漏输出模式
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOG, &GPIO_InitStructure);	
	
	PGout(9)=0;
	delay_ms(18);
	PGout(9)=1;
	delay_us(30);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; // 设置为输入模式
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	// PG9等待低电平出现，添加超时处理
	while (PGin(9))
	{
		t++;
		delay_us(1);
		
		if (t >= 4000) // 当t超过4ms时表示超时，退出函数，返回-1
		{
			return -1;
		}
	}
	
	// 检测低电平的持续时间
	t = 0;
	while (!PGin(9))
	{
		t++;
		delay_us(1);
		
		if (t >= 100) // 根据时序图，t应该为80us
		{
			return -2;
		}
	}
	
	// 检测高电平的持续时间
	t = 0;
	while (PGin(9))
	{
		t++;
		delay_us(1);
		
		if (t >= 100) // 根据时序图，t应该为80us
		{
			return -3;
		}
	}
	
	// 开始接收数据
	for (i = 0; i < 5; i++)
	{
		// 接收1个字节，高位优先接收数据
		d = 0;
		for (j = 7; j >= 0; j--)
		{
			// PG9等待高电平出现，添加超时处理
			t = 0;
			while (!PGin(9))
			{
				t++;
				delay_us(1);
				
				if (t >= 100)
				{
					return -4;
				}
			}
			
			// 延时40~60us
			delay_us(40);
			
			// 判断引脚的电平
			if (PGin(9))
			{
				//将d变量对应的bit位置1
				//0x01 1<<0:0000 0001
				//0x02 1<<1:0000 0010
				//.......
				//0x80 1<<7:1000 0000
				d|=1<<j;
				
				//等待剩下的高电平持续完毕
				t=0;
				while(PGin(9))
				{
					t++;
					delay_us(1);
					
					if(t>=100)
						return -5;
				}	
			}
		}
		
		p[i] = d;
	}
	
	// 校验数据
	check_sum = (p[0] + p[1] + p[2] + p[3])&0xFF;
	
	// 如果校验不相符，返回-6
	if (check_sum != p[4])
		return -6;
			
	return 0;
	
}

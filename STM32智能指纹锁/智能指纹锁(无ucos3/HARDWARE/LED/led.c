#include "led.h" 

//初始化 PF9，PF10，PE13 为输出口.并使能这两个口的时钟		    
//LED IO初始化

static volatile uint32_t tim1_cnt;
static volatile uint32_t tim14_cnt;

static GPIO_InitTypeDef 		GPIO_InitStructure ;
static TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
static TIM_OCInitTypeDef 		TIM_OCInitStructure;
void LED_Init(void)
{    	 
  GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOF, ENABLE);			//使能GPIOE，GPIOF时钟

  //GPIOF9,F10初始化设置 
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9 | GPIO_Pin_10;		//LED0和LED1对应IO口
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;			    //普通输出模式，
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;				//推挽输出，驱动LED需要电流驱动
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;		    //100MHz
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;				    //上拉
  GPIO_Init(GPIOF, &GPIO_InitStructure);						//初始化GPIOF，把配置的数据写入寄存器
  
  //GPIOE13,PE14初始化设置 
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13;	//LED2和LED3对应IO口
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;				//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;				//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;			//100MHz
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;					//上拉
  GPIO_Init(GPIOE, &GPIO_InitStructure);						//初始化GPIOE，把配置的数据写入寄存器
	
  GPIO_SetBits(GPIOF,GPIO_Pin_9  | GPIO_Pin_10);			    //GPIOF9,PF10设置高，灯灭
  GPIO_SetBits(GPIOE,GPIO_Pin_13);				//GPIOE13,PE14设置高，灯灭
}

// 复用模式配置LED3
void led3_init(void)
{
	//打开端口E的硬件时钟，等同于对端口E供电
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE );
	
	//初始化对应端口的引脚 
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; // 配置为复用模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; // 推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // 不使能内部上下拉电阻
	GPIO_Init(GPIOE ,&GPIO_InitStructure);
	
	//将PE14连接到TIM1
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_TIM1);	

	
	//打开TIM1的硬件时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	
	//配置TIM1的相关参数：计数值（决定定时时间）、分频值
	//TIM14的硬件时钟频率=84000000/8400=10000Hz
	//只要进行10000次计数，就是1秒时间的到达，也就是1Hz的频率输出
	//只要进行10000/100次计数，就是1秒/100时间的到达，也就是100Hz的频率输出
	TIM_TimeBaseStructure.TIM_Period = 10000/1000-1;//计数值100-1，决定了输出频率为100Hz
	TIM_TimeBaseStructure.TIM_Prescaler = 8400-1;//预分频，也就是第一次分频，当前8400分频，就是84MHz/8400
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;//时钟分频，也称之二次分频这个F407是不支持的，因此该参数是不会生效
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//就是从0开始计数，然后计数到TIM_Period
	tim1_cnt = TIM_TimeBaseStructure.TIM_Period+1;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
	
	//占空比的配置
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;				//工作在PWM1模式
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	//脉冲的输出开关，当前是输出脉冲
	TIM_OCInitStructure.TIM_Pulse = tim1_cnt;		//这个值是决定了占空比，配置比较值
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;//有效的状态为高电平
	
	//TIM1的通道4配置
	TIM_OC4Init(TIM1, &TIM_OCInitStructure);
	
	//使能定时器1工作
	TIM_Cmd(TIM1, ENABLE);
	
	//TIM1的所有通道总开关的控制，当前为打开状态
	TIM_CtrlPWMOutputs(TIM1,ENABLE);
}

// 复用模式配置LED1
void led1_init(void)
{
	//打开端口F的硬件时钟，等同于对端口F供电
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE );
	
	//初始化对应端口的引脚 
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_9;//指定第9根引脚 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF ;//配置为复用模式
	GPIO_InitStructure.GPIO_Speed =GPIO_Speed_50MHz ;//配置引脚的响应时间=1/100MHz .
	//从高电平切换到低电平1/100MHz,速度越快，功耗会越高
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP ;//推挽的输出模式，增加输出电流和灌电流的能力
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL ;//不使能内部上下拉电阻
	GPIO_Init(GPIOF ,&GPIO_InitStructure);
	
	//将PF9连接到TIM14
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource9, GPIO_AF_TIM14);

	//打开TIM14的硬件时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);

	
	//配置TIM14的相关参数：计数值（决定定时时间）、分频值
	//TIM14的硬件时钟频率=84000000/8400=10000Hz
	//只要进行10000次计数，就是1秒时间的到达，也就是1Hz的频率输出
	//只要进行10000/100次计数，就是1秒/100时间的到达，也就是100Hz的频率输出
	TIM_TimeBaseStructure.TIM_Period = 10000/1000-1;//计数值100-1，决定了输出频率为100Hz
	TIM_TimeBaseStructure.TIM_Prescaler = 8400-1;//预分频，也就是第一次分频，当前8400分频，就是84MHz/8400
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;//时钟分频，也称之二次分频这个F407是不支持的，因此该参数是不会生效
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//就是从0开始计数，然后计数到TIM_Period
	tim14_cnt = TIM_TimeBaseStructure.TIM_Period+1;
	TIM_TimeBaseInit(TIM14, &TIM_TimeBaseStructure);
	
	//占空比的配置
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;				//工作在PWM1模式
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	//脉冲的输出开关，当前是输出脉冲
	TIM_OCInitStructure.TIM_Pulse = 100;		//这个值是决定了占空比，配置比较值
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;//有效的状态为高电平
	
	//TIM14的通道1配置
	TIM_OC1Init(TIM14, &TIM_OCInitStructure);
	
	//使能定时器14工作
	TIM_Cmd(TIM14, ENABLE);
}

void tim1_set_freq(uint32_t freq)
{
    /*定时器的基本配置，用于配置定时器的输出脉冲的频率为 freq Hz */
    TIM_TimeBaseStructure.TIM_Period = (10000/freq)-1; //设置定时脉冲的频率
    TIM_TimeBaseStructure.TIM_Prescaler = 8400-1; //第一次分频，简称为预分频
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    tim1_cnt= TIM_TimeBaseStructure.TIM_Period;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
}

void tim1_ch4_set_duty(uint32_t duty)
{
	TIM_SetCompare4(TIM1,(tim1_cnt+1) * duty/100);
}

void tim14_set_freq(uint32_t freq)
{
    /*定时器的基本配置，用于配置定时器的输出脉冲的频率为 freq Hz */
    TIM_TimeBaseStructure.TIM_Period = (10000/freq)-1; //设置定时脉冲的频率
    TIM_TimeBaseStructure.TIM_Prescaler = 8400-1; //第一次分频，简称为预分频
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    tim14_cnt= TIM_TimeBaseStructure.TIM_Period;
    TIM_TimeBaseInit(TIM14, &TIM_TimeBaseStructure);
}

void tim14_set_duty(uint32_t duty)
{
	TIM_SetCompare1(TIM14,(tim14_cnt+1) * duty/100);
}

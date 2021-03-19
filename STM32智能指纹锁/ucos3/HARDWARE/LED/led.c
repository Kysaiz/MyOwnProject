#include "led.h" 

//��ʼ�� PF9��PF10��PE13 Ϊ�����.��ʹ���������ڵ�ʱ��		    
//LED IO��ʼ��

static volatile uint32_t tim1_cnt;
static volatile uint32_t tim14_cnt;

static GPIO_InitTypeDef 		GPIO_InitStructure ;
static TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
static TIM_OCInitTypeDef 		TIM_OCInitStructure;
void LED_Init(void)
{    	 
  GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOF, ENABLE);			//ʹ��GPIOE��GPIOFʱ��

  //GPIOF9,F10��ʼ������ 
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9 | GPIO_Pin_10;		//LED0��LED1��ӦIO��
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;			    //��ͨ���ģʽ��
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;				//�������������LED��Ҫ��������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;		    //100MHz
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;				    //����
  GPIO_Init(GPIOF, &GPIO_InitStructure);						//��ʼ��GPIOF�������õ�����д��Ĵ���
  
  //GPIOE13,PE14��ʼ������ 
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13;	//LED2��LED3��ӦIO��
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;				//��ͨ���ģʽ
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;				//�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;			//100MHz
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;					//����
  GPIO_Init(GPIOE, &GPIO_InitStructure);						//��ʼ��GPIOE�������õ�����д��Ĵ���
	
  GPIO_SetBits(GPIOF,GPIO_Pin_9  | GPIO_Pin_10);			    //GPIOF9,PF10���øߣ�����
  GPIO_SetBits(GPIOE,GPIO_Pin_13);				//GPIOE13,PE14���øߣ�����
}

// ����ģʽ����LED3
void led3_init(void)
{
	//�򿪶˿�E��Ӳ��ʱ�ӣ���ͬ�ڶԶ˿�E����
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE );
	
	//��ʼ����Ӧ�˿ڵ����� 
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; // ����Ϊ����ģʽ
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; // �������
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // ��ʹ���ڲ�����������
	GPIO_Init(GPIOE ,&GPIO_InitStructure);
	
	//��PE14���ӵ�TIM1
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_TIM1);	

	
	//��TIM1��Ӳ��ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	
	//����TIM1����ز���������ֵ��������ʱʱ�䣩����Ƶֵ
	//TIM14��Ӳ��ʱ��Ƶ��=84000000/8400=10000Hz
	//ֻҪ����10000�μ���������1��ʱ��ĵ��Ҳ����1Hz��Ƶ�����
	//ֻҪ����10000/100�μ���������1��/100ʱ��ĵ��Ҳ����100Hz��Ƶ�����
	TIM_TimeBaseStructure.TIM_Period = 10000/1000-1;//����ֵ100-1�����������Ƶ��Ϊ100Hz
	TIM_TimeBaseStructure.TIM_Prescaler = 8400-1;//Ԥ��Ƶ��Ҳ���ǵ�һ�η�Ƶ����ǰ8400��Ƶ������84MHz/8400
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;//ʱ�ӷ�Ƶ��Ҳ��֮���η�Ƶ���F407�ǲ�֧�ֵģ���˸ò����ǲ�����Ч
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//���Ǵ�0��ʼ������Ȼ�������TIM_Period
	tim1_cnt = TIM_TimeBaseStructure.TIM_Period+1;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
	
	//ռ�ձȵ�����
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;				//������PWM1ģʽ
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	//�����������أ���ǰ���������
	TIM_OCInitStructure.TIM_Pulse = tim1_cnt;		//���ֵ�Ǿ�����ռ�ձȣ����ñȽ�ֵ
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;//��Ч��״̬Ϊ�ߵ�ƽ
	
	//TIM1��ͨ��4����
	TIM_OC4Init(TIM1, &TIM_OCInitStructure);
	
	//ʹ�ܶ�ʱ��1����
	TIM_Cmd(TIM1, ENABLE);
	
	//TIM1������ͨ���ܿ��صĿ��ƣ���ǰΪ��״̬
	TIM_CtrlPWMOutputs(TIM1,ENABLE);
}

// ����ģʽ����LED1
void led1_init(void)
{
	//�򿪶˿�F��Ӳ��ʱ�ӣ���ͬ�ڶԶ˿�F����
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE );
	
	//��ʼ����Ӧ�˿ڵ����� 
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_9;//ָ����9������ 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF ;//����Ϊ����ģʽ
	GPIO_InitStructure.GPIO_Speed =GPIO_Speed_50MHz ;//�������ŵ���Ӧʱ��=1/100MHz .
	//�Ӹߵ�ƽ�л����͵�ƽ1/100MHz,�ٶ�Խ�죬���Ļ�Խ��
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP ;//��������ģʽ��������������͹����������
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL ;//��ʹ���ڲ�����������
	GPIO_Init(GPIOF ,&GPIO_InitStructure);
	
	//��PF9���ӵ�TIM14
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource9, GPIO_AF_TIM14);

	//��TIM14��Ӳ��ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);

	
	//����TIM14����ز���������ֵ��������ʱʱ�䣩����Ƶֵ
	//TIM14��Ӳ��ʱ��Ƶ��=84000000/8400=10000Hz
	//ֻҪ����10000�μ���������1��ʱ��ĵ��Ҳ����1Hz��Ƶ�����
	//ֻҪ����10000/100�μ���������1��/100ʱ��ĵ��Ҳ����100Hz��Ƶ�����
	TIM_TimeBaseStructure.TIM_Period = 10000/1000-1;//����ֵ100-1�����������Ƶ��Ϊ100Hz
	TIM_TimeBaseStructure.TIM_Prescaler = 8400-1;//Ԥ��Ƶ��Ҳ���ǵ�һ�η�Ƶ����ǰ8400��Ƶ������84MHz/8400
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;//ʱ�ӷ�Ƶ��Ҳ��֮���η�Ƶ���F407�ǲ�֧�ֵģ���˸ò����ǲ�����Ч
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//���Ǵ�0��ʼ������Ȼ�������TIM_Period
	tim14_cnt = TIM_TimeBaseStructure.TIM_Period+1;
	TIM_TimeBaseInit(TIM14, &TIM_TimeBaseStructure);
	
	//ռ�ձȵ�����
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;				//������PWM1ģʽ
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	//�����������أ���ǰ���������
	TIM_OCInitStructure.TIM_Pulse = 100;		//���ֵ�Ǿ�����ռ�ձȣ����ñȽ�ֵ
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;//��Ч��״̬Ϊ�ߵ�ƽ
	
	//TIM14��ͨ��1����
	TIM_OC1Init(TIM14, &TIM_OCInitStructure);
	
	//ʹ�ܶ�ʱ��14����
	TIM_Cmd(TIM14, ENABLE);
}

void tim1_set_freq(uint32_t freq)
{
    /*��ʱ���Ļ������ã��������ö�ʱ������������Ƶ��Ϊ freq Hz */
    TIM_TimeBaseStructure.TIM_Period = (10000/freq)-1; //���ö�ʱ�����Ƶ��
    TIM_TimeBaseStructure.TIM_Prescaler = 8400-1; //��һ�η�Ƶ�����ΪԤ��Ƶ
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
    /*��ʱ���Ļ������ã��������ö�ʱ������������Ƶ��Ϊ freq Hz */
    TIM_TimeBaseStructure.TIM_Period = (10000/freq)-1; //���ö�ʱ�����Ƶ��
    TIM_TimeBaseStructure.TIM_Prescaler = 8400-1; //��һ�η�Ƶ�����ΪԤ��Ƶ
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    tim14_cnt= TIM_TimeBaseStructure.TIM_Period;
    TIM_TimeBaseInit(TIM14, &TIM_TimeBaseStructure);
}

void tim14_set_duty(uint32_t duty)
{
	TIM_SetCompare1(TIM14,(tim14_cnt+1) * duty/100);
}

#include "sr04.h"


void sr04_init(void)
{
	GPIO_InitTypeDef		GPIO_InitStructure;
	
	//ʹ�ܶ˿�B���˿�E��Ӳ��ʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE );
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE );
	
	
	//PB6Ϊ���ģʽ����Ϊ���������ӵ�Trig
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6;//ָ����6������ 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT ;//����Ϊ���ģʽ
	GPIO_InitStructure.GPIO_Speed =GPIO_Speed_50MHz ;//�������ŵ���Ӧʱ��=1/100MHz .
	//�Ӹߵ�ƽ�л����͵�ƽ1/100MHz,�ٶ�Խ�죬���Ļ�Խ��
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP ;//��������ģʽ��������������͹����������
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL ;//��ʹ���ڲ�����������
	GPIO_Init(GPIOB ,&GPIO_InitStructure);	
	
	
	//PE6Ϊ����ģʽ����ΪҪ���ECHO����ߵ�ƽ�ĳ���ʱ��
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6;//ָ����6������ 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN ;//����Ϊ���ģʽ
	GPIO_InitStructure.GPIO_Speed =GPIO_Speed_50MHz ;//�������ŵ���Ӧʱ��=1/100MHz .
	//�Ӹߵ�ƽ�л����͵�ƽ1/100MHz,�ٶ�Խ�죬���Ļ�Խ��
	//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP ;//��������ģʽ��������������͹����������
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL ;//��ʹ���ڲ�����������
	GPIO_Init(GPIOE ,&GPIO_InitStructure);	
	
	
	//PB6���ų�ʼ״̬Ϊ�͵�ƽ������ʱ��ͼ�����˽⵽
	PBout(6)=0;

}

uint32_t sr04_get_distance(void)
{
	uint32_t t=0;

	//PB6����ߵ�ƽ
	PBout(6)=1;
	
	//��ʱ10us
	delay_us(10);

	//PB6����͵�ƽ
	PBout(6)=0;	
	
	//PE6Ҫ�ȴ��ߵ�ƽ����
	while(PEin(6)==0);
	
	
	//�����ߵ�ƽ�ĳ���ʱ��
	while(PEin(6))
	{
	
		t++;
		delay_us(9);//������ÿ����9usʱ�䣬����Ϊ3mm
	
	}
	
	//��Ϊ��ʱ���ǰ�������ͷ��ص�ʱ�䣬��Ҫ����2
	t/=2;
	
	return 3*t;
}

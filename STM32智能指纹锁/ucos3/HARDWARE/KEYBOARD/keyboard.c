#include "keyboard.h"

// ��ʼ������
void key_board_init(void)
{
	GPIO_InitTypeDef		GPIO_InitStructure;
	
    //ʹ�ܶ˿�A��Ӳ��ʱ�ӣ����ǶԶ˿�A����
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    
    //ʹ�ܶ˿�C��Ӳ��ʱ�ӣ����ǶԶ˿�B����
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    
    //ʹ�ܶ˿�D��Ӳ��ʱ�ӣ����ǶԶ˿�C����
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    
    //ʹ�ܶ˿�E��Ӳ��ʱ�ӣ����ǶԶ˿�E����
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	
	//ʹ�ܶ˿�G��Ӳ��ʱ�ӣ����ǶԶ˿�G����
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
    
	//PB7 PA4 PG15 PC7  
	//PC9 PB6 PE6 PA8
	//ʹ��GPIO_Init����������
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;        	//���ģʽ
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;       	//�������ģʽ,Ĭ�ϵ�
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_4; 			//ָ����4������
	GPIO_InitStructure.GPIO_Speed=GPIO_High_Speed;     	//���٣����ǹ��������
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL; 	//���������������ʹ���������裩
	GPIO_Init(GPIOA,&GPIO_InitStructure);              	//A��
    
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_7; 			//ָ����7������
    GPIO_Init(GPIOB,&GPIO_InitStructure);       	    //B��
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_7; 			//ָ����7������
    GPIO_Init(GPIOC,&GPIO_InitStructure);       	    //C��
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_15; 			//ָ����15������
    GPIO_Init(GPIOG,&GPIO_InitStructure);       	    //G��
    
	//PC9 PB6 PE6 PA8
	/* �����������ģʽ����ʹ���ڲ��������裬�������յ�ʱ��Ϊ�̶��ĸߵ�ƽ */
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;	   //����
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;         //����ģʽ
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_8;            //ָ����8������
    GPIO_Init(GPIOA,&GPIO_InitStructure);              //A��
    
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_6;            //ָ����6������
    GPIO_Init(GPIOB,&GPIO_InitStructure);              //B��
    
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_9;            //ָ����9������
    GPIO_Init(GPIOC,&GPIO_InitStructure);              //C��
    
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_6;           //ָ����6������
    GPIO_Init(GPIOE,&GPIO_InitStructure);              //E��
}

char get_key_board(void)
{
    //PB7 PA4 PG15 PC7    
    PBout(7) = 0;
    PAout(4) = 1;
    PGout(15) = 1;
    PCout(7) = 1;
    delay_ms(2);
	
    //PC9 PB6 PE6 PA8
    if( PCin(9) == 0 ) return '1';
    else if( PBin(6) == 0 ) return '2';
    else if( PEin(6) == 0 ) return '3';
    else if( PAin(8) == 0 ) return 'A';
    
    PBout(7) = 1;
    PAout(4) = 0; 
    PGout(15) = 1; 
    PCout(7) = 1;
    delay_ms(2);

    //PC11 PE5 PA6 PG9
    if( PCin(9) == 0 ) return '4'; 
    else if( PBin(6) == 0 ) return '5';
    else if( PEin(6) == 0 ) return '6'; 
    else if( PAin(8) == 0 ) return 'B';
    
    PBout(7) = 1;
    PAout(4) = 1; 
    PGout(15) = 0; 
    PCout(7) = 1;
    delay_ms(2);
    //PC11 PE5 PA6 PG9
    if( PCin(9) == 0 ) return '7'; 
    else if( PBin(6) == 0 ) return '8'; 
    else if( PEin(6) == 0 ) return '9'; 
    else if( PAin(8) == 0 ) return 'C';
    
    PBout(7) = 1;
    PAout(4) = 1; 
    PGout(15) = 1; 
    PCout(7) = 0;
    delay_ms(2);
    //PC11 PE5 PA6 PG9
    if( PCin(9) == 0 ) return '*'; 
    else if( PBin(6) == 0 ) return '0'; 
    else if( PEin(6) == 0 ) return '#'; 
    else if( PAin(8) == 0 ) return 'D';
    
    return 'N';
}


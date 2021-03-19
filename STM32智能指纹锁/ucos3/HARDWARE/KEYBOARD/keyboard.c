#include "keyboard.h"

// 初始化键盘
void key_board_init(void)
{
	GPIO_InitTypeDef		GPIO_InitStructure;
	
    //使能端口A的硬件时钟，就是对端口A供电
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    
    //使能端口C的硬件时钟，就是对端口B供电
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    
    //使能端口D的硬件时钟，就是对端口C供电
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    
    //使能端口E的硬件时钟，就是对端口E供电
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	
	//使能端口G的硬件时钟，就是对端口G供电
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
    
	//PB7 PA4 PG15 PC7  
	//PC9 PB6 PE6 PA8
	//使用GPIO_Init来配置引脚
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;        	//输出模式
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;       	//推挽输出模式,默认的
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_4; 			//指定第4根引脚
	GPIO_InitStructure.GPIO_Speed=GPIO_High_Speed;     	//高速，但是功耗是最高
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL; 	//无需上下拉（亦可使能下拉电阻）
	GPIO_Init(GPIOA,&GPIO_InitStructure);              	//A口
    
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_7; 			//指定第7根引脚
    GPIO_Init(GPIOB,&GPIO_InitStructure);       	    //B口
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_7; 			//指定第7根引脚
    GPIO_Init(GPIOC,&GPIO_InitStructure);       	    //C口
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_15; 			//指定第15根引脚
    GPIO_Init(GPIOG,&GPIO_InitStructure);       	    //G口
    
	//PC9 PB6 PE6 PA8
	/* 矩阵键盘输入模式必须使能内部上拉电阻，引脚悬空的时候为固定的高电平 */
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;	   //上拉
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;         //输入模式
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_8;            //指定第8根引脚
    GPIO_Init(GPIOA,&GPIO_InitStructure);              //A口
    
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_6;            //指定第6根引脚
    GPIO_Init(GPIOB,&GPIO_InitStructure);              //B口
    
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_9;            //指定第9根引脚
    GPIO_Init(GPIOC,&GPIO_InitStructure);              //C口
    
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_6;           //指定第6根引脚
    GPIO_Init(GPIOE,&GPIO_InitStructure);              //E口
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


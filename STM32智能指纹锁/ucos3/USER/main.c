#include <stdio.h>
#include <string.h>

#include "sys.h"
#include "delay.h"
#include "usart1.h"
#include "usart3.h"
#include "includes.h"

#include "led.h"
#include "beep.h"
#include "as608.h"
#include "dht11.h"
#include "flash.h"
#include "iwdg.h"
#include "keyboard.h"
#include "Mifare522.h"
#include "oled.h"
#include "rtc.h"
#include "usart2.h"
#include "timer.h"
#include "bmp.h"

//任务1控制块
OS_TCB app_task_tcb_beep;
void app_task_beep(void *parg);
CPU_STK app_task_stk_beep[128];				//任务1的任务堆栈，大小为128字，也就是512字节

//任务2控制块
OS_TCB app_task_tcb_oled;
void app_task_oled(void *parg);
CPU_STK app_task_stk_oled[512];			//任务2的任务堆栈，大小为512字，也就是2048字节

//任务3控制块
OS_TCB app_task_tcb_rfid;
void app_task_rfid(void *parg);
CPU_STK app_task_stk_rfid[512];				//任务3的任务堆栈，大小为512字，也就是2048字节

//任务4控制块
OS_TCB app_task_tcb_as608;
void app_task_as608(void *parg);
CPU_STK app_task_stk_as608[512];			//任务4的任务堆栈，大小为512字，也就是2048字节

//任务5控制块
OS_TCB app_tak_tcb_scan;
void app_task_scan(void *parg);
CPU_STK task7_stk[128];						//任务5的任务堆栈，大小为128字，也就是512字节

//任务6控制块
OS_TCB app_task_tcb_sta;
void app_task_sta(void *parg);
CPU_STK app_task_stk_sta[512];				//任务6的任务堆栈，大小为512字，也就是2048字节

//任务7控制块
OS_TCB app_task_tcb_key;
void app_task_key(void *parg);
CPU_STK app_task_stk_key[128];				//任务7的任务堆栈，大小为512字，也就是2048字节

//任务8控制块
OS_TCB app_task_tcb_usart;
void app_task_usart(void *parg);
CPU_STK app_task_stk_usart[512];			//任务8的任务堆栈，大小为512字，也就是2048字节

//任务init控制块
OS_TCB app_task_tcb_init;
void app_task_init(void *parg);
CPU_STK app_task_stk_init[512];				//任务9的任务堆栈，大小为512字，也就是2048字节


extern volatile uint8_t 	g_usart1_event;
extern volatile uint8_t 	g_usart1_buf[128];
extern volatile uint32_t 	g_usart1_cnt;

extern volatile uint8_t 	g_usart3_event;
extern volatile uint8_t 	g_usart3_buf[128];
extern volatile uint32_t	g_usart3_cnt;

extern volatile uint32_t 	g_rtc_wakeup_event;

static RTC_TimeTypeDef  	RTC_TimeStructure;
static RTC_DateTypeDef		RTC_DateStructure;									
									
static u16 			g_ValidN;					// 模块内有效指纹个数
static SysPara 		AS608Para;					// 指纹模块AS608参数
static char 		g_user_PW[5]={0};			// 存储用户密码
static char 		g_root_PW[5]={0};			// 存储管理员密码
static char 		g_input_PW[6] = {0};		// 当前输入密码缓冲区
static u8	 		g_count_PW = 0;				// 当前输入密码位数
static int32_t 		g_menu_time_count = 0;		// 自动上锁时间
			
	
OS_Q				g_queue_key;				//消息队列的对象
OS_Q				g_queue_usart;				//消息队列的对象

OS_FLAG_GRP			g_flag_grp;					//事件标志组的对象
OS_FLAG_GRP			g_flag_as608;				//事件标志组的对象
OS_FLAG_GRP			g_flag_rfid;				//事件标志组的对象

OS_MUTEX			g_mutex_printf;				//互斥量的对象
OS_MUTEX			g_mutex_oled;				//互斥量的对象

OS_TMR				g_soft_timer;						//软件定时器的对象

u16 Count_Card(void);
uint8_t Check_Card(u8* card);								
void List_FR(void);	
void Write_Unlock_Record(u8 mode);
u32 Count_Unlock_Record(void);
void admin_surface(u32 page_count);

void timer_iwdg_callback(OS_TMR *p_tmr, void *p_arg)
{
	IWDG_ReloadCounter();
}

//主函数
int main(void)
{
	OS_ERR err;

	systick_init();  													//时钟初始化
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);						//中断分组配置
	
	//OS初始化，它是第一个运行的函数,初始化各种的全局变量，例如中断嵌套计数器、优先级、存储器
	OSInit(&err);
	
	//创建任务
	OSTaskCreate(	&app_task_tcb_init,							//任务控制块，等同于线程id
					"app_task_init",							//任务的名字，名字可以自定义的
					app_task_init,								//任务函数，等同于线程函数
					0,											//传递参数，等同于线程的传递参数
					6,											//任务的优先级6		
					app_task_stk_init,							//任务堆栈基地址
					512/10,										//任务堆栈深度限位，用到这个位置，任务不能再继续使用
					512,										//任务堆栈大小			
					0,											//禁止任务消息队列
					0,											//默认是抢占式内核															
					0,											//不需要补充用户存储区
					OS_OPT_TASK_NONE,							//创建任务无额外操作
					&err										//返回的错误码
				);		
	
	//启动OS，进行任务调度
	OSStart(&err);
					
	printf("never run.........\r\n");
					
	while(1);
}

void app_task_init(void *parg)
{
	OS_ERR err;
	
	LED_Init();
	BEEP_Init();
	key_board_init();
	dht11_init();
	OLED_Init();
	OLED_Clear(); 
	flash_init();
	PS_StaGPIO_Init(); // 指纹模块初始化
	MFRC522_Initializtion();			//初始化MFRC522
	
	usart2_init(57600);//初始化串口2,用于与指纹模块通讯
	
	//初始化串口1的波特率为115200bps
	//注意：如果接收的数据是乱码，要检查PLL。
	usart1_init(115200);
	
	usart3_EXTI9_5_init();
	
	//串口3的波特率为9600bps，因为蓝牙模块默认使用该波特率
	usart3_init(9600);

	// 更改蓝牙名字
	usart3_send_str("AT+NAMEkkkk\r");
	delay_ms(50);
	
#if 0	//如果要重新擦除扇区重写各种数据
	printf("正在擦除扇区\r\n");
	flash_erase(7);
	printf("7\r");
	flash_erase(6);
	printf("6\r");
	flash_erase(5);
	printf("5\r");
	flash_erase(4);
	printf("4\r");
	printf("擦除扇区完成\r\n");
	
	strcpy(g_user_PW, "1234\0");
	strcpy(g_root_PW, "8888\0");
	
	WriteFlashData(FLASH7_ADDR, (u8 *)g_user_PW, 5);
	WriteFlashData(FLASH7_ADDR+5, (u8 *)g_root_PW, 5); 
	
#endif

	// 从扇区7读取密码数据
	ReadFlashData(FLASH7_ADDR, (char *)g_user_PW, 5);
	ReadFlashData(FLASH7_ADDR+5, (char *)g_root_PW, 5);
	
	printf("\n智能指纹锁-姚俊炜\r\n");
	printf("g_user_PW = %s\r\n", g_user_PW);
	printf("g_root_PW = %s\r\n", g_root_PW);	

	AS608Para.PS_max = 300;
	
	//rtc_init(); //重设时间
	rtc_reload(); //读取寄存器时间

	//创建任务1
	OSTaskCreate(	&app_task_tcb_beep,									//任务控制块，等同于线程id
					"app_task_beep",									//任务的名字，名字可以自定义的
					app_task_beep,										//任务函数，等同于线程函数
					0,												//传递参数，等同于线程的传递参数
					6,											 	//任务的优先级6		
					app_task_stk_beep,									//任务堆栈基地址
					128/10,									//任务堆栈深度限位，用到这个位置，任务不能再继续使用
					128,										//任务堆栈大小			
					0,											//禁止任务消息队列
					0,												//默认时间片长度																
					0,												//不需要补充用户存储区
					OS_OPT_TASK_NONE,								//没有任何选项
					&err													//返回的错误码
				);
	if(err != OS_ERR_NONE)
	{
		printf("[task1][CREATE]Error Code = %d\r\n",err);
	}
	//创建任务2
	OSTaskCreate(	&app_task_tcb_oled,									//任务控制块，等同于线程id
					"app_task_oled",									//任务的名字，名字可以自定义的
					app_task_oled,										//任务函数，等同于线程函数
					0,												//传递参数，等同于线程的传递参数
					6,											 	//任务的优先级6		
					app_task_stk_oled,									//任务堆栈基地址
					512/10,									//任务堆栈深度限位，用到这个位置，任务不能再继续使用
					512,										//任务堆栈大小			
					0,											//禁止任务消息队列
					0,												//默认时间片长度																
					0,												//不需要补充用户存储区
					OS_OPT_TASK_NONE,								//没有任何选项
					&err													//返回的错误码
				);
	if(err != OS_ERR_NONE)
	{
		printf("[task2][CREATE]Error Code = %d\r\n",err);
	}
	//创建任务3
	OSTaskCreate(	&app_task_tcb_rfid,									//任务控制块，等同于线程id
					"app_task_rfid",									//任务的名字，名字可以自定义的
					app_task_rfid,										//任务函数，等同于线程函数
					0,												//传递参数，等同于线程的传递参数
					6,											 	//任务的优先级6		
					app_task_stk_rfid,									//任务堆栈基地址
					512/10,									//任务堆栈深度限位，用到这个位置，任务不能再继续使用
					512,										//任务堆栈大小			
					0,											//禁止任务消息队列
					0,												//默认时间片长度																
					0,												//不需要补充用户存储区
					OS_OPT_TASK_NONE,								//没有任何选项
					&err													//返回的错误码
				);
	if(err != OS_ERR_NONE)
	{
		printf("[task3][CREATE]Error Code = %d\r\n",err);
	}
	//创建任务4
	OSTaskCreate(	&app_task_tcb_as608,									//任务控制块，等同于线程id
					"app_task_as608",									//任务的名字，名字可以自定义的
					app_task_as608,										//任务函数，等同于线程函数
					0,												//传递参数，等同于线程的传递参数
					6,											 	//任务的优先级6		
					app_task_stk_as608,									//任务堆栈基地址
					512/10,									//任务堆栈深度限位，用到这个位置，任务不能再继续使用
					512,										//任务堆栈大小			
					0,											//禁止任务消息队列
					0,												//默认时间片长度																
					0,												//不需要补充用户存储区
					OS_OPT_TASK_NONE,								//没有任何选项
					&err													//返回的错误码
				);
	if(err != OS_ERR_NONE)
	{
		printf("[task4][CREATE]Error Code = %d\r\n",err);
	}
	//创建任务5
	OSTaskCreate(	&app_task_tcb_key,									//任务控制块，等同于线程id
					"app_task_key",									//任务的名字，名字可以自定义的
					app_task_key,										//任务函数，等同于线程函数
					0,												//传递参数，等同于线程的传递参数
					6,											 	//任务的优先级6		
					app_task_stk_key,									//任务堆栈基地址
					128/10,									//任务堆栈深度限位，用到这个位置，任务不能再继续使用
					128,										//任务堆栈大小			
					0,											//禁止任务消息队列
					0,												//默认时间片长度																
					0,												//不需要补充用户存储区
					OS_OPT_TASK_NONE,								//没有任何选项
					&err													//返回的错误码
				);
	if(err != OS_ERR_NONE)
	{
		printf("[task5][CREATE]Error Code = %d\r\n",err);
	}
	//创建任务6
	OSTaskCreate(	&app_tak_tcb_scan,									//任务控制块，等同于线程id
					"app_task_scan",									//任务的名字，名字可以自定义的
					app_task_scan,										//任务函数，等同于线程函数
					0,												//传递参数，等同于线程的传递参数
					6,											 	//任务的优先级6		
					task7_stk,									//任务堆栈基地址
					128/10,									//任务堆栈深度限位，用到这个位置，任务不能再继续使用
					128,										//任务堆栈大小			
					0,											//禁止任务消息队列
					0,												//默认时间片长度																
					0,												//不需要补充用户存储区
					OS_OPT_TASK_NONE,								//没有任何选项
					&err													//返回的错误码
				);
	if(err != OS_ERR_NONE)
	{
		printf("[task6][CREATE]Error Code = %d\r\n",err);
	}
	//创建任务7
	OSTaskCreate(	&app_task_tcb_sta,									//任务控制块，等同于线程id
					"app_task_sta",									//任务的名字，名字可以自定义的
					app_task_sta,										//任务函数，等同于线程函数
					0,												//传递参数，等同于线程的传递参数
					6,											 	//任务的优先级6		
					app_task_stk_sta,									//任务堆栈基地址
					512/10,									//任务堆栈深度限位，用到这个位置，任务不能再继续使用
					512,										//任务堆栈大小			
					0,											//禁止任务消息队列
					0,												//默认时间片长度																
					0,												//不需要补充用户存储区
					OS_OPT_TASK_NONE,								//没有任何选项
					&err													//返回的错误码
				);
	if(err != OS_ERR_NONE)
	{
		printf("[task7][CREATE]Error Code = %d\r\n",err);
	}
	//创建任务8
	OSTaskCreate(	&app_task_tcb_usart,									//任务控制块，等同于线程id
					"app_task_usart",									//任务的名字，名字可以自定义的
					app_task_usart,										//任务函数，等同于线程函数
					0,												//传递参数，等同于线程的传递参数
					6,											 	//任务的优先级6		
					app_task_stk_usart,									//任务堆栈基地址
					512/10,									//任务堆栈深度限位，用到这个位置，任务不能再继续使用
					512,										//任务堆栈大小			
					0,											//禁止任务消息队列
					0,												//默认时间片长度																
					0,												//不需要补充用户存储区
					OS_OPT_TASK_NONE,								//没有任何选项
					&err													//返回的错误码
				);
	if(err != OS_ERR_NONE)
	{
		printf("[task8][CREATE]Error Code = %d\r\n",err);
	}

	OSQCreate(&g_queue_key,"g_queue_key",64,&err);			//启动消息队列
	OSQCreate(&g_queue_usart,"g_queue_usart",64,&err);		//启动消息队列
	
	
	OSFlagCreate(&g_flag_grp,"g_flag_grp",0,&err);			//创建事件标志组，所有标志位初值为0
	OSFlagCreate(&g_flag_as608,"g_flag_as608",0,&err);		//创建事件标志组，所有标志位初值为0
	OSFlagCreate(&g_flag_rfid,"g_flag_rfid",0,&err);		//创建事件标志组，所有标志位初值为0

	OSMutexCreate(&g_mutex_printf,	"g_mutex_printf",&err);	//创建互斥量
	OSMutexCreate(&g_mutex_oled,	"g_mutex_oled",&err);	//创建互斥量
	
	iwdg_init();
	OSTmrCreate(&g_soft_timer,"g_soft_timer",0,100,OS_OPT_TMR_PERIODIC,(OS_TMR_CALLBACK_PTR)timer_iwdg_callback,NULL,&err);
	OSTmrStart(&g_soft_timer,&err);
	
	//删除自身任务，进入休眠态
	OSTaskDel(NULL,&err);
}


void app_task_beep(void *parg)
{
	OS_ERR err;	
	OS_FLAGS flags=0;
	
	printf("[app_task_beep] create ok\r\n");
	
	while(1)
	{
		flags=OSFlagPend(&g_flag_grp,
							0x06,
							0,
							OS_OPT_PEND_FLAG_SET_ANY + OS_OPT_PEND_BLOCKING,
							NULL,
							&err);
		
		if (flags & 0x02) // 解锁事件成功，短鸣2下
		{
			// 挂起扫描任务一段时间
			OSTaskSuspend(&app_tak_tcb_scan,&err);
			
			BEEP(1);
			delay_ms(100);
			BEEP(0);
			delay_ms(100);
			BEEP(1);
			delay_ms(100);
			BEEP(0);
			delay_ms(1100);
			
			// 唤醒扫描任务
			OSTaskResume(&app_tak_tcb_scan,&err);
			
		}
		else if (flags & 0x04) // 解锁事件失败，长鸣1下
		{
			// 挂起扫描任务一段时间
			OSTaskSuspend(&app_tak_tcb_scan,&err);
			
			BEEP(1);
			delay_ms(1000);
			BEEP(0);
			delay_ms(1100);
			
			// 唤醒扫描任务
			OSTaskResume(&app_tak_tcb_scan,&err);
		}
		
//		//恢复任务1
//		OSTaskResume(&app_task_tcb_beep,&err);	
	}
}

void app_task_oled(void *parg)
{
	OS_ERR err;	
	OS_FLAGS flags=0;
	
	char buf_time[16]={0};
	char buf_date[16]={0};
	char buf_dht11[16]={0};
	uint8_t dht11_buf[5] = {0};		// 用于存储读取到的温湿度
	u8 dht11_time = 6;
	u8 lock_time = 15;
	u8 lock_event = 1;
	
	printf("[app_task_oled] create ok\r\n");

	while(1)
	{
		//一直阻塞等待事件标志组
		flags=OSFlagPend(&g_flag_grp,
							0x07,
							0,
							OS_OPT_PEND_FLAG_SET_ANY + OS_OPT_PEND_FLAG_CONSUME+OS_OPT_PEND_BLOCKING,
							NULL,
							&err);
		
		if(err != OS_ERR_NONE)
		{
			printf("[task2][OSFlagPend]Error Code = %d\r\n",err);
			continue;
		}
		
		OSMutexPend(&g_mutex_oled,0,OS_OPT_PEND_BLOCKING,NULL,&err);
		
		if(flags & 0x01) // 正常待机界面
		{
			//获取时间
			RTC_GetTime(RTC_Format_BCD,&RTC_TimeStructure);
			sprintf(buf_time,"%02x:%02x:%02x",RTC_TimeStructure.RTC_Hours,RTC_TimeStructure.RTC_Minutes,RTC_TimeStructure.RTC_Seconds);
			
			RTC_GetDate(RTC_Format_BCD,&RTC_DateStructure);
			sprintf(buf_date,"20%02x/%02x/%02xWeek%x",RTC_DateStructure.RTC_Year,RTC_DateStructure.RTC_Month,RTC_DateStructure.RTC_Date,RTC_DateStructure.RTC_WeekDay);

			if (g_menu_time_count == 0)
			{
				OLED_ShowString(8,0,(u8 *)"  ",16);
				OLED_ShowCHinese(24,0,0);//智
				OLED_ShowCHinese(40,0,1);//能
				OLED_ShowCHinese(56,0,2);//指
				OLED_ShowCHinese(72,0,3);//纹
				OLED_ShowCHinese(88,0,4);//锁
				OLED_ShowString(104,0,(u8 *)" ",16);
				memset(g_input_PW, 0, 5);
				g_count_PW = 0;
			}
			else if (g_menu_time_count == 3)
			{
				// 姓名
				OLED_ShowString(16,0,(u8 *)"GZ2075",16);
				OLED_ShowCHinese(64,0,98);
				OLED_ShowCHinese(80,0,99);
				OLED_ShowCHinese(96,0,100);
				
			}
			else if (g_menu_time_count >= 5)
			{
				g_menu_time_count = -1;
			}

			if (lock_time >= 15) // 15秒自动上锁，显示上锁icon
			{
				OLED_DrawBMP(112,0,128,2,(uint8_t *)icon_lock);
				lock_event = 0;
			}
			
			if (dht11_time >= 6)
			{
				
				dht11_read(dht11_buf);
				sprintf(buf_dht11, "H:%d.%d   T:%d.%d", dht11_buf[0], dht11_buf[1], dht11_buf[2], dht11_buf[3]);
				

				dht11_time = 0;
			}
			
			OLED_ShowString(32,2,(u8*)buf_time,16);
			OLED_ShowString(0,4,(u8*)buf_date,16);
			OLED_ShowString(0,6,(u8*)buf_dht11,16);
			
			g_menu_time_count++;
			dht11_time++;
			if (lock_event)
			{
				lock_time++;
			}
		}
		else if(flags & 0x02) // 解锁成功，则显示解锁成功icon+解锁成功字样
		{
			OLED_ShowString(16,0,(u8 *)"  ",16);
			OLED_ShowCHinese(32,0,5);//解
			OLED_ShowCHinese(48,0,6);//锁
			OLED_ShowCHinese(64,0,7);//成
			OLED_ShowCHinese(80,0,8);//功
			OLED_ShowString(96,0,(u8 *)"  ",16);
			
			OLED_DrawBMP(112,0,128,2,(uint8_t *)icon_unlock);
			lock_event = 1;
			lock_time = 0;
			g_menu_time_count = -2;
		}
		else if(flags & 0x04) // 解锁失败，显示“解锁失败”字样
		{
			OLED_ShowString(16,0,(u8 *)"  ",16);
			OLED_ShowCHinese(32,0,5);//解
			OLED_ShowCHinese(48,0,6);//锁
			OLED_ShowCHinese(64,0,9);//失
			OLED_ShowCHinese(80,0,10);//败
			OLED_ShowString(96,0,(u8 *)"  ",16);
			
			g_menu_time_count = -2;
		}
		else 
		
		OSMutexPost(&g_mutex_oled,OS_OPT_POST_NONE,&err);
		
	}
}


void app_task_rfid(void *parg)
{
	OS_ERR  err;
	OS_FLAGS flags=0;
	OS_MSG_SIZE msg_size;
	
	uint8_t *p=NULL;
	u8 status,card_size;
	u8 pmsg;
	int32_t i;
	int32_t j;
	u16 count;
	char buf[20] = {0};
	char buf1[20] = {0};
	char buf2[20] = {0};
	
	//MFRC522数据区
	u8  card_pydebuf[2];
	u8  card_numberbuf[5];
	u8  card_key0Abuf[6]={0xff,0xff,0xff,0xff,0xff,0xff};
	u8  card_writebuf[16]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	u8  card_readbuf[18];

	printf("[app_task_rfid] create ok\r\n");

	while(1)
	{
		flags=OSFlagPend(&g_flag_rfid,
							0x07,
							0,
							OS_OPT_PEND_FLAG_SET_ANY + OS_OPT_PEND_FLAG_CONSUME+OS_OPT_PEND_BLOCKING,
							NULL,
							&err);
		
		if (flags & 0x01) // 需要验证卡
		{
			MFRC522_Initializtion();			//初始化MFRC522
			status=MFRC522_Request(0x52, card_pydebuf);			//寻卡
		
			if (status == 0) // 如果读到卡
			{
				memset(card_readbuf, 0, 16);
				status=MFRC522_Anticoll(card_numberbuf);			//防撞处理			
				card_size=MFRC522_SelectTag(card_numberbuf);	//选卡
				status=MFRC522_Auth(0x60, 4, card_key0Abuf, card_numberbuf);	//验卡
				status=MFRC522_Write(4, card_writebuf);				//写卡（写卡要小心，特别是各区的块3）
				status=MFRC522_Read(4, card_readbuf);					//读卡
				//MFRC522_Halt();															//使卡进入休眠状态
				printf("card_pydebuf:%02X %02X \r\n",card_pydebuf[0],card_pydebuf[1]);
					
				//卡序列号显，最后一字节为卡的校验码
				printf("card_numberbuf:%02X %02X %02X %02X %02X \r\n",card_numberbuf[0],card_numberbuf[1],card_numberbuf[2],card_numberbuf[3],card_numberbuf[4]);
				//卡容量显示，单位为Kbits
				printf("card_size:%d Kbits\r\n",card_size);
				//读卡状态显示，正常为0
				printf("status=%d\r\n",status);
				//读一个块的数据显示
				printf("read:");
				for(i=0;i<16;i++)		//分两行显示
				{
					printf("%d ",card_readbuf[i]);

				}
				printf("\r\n\n");
				
				sprintf(buf,"%02x%02x%02x%02x%02x#",card_numberbuf[0],card_numberbuf[1],card_numberbuf[2],card_numberbuf[3],card_numberbuf[4]);
				
				if (Check_Card((u8*)buf)) // 如果该卡存在，则解锁
				{
						printf("RFID卡匹配，解锁\r\n");
						Write_Unlock_Record(3);
						
						OSFlagPost(&g_flag_grp,0x02,OS_OPT_POST_FLAG_SET,&err);	
				}
				else
				{
						printf("RFID卡不匹配\r\n");
						
						OSFlagPost(&g_flag_grp,0x04,OS_OPT_POST_FLAG_SET,&err);	
				}
			}
		}
		else if (flags & 0x02) // 录入卡
		{
			i = 0;
			while (1)
			{
				OLED_Clear();
				OLED_ShowCHinese(16,0,15);//请
				OLED_ShowCHinese(32,0,92);//刷
				OLED_ShowCHinese(48,0,91);//新
				OLED_ShowCHinese(64,0,88);//卡
				MFRC522_Initializtion();			//初始化MFRC522
				status=MFRC522_Request(0x52, card_pydebuf);			//寻卡
			
				if (status == 0) // 如果读到卡
				{
					memset(card_readbuf, 0, 16);
					status=MFRC522_Anticoll(card_numberbuf);			//防撞处理			
					MFRC522_SelectTag(card_numberbuf);	//选卡
					status=MFRC522_Auth(0x60, 4, card_key0Abuf, card_numberbuf);	//验卡
					status=MFRC522_Write(4, card_writebuf);				//写卡（写卡要小心，特别是各区的块3）
					status=MFRC522_Read(4, card_readbuf);					//读卡
					
					OLED_Clear();
					OLED_ShowString(56,0,"ID",16);//ID
					sprintf(buf,"%02x %02x %02x %02x %02x",card_numberbuf[0],card_numberbuf[1],card_numberbuf[2],card_numberbuf[3],card_numberbuf[4]);
					OLED_ShowString(8,2,(u8 *)buf,16);//卡ID
					
					memset(buf, 0, 20);
					sprintf(buf,"%02x%02x%02x%02x%02x#",card_numberbuf[0],card_numberbuf[1],card_numberbuf[2],card_numberbuf[3],card_numberbuf[4]);

					if (Check_Card((u8*)buf)) // 如果卡存在
					{
						OLED_ShowCHinese(24,4,93);//该
						OLED_ShowCHinese(40,4,88);//卡
						OLED_ShowCHinese(56,4,94);//存
						OLED_ShowCHinese(72,4,95);//在
						OLED_ShowCHinese(24,6,11);//录
						OLED_ShowCHinese(40,6,12);//入
						OLED_ShowCHinese(56,6,9);//失
						OLED_ShowCHinese(72,6,10);//败
						delay_ms(2000);
						break;
					}
					else // 如果卡不存在，则录入
					{
						WriteFlashData((FLASH5_ADDR+Count_Card()*11), (u8*)buf, 11);
						OLED_ShowCHinese(24,4,11);//录
						OLED_ShowCHinese(40,4,12);//入
						OLED_ShowCHinese(56,4,7);//成
						OLED_ShowCHinese(72,4,8);//功
						delay_ms(2000);
						break;
					}
				}
				
				delay_ms(1000);
				i++;
				if (i == 5) // 5次没放卡就退出
				{
					printf("超过5次没有放卡\r\n");
					break;
				}
			}
		
			OLED_Clear();
			OSFlagPost(&g_flag_rfid,0x08,OS_OPT_POST_FLAG_SET,&err);
		}
		else if (flags & 0x04) // 删除卡
		{
			count = Count_Card();
			OLED_Clear();
			i = 0;
			if (count == 0)
			{
				OLED_ShowCHinese(16,0,59);//未
				OLED_ShowCHinese(32,0,60);//检
				OLED_ShowCHinese(48,0,61);//测
				OLED_ShowCHinese(64,0,62);//到
				OLED_ShowCHinese(80,0,88);//卡
				OLED_ShowString(96,0,"ID",16);//ID
				
				delay_ms(1000);
				OLED_Clear();
			}
			else
			{
				OLED_ShowCHinese(16,0,96);//选
				OLED_ShowCHinese(32,0,97);//择
				OLED_ShowCHinese(48,0,13);//删
				OLED_ShowCHinese(64,0,14);//除
				OLED_ShowCHinese(80,0,88);//卡
				OLED_ShowString(96,0,"ID",16);//ID
				
				OLED_ShowString(0,2,(u8 *)"A",16);
				OLED_ShowCHinese(8,2,82);//确
				OLED_ShowCHinese(24,2,83);//认
				OLED_ShowString(40,2,(u8 *)"BC",16);
				OLED_ShowCHinese(56,2,84);//翻
				OLED_ShowCHinese(72,2,85);//页
				OLED_ShowString(88,2,(u8 *)"D",16);
				OLED_ShowCHinese(96,2,57);//退
				OLED_ShowCHinese(112,2,58);//出
				
				while(1)
				{
					memset(buf1,0,sizeof(buf1));
					memset(buf2,0,sizeof(buf2));
					
					ReadFlashData((FLASH5_ADDR+i*11),buf1,11);
					sprintf(buf2,"%c%c %c%c %c%c %c%c %c%c",buf1[0],buf1[1],buf1[2],buf1[3],buf1[4],buf1[5],buf1[6],buf1[7],buf1[8],buf1[9]);
					OLED_ShowString(8,5,(u8 *)buf2,16);
					
					p=OSQPend(&g_queue_key,0,OS_OPT_PEND_BLOCKING,&msg_size,NULL,&err);//阻塞等待操作
					pmsg = *p;
					OSFlagPost(&g_flag_grp,0x10,OS_OPT_POST_FLAG_SET,&err);	
					
					if (pmsg == 'B') // 上翻页
					{
						if (i != 0)
						{
							i--;
						}
						else
						{
							i = count-1;
						}
					}
					else if (pmsg == 'C') // 下翻页
					{
						if (i != count-1)
						{
							i++;
						}
						else
						{
							i = 0;
						}
					}
					else if (pmsg == 'D') // 退出
					{
						OLED_Clear();
						i = -1;
						break;
					}
					else if (pmsg == 'A') // 确认删除
					{
						break;
					}
					
					memset(p, 0, msg_size);
				}
				
				if (i != -1)
				{
					// 将扇区5的其他卡数据转存到扇区4
					printf("正在擦除扇区4\r\n");
					flash_erase(4);
					printf("擦除扇区4完成\r\n");
					for (j = 0; j < count; j++)
					{
						if (i != j)
						{
							memset(buf1, 0, 11);
							ReadFlashData((FLASH5_ADDR+j*11), buf1, 11);
							if (j < i)
							{
								WriteFlashData((FLASH4_ADDR+j*11), (u8*)buf1, 11);
							}
							else
							{
								WriteFlashData((FLASH4_ADDR+j*11-11), (u8*)buf1, 11);
							}
						}
					}
					
					// 将扇区4的数据存回扇区5
					printf("正在擦除扇区5\r\n");
					flash_erase(5);
					printf("擦除扇区5完成\r\n");
					for (j = 0; j < count-1; j++)
					{
						memset(buf1, 0, 11);
						ReadFlashData((FLASH4_ADDR+j*11), buf1, 11);
						WriteFlashData((FLASH5_ADDR+j*11), (u8*)buf1, 11);
					}
					
					printf("删除卡成功\r\n");
					
					OLED_ShowString(8,5,"               ",16);
					OLED_ShowCHinese(32,5,13);//删
					OLED_ShowCHinese(48,5,14);//除
					OLED_ShowCHinese(64,5,7);//成
					OLED_ShowCHinese(80,5,8);//功
					
					delay_ms(2000);
					OLED_Clear();
				}
			}
			
			OSFlagPost(&g_flag_rfid,0x10,OS_OPT_POST_FLAG_SET,&err);

		}

		//delay_ms(500); // 每0.5秒检测一次
	}
}

void app_task_as608(void *parg)
{
	OS_ERR err;
	OS_FLAGS flags=0;
	OS_MSG_SIZE msg_size;
	
	uint8_t *p=NULL;
	char buf[3]={0};
	SearchResult seach;
	int32_t i,j;
	u8 ensure ,processnum=0;
	u16 ID;
//	u32 buf_id[300] = {0}; // 记录ID，写到扇区7
//	u16 id_count;
	
	printf("[app_task_as608] create ok\r\n");

	while(1)
	{
		flags=OSFlagPend(&g_flag_as608,
							0x0F,
							0,
							OS_OPT_PEND_FLAG_SET_ANY + OS_OPT_PEND_FLAG_CONSUME+OS_OPT_PEND_BLOCKING,
							NULL,
							&err);
		
		if (flags & 0x01) // 需要验证指纹
		{
			ensure=PS_GetImage();
			if(ensure==0x00)//获取图像成功 
			{
				ensure=PS_GenChar(CharBuffer1);
				if(ensure==0x00) //生成特征成功
				{
					ensure=PS_HighSpeedSearch(CharBuffer1,0,AS608Para.PS_max,&seach);
					if(ensure==0x00)//搜索成功
					{				
						OSTaskResume(&app_task_tcb_beep,&err);
						printf("匹配指纹成功\r\n");
						printf("确有此人,ID:%d  匹配得分:%d\r\n",seach.pageID,seach.mathscore);
						
						//解锁成功事件标志组
						OSFlagPost(&g_flag_grp,0x02,OS_OPT_POST_FLAG_SET,&err);	
						Write_Unlock_Record(1);
						delay_ms(1000);
					}
					else 
					{
						printf("匹配指纹失败%d\r\n",ensure);	
						//解锁失败事件标志组		
						OSFlagPost(&g_flag_grp,0x04,OS_OPT_POST_FLAG_SET,&err);	
						delay_ms(1000);
					}
				}
			}
		}
		else if (flags & 0x02) // 查指纹
		{
			printf("查指纹\r\n");
			List_FR();
			OSFlagPost(&g_flag_as608,0x40,OS_OPT_POST_FLAG_SET,&err);
		}
		else if (flags & 0x04) // 录指纹
		{
			printf("录指纹\r\n");
			j = 0;
			processnum = 0;
			i = 0;
			while(j != -1)
			{
				switch (processnum)
				{
					case 0:
						i++;
						printf("请按指纹\r\n");
						OLED_Clear();
						OLED_ShowCHinese(27,0,15);//请
						OLED_ShowCHinese(45,0,17);//按
						OLED_ShowCHinese(63,0,2);//指
						OLED_ShowCHinese(81,0,3);//纹
					
						ensure=PS_GetImage();
						if(ensure==0x00) 
						{
							BEEP(1);
							ensure=PS_GenChar(CharBuffer1);//生成特征
							BEEP(0);
							if(ensure==0x00)
							{
								printf("指纹正常\r\n");
								OLED_ShowCHinese(27,2,2);//指
								OLED_ShowCHinese(45,2,3);//纹
								OLED_ShowCHinese(63,2,29);//正
								OLED_ShowCHinese(81,2,30);//常
								
								i=0;
								processnum=1;//跳到第二步						
							}else printf("error:%d\r\n",ensure);				
						}else printf("error:%d\r\n",ensure);						
						break;
					
					case 1:
						i++;
						printf("请再按一次指纹\r\n");
						OLED_Clear();
						OLED_ShowCHinese(16,0,15);//请
						OLED_ShowCHinese(32,0,16);//再
						OLED_ShowCHinese(48,0,19);//次
						OLED_ShowCHinese(64,0,17);//按
						OLED_ShowCHinese(80,0,2);//指
						OLED_ShowCHinese(96,0,3);//纹		
					
						ensure=PS_GetImage();
						if(ensure==0x00) 
						{
							BEEP(1);
							ensure=PS_GenChar(CharBuffer2);//生成特征
							BEEP(0);
							if(ensure==0x00)
							{
								printf("指纹正常\r\n");
								OLED_ShowCHinese(27,2,2);//指
								OLED_ShowCHinese(45,2,3);//纹
								OLED_ShowCHinese(63,2,29);//正
								OLED_ShowCHinese(81,2,30);//常
								
								i=0;
								processnum=2;//跳到第三步
							}else printf("error:%d\r\n",ensure);	
						}else printf("error:%d\r\n",ensure);		
						break;

					case 2:
						printf("对比两次指纹\r\n");
						OLED_Clear();
						OLED_ShowCHinese(16,0,21);//对
						OLED_ShowCHinese(32,0,22);//比
						OLED_ShowCHinese(48,0,23);//两
						OLED_ShowCHinese(64,0,24);//次
						OLED_ShowCHinese(80,0,2);//指
						OLED_ShowCHinese(96,0,3);//纹	
					
						ensure=PS_Match();
						if(ensure==0x00) 
						{
							OLED_ShowCHinese(27,2,21);//对
							OLED_ShowCHinese(45,2,22);//比
							OLED_ShowCHinese(63,2,7);//成
							OLED_ShowCHinese(81,2,8);//功
							processnum=3;//跳到第四步
						}
						else 
						{
							OLED_ShowCHinese(27,2,21);//对
							OLED_ShowCHinese(45,2,22);//比
							OLED_ShowCHinese(63,2,9);//失
							OLED_ShowCHinese(81,2,10);//败
							
							printf("error:%d\r\n",ensure);
							i=0;
							processnum=0;//跳回第一步		
						}
						delay_ms(1200);
						break;

					case 3:
						printf("生成指纹模板\r\n");
						OLED_ShowCHinese(16,4,25);//生
						OLED_ShowCHinese(32,4,26);//成
						OLED_ShowCHinese(48,4,2);//指
						OLED_ShowCHinese(64,4,3);//纹
						OLED_ShowCHinese(80,4,27);//模
						OLED_ShowCHinese(96,4,28);//板	
					
						ensure=PS_RegModel();
						if(ensure==0x00) 
						{
							OLED_ShowCHinese(27,6,25);//生
							OLED_ShowCHinese(45,6,26);//成
							OLED_ShowCHinese(63,6,7);//成
							OLED_ShowCHinese(81,6,8);//功
							processnum=4;//跳到第五步
						}
						else
						{
							OLED_ShowCHinese(27,6,25);//生
							OLED_ShowCHinese(45,6,26);//成
							OLED_ShowCHinese(63,6,9);//成
							OLED_ShowCHinese(81,6,10);//功
							
							processnum=0;
							printf("error:%d\r\n",ensure);
						}
						delay_ms(1200);
						break;
						
					case 4:	
						printf("输入指纹ID\r\n");
						printf("按'#'保存\r\n");
						OLED_Clear();
						OLED_ShowCHinese(0,0,31);//输
						OLED_ShowCHinese(16,0,32);//入
						OLED_ShowCHinese(32,0,2);//指
						OLED_ShowCHinese(48,0,3);//纹
						OLED_ShowString(64,0,(u8 *)"ID",16);
						OLED_ShowString(80,0,(u8 *)"0->",16);
						OLED_ShowString(104,0,(u8 *)"299",16);
					
						OLED_ShowCHinese(27,2,17);//按
						OLED_ShowString(49,2,(u8 *)"#",16);
						OLED_ShowCHinese(63,2,34);//保
						OLED_ShowCHinese(81,2,35);//存			
						
						OLED_ShowString(8,6,(u8 *)"*",16);
						OLED_ShowCHinese(16,6,89);//重
						OLED_ShowCHinese(32,6,31);//输			
					
						OLED_ShowString(56,6,(u8 *)"D",16);
						OLED_ShowCHinese(64,6,57);//退
						OLED_ShowCHinese(80,6,58);//出
					
						memset(buf, 0, 20);
						// 输入ID
						while (1)
						{
							//阻塞等待操作
							p=OSQPend(&g_queue_key,0,OS_OPT_PEND_BLOCKING,&msg_size,NULL,&err);
							buf[j] = *p;
							OSFlagPost(&g_flag_grp,0x10,OS_OPT_POST_FLAG_SET,&err);	
							
							j++;
							
							if (strstr(buf, "*")) // 重输
							{
								memset(buf, 0, 3);
								j = 0;
								OLED_ShowString(40,4,(u8 *)"      ",16);
							}
							else if (strstr(buf, "D")) // 退出
							{
								j = -1;
								break;
							}
							else if (strstr(buf, "#") || j == 3) // 输入完毕
							{
								ID = atoi(buf);
								break;
							}
							
							OLED_ShowString(40,4,(u8 *)buf,16);
							memset(p, 0, msg_size);
						}
						
						if (j == -1)
						{
							break;
						}
						
						//储存模板
						ensure=PS_StoreChar(CharBuffer1,ID);
						
						if(ensure==0x00) 
						{		
							printf("录入指纹成功\r\n");
							OLED_ShowCHinese(16,4,11);//录
							OLED_ShowCHinese(32,4,12);//入
							OLED_ShowCHinese(48,4,2);//指
							OLED_ShowCHinese(64,4,3);//纹
							OLED_ShowCHinese(80,4,7);//成
							OLED_ShowCHinese(96,4,8);//功
							delay_ms(1500);
							List_FR();
							j = -1;
							break;
						}
						else // 返回第一步
						{
							processnum=0;
							printf("error:%d\r\n",ensure);
							OLED_ShowString(40,4,(u8 *)"ID",16);
							OLED_ShowCHinese(56,4,51);//错
							OLED_ShowCHinese(72,4,52);//误
							delay_ms(1000);
						}					
						break;				
				}
				delay_ms(600);
				if(i==5)//超过5次没有按手指则退出
				{
					printf("超过5次没有按手指\r\n");
					break;	
				}				
			}

			OLED_Clear();
			OSFlagPost(&g_flag_as608,0x10,OS_OPT_POST_FLAG_SET,&err);
		}
		else if (flags & 0x08) // 删指纹
		{
			printf("\n删除指纹\r\n");
			printf("请输入指纹ID按Enter发送\r\n");
			printf("0=< ID <=299\r\n");
			OLED_Clear();
			OLED_ShowCHinese(0,0,13);//删
			OLED_ShowCHinese(16,0,14);//除
			OLED_ShowCHinese(32,0,2);//指
			OLED_ShowCHinese(48,0,3);//纹
			OLED_ShowString(72,0,(u8 *)"ID",16);
			OLED_ShowString(96,0,(u8 *)"0->",14);
			OLED_ShowString(96,1,(u8 *)"299",14);
			
			OLED_ShowCHinese(27,2,17);//按
			OLED_ShowString(49,2,(u8 *)"#",16);
			OLED_ShowCHinese(63,2,82);//确
			OLED_ShowCHinese(81,2,83);//认	
			
			OLED_ShowString(8,6,(u8 *)"*",16);
			OLED_ShowCHinese(16,6,89);//重
			OLED_ShowCHinese(32,6,31);//输			
		
			OLED_ShowString(56,6,(u8 *)"D",16);
			OLED_ShowCHinese(64,6,57);//退
			OLED_ShowCHinese(80,6,58);//出
			
			j = 0;
			memset(buf, 0, 3);
			// 输入ID
			while (1)
			{
				//阻塞等待操作
				p=OSQPend(&g_queue_key,0,OS_OPT_PEND_BLOCKING,&msg_size,NULL,&err);
				buf[j] = *p;
				OSFlagPost(&g_flag_grp,0x10,OS_OPT_POST_FLAG_SET,&err);	
				j++;
				
				if (strstr(buf, "*")) // 重输
				{
					memset(buf, 0, 3);
					j = 0;
					OLED_ShowString(40,4,(u8 *)"      ",16);
				}
				else if (strstr(buf, "D")) // 退出
				{
					j = -1;
					OLED_Clear();
					break;
				}
				else if (strstr(buf, "#") || j == 3) // 输入完毕
				{
					ID = atoi(buf);
					break;
				}
				OLED_ShowString(40,4,(u8 *)buf,16);
				memset(p, 0, msg_size);
			}	
			
			if (j != -1)
			{
				OLED_Clear();
				if (ID != 999)
				{
					ensure=PS_DeletChar(ID,1);//删除单个指纹
					if(ensure==0x00)
					{	
						printf("删除指纹成功\r\n");
						OLED_ShowCHinese(27,2,13);//删
						OLED_ShowCHinese(45,2,14);//除
						OLED_ShowCHinese(63,2,7);//成
						OLED_ShowCHinese(81,2,8);//功
					}
					else{
						printf("删除指纹失败%d\r\n",ensure);
						OLED_ShowCHinese(27,2,13);//删
						OLED_ShowCHinese(45,2,14);//除
						OLED_ShowCHinese(63,2,9);//失
						OLED_ShowCHinese(81,2,10);//败
					}
				}
				else // 删除所有指纹
				{
					ensure=PS_DeletChar(0,299);//删除所有指纹
					if(ensure==0x00)
					{	
						printf("删除指纹成功\r\n");
						OLED_ShowCHinese(27,2,13);//删
						OLED_ShowCHinese(45,2,14);//除
						OLED_ShowCHinese(63,2,7);//成
						OLED_ShowCHinese(81,2,8);//功
					}
					else{
						printf("删除指纹失败%d\r\n",ensure);
						OLED_ShowCHinese(27,2,13);//删
						OLED_ShowCHinese(45,2,14);//除
						OLED_ShowCHinese(63,2,9);//失
						OLED_ShowCHinese(81,2,10);//败
					}
				}
				delay_ms(1000);
				List_FR();
			}
			
			OSFlagPost(&g_flag_as608,0x20,OS_OPT_POST_FLAG_SET,&err);
		}
		
	}
}

void app_task_scan(void *parg)
{
	OS_ERR err;
	
	printf("[app_task_scan] create ok\r\n");

	while(1)
	{
		OSFlagPost(&g_flag_as608,0x01,OS_OPT_POST_FLAG_SET,&err);
		OSFlagPost(&g_flag_rfid,0x01,OS_OPT_POST_FLAG_SET,&err);

		delay_ms(300);
	}
}

void app_task_sta(void *parg)
{
	OS_ERR err;
	OS_MSG_SIZE msg_size;
	
	uint8_t *p=NULL;
	u8 pmsg;
	u8 status = 0;	// 状态0，当前在主界面，接收的是密码输入
					// 状态1，当前在管理员界面，接收的是模式选择
					// 状态2，当前在改密码界面，接收的是新密码
	u8 buf[5] = {0};
	u8 i;
	u8 page_count = 0;
	
	printf("[app_task_sta] create ok\r\n");

	while(1)
	{
		p=OSQPend(&g_queue_key,0,OS_OPT_PEND_BLOCKING,&msg_size,NULL,&err);//阻塞等待操作
		pmsg = *p;
		OSFlagPost(&g_flag_grp,0x10,OS_OPT_POST_FLAG_SET,&err);	
		
		switch(status)
		{
			case 0: // 接收的是密码登录
			{
				g_input_PW[g_count_PW] = pmsg;
				g_count_PW++;
				printf("g_input_PW = %s\r\n", g_input_PW);
				g_menu_time_count = -15;
				
				for (i = 0; i < 6; i++)
				{
					if (i < g_count_PW)
					{
						OLED_ShowString(16+i*16,0,(u8 *)"* ",16);
					}
					else 
					{
						OLED_ShowString(16+i*16,0,(u8 *)"_ ",16);
					}
				}
				i = 0;
				
				// 对当前密码进行判断
				if (strstr(g_input_PW, g_user_PW)) // 用户密码匹配，解锁
				{
					OSFlagPost(&g_flag_grp,0x02,OS_OPT_POST_FLAG_SET,&err);	
					Write_Unlock_Record(2);
					memset(g_input_PW, 0, sizeof(g_input_PW));
					g_count_PW = 0;
				}
				else if (strstr(g_input_PW, g_root_PW)) // 管理员密码匹配，进入管理员模式
				{
					Write_Unlock_Record(5);
					
					//挂起任务2
					OSTaskSuspend(&app_task_tcb_oled,&err);
					OSTaskSuspend(&app_tak_tcb_scan,&err);
					
					OLED_Clear();
					admin_surface(0);
					
					memset(g_input_PW, 0, sizeof(g_input_PW));
					g_count_PW = 0;
					status = 1;
				}
				else if (g_count_PW == 6) // 密码错误，解锁失败
				{
					OSFlagPost(&g_flag_grp,0x04,OS_OPT_POST_FLAG_SET,&err);	
					memset(g_input_PW, 0, sizeof(g_input_PW));
					g_count_PW = 0;
				}
			}break;
			
			case 1: // 管理员模式下接收的是模式选择
			{
				if (pmsg == 'B') // 上翻页
				{
					if (page_count == 0)
					{
						page_count = 5;
					}
					else
					{
						page_count--;
					}
				}
				else if (pmsg == 'C') // 下翻页
				{
					if (page_count == 5)
					{
						page_count = 0;
					}
					else
					{
						page_count++;
					}
				}
				else if (pmsg == 'A') // 确认
				{
					// 对当前选项进行判断
					if (page_count == 0) // 如果是查指纹，不需要消息队列
					{
						OSFlagPost(&g_flag_as608,0x02,OS_OPT_POST_FLAG_SET,&err);	
						
						// 阻塞等待查指纹结束
						OSFlagPend(&g_flag_as608,
							0x40,
							0,
							OS_OPT_PEND_FLAG_SET_ANY + OS_OPT_PEND_FLAG_CONSUME+OS_OPT_PEND_BLOCKING,
							NULL,
							&err);
					}
					else if (page_count == 1) // 录指纹
					{
						OSFlagPost(&g_flag_as608,0x04,OS_OPT_POST_FLAG_SET,&err);
						
						// 阻塞等待录指纹结束
						OSFlagPend(&g_flag_as608,
							0x10,
							0,
							OS_OPT_PEND_FLAG_SET_ANY + OS_OPT_PEND_FLAG_CONSUME+OS_OPT_PEND_BLOCKING,
							NULL,
							&err);
					}
					else if (page_count == 2) // 删指纹
					{
						OSFlagPost(&g_flag_as608,0x08,OS_OPT_POST_FLAG_SET,&err);
						
						// 阻塞等待删指纹结束
						OSFlagPend(&g_flag_as608,
							0x20,
							0,
							OS_OPT_PEND_FLAG_SET_ANY + OS_OPT_PEND_FLAG_CONSUME+OS_OPT_PEND_BLOCKING,
							NULL,
							&err);
					}
					else if (page_count == 3) // 改密码
					{
						OLED_Clear();
						OLED_ShowCHinese(16,0,90);//更
						OLED_ShowCHinese(32,0,87);//改
						OLED_ShowCHinese(48,0,5);//解
						OLED_ShowCHinese(64,0,6);//锁
						OLED_ShowCHinese(80,0,45);//密
						OLED_ShowCHinese(96,0,46);//码
						
						OLED_ShowString(8,2,(u8 *)"*",16);
						OLED_ShowCHinese(16,2,89);//重
						OLED_ShowCHinese(32,2,47);//输
						
						OLED_ShowString(56,2,(u8 *)"D",16);
						OLED_ShowCHinese(64,2,57);//退
						OLED_ShowCHinese(80,2,58);//出
						status = 2;
					}
					else if (page_count == 4) // 录入卡，不需要消息队列
					{
						OSFlagPost(&g_flag_rfid,0x02,OS_OPT_POST_FLAG_SET,&err);
						
						// 阻塞等待录入卡结束
						OSFlagPend(&g_flag_rfid,
							0x08,
							0,
							OS_OPT_PEND_FLAG_SET_ANY + OS_OPT_PEND_FLAG_CONSUME+OS_OPT_PEND_BLOCKING,
							NULL,
							&err);
					}
					else if (page_count == 5) // 删除卡
					{
						OSFlagPost(&g_flag_rfid,0x04,OS_OPT_POST_FLAG_SET,&err);
						
						// 阻塞等待删除卡结束
						OSFlagPend(&g_flag_rfid,
							0x10,
							0,
							OS_OPT_PEND_FLAG_SET_ANY + OS_OPT_PEND_FLAG_CONSUME+OS_OPT_PEND_BLOCKING,
							NULL,
							&err);
					}
					
				}
				else if (pmsg == 'D') // 退出管理员模式
				{
					status = 0;
					g_menu_time_count = 0;
					OLED_Clear();
					OLED_DrawBMP(112,0,128,2,(uint8_t *)icon_lock);
					//唤醒任务2
					OSTaskResume(&app_task_tcb_oled,&err);
					OSTaskResume(&app_tak_tcb_scan,&err);
					
				}
				
				if (status == 1)
				{
					admin_surface(page_count);
				}
			}break;
			
			case 2: // 改密码模式
			{
				buf[i] = pmsg;
				i++;
				OLED_ShowString(40,4,(u8 *)buf,16);
				
				if (i == 4)
				{
					OLED_ShowCHinese(16,6,29);//正
					OLED_ShowCHinese(32,6,95);//在
					OLED_ShowCHinese(48,6,90);//更
					OLED_ShowCHinese(64,6,87);//改
					OLED_ShowCHinese(80,6,45);//密
					OLED_ShowCHinese(96,6,46);//码
					
					flash_erase(7);
					buf[4] = '\0';
					strncpy(g_user_PW, (char*)buf, 5);
					WriteFlashData(FLASH7_ADDR, (uint8_t *)g_user_PW, 5);
					WriteFlashData(FLASH7_ADDR+5, (uint8_t *)g_root_PW, 5); 
					memset(buf, 0, 5);
					i = 0;
					
					OLED_ShowCHinese(16,6,90);//更
					OLED_ShowCHinese(32,6,87);//改
					OLED_ShowCHinese(48,6,45);//密
					OLED_ShowCHinese(64,6,46);//码
					OLED_ShowCHinese(80,6,7);//成
					OLED_ShowCHinese(96,6,8);//功
					
					delay_ms(1000);
					status = 1;
				}
				else if (strstr((char*)buf, "*")) // 重输
				{
					memset(buf, 0, 5);
					i = 0;
					OLED_ShowString(40,4,(u8 *)"      ",16);
				}
				else if (strstr((char*)buf, "D")) // 退出
				{
					memset(buf, 0, 5);
					i = 0;
					status = 1;
				}
				
				if (status == 1)
				{
					OLED_Clear();
					admin_surface(page_count);
				}
			}break;
			
			default:break;
		}
		
		memset(p, 0, msg_size);
	}
}

void app_task_key(void *parg)
{
	OS_ERR err;
	
	char key_buf;
	
	// 矩阵键盘标志
	uint32_t key_sta=0;
	char key_old=0;
	char key_cur=0;
	
	printf("[app_task_key] create ok\r\n");

	while(1)
	{
		/* 使用状态机思想得到按键的状态 */
		switch(key_sta)
		{
			case 0://获取按下的按键
			{
				
				key_cur = get_key_board();	

				if(key_cur != 'N')
				{
					key_old = key_cur;
					key_sta=1;
				}
					
			
			}break;
			
			
			case 1://确认按下的按键
			{
				key_cur = get_key_board();	
					
				if((key_cur != 'N') && (key_cur == key_old))
				{
					key_buf = key_cur;
					OSQPost(&g_queue_key,&key_buf,1,OS_OPT_POST_FIFO,&err);
					// 阻塞等待键盘输入被接收完毕
						OSFlagPend(&g_flag_grp,
							0x10,
							0,
							OS_OPT_PEND_FLAG_SET_ANY + OS_OPT_PEND_FLAG_CONSUME+OS_OPT_PEND_BLOCKING,
							NULL,
							&err);
					printf("%c \r", key_cur);
					key_sta=2;
				}
							
			}break;
		
			case 2://获取释放的按键
			{	
				if(get_key_board() == 'N')
				{
					key_sta=0;
					
					key_old =  'N';
				
				}
								
			
			}break;

			default:break;
		}	
		
	}
}

void app_task_usart(void *parg)
{
	OS_ERR err;
	OS_MSG_SIZE msg_size;
	
	uint8_t *p=NULL;				// 用于接收消息队列的数据
	char *s = NULL; 				// 用于分割接收到的数据
	int32_t d;
	u32 i;
	u8 flag_login = 0;				// 0表示蓝牙未登录，1表示正确输入密码
	u32 record_count;
	
	printf("[app_task_usart] create ok\r\n");

	record_count = Count_Unlock_Record();
	
	while(1)
	{
		//阻塞等待操作
		p=OSQPend(&g_queue_usart,0,OS_OPT_PEND_BLOCKING,&msg_size,NULL,&err);
		
		//正确接收到消息
		if(p && msg_size)
		{
			//关闭串口中断
			NVIC_DisableIRQ(USART1_IRQn);
			NVIC_DisableIRQ(USART3_IRQn);
			
			if (!flag_login) // 如果蓝牙未登录
			{
				if (strstr((char *)p, g_root_PW)) // 如果输入了管理员密码
				{
					flag_login = 1;
					
					Write_Unlock_Record(5);
					
					printf("管理员登录\r\n");
					delay_ms(3000); // 为了测试时可以看到返回信息的延时，可以去掉
					usart3_send_str("管理员登录成功\r\n");
				}
				else if (strstr((char *)p, g_user_PW)) // 如果输入了解锁密码
				{
					Write_Unlock_Record(4);
					OSFlagPost(&g_flag_grp,0x02,OS_OPT_POST_FLAG_SET,&err);	
					printf("解锁成功\r\n");
					usart3_send_str("解锁成功\r\n");
				}
				else // 密码输入错误
				{
					printf("密码错误\r\n");
					usart3_send_str("密码错误\r\n");
				}
				
			}
			else if (flag_login) // 蓝牙管理员模式登录
			{
				if (strstr((char *)p, "record")) // 查询解锁记录
				{
					record_count = Count_Unlock_Record();
					printf("\n");
					for (i = 0; i < record_count; i++)
					{
						d = *((volatile uint32_t *)(FLASH6_ADDR + (i * 36)));
						printf("[%04d]\r", d);
						
						d = *((volatile uint32_t *)(FLASH6_ADDR + (i * 36) + 4));
						if (d == 1) // 指纹
						{
							printf(" [指纹解锁] \r");
						}
						else if (d == 2) // 键盘密码
						{
							printf(" [密码解锁] \r");
						}
						else if (d == 3) // RFID
						{
							printf(" [ RFID卡 ] \r");
						}
						else if (d == 4) // 蓝牙解锁
						{
							printf(" [蓝牙解锁] \r");
						}
						else if (d == 5) // 管理员
						{
							printf(" [ 管理员 ] \r");
						}
						
						// 年
						d = *((volatile uint32_t *)(FLASH6_ADDR + (i * 36) + 8));
						printf("20%02x/\r", d);
						
						// 月
						d = *((volatile uint32_t *)(FLASH6_ADDR + (i * 36) + 12));
						printf("%02x/\r", d);
						
						// 日
						d = *((volatile uint32_t *)(FLASH6_ADDR + (i * 36) + 16));
						printf("%02x \r", d);
						
						// 周
						d = *((volatile uint32_t *)(FLASH6_ADDR + (i * 36) + 20));
						printf("Week %x \r", d);
						
						// 时
						d = *((volatile uint32_t *)(FLASH6_ADDR + (i * 36) + 24));
						printf("%02x:\r", d);
						
						// 分
						d = *((volatile uint32_t *)(FLASH6_ADDR + (i * 36) + 28));
						printf("%02x:\r", d);
						
						// 秒
						d = *((volatile uint32_t *)(FLASH6_ADDR + (i * 36) + 32));
						printf("%02x \r\n", d);
					}
					
					printf("\n共%d个解锁记录读取完毕\r\n\n", record_count);
				}
				else if (strstr((char *)p, "clear")) // 清除记录
				{
					printf("正在擦除扇区6\r\n");
					flash_erase(6);
					printf("擦除扇区6完成\r\n");
					usart3_send_str("记录清除完毕\r\n");
					record_count = Count_Unlock_Record();
				}
				else if (strstr((char *)p,  "newpw=")) // 设置新解锁密码
				{
					s = strtok((char*)p, "=");
					s = strtok(NULL, "=");
					
					if (strlen(s) != 5) // 长度不对
					{
						printf("新密码长度不对\r\n");
						usart3_send_str("新密码长度不对\r\n");
					}
					else
					{
						usart3_send_str("正在更改密码\r\n");
						printf("正在更改密码\r\n");
						
						strncpy(g_user_PW, s, 4);
						g_user_PW[4] = '\0';
						flash_erase(7);
						WriteFlashData(FLASH7_ADDR, (uint8_t *)g_user_PW, 5);
						WriteFlashData(FLASH7_ADDR+5, (uint8_t *)g_root_PW, 5); 
						
						OLED_ShowCHinese(16,6,90);//更
						OLED_ShowCHinese(32,6,87);//改
						OLED_ShowCHinese(48,6,45);//密
						OLED_ShowCHinese(64,6,46);//码
						OLED_ShowCHinese(80,6,7);//成
						OLED_ShowCHinese(96,6,8);//功
						
						usart3_send_str("更改密码成功\r\n");
						printf("更改密码成功\r\n");
					}
				}
				else if (strstr((char *)p, "unlock")) // 解锁
				{
					Write_Unlock_Record(4);
					OSFlagPost(&g_flag_grp,0x02,OS_OPT_POST_FLAG_SET,&err);	
					printf("管理员解锁成功\r\n");
					usart3_send_str("解锁成功\r\n");
				}
				
			}
			
			// 退出登录
			if (strstr((char *)p, "quit"))
			{
				flag_login = 0;
				printf("管理员连接断开\r\n");
			}	
			
			g_usart1_cnt = 0;
			g_usart1_event = 0;
			memset((void *)g_usart1_buf, 0, sizeof(g_usart1_buf));
			g_usart3_cnt = 0;
			g_usart3_event = 0;
			memset((void *)g_usart3_buf, 0, sizeof(g_usart3_buf));
			
			memset(p, 0, msg_size);
			
			//使能串口中断
			NVIC_EnableIRQ(USART1_IRQn);
			NVIC_EnableIRQ(USART3_IRQn);
		}
		
	}
}

// 查询FLASH5中存储的卡的数量
u16 Count_Card(void)
{
	u16 i = 0;
	char buf[11];
	
	while(1)
	{
		memset(buf, 0, 11);
		ReadFlashData((FLASH5_ADDR+i*11), (char*)buf, 11);
		if (!strstr(buf, "#"))
		{
			break;
		}
		i++;
	}
	
	return i;
}

// 验证卡
uint8_t Check_Card(u8* card)
{
	u16 i;
	char buf[11];
	
	for (i = 0; i < Count_Card(); i++)
	{
		ReadFlashData((FLASH5_ADDR+i*11), (char*)buf, 11);
		if (strstr((char*)card, buf))
		{
			return 1;	// 表示存在该卡
		}
	}
	return 0;// 表示该卡不存在
}

// 读取指纹储存
void List_FR(void)
{
	u8 buf[3]={0};
	OLED_Clear();
	PS_ValidTempleteNum(&g_ValidN);//读取库存指纹个数
	printf("已录入指纹%d\r\n", g_ValidN);
	printf("剩余指纹数%d\r\n",AS608Para.PS_max-g_ValidN);
	OLED_ShowCHinese(0,2,69);//库
	OLED_ShowCHinese(16,2,70);//存
	OLED_ShowCHinese(32,2,2);//指
	OLED_ShowCHinese(48,2,3);//纹
	OLED_ShowCHinese(64,2,71);//个
	OLED_ShowCHinese(80,2,72);//数
	memset(buf,0,3);
	sprintf((char*)buf,"%d",g_ValidN);
	OLED_ShowString(100,2,(u8*)buf,16);
	
	OLED_ShowCHinese(0,4,69);//库
	OLED_ShowCHinese(16,4,70);//存
	OLED_ShowCHinese(32,4,73);//剩
	OLED_ShowCHinese(48,4,74);//余
	OLED_ShowCHinese(64,4,75);//空
	OLED_ShowCHinese(80,4,76);//间
	sprintf((char*)buf,"%d",AS608Para.PS_max-g_ValidN);
	OLED_ShowString(100,4,(u8*)buf,16);
	delay_ms(2000);
	OLED_Clear();
}

// 写入解锁记录，mode1指纹，2键盘，3RFID，4蓝牙，5管理员
void Write_Unlock_Record(u8 mode)
{
	u32 record_count;
	
	//获取时间
	RTC_GetTime(RTC_Format_BCD,&RTC_TimeStructure);

	RTC_GetDate(RTC_Format_BCD,&RTC_DateStructure);
	
	record_count = Count_Unlock_Record();
	
	//解锁闪存
	FLASH_Unlock();
	if (FLASH_ProgramWord(FLASH6_ADDR + (36*record_count), record_count+1) != FLASH_COMPLETE)
	{
		while(1);
	}
	if (FLASH_ProgramWord(FLASH6_ADDR + (36*record_count) + 4, mode) != FLASH_COMPLETE)
	{
		while(1);
	}
	if (FLASH_ProgramWord(FLASH6_ADDR + (36*record_count) + 8, RTC_DateStructure.RTC_Year) != FLASH_COMPLETE)
	{
		while(1);
	}
	if (FLASH_ProgramWord(FLASH6_ADDR + (36*record_count) + 12, RTC_DateStructure.RTC_Month) != FLASH_COMPLETE)
	{
		while(1);
	}
	if (FLASH_ProgramWord(FLASH6_ADDR + (36*record_count) + 16, RTC_DateStructure.RTC_Date) != FLASH_COMPLETE)
	{
		while(1);
	}
	if (FLASH_ProgramWord(FLASH6_ADDR + (36*record_count) + 20, RTC_DateStructure.RTC_WeekDay) != FLASH_COMPLETE)
	{
		while(1);
	}
	
	if (FLASH_ProgramWord(FLASH6_ADDR + (36*record_count) + 24, RTC_TimeStructure.RTC_Hours) != FLASH_COMPLETE)
	{
		while(1);
	}
	if (FLASH_ProgramWord(FLASH6_ADDR + (36*record_count) + 28, RTC_TimeStructure.RTC_Minutes) != FLASH_COMPLETE)
	{
		while(1);
	}
	if (FLASH_ProgramWord(FLASH6_ADDR + (36*record_count) + 32, RTC_TimeStructure.RTC_Seconds) != FLASH_COMPLETE)
	{
		while(1);
	}
	
	printf("当前记录了%d个数据\r\n", record_count);
}

// 计算解锁记录的个数
u32 Count_Unlock_Record(void)
{
	int32_t d;
	u32 record_count = 0;
	d = *((volatile uint32_t *)(FLASH6_ADDR));
	while (d != -1)
	{
		record_count++;
		d = *((volatile uint32_t *)(FLASH6_ADDR + (record_count * 36)));
	}
	printf("当前记录了%d个数据\r\n", record_count);
	
	return record_count;
}

void admin_surface(u32 page_count)
{
	uint8_t admin_mode_menu[] = {86,2,3,	//查指纹
								11,2,3,		//录指纹
								13,2,3,		//删指纹
								87,45,46,	//改密码
								11,12,88,	//录入卡
								13,14,88};	//删除卡
	
	OLED_ShowCHinese(24,0,77);//管
	OLED_ShowCHinese(40,0,78);//理
	OLED_ShowCHinese(56,0,79);//员
	OLED_ShowCHinese(72,0,80);//模
	OLED_ShowCHinese(88,0,81);//式
	
	OLED_ShowString(0,2,(u8 *)"A",16);
	OLED_ShowCHinese(8,2,82);//确
	OLED_ShowCHinese(24,2,83);//认
	OLED_ShowString(40,2,(u8 *)"BC",16);
	OLED_ShowCHinese(56,2,84);//翻
	OLED_ShowCHinese(72,2,85);//页
	OLED_ShowString(88,2,(u8 *)"D",16);
	OLED_ShowCHinese(96,2,57);//退
	OLED_ShowCHinese(112,2,58);//出
	
	OLED_ShowCHinese(40,5,admin_mode_menu[page_count*3]);
	OLED_ShowCHinese(56,5,admin_mode_menu[page_count*3+1]);
	OLED_ShowCHinese(72,5,admin_mode_menu[page_count*3+2]);
}



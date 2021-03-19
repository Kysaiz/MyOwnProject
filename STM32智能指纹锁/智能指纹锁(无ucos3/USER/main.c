/*******************************************
智能指纹锁
********************************************/

#include <stdio.h>
#include <string.h>

#include "sys.h"
#include "delay.h"
#include "usart1.h"
#include "usart3.h"

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
#include "tim.h"
#include "usart2.h"
#include "timer.h"
#include "bmp.h"

extern volatile uint8_t g_usart1_event;
extern volatile uint8_t g_usart1_buf[128];
extern volatile uint32_t g_usart1_cnt;

extern volatile uint8_t g_usart3_event;
extern volatile uint8_t g_usart3_buf[128];
extern volatile uint32_t g_usart3_cnt;

static RTC_TimeTypeDef  		RTC_TimeStructure;
static RTC_DateTypeDef			RTC_DateStructure;

extern volatile uint32_t g_rtc_wakeup_event;
extern volatile uint32_t g_dht11_time;	// dht11读取时间
extern volatile uint32_t g_lock_time;	// 锁定时间
extern volatile uint32_t g_key_event;	// 键盘输入事件标志位，0表示未输入，1表示正在输入
extern volatile uint32_t g_key_time;	// 键盘输入时间，10秒未输入则归零

static uint8_t g_FR_Check_Flag = 0;		// 指纹匹配标志位，0表示不匹配，1表示匹配

static uint16_t admin_mode_menu[] = {86,2,3,	//查指纹
									11,2,3,		//录指纹
									13,2,3,		//删指纹
									87,45,46,	//改密码
									11,12,88,	//录入卡
									13,14,88};	//删除卡

u16 g_ValidN;//模块内有效指纹个数
SysPara AS608Para;//指纹模块AS608参数
char user_PW[5];		// 存储用户密码
char root_PW[5];		// 存储管理员密码
uint32_t g_record_count = 0;		// 解锁记录计数值
									
uint8_t MFRC522_Check(void);
uint8_t Admin_Operation_KEY(uint8_t mode);
void Admin_ChangePW(void);
void Add_Card(void);
void Del_Card(void);
u16 Count_Card(void);
uint8_t Check_Card(u8* card);								
void List_FR(void);	
void Add_FR(void);
void press_FR(void);
void Del_FR(void);	
void List_Unlock_Record(u32 count);
void Write_Unlock_Record(u8 mode);
void Count_Unlock_Record(void);
	
									
int main(void)
{
	char *s = NULL; 				// 用于分割接收到的数据
	char buf_time[16]={0};
	char buf_date[16]={0};
	char buf_dht11[16]={0};
	uint8_t dht11_buf[5] = {0};		// 用于存储读取到的温湿度
	uint8_t login_mode_flag = 0;	// 0表示未登入，1表示用户解锁，2表示进入管理员模式
	uint8_t page_count = 0;			// 管理员模式翻页下标
	char input_PW[6] = {0};			// 当前输入密码缓冲区
	uint8_t count_PW = 0;			// 当前输入密码位数
	
	int8_t menu_time_count = 0;

	int8_t login_flag2 = -1;		// 串口3标志位，0未登录，1登录成功，-1未连接	
	uint32_t i;						// 循环下标
	
	// 矩阵键盘标志
	uint32_t key_sta=0;
	char key_old=0;
	char key_cur=0;
	
	AS608Para.PS_max = 10;
	
	//rtc_init(); //重设时间
	rtc_reload(); //读取寄存器时间
	
	MFRC522_Initializtion();			//初始化MFRC522
	
	PS_StaGPIO_Init(); // 指纹模块初始化
	usart2_init(57600);//初始化串口2,用于与指纹模块通讯
	
	//初始化串口1的波特率为115200bps
	//注意：如果接收的数据是乱码，要检查PLL。
	usart1_init(115200);
	
	usart3_EXTI9_5_init();
	
	//串口3的波特率为9600bps，因为蓝牙模块默认使用该波特率
	usart3_init(9600);

	delay_ms(500);

	BEEP_Init();
	LED_Init();
	
	OLED_Init();
	OLED_Clear(); 
	key_board_init();
	tim3_init();
	dht11_init();

	delay_ms(50);
	
//	FLASH_Unlock();
	flash_init();
	
	flash_erase(6);
//	flash_erase(5);
//	flash_erase(4);
//	flash_erase(3);
	
	delay_ms(10);
	
//	WriteFlashData(FLASH3_ADDR, (char *)user_PW, 5);
//	WriteFlashData(FLASH3_ADDR+5, (char *)root_PW, 5); 
	
	ReadFlashData(FLASH3_ADDR, (char *)user_PW, 5);
	ReadFlashData(FLASH3_ADDR+5, (char *)root_PW, 5);
	
//	FLASH_Lock();	
	
	printf("\n智能指纹锁-姚俊炜\r\n");
//	printf("user_PW = %s\r\n", user_PW);
//	printf("root_PW = %s\r\n", root_PW);	
	
	
	// 更改蓝牙名字
	usart3_send_str("AT+NAMEkkkk\r");
	delay_ms(50);
	
	Count_Unlock_Record();
	
	while(PS_HandShake(&AS608Addr))//与AS608模块握手
	{
		delay_ms(400);
		OLED_Clear(); 
		//未检测到模块
		OLED_ShowCHinese(0,0,59);
		OLED_ShowCHinese(18,0,60);
		OLED_ShowCHinese(36,0,61);
		OLED_ShowCHinese(54,0,62);
		OLED_ShowCHinese(72,0,67);
		OLED_ShowCHinese(90,0,68);
		
		delay_ms(800);
		OLED_Clear(); 
		//尝试连接模块
		OLED_ShowCHinese(0,0,63);
		OLED_ShowCHinese(18,0,64);
		OLED_ShowCHinese(36,0,65);
		OLED_ShowCHinese(54,0,66);
		OLED_ShowCHinese(72,0,67);
		OLED_ShowCHinese(90,0,68); 
	}
	
	// 计算记录个数
	
//	//成功连接模块
//	OLED_Clear(); 
//	OLED_ShowCHinese(0,0,7);
//	OLED_ShowCHinese(18,0,8);
//	OLED_ShowCHinese(36,0,65);
//	OLED_ShowCHinese(54,0,66);
//	OLED_ShowCHinese(72,0,67);
//	OLED_ShowCHinese(90,0,68); 	
//	
//	delay_ms(2000);
	OLED_Clear(); 
	
	while(1)
	{
		// 判断蓝牙连接是否成功
		if (PEin(5))
		{
			// 串口3接收到数据
			if (g_usart3_event)
			{
				// 如果未登录
				if (login_flag2 == 0)
				{
					if (strstr((char*)g_usart3_buf, "pw="));
					{
						s = strtok((char*)g_usart3_buf, "=");
						s = strtok(NULL, "=");
						
						if (strstr((char*)g_usart3_buf, user_PW)) // 如果输入了解锁密码
						{
							printf("蓝牙输入解锁密码，解锁\r\n");
							usart3_send_str("密码正确，解锁成功\r\n");
							
							if (login_mode_flag == 0)
							{
								Write_Unlock_Record(4);
								g_record_count++;
								printf("当前记录了%d个数据\r\n",g_record_count);
							}
							
							g_lock_time = 0;
							login_mode_flag = 1;
							
						}
						else if (strstr((char*)g_usart3_buf, root_PW)) // 如果输入了管理员密码
						{
							printf("蓝牙输入管理员密码\r\n");
							usart3_send_str("密码正确，进入管理员模式\r\n");
							login_flag2 = 1;
							login_mode_flag = 3;
						}
						
					}
				}
				
				// 如果已登录为管理员模式
				if (login_flag2)
				{
					if (strstr((char*)g_usart3_buf, "record")) // 查询解锁记录
					{
						List_Unlock_Record(g_record_count);
					}
					else if (strstr((char*)g_usart3_buf, "clear")) // 清除记录
					{
						printf("正在擦除扇区6\r\n");
						flash_erase(6);
						printf("擦除扇区6完成\r\n");
						Count_Unlock_Record();
					}
					else if (strstr((char*)g_usart3_buf, "newpw=")) // 设置新解锁密码
					{
						s = strtok((char*)g_usart3_buf, "=");
						s = strtok(NULL, "=");
						
						if (strlen(s) != 5) // 长度不对
						{
							printf("新密码长度不对\r\n");
						}
						else
						{
							strncpy(user_PW, s, 4);
							user_PW[4] = '\0';
							flash_erase(3);
							WriteFlashData(FLASH3_ADDR, (uint8_t *)user_PW, 5);
							WriteFlashData(FLASH3_ADDR+5, (uint8_t *)root_PW, 5); 
							
							OLED_ShowCHinese(16,6,90);//更
							OLED_ShowCHinese(32,6,87);//改
							OLED_ShowCHinese(48,6,45);//密
							OLED_ShowCHinese(64,6,46);//码
							OLED_ShowCHinese(80,6,7);//成
							OLED_ShowCHinese(96,6,8);//功
							
							printf("蓝牙更改密码成功\r\n");
						}
					}
					else if (strstr((char*)g_usart3_buf, "unlock")) // 解锁
					{
						g_lock_time = 0;
						
						printf("蓝牙管理员解锁成功\r\n");
						
						Write_Unlock_Record(4);
						g_record_count++;
						printf("当前记录了%d个数据\r\n",g_record_count);
						
						
					}
					
					g_usart3_cnt = 0;
					g_usart3_event = 0;
					memset((void *)g_usart3_buf, 0, sizeof(g_usart3_buf));
					
				}
			}
		
			
			g_usart3_cnt = 0;
			g_usart3_event = 0;
			memset((void *)g_usart3_buf, 0, sizeof(g_usart3_buf));
		}
		else // 蓝牙断开连接
		{
			if (login_flag2 != -1)
			{
				printf("蓝牙断开连接\r\n");
				login_flag2 = -1;
			}
		}
		
		// 串口1修改数据，模拟蓝牙
		if (g_usart1_event)
		{
			if (strstr((char*)g_usart1_buf, "record")) // 查询解锁记录
			{
				List_Unlock_Record(g_record_count);
			}
			else if (strstr((char*)g_usart1_buf, "clear")) // 清除记录
			{
				printf("正在擦除扇区6\r\n");
				flash_erase(6);
				printf("擦除扇区6完成\r\n");
				Count_Unlock_Record();
			}
			else if (strstr((char*)g_usart1_buf, "newpw=")) // 设置新解锁密码
			{
				s = strtok((char*)g_usart1_buf, "=");
				s = strtok(NULL, "=");
				
				if (strlen(s) != 5) // 长度不对
				{
					printf("新密码长度不对\r\n");
				}
				else
				{
					strncpy(user_PW, s, 4);
					user_PW[4] = '\0';
					flash_erase(3);
					WriteFlashData(FLASH3_ADDR, (uint8_t *)user_PW, 5);
					WriteFlashData(FLASH3_ADDR+5, (uint8_t *)root_PW, 5); 
					
					OLED_ShowCHinese(16,6,90);//更
					OLED_ShowCHinese(32,6,87);//改
					OLED_ShowCHinese(48,6,45);//密
					OLED_ShowCHinese(64,6,46);//码
					OLED_ShowCHinese(80,6,7);//成
					OLED_ShowCHinese(96,6,8);//功
					
					printf("蓝牙更改密码成功\r\n");
				}
			}
			else if (strstr((char*)g_usart1_buf, "unlock")) // 解锁
			{
				g_lock_time = 0;
				
				printf("蓝牙管理员解锁成功\r\n");
				
				Write_Unlock_Record(4);
				g_record_count++;
				printf("当前记录了%d个数据\r\n",g_record_count);
				
				
			}
			
			g_usart1_cnt = 0;
			g_usart1_event = 0;
			memset((void *)g_usart1_buf, 0, sizeof(g_usart1_buf));
		}
		
		if(g_key_event)
		{
			for (i = 0; i < 6; i++)
			{
				if (i < count_PW)
				{
					OLED_ShowString(16+i*16,0,(u8 *)"* ",16);
				}
				else 
				{
					OLED_ShowString(16+i*16,0,(u8 *)"_ ",16);
				}
				
			}
		}
		
		// 每秒进行一次
		if(g_rtc_wakeup_event && (login_mode_flag == 0 || login_mode_flag == 1))
		{
			//获取时间
			RTC_GetTime(RTC_Format_BCD,&RTC_TimeStructure);
			sprintf(buf_time,"%02x:%02x:%02x",RTC_TimeStructure.RTC_Hours,RTC_TimeStructure.RTC_Minutes,RTC_TimeStructure.RTC_Seconds);
			
			RTC_GetDate(RTC_Format_BCD,&RTC_DateStructure);
			sprintf(buf_date,"20%02x/%02x/%02xWeek%x",RTC_DateStructure.RTC_Year,RTC_DateStructure.RTC_Month,RTC_DateStructure.RTC_Date,RTC_DateStructure.RTC_WeekDay);
			
			// 15秒自动上锁
			if (g_lock_time > 15)
			{
				OLED_DrawBMP(112,0,128,2,(uint8_t *)icon_lock);
				login_mode_flag = 0;
			}
			else
			{
				OLED_DrawBMP(112,0,128,2,(uint8_t *)icon_unlock);
			}
			
			if (g_dht11_time > 6)
			{
				dht11_read(dht11_buf);
				g_dht11_time = 0;
				sprintf(buf_dht11, "H:%d.%d   T:%d.%d", dht11_buf[0], dht11_buf[1], dht11_buf[2], dht11_buf[3]);
			}
			
			
			OLED_ShowString(32,2,(u8*)buf_time,16);
			OLED_ShowString(0,4,(u8*)buf_date,16);
			OLED_ShowString(0,6,(u8*)buf_dht11,16);
			
			if (g_key_time > 15)
			{
				g_key_event = 0;
				g_key_time = 0;
				menu_time_count = 0;
				printf("超时输入\r\n");
			}
			
			if (g_key_event == 0)
			{
				if (menu_time_count == 0)
				{
					OLED_ShowString(8,0,(u8 *)"  ",16);
					OLED_ShowCHinese(24,0,0);//智
					OLED_ShowCHinese(40,0,1);//能
					OLED_ShowCHinese(56,0,2);//指
					OLED_ShowCHinese(72,0,3);//纹
					OLED_ShowCHinese(88,0,4);//锁
					OLED_ShowString(104,0,(u8 *)" ",16);
				}
				else if (menu_time_count == 3)
				{
					// 姓名
					OLED_ShowString(16,0,(u8 *)"GZ2075",16);
					OLED_ShowCHinese(64,0,98);
					OLED_ShowCHinese(80,0,99);
					OLED_ShowCHinese(96,0,100);
					
				}
				else if (menu_time_count >= 6)
				{
					menu_time_count = -1;
				}
			}

			
			g_rtc_wakeup_event=0;
			menu_time_count++;
		}
		else if (login_mode_flag == 2) // 管理员模式界面
		{
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
					key_sta=2;
					
					if (login_mode_flag == 0) // 未登录，键盘输入的是密码
					{
						g_key_event = 1;
						g_key_time = 0;
						input_PW[count_PW] = key_cur;
						count_PW++;
						
//						printf("input_PW:%s \r\n", input_PW);
//						printf("user_PW:%s \r\n", user_PW);
					}
					else if (login_mode_flag == 2) // 管理员模式，键盘输入选择模式
					{
						if (key_cur == 'B') // 上翻页
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
						else if (key_cur == 'C') // 下翻页
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
						else if (key_cur == 'A') // 确认
						{
							login_mode_flag = Admin_Operation_KEY(page_count);
						}
						else if (key_cur == 'D') // 退出
						{
							OLED_Clear();
							login_mode_flag = 0;
						}
					}
				}
								
			
			}break;
		
			case 2://获取释放的按键
			{
				
				key_cur = get_key_board();	
					
				if(key_cur == 'N')
				{
					key_sta=0;
					
					key_old =  'N';
				
				}
								
			
			}break;

			default:break;
		}	
		
		if (g_key_event == 0)
		{
			memset(input_PW, 0, sizeof(input_PW));
			count_PW = 0;
		}

		// 判断密码是否匹配
		if (strstr(input_PW, user_PW)) // 用户密码匹配，解锁
		{
			
			OLED_ShowCHinese(16,0,45);//输
			OLED_ShowCHinese(32,0,46);//入
			OLED_ShowCHinese(48,0,47);//密
			OLED_ShowCHinese(64,0,48);//码
			OLED_ShowCHinese(80,0,49);//正
			OLED_ShowCHinese(96,0,50);//确
			
			printf("密码正确，解锁\r\n");
			
			if (login_mode_flag == 0)
			{
				Write_Unlock_Record(2);
				g_record_count++;
				printf("当前记录了%d个数据\r\n",g_record_count);
			}
			
			memset(input_PW, 0, sizeof(input_PW));
			count_PW = 0;
			g_key_event = 0;
			g_lock_time = 0;
			menu_time_count = -2;
			login_mode_flag = 1;
			
		}
		else if (strstr(input_PW, root_PW)) // 管理员密码匹配，进入管理员模式
		{
			OLED_Clear();
			memset(input_PW, 0, sizeof(input_PW));
			count_PW = 0;
			g_key_event = 0;
			//g_lock_time = 0;
			menu_time_count = 0;
			login_mode_flag = 2;
		}
		else if (count_PW == 6)
		{
			OLED_ShowCHinese(16,0,45);//输
			OLED_ShowCHinese(32,0,46);//入
			OLED_ShowCHinese(48,0,47);//密
			OLED_ShowCHinese(64,0,48);//码
			OLED_ShowCHinese(80,0,51);//错
			OLED_ShowCHinese(96,0,52);//误
			
			printf("密码错误，请重试\r\n");
			memset(input_PW, 0, sizeof(input_PW));
			count_PW = 0;
			g_key_event = 0;
			menu_time_count = -2;
		}

		if (login_mode_flag == 0)
		{
			// 当有按下感应时，开始刷指纹
			if(PS_Sta)
			{
				press_FR();//刷指纹
			}
			
			// 指纹匹配的话解锁
			if(g_FR_Check_Flag)
			{
				printf("指纹匹配，解锁\r\n");
				
				Write_Unlock_Record(1);
				g_record_count++;
				printf("当前记录了%d个数据\r\n",g_record_count);
				
				g_lock_time = 0;
				g_FR_Check_Flag = 0;
				login_mode_flag = 1;
				
			}
			
			// 卡匹配的话解锁
			if (MFRC522_Check())
			{
				printf("RFID卡匹配，解锁\r\n");
				
				Write_Unlock_Record(3);
				g_record_count++;
				printf("当前记录了%d个数据\r\n",g_record_count);

				g_lock_time = 0;
				login_mode_flag = 1;
				
			}
		}
		

	}
	
}

// 键盘登录管理员操作函数
uint8_t Admin_Operation_KEY(uint8_t mode)
{
	if (mode == 0) // 查指纹
	{
		List_FR();
	}
	else if (mode == 1) // 录指纹
	{
		Add_FR();
	}
	else if (mode == 2) // 删指纹
	{
		Del_FR();
	}
	else if (mode == 3) // 改密码
	{
		Admin_ChangePW();
	}
	else if (mode == 4) // 录入卡
	{
		Add_Card();
	}
	else if (mode == 5) // 删除卡
	{
		Del_Card();
	}
	
	return 2;
}

// 蓝牙连接管理员操作函数
uint8_t Admin_Operation_BLUETOOTH(uint8_t mode)
{
	if (mode == 0) // 查指纹
	{
		List_FR();
	}
	else if (mode == 1) // 录指纹
	{
		Add_FR();
	}
	else if (mode == 2) // 删指纹
	{
		Del_FR();
	}
	else if (mode == 3) // 改密码
	{
		Admin_ChangePW();
	}
	else if (mode == 4) // 录入卡
	{
		Add_Card();
	}
	else if (mode == 5) // 删除卡
	{
		Del_Card();
	}
	
	return 2;
}

// 管理员改密码
void Admin_ChangePW(void)
{
	u8 i = 0;
	u8 buf[5] = {0};
	// 矩阵键盘标志
	uint32_t key_sta=0;
	char key_old=0;
	char key_cur=0;
	
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
	
	while (1)
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
					key_sta=2;
					buf[i] = key_cur;
					i++;
				}
								
			
			}break;
		
			case 2://获取释放的按键
			{
				
				key_cur = get_key_board();	
					
				if(key_cur == 'N')
				{
					key_sta=0;
					
					key_old =  'N';
				
				}
								
			
			}break;

			default:break;
		}

		OLED_ShowString(40,4,(u8 *)buf,16);
		
		if (i == 4) // 密码输入结束
		{
			flash_erase(3);
			buf[i] = '\0';
			strncpy(user_PW, (char*)buf, 5);
			WriteFlashData(FLASH3_ADDR, (uint8_t *)user_PW, 5);
			WriteFlashData(FLASH3_ADDR+5, (uint8_t *)root_PW, 5); 
			
			OLED_ShowCHinese(16,6,90);//更
			OLED_ShowCHinese(32,6,87);//改
			OLED_ShowCHinese(48,6,45);//密
			OLED_ShowCHinese(64,6,46);//码
			OLED_ShowCHinese(80,6,7);//成
			OLED_ShowCHinese(96,6,8);//功
			
			delay_ms(1000);
			break;
		}
		else if (strstr((char*)buf, "*")) // 重输
		{
			i = 0;
			memset(buf, 0, 5);
			OLED_ShowString(40,4,(u8 *)"      ",16);
		}
		else if (strstr((char*)buf, "D")) // 退出
		{
			break;
		}
		
	}

	OLED_Clear();
	
}

// 录入卡
void Add_Card(void)
{
	u8 status;
	char buf[20] = {0};
	
	//MFRC522数据区
	u8  card_pydebuf[2];
	u8  card_numberbuf[5];
	u8  card_key0Abuf[6]={0xff,0xff,0xff,0xff,0xff,0xff};
	u8  card_writebuf[16]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	u8  card_readbuf[18];
	
	OLED_Clear();
	OLED_ShowCHinese(16,0,15);//请
	OLED_ShowCHinese(32,0,92);//刷
	OLED_ShowCHinese(48,0,91);//新
	OLED_ShowCHinese(64,0,88);//卡
	
	while(1)
	{
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
	}
	
	OLED_Clear();
}

// 删除卡
void Del_Card(void)
{
	char buf1[20] = {0};
	char buf2[20] = {0};
	u16 i = 0, j;
	u16 count = Count_Card();
	u8 flag = 1;
	
	// 矩阵键盘标志
	uint32_t key_sta=0;
	char key_old=0;
	char key_cur=0;
	
	OLED_Clear();
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
		return;
	}
	else
	{
		OLED_ShowCHinese(16,0,96);//选
		OLED_ShowCHinese(32,0,97);//择
		OLED_ShowCHinese(48,0,13);//删
		OLED_ShowCHinese(64,0,14);//除
		OLED_ShowCHinese(80,0,88);//卡
		OLED_ShowString(96,0,"ID",16);//ID
	}

	while(flag)
	{
		memset(buf1,0,sizeof(buf1));
		memset(buf2,0,sizeof(buf2));
		
		OLED_ShowString(0,2,(u8 *)"A",16);
		OLED_ShowCHinese(8,2,82);//确
		OLED_ShowCHinese(24,2,83);//认
		OLED_ShowString(40,2,(u8 *)"BC",16);
		OLED_ShowCHinese(56,2,84);//翻
		OLED_ShowCHinese(72,2,85);//页
		OLED_ShowString(88,2,(u8 *)"D",16);
		OLED_ShowCHinese(96,2,57);//退
		OLED_ShowCHinese(112,2,58);//出
		
		ReadFlashData((FLASH5_ADDR+i*11),buf1,11);
		sprintf(buf2,"%c%c %c%c %c%c %c%c %c%c",buf1[0],buf1[1],buf1[2],buf1[3],buf1[4],buf1[5],buf1[6],buf1[7],buf1[8],buf1[9]);
		OLED_ShowString(8,5,(u8 *)buf2,16);
		
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
					key_sta=2;
					
					if (key_cur == 'B')
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
					else if (key_cur == 'C')
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
					else if (key_cur == 'D')
					{
						OLED_Clear();
						return;
					}
					else if (key_cur == 'A')
					{
						flag = 0;
					}
					
				}
								
			
			}break;
		
			case 2://获取释放的按键
			{
				
				key_cur = get_key_board();	
					
				if(key_cur == 'N')
				{
					key_sta=0;
					
					key_old =  'N';
				
				}
								
			
			}break;

			default:break;
		}
		
	}
	
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

uint8_t MFRC522_Check(void)
{
	u8 i,status,card_size;
	
	char buf[20] = {0};
	
	//MFRC522数据区
	u8  card_pydebuf[2];
	u8  card_numberbuf[5];
	u8  card_key0Abuf[6]={0xff,0xff,0xff,0xff,0xff,0xff};
	u8  card_writebuf[16]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	u8  card_readbuf[18];
	
	MFRC522_Initializtion();			//初始化MFRC522
	status=MFRC522_Request(0x52, card_pydebuf);			//寻卡
	//
	if(status==0)		//如果读到卡
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
		
		if (Check_Card((u8*)buf))
		{
			return 1;
		}
		
		BEEP(1);//打开蜂鸣器	
		delay_ms(50);
		BEEP(0);//关闭蜂鸣器
		delay_ms(50);
		BEEP(1);//打开蜂鸣器	
		delay_ms(50);
		BEEP(0);//关闭蜂鸣器
	}
	
	return 0;
}

//录指纹
void Add_FR(void)
{
	u8 i,ensure ,processnum=0;
	u8 j = 0;
	u16 ID;
	char buf[3];
//	OS_ERR  err;
//	OS_MSG_SIZE msg_size;
	
	// 矩阵键盘标志
	uint32_t key_sta=0;
	char key_old=0;
	char key_cur=0;
	
	while(1)
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
			
				// 从键盘输入ID
				while (1)
				{
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
								key_sta=2;
								buf[j] = key_cur;
								j++;
							}
											
						
						}break;
					
						case 2://获取释放的按键
						{
							
							key_cur = get_key_board();	
								
							if(key_cur == 'N')
							{
								key_sta=0;
								
								key_old =  'N';
							
							}	
						
						}break;

						default:break;
					}	
					
					if (strstr(buf, "#") || j == 3) // 输入完毕
					{
						ID = atoi(buf);
						break;
					}
					else if (strstr(buf, "*")) // 重输
					{
						j = 0;
						memset(buf, 0, 3);
						OLED_ShowString(40,4,(u8 *)"      ",16);
					}
					else if (strstr(buf, "D")) // 退出
					{
						return;
					}
					OLED_ShowString(40,4,(u8 *)buf,16);
				}
				
				ensure=PS_StoreChar(CharBuffer1,ID);//储存模板
				
				if(ensure==0x00) 
				{		
					printf("录入指纹成功\r\n");
					
					OLED_Clear();
					OLED_ShowCHinese(0,0,11);//录
					OLED_ShowCHinese(16,0,12);//入
					OLED_ShowCHinese(32,0,2);//指
					OLED_ShowCHinese(48,0,3);//纹
					OLED_ShowCHinese(64,0,7);//成
					OLED_ShowCHinese(80,0,8);//功
					
					PS_ValidTempleteNum(&g_ValidN);//读库指纹个数
					printf("已录入指纹%d\r\n", g_ValidN);
					printf("剩余指纹数%d\r\n",AS608Para.PS_max-g_ValidN);
					OLED_ShowCHinese(0,2,69);//库
					OLED_ShowCHinese(16,2,70);//存
					OLED_ShowCHinese(32,2,2);//指
					OLED_ShowCHinese(48,2,3);//纹
					OLED_ShowCHinese(64,2,71);//个
					OLED_ShowCHinese(80,2,72);//数
					memset(buf,0,3);
					sprintf(buf,"%d",g_ValidN);
					OLED_ShowString(100,2,(u8*)buf,16);
					
					OLED_ShowCHinese(0,4,69);//库
					OLED_ShowCHinese(16,4,70);//存
					OLED_ShowCHinese(32,4,73);//剩
					OLED_ShowCHinese(48,4,74);//余
					OLED_ShowCHinese(64,4,75);//空
					OLED_ShowCHinese(80,4,76);//间
					sprintf(buf,"%d",AS608Para.PS_max-g_ValidN);
					OLED_ShowString(100,4,(u8*)buf,16);
					
					delay_ms(1500);
					OLED_Clear();
					return ;
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
}

//刷指纹
void press_FR(void)
{
//	OS_ERR err;
	SearchResult seach;
	u8 ensure;
	ensure=PS_GetImage();
	if(ensure==0x00)//获取图像成功 
	{	
		BEEP(1);//打开蜂鸣器	
		ensure=PS_GenChar(CharBuffer1);
		if(ensure==0x00) //生成特征成功
		{		
			BEEP(0);//关闭蜂鸣器	
			ensure=PS_HighSpeedSearch(CharBuffer1,0,AS608Para.PS_max,&seach);
			if(ensure==0x00)//搜索成功
			{				
				
				printf("匹配指纹成功\r\n");
				printf("确有此人,ID:%d  匹配得分:%d\r\n",seach.pageID,seach.mathscore);
				//设置指纹识别成功事件标志组0x02 bit1
//				OSFlagPost(&g_flag_grp,0x02,OS_OPT_POST_FLAG_SET,&err);	
				g_FR_Check_Flag = 1;
			}
			else 
			{
				printf("匹配指纹失败:%d\r\n",ensure);
				//设置指纹识别失败事件标志组0x04 Bit2				
//				OSFlagPost(&g_flag_grp,0x04,OS_OPT_POST_FLAG_SET,&err);	
				g_FR_Check_Flag = 0;
			}
		}
		else
		{
			printf("获取图像失败%d\r\n",ensure);
		}
			
		BEEP(0);//关闭蜂鸣器
	 
		delay_ms(1000);
	}
	
		
}

//删除指纹
void Del_FR(void)
{
	u8  ensure;
	u16 ID;
	u16 j = 0;
	u8 buf[3] = {0};
	
	// 矩阵键盘标志
	uint32_t key_sta=0;
	char key_old=0;
	char key_cur=0;
	
	
	printf("\n删除指纹\r\n");
	printf("请输入指纹ID按Enter发送\r\n");
	printf("0=< ID <=299\r\n");
	delay_ms(50);
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
	
	// 从键盘输入ID
	while (1)
	{
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
					key_sta=2;
					buf[j] = key_cur;
					j++;
				}
								
			
			}break;
		
			case 2://获取释放的按键
			{
				
				key_cur = get_key_board();	
					
				if(key_cur == 'N')
				{
					key_sta=0;
					
					key_old =  'N';
				
				}
								
			
			}break;

			default:break;
		}	
		
		if (strstr((char*)buf, "#") || j == 3)
		{
			ID = atoi((char*)buf);
			break;
		}
		OLED_ShowString(40,4,(u8 *)buf,16);
	}
	
	if (ID != 999)
	{
		ensure=PS_DeletChar(ID,1);//删除单个指纹
		OLED_Clear();
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
		OLED_Clear();
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
	delay_ms(3000);
	OLED_Clear();
}

// 读取解锁记录
void List_Unlock_Record(u32 count)
{
	int32_t d;
	u32 i;
	
	printf("\n");
	for (i = 0; i < count; i++)
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
	
	printf("\n共%d个解锁记录读取完毕\r\n\n", count);
}

// 写入解锁记录，mode1指纹，2键盘，3RFID，4蓝牙
void Write_Unlock_Record(u8 mode)
{
	//获取时间
	RTC_GetTime(RTC_Format_BCD,&RTC_TimeStructure);

	RTC_GetDate(RTC_Format_BCD,&RTC_DateStructure);
	
	//解锁闪存
	FLASH_Unlock();
	if (FLASH_ProgramWord(FLASH6_ADDR + (36*g_record_count), g_record_count+1) != FLASH_COMPLETE)
	{
		while(1);
	}
	if (FLASH_ProgramWord(FLASH6_ADDR + (36*g_record_count) + 4, mode) != FLASH_COMPLETE)
	{
		while(1);
	}
	if (FLASH_ProgramWord(FLASH6_ADDR + (36*g_record_count) + 8, RTC_DateStructure.RTC_Year) != FLASH_COMPLETE)
	{
		while(1);
	}
	if (FLASH_ProgramWord(FLASH6_ADDR + (36*g_record_count) + 12, RTC_DateStructure.RTC_Month) != FLASH_COMPLETE)
	{
		while(1);
	}
	if (FLASH_ProgramWord(FLASH6_ADDR + (36*g_record_count) + 16, RTC_DateStructure.RTC_Date) != FLASH_COMPLETE)
	{
		while(1);
	}
	if (FLASH_ProgramWord(FLASH6_ADDR + (36*g_record_count) + 20, RTC_DateStructure.RTC_WeekDay) != FLASH_COMPLETE)
	{
		while(1);
	}
	
	if (FLASH_ProgramWord(FLASH6_ADDR + (36*g_record_count) + 24, RTC_TimeStructure.RTC_Hours) != FLASH_COMPLETE)
	{
		while(1);
	}
	if (FLASH_ProgramWord(FLASH6_ADDR + (36*g_record_count) + 28, RTC_TimeStructure.RTC_Minutes) != FLASH_COMPLETE)
	{
		while(1);
	}
	if (FLASH_ProgramWord(FLASH6_ADDR + (36*g_record_count) + 32, RTC_TimeStructure.RTC_Seconds) != FLASH_COMPLETE)
	{
		while(1);
	}
}

// 计算解锁记录的个数
void Count_Unlock_Record(void)
{
	int32_t d;
	g_record_count = 0;
	d = *((volatile uint32_t *)(FLASH6_ADDR));
	while (d != -1)
	{
		g_record_count++;
		d = *((volatile uint32_t *)(FLASH6_ADDR + (g_record_count * 36)));
	}
	printf("当前记录了%x个数据\r\n", g_record_count);
}


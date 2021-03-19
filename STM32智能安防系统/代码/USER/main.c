/*******************************************
1. 手机蓝牙操作
查询开发板累计运行的时间  		//time?#使用定时器计时
查询当前温湿度状态			//humi?# temp?#
输入6位密码登录系统。		//连接蓝牙时进行判断，pw=123456#
 -密码正确：嘀一声示意；		//
 -密码失败：蜂鸣器长鸣一会；
修改登录密码			//Newpw=456789#，判断如果不为6位则失效并返回提示
可设置超声波模块报警距离		//dis=xx#，判断如果不在20~4000范围内则失效并返回提示
蓝牙连接状态			//判断蓝牙连接标志位
-连接：点亮LED1
-断开：熄灭LED1


// 串口1来显示矩阵键盘的操作信息
2. 矩阵键盘
输入常规6位密码登录系统		//接收到6个字符时进行判断
 -密码正确：嘀一声示意；
 -密码失败：蜂鸣器长鸣一会；
登录系统后
 -按下“A”，可查询当前温度
 -按下“B”，可查询当前湿度
 -按下“C”，可查询当前超声波测距
 	-按下“D”，可查询开发板累计运行时间
 	-按下“*”，可复位系统
 -按下“#”，可修改登录密码（选做）

支持输入10位的虚位密码登录系统（选做）	//接收到10个字符时用strstr判断6个字符
 -密码正确：嘀一声示意；
 -密码失败：蜂鸣器，长鸣一会；

3. 超声波模块
探测距离小于预设的报警距离，LED4急速闪烁


4. 串口1/蓝牙调试助手实时显示操作信息。
********************************************/


#include "sys.h"
#include <stdio.h>
#include <string.h>
#include "delay.h"
#include "usart1.h"
#include "usart3.h"
#include "led.h"
#include "dht11.h"
#include "beep.h"
#include "iwdg.h"
#include "keyboard.h"
#include "tim.h"
#include "sr04.h"

extern volatile uint8_t g_usart1_event;
extern volatile uint8_t g_usart1_buf[128];
extern volatile uint32_t g_usart1_cnt;

extern volatile uint8_t g_usart3_event;
extern volatile uint8_t g_usart3_buf[128];
extern volatile uint32_t g_usart3_cnt;

extern volatile uint32_t g_time;
extern volatile uint32_t dht11_time;

extern volatile int8_t login_flag2;

int main(void )
{
	char *s = NULL; 				// 用于分割接收到的数据
	uint32_t USdis;					// 用于存储分割出来的超声波报警距离
	int32_t dht11_ret; 				// 用于接收温湿度读取函数返回值
	uint8_t dht11_buf[5] = {0};		// 用于存储读取到的温湿度
	char msg[256] = {0};			// 用作发送数据缓冲区
	
	char pw[10] = "123456\0";		// 初始密码
	char pw_key[12] = {0};			// 存储键盘输入的登录密码
	uint8_t p_key = 0;				// 键盘密码数组下标
	
	int8_t login_flag1 = 0;			// 串口1标志位，0未登录，1登录成功
	int8_t login_flag2 = -1;		// 串口3标志位，0未登录，1登录成功，-1未连接	
	
	uint32_t d = 0x1234567;			// sr04获取到的超声波测距
	uint32_t dis = 200;				// 初始超声波报警距离
	int8_t warning = 0;				// 超声波报警标志
	
	// 矩阵键盘标志
	uint32_t key_sta=0;
	char key_old=0;
	char key_cur=0;
	
	// 密码重置时的标志
	uint32_t key_sta1=0;
	char key_old1=0;
	char key_cur1=0;
	uint8_t pwset = 0;				// 0正常状态，1修改密码状态
	
	// 优先级分组
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	
	BEEP_Init();
	LED_Init();
	led3_init();
	sr04_init();

	tim3_init();
	usart3_EXTI4_init();
	
	key_board_init();
	
	
	//初始化串口1的波特率为115200bps
	//注意：如果接收的数据是乱码，要检查PLL。
	usart1_init(115200);
	
	//串口3的波特率为9600bps，因为蓝牙模块默认使用该波特率
	usart3_init(9600);
	
	delay_ms(1000);
	
	//检测是否由独立看门狗导致的复位
	if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) == SET)
	{
		printf("iwdg reset cpu\r\n");
	}
	else
	{
		printf("other reset cpu\r\n");		
	}
	
	//清空标志位
	RCC_ClearFlag();

	iwdg_init();
	
	// 更改蓝牙名字
	usart3_send_str("AT+NAMEYaojunwei\r\n");
	delay_ms(50);
	printf("\n请输入登录密码\r\n");
	
	while(1)
	{
		
		// 判断蓝牙连接是否成功
		if (!PFout(9))
		{
			if (login_flag2 == -1)
			{
				printf("\n蓝牙设备连接成功\r\n");
				delay_ms(3000); // 为了测试时能够看到输入提示的延时，可以去掉
				usart3_send_str("请输入登录密码\r\n");
				login_flag2 = 0;
			}
			
			// 当蓝牙接收完成一次数据时
			if (g_usart3_event)
			{
				// 验证登录密码
				if (!login_flag2)
				if (strstr((char*)g_usart3_buf, pw)) // 密码正确
				{
					printf("%s\r\n", (char*)g_usart3_buf);
					printf("%s\r\n", pw);
					usart3_send_str("密码正确，登录成功\r\n");
					login_flag2 = 1;
					BEEP(1);
					delay_ms(300);
					BEEP(0);
				}
				else // 密码错误
				{
					printf("%s\r\n", (char*)g_usart3_buf);
					printf("%s\r\n", pw);
					usart3_send_str("密码错误，请重新输入\r\n");
					BEEP(1);
					delay_ms(2000);
					BEEP(0);
				}
				
				// 登录成功后的指令
				if (login_flag2)
				{
					if (strstr((char*)g_usart3_buf, "time?")) // 查询开发板累计运行的时间 
					{
						sprintf(msg, "运行时间:%dh,%dm,%ds\r\n", g_time/3600, (g_time%3600)/60, g_time%60);
						usart3_send_str(msg);
					}
					else if (strstr((char*)g_usart3_buf, "newpw=")) // 修改登录密码
					{
						s = strtok((char *)g_usart3_buf, "=");
						s = strtok(NULL, "=");
						
						if (strlen(s) != 7)	// 如果修改后的密码不是6位，则返回错误提示
						{
							sprintf(msg, "密码长度错误\r\n");
							usart3_send_str(msg);
						}
						else
						{
							strncpy(pw, s, 6);
							sprintf(msg, "修改密码成功\r\n");
							usart3_send_str(msg);
							delay_ms(10);
							sprintf(msg, "新的密码：%s\r\n", pw);
							usart3_send_str(msg);
						}
						
					}
					else if (strstr((char*)g_usart3_buf, "temp?") != NULL) // 查询温度
					{
						if (dht11_time >= 10) // 10秒进行一次测量
						{
							delay_ms(100);
							dht11_init();
							delay_ms(100);
							dht11_ret = dht11_read(dht11_buf);
							dht11_time = 0;
						}
						
						if (dht11_ret == 0)
						{
							sprintf(msg, "温度：%d.%d\r\n", dht11_buf[2], dht11_buf[3]);
							usart3_send_str(msg);
						}
						else
						{
							usart3_send_str("失败，请稍后再试\r\n");
							printf("dht11 error code = %d\r\n", dht11_ret);
						}
						
					}
					else if (strstr((char*)g_usart3_buf, "humi?") != NULL) // 查询湿度
					{
						if (dht11_time >= 10)
						{
							delay_ms(100);
							dht11_init();
							delay_ms(100);
							dht11_ret = dht11_read(dht11_buf);
							dht11_time = 0;
						}
						if (dht11_ret == 0)
						{
							sprintf(msg, "湿度：%d.%d\r\n", dht11_buf[0], dht11_buf[1]);
							usart3_send_str(msg);
						}
						else
						{
							usart3_send_str("失败，请稍后再试\r\n");
							printf("dht11 error code = %d\r\n", dht11_ret);
						}
					}	
					else if (strstr((char*)g_usart3_buf, "dis=") != NULL) // 设置超声波报警距离
					{
						s = strtok((char *)g_usart3_buf, "=");
						s = strtok(NULL, "=");
						
						USdis = atoi(s);
						
						if (USdis >= 20 && USdis <= 4000)	// 合法距离
						{
							dis = USdis;
							usart3_send_str("设置距离成功\r\n");
						}
						else // 设置的距离不合法
						{
							usart3_send_str("设置距离不合法\r\n");
						}
					}
					
					memset((void*)msg, 0, sizeof(msg));
				}
				
				g_usart3_cnt = 0;
				g_usart3_event = 0;
				memset((void *)g_usart3_buf, 0, sizeof(g_usart3_buf));
			}
			
		}
		else // 蓝牙断开连接
		{
			if (login_flag2 != -1)
			{
				printf("蓝牙断开连接\r\n");
				login_flag2 = -1;
			}
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
					
					// 按键按下时的提示
					BEEP(1);
					delay_ms(100);
					BEEP(0);
					
					// 判断键盘有登录
					if (!login_flag1) // 还没登录
					{	
						pw_key[p_key] = key_cur;
						p_key++;
						// 判断输入了10个字符
						if (p_key == 10)
						{
							// 判断密码是否匹配
							if (strstr(pw_key, pw)) // 登录成功
							{
								login_flag1 = 1;
								printf("\n密码正确，登录成功\r\n");
								BEEP(1);
								delay_ms(300);
								BEEP(0);
								
								// LED1亮灯
								if (PFout(10))
								{
									PFout(10) = 0;
								}
								
							}
							else // 登录失败
							{
								printf("\n密码错误，登录失败\r\n");
								BEEP(1);
								delay_ms(2000);
								BEEP(0);
							}
							
							// 清空缓冲区
							memset((void*)pw_key, 0, sizeof(pw_key));
							p_key = 0;
						}
					}
					else // 已经登录
					{	
						// 正常状态
						if (!pwset)
						{
							// 判断按下的是什么键
							if (key_cur == 'A') // 查询温度
							{
								if (dht11_time >= 10)
								{
									delay_ms(100);
									dht11_init();
									delay_ms(100);
									dht11_ret = dht11_read(dht11_buf);
									dht11_time = 0;
								}
								if (dht11_ret == 0)
								{
									printf("\n温度：%d.%d\r\n", dht11_buf[2], dht11_buf[3]);
								}
								else
								{
									printf("dht11 error code = %d\r\n", dht11_ret);
								}
							}
							else if (key_cur == 'B') // 查询湿度
							{
								if (dht11_time >= 10)
								{
									delay_ms(100);
									dht11_init();
									delay_ms(100);
									dht11_ret = dht11_read(dht11_buf);
									dht11_time = 0;
								}
								if (dht11_ret == 0)
								{
									printf("\n湿度：%d.%d\r\n", dht11_buf[0], dht11_buf[1]);
								}
								else
								{
									printf("dht11 error code = %d\r\n", dht11_ret);
								}
							}
							else if (key_cur == 'C') // 查询超声波测距
							{
								d = sr04_get_distance();
								printf("\n当前设定的安全范围为%d\r\n", dis);
								printf("当前超声波测距为%d\r\n", d);
								
								if (d < dis)
								{
									printf("进入超声波报警范围\r\n");
								}
								else
								{
									printf("现在在安全范围内\r\n");
								}
								
							}
							else if (key_cur == 'D') // 查询系统运行时间
							{
								printf("\n运行时间:%dh,%dm,%ds\r\n", g_time/3600, (g_time%3600)/60, g_time%60);
							}
							else if (key_cur == '*') // 复位系统
							{
								usart3_send_str("系统复位，请重新连接\r\n");
								printf("系统复位\r\n");
								
								// 重置独立看门狗的计数值
								IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
								IWDG_SetReload(0);
								IWDG_ReloadCounter();
								IWDG_Enable();
							}
							else if (key_cur == '#') // 修改登录密码
							{
								printf("\n请输入修改后的密码\r\n");
								pwset = 1; // 进入修改密码状态
							}							
						}
						else // 修改密码状态
						{
							pw_key[p_key] = key_cur;
							p_key++;
							
							if (p_key == 6) // 新密码输入完成
							{
								sprintf(pw, "%s", pw_key);
								printf("\n修改密码完成\r\n");
								printf("新的密码：%s\r\n", pw);
								pwset = 0;	// 正常状态
								;
							}
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
		
		
		d = sr04_get_distance();
		if (dis > d) // 在警告范围内
		{
			if (!warning)
			{
				tim1_ch4_set_duty(50);
				warning = 1;
			}
			
		}
		else
		{
			if (warning)
			{
				tim1_ch4_set_duty(100);
				warning = 0;
			}
			
		}
	}
	
	
}

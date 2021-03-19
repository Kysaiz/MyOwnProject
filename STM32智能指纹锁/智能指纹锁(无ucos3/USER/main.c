/*******************************************
����ָ����
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
extern volatile uint32_t g_dht11_time;	// dht11��ȡʱ��
extern volatile uint32_t g_lock_time;	// ����ʱ��
extern volatile uint32_t g_key_event;	// ���������¼���־λ��0��ʾδ���룬1��ʾ��������
extern volatile uint32_t g_key_time;	// ��������ʱ�䣬10��δ���������

static uint8_t g_FR_Check_Flag = 0;		// ָ��ƥ���־λ��0��ʾ��ƥ�䣬1��ʾƥ��

static uint16_t admin_mode_menu[] = {86,2,3,	//��ָ��
									11,2,3,		//¼ָ��
									13,2,3,		//ɾָ��
									87,45,46,	//������
									11,12,88,	//¼�뿨
									13,14,88};	//ɾ����

u16 g_ValidN;//ģ������Чָ�Ƹ���
SysPara AS608Para;//ָ��ģ��AS608����
char user_PW[5];		// �洢�û�����
char root_PW[5];		// �洢����Ա����
uint32_t g_record_count = 0;		// ������¼����ֵ
									
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
	char *s = NULL; 				// ���ڷָ���յ�������
	char buf_time[16]={0};
	char buf_date[16]={0};
	char buf_dht11[16]={0};
	uint8_t dht11_buf[5] = {0};		// ���ڴ洢��ȡ������ʪ��
	uint8_t login_mode_flag = 0;	// 0��ʾδ���룬1��ʾ�û�������2��ʾ�������Աģʽ
	uint8_t page_count = 0;			// ����Աģʽ��ҳ�±�
	char input_PW[6] = {0};			// ��ǰ�������뻺����
	uint8_t count_PW = 0;			// ��ǰ��������λ��
	
	int8_t menu_time_count = 0;

	int8_t login_flag2 = -1;		// ����3��־λ��0δ��¼��1��¼�ɹ���-1δ����	
	uint32_t i;						// ѭ���±�
	
	// ������̱�־
	uint32_t key_sta=0;
	char key_old=0;
	char key_cur=0;
	
	AS608Para.PS_max = 10;
	
	//rtc_init(); //����ʱ��
	rtc_reload(); //��ȡ�Ĵ���ʱ��
	
	MFRC522_Initializtion();			//��ʼ��MFRC522
	
	PS_StaGPIO_Init(); // ָ��ģ���ʼ��
	usart2_init(57600);//��ʼ������2,������ָ��ģ��ͨѶ
	
	//��ʼ������1�Ĳ�����Ϊ115200bps
	//ע�⣺������յ����������룬Ҫ���PLL��
	usart1_init(115200);
	
	usart3_EXTI9_5_init();
	
	//����3�Ĳ�����Ϊ9600bps����Ϊ����ģ��Ĭ��ʹ�øò�����
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
	
	printf("\n����ָ����-Ҧ���\r\n");
//	printf("user_PW = %s\r\n", user_PW);
//	printf("root_PW = %s\r\n", root_PW);	
	
	
	// ������������
	usart3_send_str("AT+NAMEkkkk\r");
	delay_ms(50);
	
	Count_Unlock_Record();
	
	while(PS_HandShake(&AS608Addr))//��AS608ģ������
	{
		delay_ms(400);
		OLED_Clear(); 
		//δ��⵽ģ��
		OLED_ShowCHinese(0,0,59);
		OLED_ShowCHinese(18,0,60);
		OLED_ShowCHinese(36,0,61);
		OLED_ShowCHinese(54,0,62);
		OLED_ShowCHinese(72,0,67);
		OLED_ShowCHinese(90,0,68);
		
		delay_ms(800);
		OLED_Clear(); 
		//��������ģ��
		OLED_ShowCHinese(0,0,63);
		OLED_ShowCHinese(18,0,64);
		OLED_ShowCHinese(36,0,65);
		OLED_ShowCHinese(54,0,66);
		OLED_ShowCHinese(72,0,67);
		OLED_ShowCHinese(90,0,68); 
	}
	
	// �����¼����
	
//	//�ɹ�����ģ��
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
		// �ж����������Ƿ�ɹ�
		if (PEin(5))
		{
			// ����3���յ�����
			if (g_usart3_event)
			{
				// ���δ��¼
				if (login_flag2 == 0)
				{
					if (strstr((char*)g_usart3_buf, "pw="));
					{
						s = strtok((char*)g_usart3_buf, "=");
						s = strtok(NULL, "=");
						
						if (strstr((char*)g_usart3_buf, user_PW)) // ��������˽�������
						{
							printf("��������������룬����\r\n");
							usart3_send_str("������ȷ�������ɹ�\r\n");
							
							if (login_mode_flag == 0)
							{
								Write_Unlock_Record(4);
								g_record_count++;
								printf("��ǰ��¼��%d������\r\n",g_record_count);
							}
							
							g_lock_time = 0;
							login_mode_flag = 1;
							
						}
						else if (strstr((char*)g_usart3_buf, root_PW)) // ��������˹���Ա����
						{
							printf("�����������Ա����\r\n");
							usart3_send_str("������ȷ���������Աģʽ\r\n");
							login_flag2 = 1;
							login_mode_flag = 3;
						}
						
					}
				}
				
				// ����ѵ�¼Ϊ����Աģʽ
				if (login_flag2)
				{
					if (strstr((char*)g_usart3_buf, "record")) // ��ѯ������¼
					{
						List_Unlock_Record(g_record_count);
					}
					else if (strstr((char*)g_usart3_buf, "clear")) // �����¼
					{
						printf("���ڲ�������6\r\n");
						flash_erase(6);
						printf("��������6���\r\n");
						Count_Unlock_Record();
					}
					else if (strstr((char*)g_usart3_buf, "newpw=")) // �����½�������
					{
						s = strtok((char*)g_usart3_buf, "=");
						s = strtok(NULL, "=");
						
						if (strlen(s) != 5) // ���Ȳ���
						{
							printf("�����볤�Ȳ���\r\n");
						}
						else
						{
							strncpy(user_PW, s, 4);
							user_PW[4] = '\0';
							flash_erase(3);
							WriteFlashData(FLASH3_ADDR, (uint8_t *)user_PW, 5);
							WriteFlashData(FLASH3_ADDR+5, (uint8_t *)root_PW, 5); 
							
							OLED_ShowCHinese(16,6,90);//��
							OLED_ShowCHinese(32,6,87);//��
							OLED_ShowCHinese(48,6,45);//��
							OLED_ShowCHinese(64,6,46);//��
							OLED_ShowCHinese(80,6,7);//��
							OLED_ShowCHinese(96,6,8);//��
							
							printf("������������ɹ�\r\n");
						}
					}
					else if (strstr((char*)g_usart3_buf, "unlock")) // ����
					{
						g_lock_time = 0;
						
						printf("��������Ա�����ɹ�\r\n");
						
						Write_Unlock_Record(4);
						g_record_count++;
						printf("��ǰ��¼��%d������\r\n",g_record_count);
						
						
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
		else // �����Ͽ�����
		{
			if (login_flag2 != -1)
			{
				printf("�����Ͽ�����\r\n");
				login_flag2 = -1;
			}
		}
		
		// ����1�޸����ݣ�ģ������
		if (g_usart1_event)
		{
			if (strstr((char*)g_usart1_buf, "record")) // ��ѯ������¼
			{
				List_Unlock_Record(g_record_count);
			}
			else if (strstr((char*)g_usart1_buf, "clear")) // �����¼
			{
				printf("���ڲ�������6\r\n");
				flash_erase(6);
				printf("��������6���\r\n");
				Count_Unlock_Record();
			}
			else if (strstr((char*)g_usart1_buf, "newpw=")) // �����½�������
			{
				s = strtok((char*)g_usart1_buf, "=");
				s = strtok(NULL, "=");
				
				if (strlen(s) != 5) // ���Ȳ���
				{
					printf("�����볤�Ȳ���\r\n");
				}
				else
				{
					strncpy(user_PW, s, 4);
					user_PW[4] = '\0';
					flash_erase(3);
					WriteFlashData(FLASH3_ADDR, (uint8_t *)user_PW, 5);
					WriteFlashData(FLASH3_ADDR+5, (uint8_t *)root_PW, 5); 
					
					OLED_ShowCHinese(16,6,90);//��
					OLED_ShowCHinese(32,6,87);//��
					OLED_ShowCHinese(48,6,45);//��
					OLED_ShowCHinese(64,6,46);//��
					OLED_ShowCHinese(80,6,7);//��
					OLED_ShowCHinese(96,6,8);//��
					
					printf("������������ɹ�\r\n");
				}
			}
			else if (strstr((char*)g_usart1_buf, "unlock")) // ����
			{
				g_lock_time = 0;
				
				printf("��������Ա�����ɹ�\r\n");
				
				Write_Unlock_Record(4);
				g_record_count++;
				printf("��ǰ��¼��%d������\r\n",g_record_count);
				
				
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
		
		// ÿ�����һ��
		if(g_rtc_wakeup_event && (login_mode_flag == 0 || login_mode_flag == 1))
		{
			//��ȡʱ��
			RTC_GetTime(RTC_Format_BCD,&RTC_TimeStructure);
			sprintf(buf_time,"%02x:%02x:%02x",RTC_TimeStructure.RTC_Hours,RTC_TimeStructure.RTC_Minutes,RTC_TimeStructure.RTC_Seconds);
			
			RTC_GetDate(RTC_Format_BCD,&RTC_DateStructure);
			sprintf(buf_date,"20%02x/%02x/%02xWeek%x",RTC_DateStructure.RTC_Year,RTC_DateStructure.RTC_Month,RTC_DateStructure.RTC_Date,RTC_DateStructure.RTC_WeekDay);
			
			// 15���Զ�����
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
				printf("��ʱ����\r\n");
			}
			
			if (g_key_event == 0)
			{
				if (menu_time_count == 0)
				{
					OLED_ShowString(8,0,(u8 *)"  ",16);
					OLED_ShowCHinese(24,0,0);//��
					OLED_ShowCHinese(40,0,1);//��
					OLED_ShowCHinese(56,0,2);//ָ
					OLED_ShowCHinese(72,0,3);//��
					OLED_ShowCHinese(88,0,4);//��
					OLED_ShowString(104,0,(u8 *)" ",16);
				}
				else if (menu_time_count == 3)
				{
					// ����
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
		else if (login_mode_flag == 2) // ����Աģʽ����
		{
			OLED_ShowCHinese(24,0,77);//��
			OLED_ShowCHinese(40,0,78);//��
			OLED_ShowCHinese(56,0,79);//Ա
			OLED_ShowCHinese(72,0,80);//ģ
			OLED_ShowCHinese(88,0,81);//ʽ
			
			OLED_ShowString(0,2,(u8 *)"A",16);
			OLED_ShowCHinese(8,2,82);//ȷ
			OLED_ShowCHinese(24,2,83);//��
			OLED_ShowString(40,2,(u8 *)"BC",16);
			OLED_ShowCHinese(56,2,84);//��
			OLED_ShowCHinese(72,2,85);//ҳ
			OLED_ShowString(88,2,(u8 *)"D",16);
			OLED_ShowCHinese(96,2,57);//��
			OLED_ShowCHinese(112,2,58);//��
			
			OLED_ShowCHinese(40,5,admin_mode_menu[page_count*3]);
			OLED_ShowCHinese(56,5,admin_mode_menu[page_count*3+1]);
			OLED_ShowCHinese(72,5,admin_mode_menu[page_count*3+2]);
		}
		
		/* ʹ��״̬��˼��õ�������״̬ */
		switch(key_sta)
		{
			case 0://��ȡ���µİ���
			{
				
				key_cur = get_key_board();	

				if(key_cur != 'N')
				{
					key_old = key_cur;
					key_sta=1;
				}
					
			
			}break;
			
			
			case 1://ȷ�ϰ��µİ���
			{
				key_cur = get_key_board();	
					
				if((key_cur != 'N') && (key_cur == key_old))
				{
					key_sta=2;
					
					if (login_mode_flag == 0) // δ��¼�����������������
					{
						g_key_event = 1;
						g_key_time = 0;
						input_PW[count_PW] = key_cur;
						count_PW++;
						
//						printf("input_PW:%s \r\n", input_PW);
//						printf("user_PW:%s \r\n", user_PW);
					}
					else if (login_mode_flag == 2) // ����Աģʽ����������ѡ��ģʽ
					{
						if (key_cur == 'B') // �Ϸ�ҳ
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
						else if (key_cur == 'C') // �·�ҳ
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
						else if (key_cur == 'A') // ȷ��
						{
							login_mode_flag = Admin_Operation_KEY(page_count);
						}
						else if (key_cur == 'D') // �˳�
						{
							OLED_Clear();
							login_mode_flag = 0;
						}
					}
				}
								
			
			}break;
		
			case 2://��ȡ�ͷŵİ���
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

		// �ж������Ƿ�ƥ��
		if (strstr(input_PW, user_PW)) // �û�����ƥ�䣬����
		{
			
			OLED_ShowCHinese(16,0,45);//��
			OLED_ShowCHinese(32,0,46);//��
			OLED_ShowCHinese(48,0,47);//��
			OLED_ShowCHinese(64,0,48);//��
			OLED_ShowCHinese(80,0,49);//��
			OLED_ShowCHinese(96,0,50);//ȷ
			
			printf("������ȷ������\r\n");
			
			if (login_mode_flag == 0)
			{
				Write_Unlock_Record(2);
				g_record_count++;
				printf("��ǰ��¼��%d������\r\n",g_record_count);
			}
			
			memset(input_PW, 0, sizeof(input_PW));
			count_PW = 0;
			g_key_event = 0;
			g_lock_time = 0;
			menu_time_count = -2;
			login_mode_flag = 1;
			
		}
		else if (strstr(input_PW, root_PW)) // ����Ա����ƥ�䣬�������Աģʽ
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
			OLED_ShowCHinese(16,0,45);//��
			OLED_ShowCHinese(32,0,46);//��
			OLED_ShowCHinese(48,0,47);//��
			OLED_ShowCHinese(64,0,48);//��
			OLED_ShowCHinese(80,0,51);//��
			OLED_ShowCHinese(96,0,52);//��
			
			printf("�������������\r\n");
			memset(input_PW, 0, sizeof(input_PW));
			count_PW = 0;
			g_key_event = 0;
			menu_time_count = -2;
		}

		if (login_mode_flag == 0)
		{
			// ���а��¸�Ӧʱ����ʼˢָ��
			if(PS_Sta)
			{
				press_FR();//ˢָ��
			}
			
			// ָ��ƥ��Ļ�����
			if(g_FR_Check_Flag)
			{
				printf("ָ��ƥ�䣬����\r\n");
				
				Write_Unlock_Record(1);
				g_record_count++;
				printf("��ǰ��¼��%d������\r\n",g_record_count);
				
				g_lock_time = 0;
				g_FR_Check_Flag = 0;
				login_mode_flag = 1;
				
			}
			
			// ��ƥ��Ļ�����
			if (MFRC522_Check())
			{
				printf("RFID��ƥ�䣬����\r\n");
				
				Write_Unlock_Record(3);
				g_record_count++;
				printf("��ǰ��¼��%d������\r\n",g_record_count);

				g_lock_time = 0;
				login_mode_flag = 1;
				
			}
		}
		

	}
	
}

// ���̵�¼����Ա��������
uint8_t Admin_Operation_KEY(uint8_t mode)
{
	if (mode == 0) // ��ָ��
	{
		List_FR();
	}
	else if (mode == 1) // ¼ָ��
	{
		Add_FR();
	}
	else if (mode == 2) // ɾָ��
	{
		Del_FR();
	}
	else if (mode == 3) // ������
	{
		Admin_ChangePW();
	}
	else if (mode == 4) // ¼�뿨
	{
		Add_Card();
	}
	else if (mode == 5) // ɾ����
	{
		Del_Card();
	}
	
	return 2;
}

// �������ӹ���Ա��������
uint8_t Admin_Operation_BLUETOOTH(uint8_t mode)
{
	if (mode == 0) // ��ָ��
	{
		List_FR();
	}
	else if (mode == 1) // ¼ָ��
	{
		Add_FR();
	}
	else if (mode == 2) // ɾָ��
	{
		Del_FR();
	}
	else if (mode == 3) // ������
	{
		Admin_ChangePW();
	}
	else if (mode == 4) // ¼�뿨
	{
		Add_Card();
	}
	else if (mode == 5) // ɾ����
	{
		Del_Card();
	}
	
	return 2;
}

// ����Ա������
void Admin_ChangePW(void)
{
	u8 i = 0;
	u8 buf[5] = {0};
	// ������̱�־
	uint32_t key_sta=0;
	char key_old=0;
	char key_cur=0;
	
	OLED_Clear();
	OLED_ShowCHinese(16,0,90);//��
	OLED_ShowCHinese(32,0,87);//��
	OLED_ShowCHinese(48,0,5);//��
	OLED_ShowCHinese(64,0,6);//��
	OLED_ShowCHinese(80,0,45);//��
	OLED_ShowCHinese(96,0,46);//��
	
	OLED_ShowString(8,2,(u8 *)"*",16);
	OLED_ShowCHinese(16,2,89);//��
	OLED_ShowCHinese(32,2,47);//��
	
	OLED_ShowString(56,2,(u8 *)"D",16);
	OLED_ShowCHinese(64,2,57);//��
	OLED_ShowCHinese(80,2,58);//��
	
	while (1)
	{
		/* ʹ��״̬��˼��õ�������״̬ */
		switch(key_sta)
		{
			case 0://��ȡ���µİ���
			{
				
				key_cur = get_key_board();	

				if(key_cur != 'N')
				{
					key_old = key_cur;
					key_sta=1;
				}
					
			
			}break;
			
			
			case 1://ȷ�ϰ��µİ���
			{
				key_cur = get_key_board();	
					
				if((key_cur != 'N') && (key_cur == key_old))
				{
					key_sta=2;
					buf[i] = key_cur;
					i++;
				}
								
			
			}break;
		
			case 2://��ȡ�ͷŵİ���
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
		
		if (i == 4) // �����������
		{
			flash_erase(3);
			buf[i] = '\0';
			strncpy(user_PW, (char*)buf, 5);
			WriteFlashData(FLASH3_ADDR, (uint8_t *)user_PW, 5);
			WriteFlashData(FLASH3_ADDR+5, (uint8_t *)root_PW, 5); 
			
			OLED_ShowCHinese(16,6,90);//��
			OLED_ShowCHinese(32,6,87);//��
			OLED_ShowCHinese(48,6,45);//��
			OLED_ShowCHinese(64,6,46);//��
			OLED_ShowCHinese(80,6,7);//��
			OLED_ShowCHinese(96,6,8);//��
			
			delay_ms(1000);
			break;
		}
		else if (strstr((char*)buf, "*")) // ����
		{
			i = 0;
			memset(buf, 0, 5);
			OLED_ShowString(40,4,(u8 *)"      ",16);
		}
		else if (strstr((char*)buf, "D")) // �˳�
		{
			break;
		}
		
	}

	OLED_Clear();
	
}

// ¼�뿨
void Add_Card(void)
{
	u8 status;
	char buf[20] = {0};
	
	//MFRC522������
	u8  card_pydebuf[2];
	u8  card_numberbuf[5];
	u8  card_key0Abuf[6]={0xff,0xff,0xff,0xff,0xff,0xff};
	u8  card_writebuf[16]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	u8  card_readbuf[18];
	
	OLED_Clear();
	OLED_ShowCHinese(16,0,15);//��
	OLED_ShowCHinese(32,0,92);//ˢ
	OLED_ShowCHinese(48,0,91);//��
	OLED_ShowCHinese(64,0,88);//��
	
	while(1)
	{
		MFRC522_Initializtion();			//��ʼ��MFRC522
		status=MFRC522_Request(0x52, card_pydebuf);			//Ѱ��
		
		if (status == 0) // ���������
		{
			memset(card_readbuf, 0, 16);
			status=MFRC522_Anticoll(card_numberbuf);			//��ײ����			
			MFRC522_SelectTag(card_numberbuf);	//ѡ��
			status=MFRC522_Auth(0x60, 4, card_key0Abuf, card_numberbuf);	//�鿨
			status=MFRC522_Write(4, card_writebuf);				//д����д��ҪС�ģ��ر��Ǹ����Ŀ�3��
			status=MFRC522_Read(4, card_readbuf);					//����
			
			OLED_Clear();
			OLED_ShowString(56,0,"ID",16);//ID
			sprintf(buf,"%02x %02x %02x %02x %02x",card_numberbuf[0],card_numberbuf[1],card_numberbuf[2],card_numberbuf[3],card_numberbuf[4]);
			OLED_ShowString(8,2,(u8 *)buf,16);//��ID
			
			memset(buf, 0, 20);
			sprintf(buf,"%02x%02x%02x%02x%02x#",card_numberbuf[0],card_numberbuf[1],card_numberbuf[2],card_numberbuf[3],card_numberbuf[4]);
		
			if (Check_Card((u8*)buf)) // ���������
			{
				OLED_ShowCHinese(24,4,93);//��
				OLED_ShowCHinese(40,4,88);//��
				OLED_ShowCHinese(56,4,94);//��
				OLED_ShowCHinese(72,4,95);//��
				OLED_ShowCHinese(24,6,11);//¼
				OLED_ShowCHinese(40,6,12);//��
				OLED_ShowCHinese(56,6,9);//ʧ
				OLED_ShowCHinese(72,6,10);//��
				delay_ms(2000);
				break;
			}
			else // ����������ڣ���¼��
			{
				WriteFlashData((FLASH5_ADDR+Count_Card()*11), (u8*)buf, 11);
				OLED_ShowCHinese(24,4,11);//¼
				OLED_ShowCHinese(40,4,12);//��
				OLED_ShowCHinese(56,4,7);//��
				OLED_ShowCHinese(72,4,8);//��
				delay_ms(2000);
				break;
			}
		}
	}
	
	OLED_Clear();
}

// ɾ����
void Del_Card(void)
{
	char buf1[20] = {0};
	char buf2[20] = {0};
	u16 i = 0, j;
	u16 count = Count_Card();
	u8 flag = 1;
	
	// ������̱�־
	uint32_t key_sta=0;
	char key_old=0;
	char key_cur=0;
	
	OLED_Clear();
	if (count == 0)
	{
		OLED_ShowCHinese(16,0,59);//δ
		OLED_ShowCHinese(32,0,60);//��
		OLED_ShowCHinese(48,0,61);//��
		OLED_ShowCHinese(64,0,62);//��
		OLED_ShowCHinese(80,0,88);//��
		OLED_ShowString(96,0,"ID",16);//ID
		
		delay_ms(1000);
		OLED_Clear();
		return;
	}
	else
	{
		OLED_ShowCHinese(16,0,96);//ѡ
		OLED_ShowCHinese(32,0,97);//��
		OLED_ShowCHinese(48,0,13);//ɾ
		OLED_ShowCHinese(64,0,14);//��
		OLED_ShowCHinese(80,0,88);//��
		OLED_ShowString(96,0,"ID",16);//ID
	}

	while(flag)
	{
		memset(buf1,0,sizeof(buf1));
		memset(buf2,0,sizeof(buf2));
		
		OLED_ShowString(0,2,(u8 *)"A",16);
		OLED_ShowCHinese(8,2,82);//ȷ
		OLED_ShowCHinese(24,2,83);//��
		OLED_ShowString(40,2,(u8 *)"BC",16);
		OLED_ShowCHinese(56,2,84);//��
		OLED_ShowCHinese(72,2,85);//ҳ
		OLED_ShowString(88,2,(u8 *)"D",16);
		OLED_ShowCHinese(96,2,57);//��
		OLED_ShowCHinese(112,2,58);//��
		
		ReadFlashData((FLASH5_ADDR+i*11),buf1,11);
		sprintf(buf2,"%c%c %c%c %c%c %c%c %c%c",buf1[0],buf1[1],buf1[2],buf1[3],buf1[4],buf1[5],buf1[6],buf1[7],buf1[8],buf1[9]);
		OLED_ShowString(8,5,(u8 *)buf2,16);
		
		/* ʹ��״̬��˼��õ�������״̬ */
		switch(key_sta)
		{
			case 0://��ȡ���µİ���
			{
				
				key_cur = get_key_board();	

				if(key_cur != 'N')
				{
					key_old = key_cur;
					key_sta=1;
				}
					
			
			}break;
			
			
			case 1://ȷ�ϰ��µİ���
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
		
			case 2://��ȡ�ͷŵİ���
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
	
	// ������5������������ת�浽����4
	printf("���ڲ�������4\r\n");
	flash_erase(4);
	printf("��������4���\r\n");
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
	
	// ������4�����ݴ������5
	printf("���ڲ�������5\r\n");
	flash_erase(5);
	printf("��������5���\r\n");
	for (j = 0; j < count-1; j++)
	{
		memset(buf1, 0, 11);
		ReadFlashData((FLASH4_ADDR+j*11), buf1, 11);
		WriteFlashData((FLASH5_ADDR+j*11), (u8*)buf1, 11);
	}
	
	printf("ɾ�����ɹ�\r\n");
	
	OLED_ShowString(8,5,"               ",16);
	OLED_ShowCHinese(32,5,13);//ɾ
	OLED_ShowCHinese(48,5,14);//��
	OLED_ShowCHinese(64,5,7);//��
	OLED_ShowCHinese(80,5,8);//��
	
	delay_ms(2000);
	OLED_Clear();
}

// ��ѯFLASH5�д洢�Ŀ�������
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

// ��֤��
uint8_t Check_Card(u8* card)
{
	u16 i;
	char buf[11];
	
	for (i = 0; i < Count_Card(); i++)
	{
		ReadFlashData((FLASH5_ADDR+i*11), (char*)buf, 11);
		if (strstr((char*)card, buf))
		{
			return 1;	// ��ʾ���ڸÿ�
		}
	}
	return 0;// ��ʾ�ÿ�������
}

uint8_t MFRC522_Check(void)
{
	u8 i,status,card_size;
	
	char buf[20] = {0};
	
	//MFRC522������
	u8  card_pydebuf[2];
	u8  card_numberbuf[5];
	u8  card_key0Abuf[6]={0xff,0xff,0xff,0xff,0xff,0xff};
	u8  card_writebuf[16]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	u8  card_readbuf[18];
	
	MFRC522_Initializtion();			//��ʼ��MFRC522
	status=MFRC522_Request(0x52, card_pydebuf);			//Ѱ��
	//
	if(status==0)		//���������
	{
		memset(card_readbuf, 0, 16);
		status=MFRC522_Anticoll(card_numberbuf);			//��ײ����			
		card_size=MFRC522_SelectTag(card_numberbuf);	//ѡ��
		status=MFRC522_Auth(0x60, 4, card_key0Abuf, card_numberbuf);	//�鿨
		status=MFRC522_Write(4, card_writebuf);				//д����д��ҪС�ģ��ر��Ǹ����Ŀ�3��
		status=MFRC522_Read(4, card_readbuf);					//����
		//MFRC522_Halt();															//ʹ����������״̬
		printf("card_pydebuf:%02X %02X \r\n",card_pydebuf[0],card_pydebuf[1]);
			
		//�����к��ԣ����һ�ֽ�Ϊ����У����
		printf("card_numberbuf:%02X %02X %02X %02X %02X \r\n",card_numberbuf[0],card_numberbuf[1],card_numberbuf[2],card_numberbuf[3],card_numberbuf[4]);
		//��������ʾ����λΪKbits
		printf("card_size:%d Kbits\r\n",card_size);
		//����״̬��ʾ������Ϊ0
		printf("status=%d\r\n",status);
		//��һ�����������ʾ
		printf("read:");
		for(i=0;i<16;i++)		//��������ʾ
		{
			printf("%d ",card_readbuf[i]);

		}
		printf("\r\n\n");
		
		sprintf(buf,"%02x%02x%02x%02x%02x#",card_numberbuf[0],card_numberbuf[1],card_numberbuf[2],card_numberbuf[3],card_numberbuf[4]);
		
		if (Check_Card((u8*)buf))
		{
			return 1;
		}
		
		BEEP(1);//�򿪷�����	
		delay_ms(50);
		BEEP(0);//�رշ�����
		delay_ms(50);
		BEEP(1);//�򿪷�����	
		delay_ms(50);
		BEEP(0);//�رշ�����
	}
	
	return 0;
}

//¼ָ��
void Add_FR(void)
{
	u8 i,ensure ,processnum=0;
	u8 j = 0;
	u16 ID;
	char buf[3];
//	OS_ERR  err;
//	OS_MSG_SIZE msg_size;
	
	// ������̱�־
	uint32_t key_sta=0;
	char key_old=0;
	char key_cur=0;
	
	while(1)
	{
		switch (processnum)
		{
			case 0:
				i++;
				printf("�밴ָ��\r\n");
				OLED_Clear();
				OLED_ShowCHinese(27,0,15);//��
				OLED_ShowCHinese(45,0,17);//��
				OLED_ShowCHinese(63,0,2);//ָ
				OLED_ShowCHinese(81,0,3);//��
			
				ensure=PS_GetImage();
				if(ensure==0x00) 
				{
					BEEP(1);
					ensure=PS_GenChar(CharBuffer1);//��������
					BEEP(0);
					if(ensure==0x00)
					{
						printf("ָ������\r\n");
						OLED_ShowCHinese(27,2,2);//ָ
						OLED_ShowCHinese(45,2,3);//��
						OLED_ShowCHinese(63,2,29);//��
						OLED_ShowCHinese(81,2,30);//��
						
						i=0;
						processnum=1;//�����ڶ���						
					}else printf("error:%d\r\n",ensure);				
				}else printf("error:%d\r\n",ensure);						
				break;
			
			case 1:
				i++;
				printf("���ٰ�һ��ָ��\r\n");
				OLED_Clear();
				OLED_ShowCHinese(16,0,15);//��
				OLED_ShowCHinese(32,0,16);//��
				OLED_ShowCHinese(48,0,19);//��
				OLED_ShowCHinese(64,0,17);//��
				OLED_ShowCHinese(80,0,2);//ָ
				OLED_ShowCHinese(96,0,3);//��		
			
				ensure=PS_GetImage();
				if(ensure==0x00) 
				{
					BEEP(1);
					ensure=PS_GenChar(CharBuffer2);//��������
					BEEP(0);
					if(ensure==0x00)
					{
						printf("ָ������\r\n");
						OLED_ShowCHinese(27,2,2);//ָ
						OLED_ShowCHinese(45,2,3);//��
						OLED_ShowCHinese(63,2,29);//��
						OLED_ShowCHinese(81,2,30);//��
						
						i=0;
						processnum=2;//����������
					}else printf("error:%d\r\n",ensure);	
				}else printf("error:%d\r\n",ensure);		
				break;

			case 2:
				printf("�Ա�����ָ��\r\n");
				OLED_Clear();
				OLED_ShowCHinese(16,0,21);//��
				OLED_ShowCHinese(32,0,22);//��
				OLED_ShowCHinese(48,0,23);//��
				OLED_ShowCHinese(64,0,24);//��
				OLED_ShowCHinese(80,0,2);//ָ
				OLED_ShowCHinese(96,0,3);//��	
			
				ensure=PS_Match();
				if(ensure==0x00) 
				{
					OLED_ShowCHinese(27,2,21);//��
					OLED_ShowCHinese(45,2,22);//��
					OLED_ShowCHinese(63,2,7);//��
					OLED_ShowCHinese(81,2,8);//��
					processnum=3;//�������Ĳ�
				}
				else 
				{
					OLED_ShowCHinese(27,2,21);//��
					OLED_ShowCHinese(45,2,22);//��
					OLED_ShowCHinese(63,2,9);//ʧ
					OLED_ShowCHinese(81,2,10);//��
					
					printf("error:%d\r\n",ensure);
					i=0;
					processnum=0;//���ص�һ��		
				}
				delay_ms(1200);
				break;

			case 3:
				printf("����ָ��ģ��\r\n");
				OLED_ShowCHinese(16,4,25);//��
				OLED_ShowCHinese(32,4,26);//��
				OLED_ShowCHinese(48,4,2);//ָ
				OLED_ShowCHinese(64,4,3);//��
				OLED_ShowCHinese(80,4,27);//ģ
				OLED_ShowCHinese(96,4,28);//��	
			
				ensure=PS_RegModel();
				if(ensure==0x00) 
				{
					OLED_ShowCHinese(27,6,25);//��
					OLED_ShowCHinese(45,6,26);//��
					OLED_ShowCHinese(63,6,7);//��
					OLED_ShowCHinese(81,6,8);//��
					processnum=4;//�������岽
				}
				else
				{
					OLED_ShowCHinese(27,6,25);//��
					OLED_ShowCHinese(45,6,26);//��
					OLED_ShowCHinese(63,6,9);//��
					OLED_ShowCHinese(81,6,10);//��
					
					processnum=0;
					printf("error:%d\r\n",ensure);
				}
				delay_ms(1200);
				break;
				
			case 4:	
				printf("����ָ��ID\r\n");
				printf("��'#'����\r\n");
				OLED_Clear();
				OLED_ShowCHinese(0,0,31);//��
				OLED_ShowCHinese(16,0,32);//��
				OLED_ShowCHinese(32,0,2);//ָ
				OLED_ShowCHinese(48,0,3);//��
				OLED_ShowString(64,0,(u8 *)"ID",16);
				OLED_ShowString(80,0,(u8 *)"0->",16);
				OLED_ShowString(104,0,(u8 *)"299",16);
			
				OLED_ShowCHinese(27,2,17);//��
				OLED_ShowString(49,2,(u8 *)"#",16);
				OLED_ShowCHinese(63,2,34);//��
				OLED_ShowCHinese(81,2,35);//��			
				
				OLED_ShowString(8,6,(u8 *)"*",16);
				OLED_ShowCHinese(16,6,89);//��
				OLED_ShowCHinese(32,6,31);//��			
			
				OLED_ShowString(56,6,(u8 *)"D",16);
				OLED_ShowCHinese(64,6,57);//��
				OLED_ShowCHinese(80,6,58);//��
			
				// �Ӽ�������ID
				while (1)
				{
					switch(key_sta)
					{
						case 0://��ȡ���µİ���
						{
							
							key_cur = get_key_board();	

							if(key_cur != 'N')
							{
								key_old = key_cur;
								key_sta=1;
							}
								
						
						}break;
						
						
						case 1://ȷ�ϰ��µİ���
						{
							key_cur = get_key_board();	
								
							if((key_cur != 'N') && (key_cur == key_old))
							{
								key_sta=2;
								buf[j] = key_cur;
								j++;
							}
											
						
						}break;
					
						case 2://��ȡ�ͷŵİ���
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
					
					if (strstr(buf, "#") || j == 3) // �������
					{
						ID = atoi(buf);
						break;
					}
					else if (strstr(buf, "*")) // ����
					{
						j = 0;
						memset(buf, 0, 3);
						OLED_ShowString(40,4,(u8 *)"      ",16);
					}
					else if (strstr(buf, "D")) // �˳�
					{
						return;
					}
					OLED_ShowString(40,4,(u8 *)buf,16);
				}
				
				ensure=PS_StoreChar(CharBuffer1,ID);//����ģ��
				
				if(ensure==0x00) 
				{		
					printf("¼��ָ�Ƴɹ�\r\n");
					
					OLED_Clear();
					OLED_ShowCHinese(0,0,11);//¼
					OLED_ShowCHinese(16,0,12);//��
					OLED_ShowCHinese(32,0,2);//ָ
					OLED_ShowCHinese(48,0,3);//��
					OLED_ShowCHinese(64,0,7);//��
					OLED_ShowCHinese(80,0,8);//��
					
					PS_ValidTempleteNum(&g_ValidN);//����ָ�Ƹ���
					printf("��¼��ָ��%d\r\n", g_ValidN);
					printf("ʣ��ָ����%d\r\n",AS608Para.PS_max-g_ValidN);
					OLED_ShowCHinese(0,2,69);//��
					OLED_ShowCHinese(16,2,70);//��
					OLED_ShowCHinese(32,2,2);//ָ
					OLED_ShowCHinese(48,2,3);//��
					OLED_ShowCHinese(64,2,71);//��
					OLED_ShowCHinese(80,2,72);//��
					memset(buf,0,3);
					sprintf(buf,"%d",g_ValidN);
					OLED_ShowString(100,2,(u8*)buf,16);
					
					OLED_ShowCHinese(0,4,69);//��
					OLED_ShowCHinese(16,4,70);//��
					OLED_ShowCHinese(32,4,73);//ʣ
					OLED_ShowCHinese(48,4,74);//��
					OLED_ShowCHinese(64,4,75);//��
					OLED_ShowCHinese(80,4,76);//��
					sprintf(buf,"%d",AS608Para.PS_max-g_ValidN);
					OLED_ShowString(100,4,(u8*)buf,16);
					
					delay_ms(1500);
					OLED_Clear();
					return ;
				}
				else // ���ص�һ��
				{
					processnum=0;
					printf("error:%d\r\n",ensure);
					OLED_ShowString(40,4,(u8 *)"ID",16);
					OLED_ShowCHinese(56,4,51);//��
					OLED_ShowCHinese(72,4,52);//��
					delay_ms(1000);
				}					
				break;				
		}
		delay_ms(600);
		if(i==5)//����5��û�а���ָ���˳�
		{
			printf("����5��û�а���ָ\r\n");
			break;	
		}				
	}
	OLED_Clear();
}

//ˢָ��
void press_FR(void)
{
//	OS_ERR err;
	SearchResult seach;
	u8 ensure;
	ensure=PS_GetImage();
	if(ensure==0x00)//��ȡͼ��ɹ� 
	{	
		BEEP(1);//�򿪷�����	
		ensure=PS_GenChar(CharBuffer1);
		if(ensure==0x00) //���������ɹ�
		{		
			BEEP(0);//�رշ�����	
			ensure=PS_HighSpeedSearch(CharBuffer1,0,AS608Para.PS_max,&seach);
			if(ensure==0x00)//�����ɹ�
			{				
				
				printf("ƥ��ָ�Ƴɹ�\r\n");
				printf("ȷ�д���,ID:%d  ƥ��÷�:%d\r\n",seach.pageID,seach.mathscore);
				//����ָ��ʶ��ɹ��¼���־��0x02 bit1
//				OSFlagPost(&g_flag_grp,0x02,OS_OPT_POST_FLAG_SET,&err);	
				g_FR_Check_Flag = 1;
			}
			else 
			{
				printf("ƥ��ָ��ʧ��:%d\r\n",ensure);
				//����ָ��ʶ��ʧ���¼���־��0x04 Bit2				
//				OSFlagPost(&g_flag_grp,0x04,OS_OPT_POST_FLAG_SET,&err);	
				g_FR_Check_Flag = 0;
			}
		}
		else
		{
			printf("��ȡͼ��ʧ��%d\r\n",ensure);
		}
			
		BEEP(0);//�رշ�����
	 
		delay_ms(1000);
	}
	
		
}

//ɾ��ָ��
void Del_FR(void)
{
	u8  ensure;
	u16 ID;
	u16 j = 0;
	u8 buf[3] = {0};
	
	// ������̱�־
	uint32_t key_sta=0;
	char key_old=0;
	char key_cur=0;
	
	
	printf("\nɾ��ָ��\r\n");
	printf("������ָ��ID��Enter����\r\n");
	printf("0=< ID <=299\r\n");
	delay_ms(50);
	OLED_Clear();
	OLED_ShowCHinese(0,0,13);//ɾ
	OLED_ShowCHinese(16,0,14);//��
	OLED_ShowCHinese(32,0,2);//ָ
	OLED_ShowCHinese(48,0,3);//��
	OLED_ShowString(72,0,(u8 *)"ID",16);
	OLED_ShowString(96,0,(u8 *)"0->",14);
	OLED_ShowString(96,1,(u8 *)"299",14);
	
	OLED_ShowCHinese(27,2,17);//��
	OLED_ShowString(49,2,(u8 *)"#",16);
	OLED_ShowCHinese(63,2,82);//ȷ
	OLED_ShowCHinese(81,2,83);//��	
	
	// �Ӽ�������ID
	while (1)
	{
		switch(key_sta)
		{
			case 0://��ȡ���µİ���
			{
				
				key_cur = get_key_board();	

				if(key_cur != 'N')
				{
					key_old = key_cur;
					key_sta=1;
				}
					
			
			}break;
			
			
			case 1://ȷ�ϰ��µİ���
			{
				key_cur = get_key_board();	
					
				if((key_cur != 'N') && (key_cur == key_old))
				{
					key_sta=2;
					buf[j] = key_cur;
					j++;
				}
								
			
			}break;
		
			case 2://��ȡ�ͷŵİ���
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
		ensure=PS_DeletChar(ID,1);//ɾ������ָ��
		OLED_Clear();
		if(ensure==0x00)
		{	
			printf("ɾ��ָ�Ƴɹ�\r\n");
			OLED_ShowCHinese(27,2,13);//ɾ
			OLED_ShowCHinese(45,2,14);//��
			OLED_ShowCHinese(63,2,7);//��
			OLED_ShowCHinese(81,2,8);//��
		}
		else{
			printf("ɾ��ָ��ʧ��%d\r\n",ensure);
			OLED_ShowCHinese(27,2,13);//ɾ
			OLED_ShowCHinese(45,2,14);//��
			OLED_ShowCHinese(63,2,9);//ʧ
			OLED_ShowCHinese(81,2,10);//��
		}
	}
	else // ɾ������ָ��
	{
		ensure=PS_DeletChar(0,299);//ɾ������ָ��
		OLED_Clear();
		if(ensure==0x00)
		{	
			printf("ɾ��ָ�Ƴɹ�\r\n");
			OLED_ShowCHinese(27,2,13);//ɾ
			OLED_ShowCHinese(45,2,14);//��
			OLED_ShowCHinese(63,2,7);//��
			OLED_ShowCHinese(81,2,8);//��
		}
		else{
			printf("ɾ��ָ��ʧ��%d\r\n",ensure);
			OLED_ShowCHinese(27,2,13);//ɾ
			OLED_ShowCHinese(45,2,14);//��
			OLED_ShowCHinese(63,2,9);//ʧ
			OLED_ShowCHinese(81,2,10);//��
		}
	}
	

	delay_ms(1000);
	List_FR();
	
}

// ��ȡָ�ƴ���
void List_FR(void)
{
	u8 buf[3]={0};
	OLED_Clear();
	PS_ValidTempleteNum(&g_ValidN);//��ȡ���ָ�Ƹ���
	printf("��¼��ָ��%d\r\n", g_ValidN);
	printf("ʣ��ָ����%d\r\n",AS608Para.PS_max-g_ValidN);
	OLED_ShowCHinese(0,2,69);//��
	OLED_ShowCHinese(16,2,70);//��
	OLED_ShowCHinese(32,2,2);//ָ
	OLED_ShowCHinese(48,2,3);//��
	OLED_ShowCHinese(64,2,71);//��
	OLED_ShowCHinese(80,2,72);//��
	memset(buf,0,3);
	sprintf((char*)buf,"%d",g_ValidN);
	OLED_ShowString(100,2,(u8*)buf,16);
	
	OLED_ShowCHinese(0,4,69);//��
	OLED_ShowCHinese(16,4,70);//��
	OLED_ShowCHinese(32,4,73);//ʣ
	OLED_ShowCHinese(48,4,74);//��
	OLED_ShowCHinese(64,4,75);//��
	OLED_ShowCHinese(80,4,76);//��
	sprintf((char*)buf,"%d",AS608Para.PS_max-g_ValidN);
	OLED_ShowString(100,4,(u8*)buf,16);
	delay_ms(3000);
	OLED_Clear();
}

// ��ȡ������¼
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
		if (d == 1) // ָ��
		{
			printf(" [ָ�ƽ���] \r");
		}
		else if (d == 2) // ��������
		{
			printf(" [�������] \r");
		}
		else if (d == 3) // RFID
		{
			printf(" [ RFID�� ] \r");
		}
		else if (d == 4) // ��������
		{
			printf(" [��������] \r");
		}
		
		// ��
		d = *((volatile uint32_t *)(FLASH6_ADDR + (i * 36) + 8));
		printf("20%02x/\r", d);
		
		// ��
		d = *((volatile uint32_t *)(FLASH6_ADDR + (i * 36) + 12));
		printf("%02x/\r", d);
		
		// ��
		d = *((volatile uint32_t *)(FLASH6_ADDR + (i * 36) + 16));
		printf("%02x \r", d);
		
		// ��
		d = *((volatile uint32_t *)(FLASH6_ADDR + (i * 36) + 20));
		printf("Week %x \r", d);
		
		// ʱ
		d = *((volatile uint32_t *)(FLASH6_ADDR + (i * 36) + 24));
		printf("%02x:\r", d);
		
		// ��
		d = *((volatile uint32_t *)(FLASH6_ADDR + (i * 36) + 28));
		printf("%02x:\r", d);
		
		// ��
		d = *((volatile uint32_t *)(FLASH6_ADDR + (i * 36) + 32));
		printf("%02x \r\n", d);
	}
	
	printf("\n��%d��������¼��ȡ���\r\n\n", count);
}

// д�������¼��mode1ָ�ƣ�2���̣�3RFID��4����
void Write_Unlock_Record(u8 mode)
{
	//��ȡʱ��
	RTC_GetTime(RTC_Format_BCD,&RTC_TimeStructure);

	RTC_GetDate(RTC_Format_BCD,&RTC_DateStructure);
	
	//��������
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

// ���������¼�ĸ���
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
	printf("��ǰ��¼��%x������\r\n", g_record_count);
}


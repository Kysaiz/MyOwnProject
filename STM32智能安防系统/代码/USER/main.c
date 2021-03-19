/*******************************************
1. �ֻ���������
��ѯ�������ۼ����е�ʱ��  		//time?#ʹ�ö�ʱ����ʱ
��ѯ��ǰ��ʪ��״̬			//humi?# temp?#
����6λ�����¼ϵͳ��		//��������ʱ�����жϣ�pw=123456#
 -������ȷ����һ��ʾ�⣻		//
 -����ʧ�ܣ�����������һ�᣻
�޸ĵ�¼����			//Newpw=456789#���ж������Ϊ6λ��ʧЧ��������ʾ
�����ó�����ģ�鱨������		//dis=xx#���ж��������20~4000��Χ����ʧЧ��������ʾ
��������״̬			//�ж��������ӱ�־λ
-���ӣ�����LED1
-�Ͽ���Ϩ��LED1


// ����1����ʾ������̵Ĳ�����Ϣ
2. �������
���볣��6λ�����¼ϵͳ		//���յ�6���ַ�ʱ�����ж�
 -������ȷ����һ��ʾ�⣻
 -����ʧ�ܣ�����������һ�᣻
��¼ϵͳ��
 -���¡�A�����ɲ�ѯ��ǰ�¶�
 -���¡�B�����ɲ�ѯ��ǰʪ��
 -���¡�C�����ɲ�ѯ��ǰ���������
 	-���¡�D�����ɲ�ѯ�������ۼ�����ʱ��
 	-���¡�*�����ɸ�λϵͳ
 -���¡�#�������޸ĵ�¼���루ѡ����

֧������10λ����λ�����¼ϵͳ��ѡ����	//���յ�10���ַ�ʱ��strstr�ж�6���ַ�
 -������ȷ����һ��ʾ�⣻
 -����ʧ�ܣ�������������һ�᣻

3. ������ģ��
̽�����С��Ԥ��ı������룬LED4������˸


4. ����1/������������ʵʱ��ʾ������Ϣ��
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
	char *s = NULL; 				// ���ڷָ���յ�������
	uint32_t USdis;					// ���ڴ洢�ָ�����ĳ�������������
	int32_t dht11_ret; 				// ���ڽ�����ʪ�ȶ�ȡ��������ֵ
	uint8_t dht11_buf[5] = {0};		// ���ڴ洢��ȡ������ʪ��
	char msg[256] = {0};			// �����������ݻ�����
	
	char pw[10] = "123456\0";		// ��ʼ����
	char pw_key[12] = {0};			// �洢��������ĵ�¼����
	uint8_t p_key = 0;				// �������������±�
	
	int8_t login_flag1 = 0;			// ����1��־λ��0δ��¼��1��¼�ɹ�
	int8_t login_flag2 = -1;		// ����3��־λ��0δ��¼��1��¼�ɹ���-1δ����	
	
	uint32_t d = 0x1234567;			// sr04��ȡ���ĳ��������
	uint32_t dis = 200;				// ��ʼ��������������
	int8_t warning = 0;				// ������������־
	
	// ������̱�־
	uint32_t key_sta=0;
	char key_old=0;
	char key_cur=0;
	
	// ��������ʱ�ı�־
	uint32_t key_sta1=0;
	char key_old1=0;
	char key_cur1=0;
	uint8_t pwset = 0;				// 0����״̬��1�޸�����״̬
	
	// ���ȼ�����
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	
	BEEP_Init();
	LED_Init();
	led3_init();
	sr04_init();

	tim3_init();
	usart3_EXTI4_init();
	
	key_board_init();
	
	
	//��ʼ������1�Ĳ�����Ϊ115200bps
	//ע�⣺������յ����������룬Ҫ���PLL��
	usart1_init(115200);
	
	//����3�Ĳ�����Ϊ9600bps����Ϊ����ģ��Ĭ��ʹ�øò�����
	usart3_init(9600);
	
	delay_ms(1000);
	
	//����Ƿ��ɶ������Ź����µĸ�λ
	if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) == SET)
	{
		printf("iwdg reset cpu\r\n");
	}
	else
	{
		printf("other reset cpu\r\n");		
	}
	
	//��ձ�־λ
	RCC_ClearFlag();

	iwdg_init();
	
	// ������������
	usart3_send_str("AT+NAMEYaojunwei\r\n");
	delay_ms(50);
	printf("\n�������¼����\r\n");
	
	while(1)
	{
		
		// �ж����������Ƿ�ɹ�
		if (!PFout(9))
		{
			if (login_flag2 == -1)
			{
				printf("\n�����豸���ӳɹ�\r\n");
				delay_ms(3000); // Ϊ�˲���ʱ�ܹ�����������ʾ����ʱ������ȥ��
				usart3_send_str("�������¼����\r\n");
				login_flag2 = 0;
			}
			
			// �������������һ������ʱ
			if (g_usart3_event)
			{
				// ��֤��¼����
				if (!login_flag2)
				if (strstr((char*)g_usart3_buf, pw)) // ������ȷ
				{
					printf("%s\r\n", (char*)g_usart3_buf);
					printf("%s\r\n", pw);
					usart3_send_str("������ȷ����¼�ɹ�\r\n");
					login_flag2 = 1;
					BEEP(1);
					delay_ms(300);
					BEEP(0);
				}
				else // �������
				{
					printf("%s\r\n", (char*)g_usart3_buf);
					printf("%s\r\n", pw);
					usart3_send_str("�����������������\r\n");
					BEEP(1);
					delay_ms(2000);
					BEEP(0);
				}
				
				// ��¼�ɹ����ָ��
				if (login_flag2)
				{
					if (strstr((char*)g_usart3_buf, "time?")) // ��ѯ�������ۼ����е�ʱ�� 
					{
						sprintf(msg, "����ʱ��:%dh,%dm,%ds\r\n", g_time/3600, (g_time%3600)/60, g_time%60);
						usart3_send_str(msg);
					}
					else if (strstr((char*)g_usart3_buf, "newpw=")) // �޸ĵ�¼����
					{
						s = strtok((char *)g_usart3_buf, "=");
						s = strtok(NULL, "=");
						
						if (strlen(s) != 7)	// ����޸ĺ�����벻��6λ���򷵻ش�����ʾ
						{
							sprintf(msg, "���볤�ȴ���\r\n");
							usart3_send_str(msg);
						}
						else
						{
							strncpy(pw, s, 6);
							sprintf(msg, "�޸�����ɹ�\r\n");
							usart3_send_str(msg);
							delay_ms(10);
							sprintf(msg, "�µ����룺%s\r\n", pw);
							usart3_send_str(msg);
						}
						
					}
					else if (strstr((char*)g_usart3_buf, "temp?") != NULL) // ��ѯ�¶�
					{
						if (dht11_time >= 10) // 10�����һ�β���
						{
							delay_ms(100);
							dht11_init();
							delay_ms(100);
							dht11_ret = dht11_read(dht11_buf);
							dht11_time = 0;
						}
						
						if (dht11_ret == 0)
						{
							sprintf(msg, "�¶ȣ�%d.%d\r\n", dht11_buf[2], dht11_buf[3]);
							usart3_send_str(msg);
						}
						else
						{
							usart3_send_str("ʧ�ܣ����Ժ�����\r\n");
							printf("dht11 error code = %d\r\n", dht11_ret);
						}
						
					}
					else if (strstr((char*)g_usart3_buf, "humi?") != NULL) // ��ѯʪ��
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
							sprintf(msg, "ʪ�ȣ�%d.%d\r\n", dht11_buf[0], dht11_buf[1]);
							usart3_send_str(msg);
						}
						else
						{
							usart3_send_str("ʧ�ܣ����Ժ�����\r\n");
							printf("dht11 error code = %d\r\n", dht11_ret);
						}
					}	
					else if (strstr((char*)g_usart3_buf, "dis=") != NULL) // ���ó�������������
					{
						s = strtok((char *)g_usart3_buf, "=");
						s = strtok(NULL, "=");
						
						USdis = atoi(s);
						
						if (USdis >= 20 && USdis <= 4000)	// �Ϸ�����
						{
							dis = USdis;
							usart3_send_str("���þ���ɹ�\r\n");
						}
						else // ���õľ��벻�Ϸ�
						{
							usart3_send_str("���þ��벻�Ϸ�\r\n");
						}
					}
					
					memset((void*)msg, 0, sizeof(msg));
				}
				
				g_usart3_cnt = 0;
				g_usart3_event = 0;
				memset((void *)g_usart3_buf, 0, sizeof(g_usart3_buf));
			}
			
		}
		else // �����Ͽ�����
		{
			if (login_flag2 != -1)
			{
				printf("�����Ͽ�����\r\n");
				login_flag2 = -1;
			}
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
					
					// ��������ʱ����ʾ
					BEEP(1);
					delay_ms(100);
					BEEP(0);
					
					// �жϼ����е�¼
					if (!login_flag1) // ��û��¼
					{	
						pw_key[p_key] = key_cur;
						p_key++;
						// �ж�������10���ַ�
						if (p_key == 10)
						{
							// �ж������Ƿ�ƥ��
							if (strstr(pw_key, pw)) // ��¼�ɹ�
							{
								login_flag1 = 1;
								printf("\n������ȷ����¼�ɹ�\r\n");
								BEEP(1);
								delay_ms(300);
								BEEP(0);
								
								// LED1����
								if (PFout(10))
								{
									PFout(10) = 0;
								}
								
							}
							else // ��¼ʧ��
							{
								printf("\n������󣬵�¼ʧ��\r\n");
								BEEP(1);
								delay_ms(2000);
								BEEP(0);
							}
							
							// ��ջ�����
							memset((void*)pw_key, 0, sizeof(pw_key));
							p_key = 0;
						}
					}
					else // �Ѿ���¼
					{	
						// ����״̬
						if (!pwset)
						{
							// �жϰ��µ���ʲô��
							if (key_cur == 'A') // ��ѯ�¶�
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
									printf("\n�¶ȣ�%d.%d\r\n", dht11_buf[2], dht11_buf[3]);
								}
								else
								{
									printf("dht11 error code = %d\r\n", dht11_ret);
								}
							}
							else if (key_cur == 'B') // ��ѯʪ��
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
									printf("\nʪ�ȣ�%d.%d\r\n", dht11_buf[0], dht11_buf[1]);
								}
								else
								{
									printf("dht11 error code = %d\r\n", dht11_ret);
								}
							}
							else if (key_cur == 'C') // ��ѯ���������
							{
								d = sr04_get_distance();
								printf("\n��ǰ�趨�İ�ȫ��ΧΪ%d\r\n", dis);
								printf("��ǰ���������Ϊ%d\r\n", d);
								
								if (d < dis)
								{
									printf("���볬����������Χ\r\n");
								}
								else
								{
									printf("�����ڰ�ȫ��Χ��\r\n");
								}
								
							}
							else if (key_cur == 'D') // ��ѯϵͳ����ʱ��
							{
								printf("\n����ʱ��:%dh,%dm,%ds\r\n", g_time/3600, (g_time%3600)/60, g_time%60);
							}
							else if (key_cur == '*') // ��λϵͳ
							{
								usart3_send_str("ϵͳ��λ������������\r\n");
								printf("ϵͳ��λ\r\n");
								
								// ���ö������Ź��ļ���ֵ
								IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
								IWDG_SetReload(0);
								IWDG_ReloadCounter();
								IWDG_Enable();
							}
							else if (key_cur == '#') // �޸ĵ�¼����
							{
								printf("\n�������޸ĺ������\r\n");
								pwset = 1; // �����޸�����״̬
							}							
						}
						else // �޸�����״̬
						{
							pw_key[p_key] = key_cur;
							p_key++;
							
							if (p_key == 6) // �������������
							{
								sprintf(pw, "%s", pw_key);
								printf("\n�޸��������\r\n");
								printf("�µ����룺%s\r\n", pw);
								pwset = 0;	// ����״̬
								;
							}
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
		
		
		d = sr04_get_distance();
		if (dis > d) // �ھ��淶Χ��
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

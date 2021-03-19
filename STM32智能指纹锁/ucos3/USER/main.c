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

//����1���ƿ�
OS_TCB app_task_tcb_beep;
void app_task_beep(void *parg);
CPU_STK app_task_stk_beep[128];				//����1�������ջ����СΪ128�֣�Ҳ����512�ֽ�

//����2���ƿ�
OS_TCB app_task_tcb_oled;
void app_task_oled(void *parg);
CPU_STK app_task_stk_oled[512];			//����2�������ջ����СΪ512�֣�Ҳ����2048�ֽ�

//����3���ƿ�
OS_TCB app_task_tcb_rfid;
void app_task_rfid(void *parg);
CPU_STK app_task_stk_rfid[512];				//����3�������ջ����СΪ512�֣�Ҳ����2048�ֽ�

//����4���ƿ�
OS_TCB app_task_tcb_as608;
void app_task_as608(void *parg);
CPU_STK app_task_stk_as608[512];			//����4�������ջ����СΪ512�֣�Ҳ����2048�ֽ�

//����5���ƿ�
OS_TCB app_tak_tcb_scan;
void app_task_scan(void *parg);
CPU_STK task7_stk[128];						//����5�������ջ����СΪ128�֣�Ҳ����512�ֽ�

//����6���ƿ�
OS_TCB app_task_tcb_sta;
void app_task_sta(void *parg);
CPU_STK app_task_stk_sta[512];				//����6�������ջ����СΪ512�֣�Ҳ����2048�ֽ�

//����7���ƿ�
OS_TCB app_task_tcb_key;
void app_task_key(void *parg);
CPU_STK app_task_stk_key[128];				//����7�������ջ����СΪ512�֣�Ҳ����2048�ֽ�

//����8���ƿ�
OS_TCB app_task_tcb_usart;
void app_task_usart(void *parg);
CPU_STK app_task_stk_usart[512];			//����8�������ջ����СΪ512�֣�Ҳ����2048�ֽ�

//����init���ƿ�
OS_TCB app_task_tcb_init;
void app_task_init(void *parg);
CPU_STK app_task_stk_init[512];				//����9�������ջ����СΪ512�֣�Ҳ����2048�ֽ�


extern volatile uint8_t 	g_usart1_event;
extern volatile uint8_t 	g_usart1_buf[128];
extern volatile uint32_t 	g_usart1_cnt;

extern volatile uint8_t 	g_usart3_event;
extern volatile uint8_t 	g_usart3_buf[128];
extern volatile uint32_t	g_usart3_cnt;

extern volatile uint32_t 	g_rtc_wakeup_event;

static RTC_TimeTypeDef  	RTC_TimeStructure;
static RTC_DateTypeDef		RTC_DateStructure;									
									
static u16 			g_ValidN;					// ģ������Чָ�Ƹ���
static SysPara 		AS608Para;					// ָ��ģ��AS608����
static char 		g_user_PW[5]={0};			// �洢�û�����
static char 		g_root_PW[5]={0};			// �洢����Ա����
static char 		g_input_PW[6] = {0};		// ��ǰ�������뻺����
static u8	 		g_count_PW = 0;				// ��ǰ��������λ��
static int32_t 		g_menu_time_count = 0;		// �Զ�����ʱ��
			
	
OS_Q				g_queue_key;				//��Ϣ���еĶ���
OS_Q				g_queue_usart;				//��Ϣ���еĶ���

OS_FLAG_GRP			g_flag_grp;					//�¼���־��Ķ���
OS_FLAG_GRP			g_flag_as608;				//�¼���־��Ķ���
OS_FLAG_GRP			g_flag_rfid;				//�¼���־��Ķ���

OS_MUTEX			g_mutex_printf;				//�������Ķ���
OS_MUTEX			g_mutex_oled;				//�������Ķ���

OS_TMR				g_soft_timer;						//�����ʱ���Ķ���

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

//������
int main(void)
{
	OS_ERR err;

	systick_init();  													//ʱ�ӳ�ʼ��
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);						//�жϷ�������
	
	//OS��ʼ�������ǵ�һ�����еĺ���,��ʼ�����ֵ�ȫ�ֱ����������ж�Ƕ�׼����������ȼ����洢��
	OSInit(&err);
	
	//��������
	OSTaskCreate(	&app_task_tcb_init,							//������ƿ飬��ͬ���߳�id
					"app_task_init",							//��������֣����ֿ����Զ����
					app_task_init,								//����������ͬ���̺߳���
					0,											//���ݲ�������ͬ���̵߳Ĵ��ݲ���
					6,											//��������ȼ�6		
					app_task_stk_init,							//�����ջ����ַ
					512/10,										//�����ջ�����λ���õ����λ�ã��������ټ���ʹ��
					512,										//�����ջ��С			
					0,											//��ֹ������Ϣ����
					0,											//Ĭ������ռʽ�ں�															
					0,											//����Ҫ�����û��洢��
					OS_OPT_TASK_NONE,							//���������޶������
					&err										//���صĴ�����
				);		
	
	//����OS�������������
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
	PS_StaGPIO_Init(); // ָ��ģ���ʼ��
	MFRC522_Initializtion();			//��ʼ��MFRC522
	
	usart2_init(57600);//��ʼ������2,������ָ��ģ��ͨѶ
	
	//��ʼ������1�Ĳ�����Ϊ115200bps
	//ע�⣺������յ����������룬Ҫ���PLL��
	usart1_init(115200);
	
	usart3_EXTI9_5_init();
	
	//����3�Ĳ�����Ϊ9600bps����Ϊ����ģ��Ĭ��ʹ�øò�����
	usart3_init(9600);

	// ������������
	usart3_send_str("AT+NAMEkkkk\r");
	delay_ms(50);
	
#if 0	//���Ҫ���²���������д��������
	printf("���ڲ�������\r\n");
	flash_erase(7);
	printf("7\r");
	flash_erase(6);
	printf("6\r");
	flash_erase(5);
	printf("5\r");
	flash_erase(4);
	printf("4\r");
	printf("�����������\r\n");
	
	strcpy(g_user_PW, "1234\0");
	strcpy(g_root_PW, "8888\0");
	
	WriteFlashData(FLASH7_ADDR, (u8 *)g_user_PW, 5);
	WriteFlashData(FLASH7_ADDR+5, (u8 *)g_root_PW, 5); 
	
#endif

	// ������7��ȡ��������
	ReadFlashData(FLASH7_ADDR, (char *)g_user_PW, 5);
	ReadFlashData(FLASH7_ADDR+5, (char *)g_root_PW, 5);
	
	printf("\n����ָ����-Ҧ���\r\n");
	printf("g_user_PW = %s\r\n", g_user_PW);
	printf("g_root_PW = %s\r\n", g_root_PW);	

	AS608Para.PS_max = 300;
	
	//rtc_init(); //����ʱ��
	rtc_reload(); //��ȡ�Ĵ���ʱ��

	//��������1
	OSTaskCreate(	&app_task_tcb_beep,									//������ƿ飬��ͬ���߳�id
					"app_task_beep",									//��������֣����ֿ����Զ����
					app_task_beep,										//����������ͬ���̺߳���
					0,												//���ݲ�������ͬ���̵߳Ĵ��ݲ���
					6,											 	//��������ȼ�6		
					app_task_stk_beep,									//�����ջ����ַ
					128/10,									//�����ջ�����λ���õ����λ�ã��������ټ���ʹ��
					128,										//�����ջ��С			
					0,											//��ֹ������Ϣ����
					0,												//Ĭ��ʱ��Ƭ����																
					0,												//����Ҫ�����û��洢��
					OS_OPT_TASK_NONE,								//û���κ�ѡ��
					&err													//���صĴ�����
				);
	if(err != OS_ERR_NONE)
	{
		printf("[task1][CREATE]Error Code = %d\r\n",err);
	}
	//��������2
	OSTaskCreate(	&app_task_tcb_oled,									//������ƿ飬��ͬ���߳�id
					"app_task_oled",									//��������֣����ֿ����Զ����
					app_task_oled,										//����������ͬ���̺߳���
					0,												//���ݲ�������ͬ���̵߳Ĵ��ݲ���
					6,											 	//��������ȼ�6		
					app_task_stk_oled,									//�����ջ����ַ
					512/10,									//�����ջ�����λ���õ����λ�ã��������ټ���ʹ��
					512,										//�����ջ��С			
					0,											//��ֹ������Ϣ����
					0,												//Ĭ��ʱ��Ƭ����																
					0,												//����Ҫ�����û��洢��
					OS_OPT_TASK_NONE,								//û���κ�ѡ��
					&err													//���صĴ�����
				);
	if(err != OS_ERR_NONE)
	{
		printf("[task2][CREATE]Error Code = %d\r\n",err);
	}
	//��������3
	OSTaskCreate(	&app_task_tcb_rfid,									//������ƿ飬��ͬ���߳�id
					"app_task_rfid",									//��������֣����ֿ����Զ����
					app_task_rfid,										//����������ͬ���̺߳���
					0,												//���ݲ�������ͬ���̵߳Ĵ��ݲ���
					6,											 	//��������ȼ�6		
					app_task_stk_rfid,									//�����ջ����ַ
					512/10,									//�����ջ�����λ���õ����λ�ã��������ټ���ʹ��
					512,										//�����ջ��С			
					0,											//��ֹ������Ϣ����
					0,												//Ĭ��ʱ��Ƭ����																
					0,												//����Ҫ�����û��洢��
					OS_OPT_TASK_NONE,								//û���κ�ѡ��
					&err													//���صĴ�����
				);
	if(err != OS_ERR_NONE)
	{
		printf("[task3][CREATE]Error Code = %d\r\n",err);
	}
	//��������4
	OSTaskCreate(	&app_task_tcb_as608,									//������ƿ飬��ͬ���߳�id
					"app_task_as608",									//��������֣����ֿ����Զ����
					app_task_as608,										//����������ͬ���̺߳���
					0,												//���ݲ�������ͬ���̵߳Ĵ��ݲ���
					6,											 	//��������ȼ�6		
					app_task_stk_as608,									//�����ջ����ַ
					512/10,									//�����ջ�����λ���õ����λ�ã��������ټ���ʹ��
					512,										//�����ջ��С			
					0,											//��ֹ������Ϣ����
					0,												//Ĭ��ʱ��Ƭ����																
					0,												//����Ҫ�����û��洢��
					OS_OPT_TASK_NONE,								//û���κ�ѡ��
					&err													//���صĴ�����
				);
	if(err != OS_ERR_NONE)
	{
		printf("[task4][CREATE]Error Code = %d\r\n",err);
	}
	//��������5
	OSTaskCreate(	&app_task_tcb_key,									//������ƿ飬��ͬ���߳�id
					"app_task_key",									//��������֣����ֿ����Զ����
					app_task_key,										//����������ͬ���̺߳���
					0,												//���ݲ�������ͬ���̵߳Ĵ��ݲ���
					6,											 	//��������ȼ�6		
					app_task_stk_key,									//�����ջ����ַ
					128/10,									//�����ջ�����λ���õ����λ�ã��������ټ���ʹ��
					128,										//�����ջ��С			
					0,											//��ֹ������Ϣ����
					0,												//Ĭ��ʱ��Ƭ����																
					0,												//����Ҫ�����û��洢��
					OS_OPT_TASK_NONE,								//û���κ�ѡ��
					&err													//���صĴ�����
				);
	if(err != OS_ERR_NONE)
	{
		printf("[task5][CREATE]Error Code = %d\r\n",err);
	}
	//��������6
	OSTaskCreate(	&app_tak_tcb_scan,									//������ƿ飬��ͬ���߳�id
					"app_task_scan",									//��������֣����ֿ����Զ����
					app_task_scan,										//����������ͬ���̺߳���
					0,												//���ݲ�������ͬ���̵߳Ĵ��ݲ���
					6,											 	//��������ȼ�6		
					task7_stk,									//�����ջ����ַ
					128/10,									//�����ջ�����λ���õ����λ�ã��������ټ���ʹ��
					128,										//�����ջ��С			
					0,											//��ֹ������Ϣ����
					0,												//Ĭ��ʱ��Ƭ����																
					0,												//����Ҫ�����û��洢��
					OS_OPT_TASK_NONE,								//û���κ�ѡ��
					&err													//���صĴ�����
				);
	if(err != OS_ERR_NONE)
	{
		printf("[task6][CREATE]Error Code = %d\r\n",err);
	}
	//��������7
	OSTaskCreate(	&app_task_tcb_sta,									//������ƿ飬��ͬ���߳�id
					"app_task_sta",									//��������֣����ֿ����Զ����
					app_task_sta,										//����������ͬ���̺߳���
					0,												//���ݲ�������ͬ���̵߳Ĵ��ݲ���
					6,											 	//��������ȼ�6		
					app_task_stk_sta,									//�����ջ����ַ
					512/10,									//�����ջ�����λ���õ����λ�ã��������ټ���ʹ��
					512,										//�����ջ��С			
					0,											//��ֹ������Ϣ����
					0,												//Ĭ��ʱ��Ƭ����																
					0,												//����Ҫ�����û��洢��
					OS_OPT_TASK_NONE,								//û���κ�ѡ��
					&err													//���صĴ�����
				);
	if(err != OS_ERR_NONE)
	{
		printf("[task7][CREATE]Error Code = %d\r\n",err);
	}
	//��������8
	OSTaskCreate(	&app_task_tcb_usart,									//������ƿ飬��ͬ���߳�id
					"app_task_usart",									//��������֣����ֿ����Զ����
					app_task_usart,										//����������ͬ���̺߳���
					0,												//���ݲ�������ͬ���̵߳Ĵ��ݲ���
					6,											 	//��������ȼ�6		
					app_task_stk_usart,									//�����ջ����ַ
					512/10,									//�����ջ�����λ���õ����λ�ã��������ټ���ʹ��
					512,										//�����ջ��С			
					0,											//��ֹ������Ϣ����
					0,												//Ĭ��ʱ��Ƭ����																
					0,												//����Ҫ�����û��洢��
					OS_OPT_TASK_NONE,								//û���κ�ѡ��
					&err													//���صĴ�����
				);
	if(err != OS_ERR_NONE)
	{
		printf("[task8][CREATE]Error Code = %d\r\n",err);
	}

	OSQCreate(&g_queue_key,"g_queue_key",64,&err);			//������Ϣ����
	OSQCreate(&g_queue_usart,"g_queue_usart",64,&err);		//������Ϣ����
	
	
	OSFlagCreate(&g_flag_grp,"g_flag_grp",0,&err);			//�����¼���־�飬���б�־λ��ֵΪ0
	OSFlagCreate(&g_flag_as608,"g_flag_as608",0,&err);		//�����¼���־�飬���б�־λ��ֵΪ0
	OSFlagCreate(&g_flag_rfid,"g_flag_rfid",0,&err);		//�����¼���־�飬���б�־λ��ֵΪ0

	OSMutexCreate(&g_mutex_printf,	"g_mutex_printf",&err);	//����������
	OSMutexCreate(&g_mutex_oled,	"g_mutex_oled",&err);	//����������
	
	iwdg_init();
	OSTmrCreate(&g_soft_timer,"g_soft_timer",0,100,OS_OPT_TMR_PERIODIC,(OS_TMR_CALLBACK_PTR)timer_iwdg_callback,NULL,&err);
	OSTmrStart(&g_soft_timer,&err);
	
	//ɾ���������񣬽�������̬
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
		
		if (flags & 0x02) // �����¼��ɹ�������2��
		{
			// ����ɨ������һ��ʱ��
			OSTaskSuspend(&app_tak_tcb_scan,&err);
			
			BEEP(1);
			delay_ms(100);
			BEEP(0);
			delay_ms(100);
			BEEP(1);
			delay_ms(100);
			BEEP(0);
			delay_ms(1100);
			
			// ����ɨ������
			OSTaskResume(&app_tak_tcb_scan,&err);
			
		}
		else if (flags & 0x04) // �����¼�ʧ�ܣ�����1��
		{
			// ����ɨ������һ��ʱ��
			OSTaskSuspend(&app_tak_tcb_scan,&err);
			
			BEEP(1);
			delay_ms(1000);
			BEEP(0);
			delay_ms(1100);
			
			// ����ɨ������
			OSTaskResume(&app_tak_tcb_scan,&err);
		}
		
//		//�ָ�����1
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
	uint8_t dht11_buf[5] = {0};		// ���ڴ洢��ȡ������ʪ��
	u8 dht11_time = 6;
	u8 lock_time = 15;
	u8 lock_event = 1;
	
	printf("[app_task_oled] create ok\r\n");

	while(1)
	{
		//һֱ�����ȴ��¼���־��
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
		
		if(flags & 0x01) // ������������
		{
			//��ȡʱ��
			RTC_GetTime(RTC_Format_BCD,&RTC_TimeStructure);
			sprintf(buf_time,"%02x:%02x:%02x",RTC_TimeStructure.RTC_Hours,RTC_TimeStructure.RTC_Minutes,RTC_TimeStructure.RTC_Seconds);
			
			RTC_GetDate(RTC_Format_BCD,&RTC_DateStructure);
			sprintf(buf_date,"20%02x/%02x/%02xWeek%x",RTC_DateStructure.RTC_Year,RTC_DateStructure.RTC_Month,RTC_DateStructure.RTC_Date,RTC_DateStructure.RTC_WeekDay);

			if (g_menu_time_count == 0)
			{
				OLED_ShowString(8,0,(u8 *)"  ",16);
				OLED_ShowCHinese(24,0,0);//��
				OLED_ShowCHinese(40,0,1);//��
				OLED_ShowCHinese(56,0,2);//ָ
				OLED_ShowCHinese(72,0,3);//��
				OLED_ShowCHinese(88,0,4);//��
				OLED_ShowString(104,0,(u8 *)" ",16);
				memset(g_input_PW, 0, 5);
				g_count_PW = 0;
			}
			else if (g_menu_time_count == 3)
			{
				// ����
				OLED_ShowString(16,0,(u8 *)"GZ2075",16);
				OLED_ShowCHinese(64,0,98);
				OLED_ShowCHinese(80,0,99);
				OLED_ShowCHinese(96,0,100);
				
			}
			else if (g_menu_time_count >= 5)
			{
				g_menu_time_count = -1;
			}

			if (lock_time >= 15) // 15���Զ���������ʾ����icon
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
		else if(flags & 0x02) // �����ɹ�������ʾ�����ɹ�icon+�����ɹ�����
		{
			OLED_ShowString(16,0,(u8 *)"  ",16);
			OLED_ShowCHinese(32,0,5);//��
			OLED_ShowCHinese(48,0,6);//��
			OLED_ShowCHinese(64,0,7);//��
			OLED_ShowCHinese(80,0,8);//��
			OLED_ShowString(96,0,(u8 *)"  ",16);
			
			OLED_DrawBMP(112,0,128,2,(uint8_t *)icon_unlock);
			lock_event = 1;
			lock_time = 0;
			g_menu_time_count = -2;
		}
		else if(flags & 0x04) // ����ʧ�ܣ���ʾ������ʧ�ܡ�����
		{
			OLED_ShowString(16,0,(u8 *)"  ",16);
			OLED_ShowCHinese(32,0,5);//��
			OLED_ShowCHinese(48,0,6);//��
			OLED_ShowCHinese(64,0,9);//ʧ
			OLED_ShowCHinese(80,0,10);//��
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
	
	//MFRC522������
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
		
		if (flags & 0x01) // ��Ҫ��֤��
		{
			MFRC522_Initializtion();			//��ʼ��MFRC522
			status=MFRC522_Request(0x52, card_pydebuf);			//Ѱ��
		
			if (status == 0) // ���������
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
				
				if (Check_Card((u8*)buf)) // ����ÿ����ڣ������
				{
						printf("RFID��ƥ�䣬����\r\n");
						Write_Unlock_Record(3);
						
						OSFlagPost(&g_flag_grp,0x02,OS_OPT_POST_FLAG_SET,&err);	
				}
				else
				{
						printf("RFID����ƥ��\r\n");
						
						OSFlagPost(&g_flag_grp,0x04,OS_OPT_POST_FLAG_SET,&err);	
				}
			}
		}
		else if (flags & 0x02) // ¼�뿨
		{
			i = 0;
			while (1)
			{
				OLED_Clear();
				OLED_ShowCHinese(16,0,15);//��
				OLED_ShowCHinese(32,0,92);//ˢ
				OLED_ShowCHinese(48,0,91);//��
				OLED_ShowCHinese(64,0,88);//��
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
				
				delay_ms(1000);
				i++;
				if (i == 5) // 5��û�ſ����˳�
				{
					printf("����5��û�зſ�\r\n");
					break;
				}
			}
		
			OLED_Clear();
			OSFlagPost(&g_flag_rfid,0x08,OS_OPT_POST_FLAG_SET,&err);
		}
		else if (flags & 0x04) // ɾ����
		{
			count = Count_Card();
			OLED_Clear();
			i = 0;
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
			}
			else
			{
				OLED_ShowCHinese(16,0,96);//ѡ
				OLED_ShowCHinese(32,0,97);//��
				OLED_ShowCHinese(48,0,13);//ɾ
				OLED_ShowCHinese(64,0,14);//��
				OLED_ShowCHinese(80,0,88);//��
				OLED_ShowString(96,0,"ID",16);//ID
				
				OLED_ShowString(0,2,(u8 *)"A",16);
				OLED_ShowCHinese(8,2,82);//ȷ
				OLED_ShowCHinese(24,2,83);//��
				OLED_ShowString(40,2,(u8 *)"BC",16);
				OLED_ShowCHinese(56,2,84);//��
				OLED_ShowCHinese(72,2,85);//ҳ
				OLED_ShowString(88,2,(u8 *)"D",16);
				OLED_ShowCHinese(96,2,57);//��
				OLED_ShowCHinese(112,2,58);//��
				
				while(1)
				{
					memset(buf1,0,sizeof(buf1));
					memset(buf2,0,sizeof(buf2));
					
					ReadFlashData((FLASH5_ADDR+i*11),buf1,11);
					sprintf(buf2,"%c%c %c%c %c%c %c%c %c%c",buf1[0],buf1[1],buf1[2],buf1[3],buf1[4],buf1[5],buf1[6],buf1[7],buf1[8],buf1[9]);
					OLED_ShowString(8,5,(u8 *)buf2,16);
					
					p=OSQPend(&g_queue_key,0,OS_OPT_PEND_BLOCKING,&msg_size,NULL,&err);//�����ȴ�����
					pmsg = *p;
					OSFlagPost(&g_flag_grp,0x10,OS_OPT_POST_FLAG_SET,&err);	
					
					if (pmsg == 'B') // �Ϸ�ҳ
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
					else if (pmsg == 'C') // �·�ҳ
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
					else if (pmsg == 'D') // �˳�
					{
						OLED_Clear();
						i = -1;
						break;
					}
					else if (pmsg == 'A') // ȷ��ɾ��
					{
						break;
					}
					
					memset(p, 0, msg_size);
				}
				
				if (i != -1)
				{
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
			}
			
			OSFlagPost(&g_flag_rfid,0x10,OS_OPT_POST_FLAG_SET,&err);

		}

		//delay_ms(500); // ÿ0.5����һ��
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
//	u32 buf_id[300] = {0}; // ��¼ID��д������7
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
		
		if (flags & 0x01) // ��Ҫ��ָ֤��
		{
			ensure=PS_GetImage();
			if(ensure==0x00)//��ȡͼ��ɹ� 
			{
				ensure=PS_GenChar(CharBuffer1);
				if(ensure==0x00) //���������ɹ�
				{
					ensure=PS_HighSpeedSearch(CharBuffer1,0,AS608Para.PS_max,&seach);
					if(ensure==0x00)//�����ɹ�
					{				
						OSTaskResume(&app_task_tcb_beep,&err);
						printf("ƥ��ָ�Ƴɹ�\r\n");
						printf("ȷ�д���,ID:%d  ƥ��÷�:%d\r\n",seach.pageID,seach.mathscore);
						
						//�����ɹ��¼���־��
						OSFlagPost(&g_flag_grp,0x02,OS_OPT_POST_FLAG_SET,&err);	
						Write_Unlock_Record(1);
						delay_ms(1000);
					}
					else 
					{
						printf("ƥ��ָ��ʧ��%d\r\n",ensure);	
						//����ʧ���¼���־��		
						OSFlagPost(&g_flag_grp,0x04,OS_OPT_POST_FLAG_SET,&err);	
						delay_ms(1000);
					}
				}
			}
		}
		else if (flags & 0x02) // ��ָ��
		{
			printf("��ָ��\r\n");
			List_FR();
			OSFlagPost(&g_flag_as608,0x40,OS_OPT_POST_FLAG_SET,&err);
		}
		else if (flags & 0x04) // ¼ָ��
		{
			printf("¼ָ��\r\n");
			j = 0;
			processnum = 0;
			i = 0;
			while(j != -1)
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
					
						memset(buf, 0, 20);
						// ����ID
						while (1)
						{
							//�����ȴ�����
							p=OSQPend(&g_queue_key,0,OS_OPT_PEND_BLOCKING,&msg_size,NULL,&err);
							buf[j] = *p;
							OSFlagPost(&g_flag_grp,0x10,OS_OPT_POST_FLAG_SET,&err);	
							
							j++;
							
							if (strstr(buf, "*")) // ����
							{
								memset(buf, 0, 3);
								j = 0;
								OLED_ShowString(40,4,(u8 *)"      ",16);
							}
							else if (strstr(buf, "D")) // �˳�
							{
								j = -1;
								break;
							}
							else if (strstr(buf, "#") || j == 3) // �������
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
						
						//����ģ��
						ensure=PS_StoreChar(CharBuffer1,ID);
						
						if(ensure==0x00) 
						{		
							printf("¼��ָ�Ƴɹ�\r\n");
							OLED_ShowCHinese(16,4,11);//¼
							OLED_ShowCHinese(32,4,12);//��
							OLED_ShowCHinese(48,4,2);//ָ
							OLED_ShowCHinese(64,4,3);//��
							OLED_ShowCHinese(80,4,7);//��
							OLED_ShowCHinese(96,4,8);//��
							delay_ms(1500);
							List_FR();
							j = -1;
							break;
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
			OSFlagPost(&g_flag_as608,0x10,OS_OPT_POST_FLAG_SET,&err);
		}
		else if (flags & 0x08) // ɾָ��
		{
			printf("\nɾ��ָ��\r\n");
			printf("������ָ��ID��Enter����\r\n");
			printf("0=< ID <=299\r\n");
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
			
			OLED_ShowString(8,6,(u8 *)"*",16);
			OLED_ShowCHinese(16,6,89);//��
			OLED_ShowCHinese(32,6,31);//��			
		
			OLED_ShowString(56,6,(u8 *)"D",16);
			OLED_ShowCHinese(64,6,57);//��
			OLED_ShowCHinese(80,6,58);//��
			
			j = 0;
			memset(buf, 0, 3);
			// ����ID
			while (1)
			{
				//�����ȴ�����
				p=OSQPend(&g_queue_key,0,OS_OPT_PEND_BLOCKING,&msg_size,NULL,&err);
				buf[j] = *p;
				OSFlagPost(&g_flag_grp,0x10,OS_OPT_POST_FLAG_SET,&err);	
				j++;
				
				if (strstr(buf, "*")) // ����
				{
					memset(buf, 0, 3);
					j = 0;
					OLED_ShowString(40,4,(u8 *)"      ",16);
				}
				else if (strstr(buf, "D")) // �˳�
				{
					j = -1;
					OLED_Clear();
					break;
				}
				else if (strstr(buf, "#") || j == 3) // �������
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
					ensure=PS_DeletChar(ID,1);//ɾ������ָ��
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
	u8 status = 0;	// ״̬0����ǰ�������棬���յ�����������
					// ״̬1����ǰ�ڹ���Ա���棬���յ���ģʽѡ��
					// ״̬2����ǰ�ڸ�������棬���յ���������
	u8 buf[5] = {0};
	u8 i;
	u8 page_count = 0;
	
	printf("[app_task_sta] create ok\r\n");

	while(1)
	{
		p=OSQPend(&g_queue_key,0,OS_OPT_PEND_BLOCKING,&msg_size,NULL,&err);//�����ȴ�����
		pmsg = *p;
		OSFlagPost(&g_flag_grp,0x10,OS_OPT_POST_FLAG_SET,&err);	
		
		switch(status)
		{
			case 0: // ���յ��������¼
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
				
				// �Ե�ǰ��������ж�
				if (strstr(g_input_PW, g_user_PW)) // �û�����ƥ�䣬����
				{
					OSFlagPost(&g_flag_grp,0x02,OS_OPT_POST_FLAG_SET,&err);	
					Write_Unlock_Record(2);
					memset(g_input_PW, 0, sizeof(g_input_PW));
					g_count_PW = 0;
				}
				else if (strstr(g_input_PW, g_root_PW)) // ����Ա����ƥ�䣬�������Աģʽ
				{
					Write_Unlock_Record(5);
					
					//��������2
					OSTaskSuspend(&app_task_tcb_oled,&err);
					OSTaskSuspend(&app_tak_tcb_scan,&err);
					
					OLED_Clear();
					admin_surface(0);
					
					memset(g_input_PW, 0, sizeof(g_input_PW));
					g_count_PW = 0;
					status = 1;
				}
				else if (g_count_PW == 6) // ������󣬽���ʧ��
				{
					OSFlagPost(&g_flag_grp,0x04,OS_OPT_POST_FLAG_SET,&err);	
					memset(g_input_PW, 0, sizeof(g_input_PW));
					g_count_PW = 0;
				}
			}break;
			
			case 1: // ����Աģʽ�½��յ���ģʽѡ��
			{
				if (pmsg == 'B') // �Ϸ�ҳ
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
				else if (pmsg == 'C') // �·�ҳ
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
				else if (pmsg == 'A') // ȷ��
				{
					// �Ե�ǰѡ������ж�
					if (page_count == 0) // ����ǲ�ָ�ƣ�����Ҫ��Ϣ����
					{
						OSFlagPost(&g_flag_as608,0x02,OS_OPT_POST_FLAG_SET,&err);	
						
						// �����ȴ���ָ�ƽ���
						OSFlagPend(&g_flag_as608,
							0x40,
							0,
							OS_OPT_PEND_FLAG_SET_ANY + OS_OPT_PEND_FLAG_CONSUME+OS_OPT_PEND_BLOCKING,
							NULL,
							&err);
					}
					else if (page_count == 1) // ¼ָ��
					{
						OSFlagPost(&g_flag_as608,0x04,OS_OPT_POST_FLAG_SET,&err);
						
						// �����ȴ�¼ָ�ƽ���
						OSFlagPend(&g_flag_as608,
							0x10,
							0,
							OS_OPT_PEND_FLAG_SET_ANY + OS_OPT_PEND_FLAG_CONSUME+OS_OPT_PEND_BLOCKING,
							NULL,
							&err);
					}
					else if (page_count == 2) // ɾָ��
					{
						OSFlagPost(&g_flag_as608,0x08,OS_OPT_POST_FLAG_SET,&err);
						
						// �����ȴ�ɾָ�ƽ���
						OSFlagPend(&g_flag_as608,
							0x20,
							0,
							OS_OPT_PEND_FLAG_SET_ANY + OS_OPT_PEND_FLAG_CONSUME+OS_OPT_PEND_BLOCKING,
							NULL,
							&err);
					}
					else if (page_count == 3) // ������
					{
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
						status = 2;
					}
					else if (page_count == 4) // ¼�뿨������Ҫ��Ϣ����
					{
						OSFlagPost(&g_flag_rfid,0x02,OS_OPT_POST_FLAG_SET,&err);
						
						// �����ȴ�¼�뿨����
						OSFlagPend(&g_flag_rfid,
							0x08,
							0,
							OS_OPT_PEND_FLAG_SET_ANY + OS_OPT_PEND_FLAG_CONSUME+OS_OPT_PEND_BLOCKING,
							NULL,
							&err);
					}
					else if (page_count == 5) // ɾ����
					{
						OSFlagPost(&g_flag_rfid,0x04,OS_OPT_POST_FLAG_SET,&err);
						
						// �����ȴ�ɾ��������
						OSFlagPend(&g_flag_rfid,
							0x10,
							0,
							OS_OPT_PEND_FLAG_SET_ANY + OS_OPT_PEND_FLAG_CONSUME+OS_OPT_PEND_BLOCKING,
							NULL,
							&err);
					}
					
				}
				else if (pmsg == 'D') // �˳�����Աģʽ
				{
					status = 0;
					g_menu_time_count = 0;
					OLED_Clear();
					OLED_DrawBMP(112,0,128,2,(uint8_t *)icon_lock);
					//��������2
					OSTaskResume(&app_task_tcb_oled,&err);
					OSTaskResume(&app_tak_tcb_scan,&err);
					
				}
				
				if (status == 1)
				{
					admin_surface(page_count);
				}
			}break;
			
			case 2: // ������ģʽ
			{
				buf[i] = pmsg;
				i++;
				OLED_ShowString(40,4,(u8 *)buf,16);
				
				if (i == 4)
				{
					OLED_ShowCHinese(16,6,29);//��
					OLED_ShowCHinese(32,6,95);//��
					OLED_ShowCHinese(48,6,90);//��
					OLED_ShowCHinese(64,6,87);//��
					OLED_ShowCHinese(80,6,45);//��
					OLED_ShowCHinese(96,6,46);//��
					
					flash_erase(7);
					buf[4] = '\0';
					strncpy(g_user_PW, (char*)buf, 5);
					WriteFlashData(FLASH7_ADDR, (uint8_t *)g_user_PW, 5);
					WriteFlashData(FLASH7_ADDR+5, (uint8_t *)g_root_PW, 5); 
					memset(buf, 0, 5);
					i = 0;
					
					OLED_ShowCHinese(16,6,90);//��
					OLED_ShowCHinese(32,6,87);//��
					OLED_ShowCHinese(48,6,45);//��
					OLED_ShowCHinese(64,6,46);//��
					OLED_ShowCHinese(80,6,7);//��
					OLED_ShowCHinese(96,6,8);//��
					
					delay_ms(1000);
					status = 1;
				}
				else if (strstr((char*)buf, "*")) // ����
				{
					memset(buf, 0, 5);
					i = 0;
					OLED_ShowString(40,4,(u8 *)"      ",16);
				}
				else if (strstr((char*)buf, "D")) // �˳�
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
	
	// ������̱�־
	uint32_t key_sta=0;
	char key_old=0;
	char key_cur=0;
	
	printf("[app_task_key] create ok\r\n");

	while(1)
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
					key_buf = key_cur;
					OSQPost(&g_queue_key,&key_buf,1,OS_OPT_POST_FIFO,&err);
					// �����ȴ��������뱻�������
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
		
			case 2://��ȡ�ͷŵİ���
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
	
	uint8_t *p=NULL;				// ���ڽ�����Ϣ���е�����
	char *s = NULL; 				// ���ڷָ���յ�������
	int32_t d;
	u32 i;
	u8 flag_login = 0;				// 0��ʾ����δ��¼��1��ʾ��ȷ��������
	u32 record_count;
	
	printf("[app_task_usart] create ok\r\n");

	record_count = Count_Unlock_Record();
	
	while(1)
	{
		//�����ȴ�����
		p=OSQPend(&g_queue_usart,0,OS_OPT_PEND_BLOCKING,&msg_size,NULL,&err);
		
		//��ȷ���յ���Ϣ
		if(p && msg_size)
		{
			//�رմ����ж�
			NVIC_DisableIRQ(USART1_IRQn);
			NVIC_DisableIRQ(USART3_IRQn);
			
			if (!flag_login) // �������δ��¼
			{
				if (strstr((char *)p, g_root_PW)) // ��������˹���Ա����
				{
					flag_login = 1;
					
					Write_Unlock_Record(5);
					
					printf("����Ա��¼\r\n");
					delay_ms(3000); // Ϊ�˲���ʱ���Կ���������Ϣ����ʱ������ȥ��
					usart3_send_str("����Ա��¼�ɹ�\r\n");
				}
				else if (strstr((char *)p, g_user_PW)) // ��������˽�������
				{
					Write_Unlock_Record(4);
					OSFlagPost(&g_flag_grp,0x02,OS_OPT_POST_FLAG_SET,&err);	
					printf("�����ɹ�\r\n");
					usart3_send_str("�����ɹ�\r\n");
				}
				else // �����������
				{
					printf("�������\r\n");
					usart3_send_str("�������\r\n");
				}
				
			}
			else if (flag_login) // ��������Աģʽ��¼
			{
				if (strstr((char *)p, "record")) // ��ѯ������¼
				{
					record_count = Count_Unlock_Record();
					printf("\n");
					for (i = 0; i < record_count; i++)
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
						else if (d == 5) // ����Ա
						{
							printf(" [ ����Ա ] \r");
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
					
					printf("\n��%d��������¼��ȡ���\r\n\n", record_count);
				}
				else if (strstr((char *)p, "clear")) // �����¼
				{
					printf("���ڲ�������6\r\n");
					flash_erase(6);
					printf("��������6���\r\n");
					usart3_send_str("��¼������\r\n");
					record_count = Count_Unlock_Record();
				}
				else if (strstr((char *)p,  "newpw=")) // �����½�������
				{
					s = strtok((char*)p, "=");
					s = strtok(NULL, "=");
					
					if (strlen(s) != 5) // ���Ȳ���
					{
						printf("�����볤�Ȳ���\r\n");
						usart3_send_str("�����볤�Ȳ���\r\n");
					}
					else
					{
						usart3_send_str("���ڸ�������\r\n");
						printf("���ڸ�������\r\n");
						
						strncpy(g_user_PW, s, 4);
						g_user_PW[4] = '\0';
						flash_erase(7);
						WriteFlashData(FLASH7_ADDR, (uint8_t *)g_user_PW, 5);
						WriteFlashData(FLASH7_ADDR+5, (uint8_t *)g_root_PW, 5); 
						
						OLED_ShowCHinese(16,6,90);//��
						OLED_ShowCHinese(32,6,87);//��
						OLED_ShowCHinese(48,6,45);//��
						OLED_ShowCHinese(64,6,46);//��
						OLED_ShowCHinese(80,6,7);//��
						OLED_ShowCHinese(96,6,8);//��
						
						usart3_send_str("��������ɹ�\r\n");
						printf("��������ɹ�\r\n");
					}
				}
				else if (strstr((char *)p, "unlock")) // ����
				{
					Write_Unlock_Record(4);
					OSFlagPost(&g_flag_grp,0x02,OS_OPT_POST_FLAG_SET,&err);	
					printf("����Ա�����ɹ�\r\n");
					usart3_send_str("�����ɹ�\r\n");
				}
				
			}
			
			// �˳���¼
			if (strstr((char *)p, "quit"))
			{
				flag_login = 0;
				printf("����Ա���ӶϿ�\r\n");
			}	
			
			g_usart1_cnt = 0;
			g_usart1_event = 0;
			memset((void *)g_usart1_buf, 0, sizeof(g_usart1_buf));
			g_usart3_cnt = 0;
			g_usart3_event = 0;
			memset((void *)g_usart3_buf, 0, sizeof(g_usart3_buf));
			
			memset(p, 0, msg_size);
			
			//ʹ�ܴ����ж�
			NVIC_EnableIRQ(USART1_IRQn);
			NVIC_EnableIRQ(USART3_IRQn);
		}
		
	}
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
	delay_ms(2000);
	OLED_Clear();
}

// д�������¼��mode1ָ�ƣ�2���̣�3RFID��4������5����Ա
void Write_Unlock_Record(u8 mode)
{
	u32 record_count;
	
	//��ȡʱ��
	RTC_GetTime(RTC_Format_BCD,&RTC_TimeStructure);

	RTC_GetDate(RTC_Format_BCD,&RTC_DateStructure);
	
	record_count = Count_Unlock_Record();
	
	//��������
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
	
	printf("��ǰ��¼��%d������\r\n", record_count);
}

// ���������¼�ĸ���
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
	printf("��ǰ��¼��%d������\r\n", record_count);
	
	return record_count;
}

void admin_surface(u32 page_count)
{
	uint8_t admin_mode_menu[] = {86,2,3,	//��ָ��
								11,2,3,		//¼ָ��
								13,2,3,		//ɾָ��
								87,45,46,	//������
								11,12,88,	//¼�뿨
								13,14,88};	//ɾ����
	
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



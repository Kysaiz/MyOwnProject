#include "rtc.h"
#include <stdio.h>

static RTC_InitTypeDef   		RTC_InitStructure;
static RTC_TimeTypeDef  		RTC_TimeStructure;
static RTC_DateTypeDef			RTC_DateStructure;
static EXTI_InitTypeDef			EXTI_InitStructure;
static RTC_AlarmTypeDef 		RTC_AlarmStructure;
static NVIC_InitTypeDef   		NVIC_InitStructure;

volatile uint32_t alarm_flag = 0;	// ���ӱ�־λ��0��ʾû���㣬1��ʾ��������
volatile uint32_t g_rtc_wakeup_event=0;

void rtc_init(void)
{

	/* Enable the PWR clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	
	/* Allow access to RTC��RTC�ĵ�Դ����ʹ�� */
	PWR_BackupAccessCmd(ENABLE);

	//ʹ��LSE��32.768KHz��
	RCC_LSEConfig(RCC_LSE_ON);
	
	/* Wait till LSE is ready ���ȴ�LSE����*/  
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
	{
	}
	
	/* Select the RTC Clock Source��ΪRTCѡ�е�ʱ��ԴΪLSE */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);


	/* Enable the RTC Clock��ʹ��RTC��Ӳ��ʱ�� */
	RCC_RTCCLKCmd(ENABLE);
	
	/* Wait for RTC APB registers synchronisation ���ȴ�RTC��ؼĴ�������*/
	RTC_WaitForSynchro();
	
	/* Configure the RTC data register and RTC prescaler */
	/* ck_spre(1Hz) = RTCCLK(LSE) /((uwAsynchPrediv + 1)*(uwSynchPrediv + 1))*/

	//RTC��Ӳ��ʱ��=32768Hz/(0x7F+1)/(0xFF+1)=1Hz
	RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
	RTC_InitStructure.RTC_SynchPrediv = 0xFF;
	RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;//24Сʱ��ʽ
	RTC_Init(&RTC_InitStructure);


	/* Set the date: Monday February 22th 2021 */
	RTC_DateStructure.RTC_Year = 0x21;
	RTC_DateStructure.RTC_Month = RTC_Month_March;
	RTC_DateStructure.RTC_Date = 0x03;
	RTC_DateStructure.RTC_WeekDay = RTC_Weekday_Wednesday;
	RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);
	
	/* Set the time to 16h 28mn 30s PM�� */
	RTC_TimeStructure.RTC_H12     = RTC_H12_PM;
	RTC_TimeStructure.RTC_Hours   = 0x10;
	RTC_TimeStructure.RTC_Minutes = 0x57;
	RTC_TimeStructure.RTC_Seconds = 0x00; 
	RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure);


	//�رջ��ѹ���
	RTC_WakeUpCmd(DISABLE);
	
	//Ϊ���ѹ���ѡ��RTC���úõ�ʱ��Դ
	RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);
	
	//���û��Ѽ���ֵΪ�Զ����أ�д��ֵĬ����0
	RTC_SetWakeUpCounter(1-1);
	
	//���RTC�����жϱ�־
	RTC_ClearITPendingBit(RTC_IT_WUT);
	
	//ʹ��RTC�����ж�
	RTC_ITConfig(RTC_IT_WUT, ENABLE);

	//ʹ�ܻ��ѹ���
	RTC_WakeUpCmd(ENABLE);

	/* Configure EXTI Line22�������ⲿ�жϿ�����22 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line22;			//��ǰʹ���ⲿ�жϿ�����22
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;		//�ж�ģʽ
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;		//�����ش����ж� 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;			//ʹ���ⲿ�жϿ�����22
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;		//����RTC�����жϴ���
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x03;	//��ռ���ȼ�Ϊ0x3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;		//��Ӧ���ȼ�Ϊ0x3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//ʹ��
	NVIC_Init(&NVIC_InitStructure);

}

void rtc_alarm_init(void)
{
	/* ����RTC��A���Ӵ����ж� */
	RTC_ITConfig(RTC_IT_ALRA, ENABLE);
	
	/* ��ձ�־λ */
	RTC_ClearFlag(RTC_FLAG_ALRAF);

	/*ʹ���ⲿ�жϿ�����17���ж�*/
	EXTI_ClearITPendingBit(EXTI_Line17);
	EXTI_InitStructure.EXTI_Line = EXTI_Line17;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	/*ʹ�����ӵ��ж� */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void rtc_alarm_set(RTC_AlarmTypeDef RTC_AlarmStructure)
{
	/* �ر����ӣ������رգ��������Ӵ������ж���BUG��������ô���ã�ֻҪ��00�룬�򴥷��ж�*/
	RTC_AlarmCmd(RTC_Alarm_A, DISABLE);
	
	/* ����RTC��A���ӣ�ע��RTC���������������ֱ�Ϊ����A������B */
	RTC_SetAlarm(RTC_Format_BCD, RTC_Alarm_A, &RTC_AlarmStructure);
	
	/* ��RTC������A����*/
	RTC_AlarmCmd(RTC_Alarm_A, ENABLE);
}

void RTC_WKUP_IRQHandler(void)
{
	if(RTC_GetITStatus(RTC_IT_WUT) != RESET)
	{
		g_rtc_wakeup_event=1;
		RTC_ClearITPendingBit(RTC_IT_WUT);
		EXTI_ClearITPendingBit(EXTI_Line22);
	} 
}

void RTC_Alarm_IRQHandler(void)
{    
	if(RTC_GetFlagStatus(RTC_FLAG_ALRAF)==SET)//ALARM A�ж�
	{
		alarm_flag = 1;
		RTC_ClearFlag(RTC_FLAG_ALRAF);//����жϱ�־
	}   
	EXTI_ClearITPendingBit(EXTI_Line17);	//����ж���17���жϱ�־ 											 
}

void rtc_reload(void)
{
	//ÿ�θ�λ��ʱ�򣬶�ȡ��ȡ���ݼĴ���0
	if(0x1688!=RTC_ReadBackupRegister(RTC_BKP_DR0))
	{
		//rtc�ĳ�ʼ��
		rtc_init();
	
		//�����ݼĴ���0д������Ϊ0x1688
		RTC_WriteBackupRegister(RTC_BKP_DR0, 0x1688);	
	}
	else
	{
		/* Enable the PWR clock */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
		
		/* Allow access to RTC��RTC�ĵ�Դ����ʹ�� */
		PWR_BackupAccessCmd(ENABLE);

		//ʹ��LSE��32.768KHz��
		RCC_LSEConfig(RCC_LSE_ON);
		
		/* Wait till LSE is ready ���ȴ�LSE����*/  
		while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
		{
		}
		
		/* Select the RTC Clock Source��ΪRTCѡ�е�ʱ��ԴΪLSE */
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);


		/* Enable the RTC Clock��ʹ��RTC��Ӳ��ʱ�� */
		RCC_RTCCLKCmd(ENABLE);
		
		/* Wait for RTC APB registers synchronisation ���ȴ�RTC��ؼĴ�������*/
		RTC_WaitForSynchro();
		
		
		//�رջ��ѹ���
		RTC_WakeUpCmd(DISABLE);
		
		//Ϊ���ѹ���ѡ��RTC���úõ�ʱ��Դ
		RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);
		
		//���û��Ѽ���ֵΪ�Զ����أ�д��ֵĬ����0
		RTC_SetWakeUpCounter(1-1);
		
		//���RTC�����жϱ�־
		RTC_ClearITPendingBit(RTC_IT_WUT);
		
		//ʹ��RTC�����ж�
		RTC_ITConfig(RTC_IT_WUT, ENABLE);

		//ʹ�ܻ��ѹ���
		RTC_WakeUpCmd(ENABLE);

		/* Configure EXTI Line22�������ⲿ�жϿ�����22 */
		EXTI_InitStructure.EXTI_Line = EXTI_Line22;			//��ǰʹ���ⲿ�жϿ�����22
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;		//�ж�ģʽ
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;		//�����ش����ж� 
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;			//ʹ���ⲿ�жϿ�����22
		EXTI_Init(&EXTI_InitStructure);
		
		NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;		//����RTC�����жϴ���
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x03;	//��ռ���ȼ�Ϊ0x3
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;		//��Ӧ���ȼ�Ϊ0x3
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//ʹ��
		NVIC_Init(&NVIC_InitStructure);	
	}
}

void rtc_my_date_set(uint32_t year, uint32_t month, uint32_t date, uint32_t week)
{
	RTC_DateStructure.RTC_Year = year;
	RTC_DateStructure.RTC_Month = month;
	RTC_DateStructure.RTC_Date = date;
	RTC_DateStructure.RTC_WeekDay = week;
	
	RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);
}

void rtc_my_time_set(uint32_t hours, uint32_t minutes, uint32_t seconds)
{
	
	RTC_TimeStructure.RTC_H12     = RTC_H12_PM;
	RTC_TimeStructure.RTC_Hours   = hours;
	RTC_TimeStructure.RTC_Minutes = minutes;
	RTC_TimeStructure.RTC_Seconds = seconds; 
	
	RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure); 
}

void rtc_my_alarmA_set(uint32_t hours, uint32_t minutes, uint32_t seconds)
{
	// ��������
    RTC_AlarmStructure.RTC_AlarmTime.RTC_H12 = RTC_H12_PM;
    RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours = hours ;
    RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = minutes;
    RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = seconds;
	RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay;
	
	rtc_alarm_set(RTC_AlarmStructure);	
}

void rtc_print_time(void)
{
	//��ȡ����
	RTC_GetDate(RTC_Format_BCD,&RTC_DateStructure);
	printf("20%02x/%02x/%02x Week%x\r\n",RTC_DateStructure.RTC_Year,RTC_DateStructure.RTC_Month,RTC_DateStructure.RTC_Date,RTC_DateStructure.RTC_WeekDay);

	//��ȡʱ��
	RTC_GetTime(RTC_Format_BCD,&RTC_TimeStructure);
	printf("%02x:%02x:%02x\r\n\n",RTC_TimeStructure.RTC_Hours,RTC_TimeStructure.RTC_Minutes,RTC_TimeStructure.RTC_Seconds);

}

#include "rtc.h"
#include <stdio.h>

static RTC_InitTypeDef   		RTC_InitStructure;
static RTC_TimeTypeDef  		RTC_TimeStructure;
static RTC_DateTypeDef			RTC_DateStructure;
static EXTI_InitTypeDef			EXTI_InitStructure;
static RTC_AlarmTypeDef 		RTC_AlarmStructure;
static NVIC_InitTypeDef   		NVIC_InitStructure;

volatile uint32_t alarm_flag = 0;	// 闹钟标志位，0表示没到点，1表示闹钟启动
volatile uint32_t g_rtc_wakeup_event=0;

void rtc_init(void)
{

	/* Enable the PWR clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	
	/* Allow access to RTC，RTC的电源开关使能 */
	PWR_BackupAccessCmd(ENABLE);

	//使能LSE（32.768KHz）
	RCC_LSEConfig(RCC_LSE_ON);
	
	/* Wait till LSE is ready ，等待LSE就绪*/  
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
	{
	}
	
	/* Select the RTC Clock Source，为RTC选中的时钟源为LSE */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);


	/* Enable the RTC Clock，使能RTC的硬件时钟 */
	RCC_RTCCLKCmd(ENABLE);
	
	/* Wait for RTC APB registers synchronisation ，等待RTC相关寄存器就绪*/
	RTC_WaitForSynchro();
	
	/* Configure the RTC data register and RTC prescaler */
	/* ck_spre(1Hz) = RTCCLK(LSE) /((uwAsynchPrediv + 1)*(uwSynchPrediv + 1))*/

	//RTC的硬件时钟=32768Hz/(0x7F+1)/(0xFF+1)=1Hz
	RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
	RTC_InitStructure.RTC_SynchPrediv = 0xFF;
	RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;//24小时格式
	RTC_Init(&RTC_InitStructure);


	/* Set the date: Monday February 22th 2021 */
	RTC_DateStructure.RTC_Year = 0x21;
	RTC_DateStructure.RTC_Month = RTC_Month_March;
	RTC_DateStructure.RTC_Date = 0x03;
	RTC_DateStructure.RTC_WeekDay = RTC_Weekday_Wednesday;
	RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);
	
	/* Set the time to 16h 28mn 30s PM， */
	RTC_TimeStructure.RTC_H12     = RTC_H12_PM;
	RTC_TimeStructure.RTC_Hours   = 0x10;
	RTC_TimeStructure.RTC_Minutes = 0x57;
	RTC_TimeStructure.RTC_Seconds = 0x00; 
	RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure);


	//关闭唤醒功能
	RTC_WakeUpCmd(DISABLE);
	
	//为唤醒功能选择RTC配置好的时钟源
	RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);
	
	//设置唤醒计数值为自动重载，写入值默认是0
	RTC_SetWakeUpCounter(1-1);
	
	//清除RTC唤醒中断标志
	RTC_ClearITPendingBit(RTC_IT_WUT);
	
	//使能RTC唤醒中断
	RTC_ITConfig(RTC_IT_WUT, ENABLE);

	//使能唤醒功能
	RTC_WakeUpCmd(ENABLE);

	/* Configure EXTI Line22，配置外部中断控制线22 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line22;			//当前使用外部中断控制线22
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;		//中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;		//上升沿触发中断 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;			//使能外部中断控制线22
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;		//允许RTC唤醒中断触发
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x03;	//抢占优先级为0x3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;		//响应优先级为0x3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//使能
	NVIC_Init(&NVIC_InitStructure);

}

void rtc_alarm_init(void)
{
	/* 允许RTC的A闹钟触发中断 */
	RTC_ITConfig(RTC_IT_ALRA, ENABLE);
	
	/* 清空标志位 */
	RTC_ClearFlag(RTC_FLAG_ALRAF);

	/*使能外部中断控制线17的中断*/
	EXTI_ClearITPendingBit(EXTI_Line17);
	EXTI_InitStructure.EXTI_Line = EXTI_Line17;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	/*使能闹钟的中断 */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void rtc_alarm_set(RTC_AlarmTypeDef RTC_AlarmStructure)
{
	/* 关闭闹钟，若不关闭，配置闹钟触发的中断有BUG，无论怎么配置，只要到00秒，则触发中断*/
	RTC_AlarmCmd(RTC_Alarm_A, DISABLE);
	
	/* 配置RTC的A闹钟，注：RTC的闹钟有两个，分别为闹钟A与闹钟B */
	RTC_SetAlarm(RTC_Format_BCD, RTC_Alarm_A, &RTC_AlarmStructure);
	
	/* 让RTC的闹钟A工作*/
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
	if(RTC_GetFlagStatus(RTC_FLAG_ALRAF)==SET)//ALARM A中断
	{
		alarm_flag = 1;
		RTC_ClearFlag(RTC_FLAG_ALRAF);//清除中断标志
	}   
	EXTI_ClearITPendingBit(EXTI_Line17);	//清除中断线17的中断标志 											 
}

void rtc_reload(void)
{
	//每次复位的时候，都取读取备份寄存器0
	if(0x1688!=RTC_ReadBackupRegister(RTC_BKP_DR0))
	{
		//rtc的初始化
		rtc_init();
	
		//往备份寄存器0写入数据为0x1688
		RTC_WriteBackupRegister(RTC_BKP_DR0, 0x1688);	
	}
	else
	{
		/* Enable the PWR clock */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
		
		/* Allow access to RTC，RTC的电源开关使能 */
		PWR_BackupAccessCmd(ENABLE);

		//使能LSE（32.768KHz）
		RCC_LSEConfig(RCC_LSE_ON);
		
		/* Wait till LSE is ready ，等待LSE就绪*/  
		while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
		{
		}
		
		/* Select the RTC Clock Source，为RTC选中的时钟源为LSE */
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);


		/* Enable the RTC Clock，使能RTC的硬件时钟 */
		RCC_RTCCLKCmd(ENABLE);
		
		/* Wait for RTC APB registers synchronisation ，等待RTC相关寄存器就绪*/
		RTC_WaitForSynchro();
		
		
		//关闭唤醒功能
		RTC_WakeUpCmd(DISABLE);
		
		//为唤醒功能选择RTC配置好的时钟源
		RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);
		
		//设置唤醒计数值为自动重载，写入值默认是0
		RTC_SetWakeUpCounter(1-1);
		
		//清除RTC唤醒中断标志
		RTC_ClearITPendingBit(RTC_IT_WUT);
		
		//使能RTC唤醒中断
		RTC_ITConfig(RTC_IT_WUT, ENABLE);

		//使能唤醒功能
		RTC_WakeUpCmd(ENABLE);

		/* Configure EXTI Line22，配置外部中断控制线22 */
		EXTI_InitStructure.EXTI_Line = EXTI_Line22;			//当前使用外部中断控制线22
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;		//中断模式
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;		//上升沿触发中断 
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;			//使能外部中断控制线22
		EXTI_Init(&EXTI_InitStructure);
		
		NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;		//允许RTC唤醒中断触发
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x03;	//抢占优先级为0x3
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;		//响应优先级为0x3
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//使能
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
	// 配置闹钟
    RTC_AlarmStructure.RTC_AlarmTime.RTC_H12 = RTC_H12_PM;
    RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours = hours ;
    RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = minutes;
    RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = seconds;
	RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay;
	
	rtc_alarm_set(RTC_AlarmStructure);	
}

void rtc_print_time(void)
{
	//获取日期
	RTC_GetDate(RTC_Format_BCD,&RTC_DateStructure);
	printf("20%02x/%02x/%02x Week%x\r\n",RTC_DateStructure.RTC_Year,RTC_DateStructure.RTC_Month,RTC_DateStructure.RTC_Date,RTC_DateStructure.RTC_WeekDay);

	//获取时间
	RTC_GetTime(RTC_Format_BCD,&RTC_TimeStructure);
	printf("%02x:%02x:%02x\r\n\n",RTC_TimeStructure.RTC_Hours,RTC_TimeStructure.RTC_Minutes,RTC_TimeStructure.RTC_Seconds);

}

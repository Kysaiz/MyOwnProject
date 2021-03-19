#ifndef __MYRTC_H
#define __MYRTC_H 	

#include "stm32f4xx.h" 
#include "sys.h"

extern void rtc_init(void);
extern void rtc_alarm_init(void);
extern void rtc_alarm_set(RTC_AlarmTypeDef RTC_AlarmStructure);
extern void RTC_WKUP_IRQHandler(void);
extern void RTC_Alarm_IRQHandler(void);
extern void rtc_reload(void);
extern void rtc_my_date_set(uint32_t year, uint32_t month, uint32_t date, uint32_t week);
extern void rtc_my_time_set(uint32_t hours, uint32_t minutes, uint32_t seconds);
extern void rtc_my_alarmA_set(uint32_t hours, uint32_t minutes, uint32_t seconds);
extern void rtc_print_time(void);

#endif

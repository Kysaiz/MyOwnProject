#include "iwdg.h" 

void iwdg_init(void)
{    	 
	//�����������Ź��ļĴ���
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	
	//����32��Ƶ���������Ź���Ӳ��ʱ��Ƶ��=32KHz/256=125Hz
	//���Ƕ������Ź�����125�μ���������1��ʱ��ĵ���
	IWDG_SetPrescaler(IWDG_Prescaler_256);
	
	
	//���ö������Ź��ļ���ֵ�����Ҫ��ʱʱ��Ϊ1�룬�������ֵ��дΪ124
	IWDG_SetReload(249-1);
	
	//ˢ�¼���ֵ
	IWDG_ReloadCounter();

	//ʹ�ܶ����Ŀ��Ź�
	IWDG_Enable();

}

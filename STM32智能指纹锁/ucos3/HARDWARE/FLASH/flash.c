#include "flash.h" 
#include "stdio.h"

void flash_init(void)
{
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_PGSERR | FLASH_FLAG_PGPERR |   FLASH_FLAG_PGAERR | FLASH_FLAG_WRPERR |    FLASH_FLAG_OPERR | FLASH_FLAG_EOP); 

	FLASH_Lock();
}

void flash_erase(int32_t num)
{
	flash_init();
	FLASH_Unlock();
	if (num == 0)
	{
		if (FLASH_EraseSector(FLASH_Sector_0, VoltageRange_3) != FLASH_COMPLETE)
		{
			printf("FLASH_EraseSector0 error\r\n");
			while(1);
		}
	}
	else if (num == 1)
	{
		if (FLASH_EraseSector(FLASH_Sector_1, VoltageRange_3) != FLASH_COMPLETE)
		{
			printf("FLASH_EraseSector1 error\r\n");
			while(1);
		}
	}
	else if (num == 2)
	{
		if (FLASH_EraseSector(FLASH_Sector_2, VoltageRange_3) != FLASH_COMPLETE)
		{
			printf("FLASH_EraseSector2 error\r\n");
			while(1);
		}
	}
	else if (num == 3)
	{
		if (FLASH_EraseSector(FLASH_Sector_3, VoltageRange_3) != FLASH_COMPLETE)
		{
			printf("FLASH_EraseSector3 error\r\n");
			while(1);
		}
	}
	else if (num == 4)
	{
		if (FLASH_EraseSector(FLASH_Sector_4, VoltageRange_3) != FLASH_COMPLETE)
		{
			printf("FLASH_EraseSector4 error\r\n");
			while(1);
		}
	}
	else if (num == 5)
	{
		if (FLASH_EraseSector(FLASH_Sector_5, VoltageRange_3) != FLASH_COMPLETE)
		{
			printf("FLASH_EraseSector5 error\r\n");
			while(1);
		}
	}
	else if (num == 6)
	{
		if (FLASH_EraseSector(FLASH_Sector_6, VoltageRange_3) != FLASH_COMPLETE)
		{
			printf("FLASH_EraseSector6 error\r\n");
			while(1);
		}
	}
	else if (num == 7)
	{
		if (FLASH_EraseSector(FLASH_Sector_7, VoltageRange_3) != FLASH_COMPLETE)
		{
			printf("FLASH_EraseSector7 error\r\n");
			while(1);
		}
	}
	FLASH_Lock();
}

//写入字（32位）
void WriteFlashOneWord(uint32_t WriteAddress, uint32_t WriteData)
{
	//解锁FLASH，允许对FLASH进行修改
	flash_init();
	FLASH_Unlock();
	//数据编程
	//向扇区4首地址0x8010000写入数据0x12345678
	if (FLASH_ProgramWord(WriteAddress, WriteData) != FLASH_COMPLETE)
	{
		printf("FLASH_ProgramWord error\r\n");
		while(1);
	}
	
	//锁定FLASH，不允许对FLASH进行修改
	FLASH_Lock();
	

}


//写入字符串（8位）
void WriteFlashData(uint32_t WriteAddress, uint8_t data[], uint32_t num)
{
	uint32_t i = 0;
    uint16_t temp = 0;
	
	
	//解锁FLASH，允许对FLASH进行修改
	flash_init();
	FLASH_Unlock();
	
	for(i=0; i<num; i++)
	{
		temp = (uint16_t)data[i];
		if (FLASH_COMPLETE != FLASH_ProgramByte((WriteAddress+i), temp))//写入数据
		{
			printf("FLASH_ProgramWord error\r\n");
			while(1);
		}
	}


	//锁定FLASH，不允许对FLASH进行修改
	FLASH_Lock();
}


//读取一个字（32位）的数据
void ReadFlashOneWord(uint32_t WriteAddress,uint32_t num)
{
	uint32_t i = 0;
	uint32_t d=0;
	
	
	for(i=0;i<num*4;i=i+4)
	{
		//读取数据
		d = *(volatile uint32_t *)(WriteAddress+i);
		
		printf("d=%X\r\n",d);	
	}
}

//读取字符串（8位）
void ReadFlashData(uint32_t WriteAddress,char *buf,uint32_t num)
{
	uint32_t i = 0;
	uint32_t d=0;

	for(i=0;i<num;i++)
	{
		//读取数据
		d = *(volatile uint32_t *)(WriteAddress+i);
		
		buf[i]=d;
	}
}

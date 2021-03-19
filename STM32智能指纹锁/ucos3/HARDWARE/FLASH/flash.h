#ifndef __FLASH_H__
#define __FLASH_H__

#include "stm32f4xx.h" 
#include "sys.h"

#define FLASH3_ADDR 0x0800C000	//����3
#define FLASH4_ADDR 0x08010000	//����4
#define FLASH5_ADDR 0x08020000	//����5
#define FLASH6_ADDR 0x08040000	//����6
#define FLASH7_ADDR 0x08060000	//����7

void flash_init(void);
void flash_erase(int32_t num);
void WriteFlashOneWord(uint32_t WriteAddress, uint32_t WriteData);
void WriteFlashData(uint32_t WriteAddress, uint8_t data[], uint32_t num);
void ReadFlashOneWord(uint32_t WriteAddress,uint32_t num);
void ReadFlashData(uint32_t WriteAddress,char *buf,uint32_t num);

#endif

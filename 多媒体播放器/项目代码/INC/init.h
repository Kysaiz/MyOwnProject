#ifndef _INIT_H
#define _INIT_H

// 打开lcd
int lcd_open();

// lcd内存映射
int mmap_lcd();

// 打开触摸屏设备
int touch_open();

#endif
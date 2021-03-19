#include "../INC/myhead.h"

#define     TOUCH_PATH      "/dev/input/event0"
#define     LCD_PATH        "/dev/fb0"

// 打开lcd
int lcd_open()
{
    lcd_fd = open(LCD_PATH, O_RDWR);
	
	if(lcd_fd<0)
	{
			printf("open lcd error\n");
			return -1;
	}
    return lcd_fd;
}

// lcd内存映射
int mmap_lcd()
{
    mmap_fd  = (int *)mmap(	    NULL, 					//映射区的开始地址，设置为NULL时表示由系统决定映射区的起始地址
                                800*480*4, 				//映射区的长度
                                PROT_READ|PROT_WRITE, 	//内容可以被读取和写入
                                MAP_SHARED,				//共享内存
                                lcd_fd, 				//有效的文件描述词
                                0						//被映射对象内容的起点
                            );
}

// 打开触摸屏设备
int touch_open()
{
    // 访问触摸屏设备
    ts_fd = open(TOUCH_PATH, O_RDONLY);
    if(ts_fd < 0)
    {
        printf("open touch error!\n");
        return -1;
    }

    return ts_fd;
}
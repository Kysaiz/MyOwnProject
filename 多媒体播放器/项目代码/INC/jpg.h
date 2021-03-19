#ifndef __LCD_H__
#define __LCD_H__

// 显示一张小于800*480的JPG，可以指定开始的坐标值
int lcd_draw_jpg(unsigned int x,unsigned int y,const char *pjpg_path);

//获取jpg文件的大小
unsigned long file_size_get(const char *pfile_path);

/*
	最邻插值图像缩放算法
	可实现图像的放大和缩小
	参数:	src---转换前的图像宽度高度
			dst---转换后的图像宽度高度
			pre_lcd---要处理的图像数据数组
			format---图像格式bmp或jpeg
	返回值: 返回处理后的图像数据数组
*/
unsigned char *insert_near(unsigned char pre_lcd[], int src_width, int src_height, int dst_width, int dst_height);

// 利用最邻插值图像缩放算法将JPG文件缩放成800*480大小显示
int show_jpg(const char *pathname);

#endif
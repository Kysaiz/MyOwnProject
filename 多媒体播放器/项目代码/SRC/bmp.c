#include "../INC/myhead.h"
 
typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;
typedef long LONG;
 
typedef struct tagBITMAPINFOHEADER
{
	DWORD      biSize;
	LONG       biWidth;
	LONG       biHeight;
	WORD       biPlanes;
	WORD       biBitCount;
	DWORD      biCompression;
	DWORD      biSizeImage;
	LONG       biXPelsPerMeter;
	LONG       biYPelsPerMeter;
	DWORD      biClrUsed;
	DWORD      biClrImportant;
} BITMAPINFOHEADER;
 
int show_bmp(char *name) //显示bmp图片
{
	BITMAPINFOHEADER info_head;
	unsigned char *bmp_buf;
 
	int fd_bmp = open(name, O_RDWR);
	if (fd_bmp < 0)
	{
		perror("bmp open fail");
		return -1;
	}
 
	//跳过14个字节的头文件信息，直接读取BITMAPINFOHEADER结构体
	lseek(fd_bmp, 14, SEEK_SET);
	read(fd_bmp, &info_head, sizeof(BITMAPINFOHEADER));
 
	if ((info_head.biHeight != 480) || (info_head.biWidth != 800))
	{
		printf("This bmp_image is (%d*%d),I have changed it to 800*480.\n", info_head.biWidth, info_head.biHeight);
	}
 
	unsigned char bmp_prebuf[info_head.biWidth * info_head.biHeight *3];
	read(fd_bmp, bmp_prebuf, sizeof(bmp_prebuf));
	close(fd_bmp);
 
	//用最邻近插值来实现图像的变换
	bmp_buf = insert_near(bmp_prebuf, info_head.biWidth, info_head.biHeight, 800, 480);
 
	/*由于bmp图片只有三个字节数据，而且刷图顺序是从下往上，要进行数据转换*/
	int pixel, line;
	for (line = 0; line < 480; ++line)
	{
		for (pixel = 0; pixel < 800; ++pixel)
		{
			int s = (line*800 + pixel) * 3;
			/* *(mmap_fd + pixel + (479-line)*800) = */
            BG_TMP[479-line][pixel] = (bmp_buf[s+0]) | (bmp_buf[s+1]<<8) | (bmp_buf[s+2]<<16);

		}
	}
 
	return 0;
}

#include "../INC/myhead.h"

// static unsigned char g_color_buf[800 * 480 * 4]={0};

// 显示一张小于800*480的JPG，可以指定开始的坐标值
int lcd_draw_jpg(unsigned int x,unsigned int y,const char *pjpg_path)  
{
	printf("JPG_PATH = %s\n", pjpg_path);

	/*定义解码对象，错误处理对象*/
	struct 	jpeg_decompress_struct 	cinfo;
	struct 	jpeg_error_mgr 			jerr;	
	
	unsigned char g_color_buf[800 * 480 * 4]={0};
	unsigned char 	*pcolor_buf = g_color_buf;
	char 	*pjpg;
	
	unsigned int 	i=0;
	unsigned int	color =0;
	
	unsigned int 	x_s = x;
	unsigned int	y_n	= y;
	unsigned int	x_n	= x;
	
			 int	lcd_buf[480][800] = {0};
			 int	jpg_fd;
	unsigned int 	jpg_size;


	if(pjpg_path!=NULL)
	{
		/* 申请jpg资源，权限可读可写 */	
		jpg_fd=open(pjpg_path,O_RDWR);
		
		if(jpg_fd == -1)
		{
		   printf("open %s error\n",pjpg_path);
		   
		   return -1;	
		}	
		
		/* 获取jpg文件的大小 */
		jpg_size=file_size_get(pjpg_path);	
		if(jpg_size<3000)
			return -1;
		
		/* 为jpg文件申请内存空间 */	
		pjpg = malloc(jpg_size);

		/* 读取jpg文件所有内容到内存 */		
		read(jpg_fd,pjpg,jpg_size);
	}
	else
	{
		printf("jpeg path error : %s \n", pjpg_path );
		return -1;
	}

	/*注册出错处理*/
	cinfo.err = jpeg_std_error(&jerr);

	/*创建解码*/
	jpeg_create_decompress(&cinfo);

	/*直接解码内存数据*/		
	jpeg_mem_src(&cinfo,pjpg,jpg_size);
	
	/*读文件头*/
	jpeg_read_header(&cinfo, TRUE);

	/*开始解码*/
	jpeg_start_decompress(&cinfo);

	/*读解码数据*/
	while (cinfo.output_scanline < cinfo.output_height)
	{
		pcolor_buf = g_color_buf;
		
		/* 读取jpg一行的rgb值 */
		jpeg_read_scanlines(&cinfo, &pcolor_buf, 1);
		
		for (i = 0; i < cinfo.output_width; i ++)
		{		
			/* 获取rgb值 */
			color = 		*(pcolor_buf+2);
			color = color | *(pcolor_buf+1)<<8;
			color = color | *(pcolor_buf)<<16;	
			
			/* 显示像素点 */
			*(mmap_fd + y_n * 800 + x_n) = color;
			
			pcolor_buf += 3;
			x_n ++;
		}

		/* 换行 */
		y_n ++;		
		x_n = x_s;
	}

	printf("mmap is over!\n");
			
	/*解码完成*/
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	if(pjpg_path!=NULL)
	{
		/* 关闭jpg文件 */
		close(jpg_fd);	
		
		/* 释放jpg文件内存空间 */
		free(pjpg);		
	}
	
	return 0;
}

//获取jpg文件的大小
unsigned long file_size_get(const char *pfile_path)
{
	unsigned long filesize = -1;	
	struct stat statbuff;
	
	if(stat(pfile_path, &statbuff) < 0)
	{
		return filesize;
	}
	else
	{
		filesize = statbuff.st_size;
	}
	
	return filesize;
}

// 最邻插值图像缩放算法
unsigned char *insert_near(unsigned char pre_lcd[], int src_width, int src_height, int dst_width, int dst_height)
{
	int byte_size = 3;
 
	int hadd, wadd; //用于图像精确四舍五入的参数
	unsigned char *buffer = (unsigned char *)malloc(dst_width*dst_height*byte_size); //存储转化后的图像数据
	memset(buffer, 0, 800*480*byte_size); //对数组清零
 
	float fw = (float)dst_width / src_width; //计算图像宽度缩放比例
	float fh = (float)dst_height / src_height; //计算图像高度缩放比例
 
	float fw_yu = fw - (int)fw; //求出缩放比例的小数部分
	float fh_yu = fh - (int)fh;
 
	if (fw_yu > 0.5) wadd = 1; //对其小数部分进行四舍五入
	else wadd = 0;
 
	if (fh_yu > 0.5) hadd = 1;
	else hadd = 0;
 
	int x,y;
 
	int hnum;
	for (hnum = 0; hnum < dst_height; ++hnum) //按照从左到右，从上到下的顺序进行转换
	{
		y = (int)(hnum / fh)+hadd;   //计算当前临近坐标的y值
		
		int wnum;
		for (wnum = 0; wnum < dst_width; ++wnum)
		{
			x = (int)(wnum / fw)+wadd; //计算当前临近坐标的x值
 
			long pixel_point = hnum*dst_width*byte_size + wnum*byte_size; //计算转化后图像的数组偏移量
 
			//RGB数据依次进行数据变换
			buffer[pixel_point] = pre_lcd[y * src_width * byte_size + x * byte_size];
			buffer[pixel_point + 1] = pre_lcd[y * src_width * byte_size + x * byte_size + 1];
			buffer[pixel_point + 2] = pre_lcd[y * src_width * byte_size + x * byte_size + 2];
		}
	}
 
	printf("Change Finished!!!\n");
 
	return buffer;
}

// 利用最邻插值图像缩放算法将JPG文件缩放成800*480大小显示
int show_jpg(const char *pathname)
{
	printf("JPG_PATH = %s\n", pathname);

	unsigned int pre_lcd[480][800];
	memset(pre_lcd, 0, sizeof(pre_lcd));

	unsigned char *buffer;
	unsigned char *jpeg_buff;
	unsigned char *pre_buff;
	int x = 0;
	FILE *infile; 
 
	/*1.为JPEG对象分配空间并初始化*/
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
 
	/*
		将错误处理结构体对象绑定在JPEG对象上
		jpeg_std_error在程序出现错误的时候自动运行exit退出
	*/
	cinfo.err = jpeg_std_error(&jerr);
 
	/*初始化cinfo结构*/
	jpeg_create_decompress(&cinfo);
 
	/*2.指定解压缩数据源*/
	if ((infile = fopen(pathname, "r")) == NULL)
	{
		printf("open %s fail\n", pathname);
		return -1;
	}
	jpeg_stdio_src(&cinfo, infile);
 
	/*3.获取文件信息*/
	(void)jpeg_read_header(&cinfo, TRUE);
	printf("width = %d, height = %d\n", cinfo.image_width, cinfo.image_height);
	if ((cinfo.image_height != 480) || (cinfo.image_width != 800))
	{
		printf("This jpeg_image is not 800*480,I need change it\n");
	}
 
	/*4.解压图片，以行为单位*/
	jpeg_start_decompress(&cinfo);
 
	/*5.取出数据*/
	int row_stride = cinfo.output_width * cinfo.output_components; //计算每行图像所需的字节大小
	buffer = malloc(row_stride);
	pre_buff = malloc(cinfo.output_width * cinfo.output_components * cinfo.output_height); //很关键，不能少
 
	while (cinfo.output_scanline < cinfo.output_height)
	{
		(void)jpeg_read_scanlines(&cinfo, (JSAMPARRAY)&buffer, 1);

		int i;
		for (i = 0; i < (cinfo.output_width * cinfo.output_components); ++i)
		{
			pre_buff[(cinfo.output_scanline - 1) * cinfo.output_width  * cinfo.output_components + i] = buffer[i];
		}
	}
 
	//利用最邻近插值进行图像变换
	jpeg_buff = insert_near(pre_buff, cinfo.image_width, cinfo.image_height, 800, 480);
	
	// bzero(BG_TMP, 0, sizeof(BG_TMP));
	int i;
	for (i = 0; i < 480; ++i)
	{
		int j;
		for (j = 0, x = 0; j < 800; ++j)
		{
			/* *(mmap_fd+j+i*800) = */
			BG_TMP[i][j] = jpeg_buff[i*800*3 + x + 2] | jpeg_buff[i*800*3 + x + 1] << 8 | jpeg_buff[i*800*3 + x] << 16;
			x += 3;
		}
	}
 
	/*6.解压缩完毕*/
	(void)jpeg_finish_decompress(&cinfo);
 
	/*7.释放资源*/
	jpeg_destroy_decompress(&cinfo);
	fclose(infile);
 
	/*8.退出程序*/
	jpeg_destroy_decompress(&cinfo);
 
	return 0;
}
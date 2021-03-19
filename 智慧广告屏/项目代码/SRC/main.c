#include "../INC/myhead.h"

int main(int argc,char **argv)
{
    lcd_open();
    mmap_lcd();
    touch_open();

    // 字体函数初始化
    Init_Font();

    // 初始化媒体文件、BMP+JPG图片文件的链表
    // 开机从dat文件中读取列表信息到链表中
    PATH_LIST *MEDIA_HEAD = list_read("LIST_MEDIA.dat");
    PATH_LIST *PIC_HEAD = list_read("LIST_PIC.dat");
    
    // 从指定的资源文件夹中读取文件
    // dir_new("/Kysaiz/project1/media/", MEDIA_HEAD, PIC_HEAD);
    // dir_new("/Kysaiz/project1/album/", MEDIA_HEAD, PIC_HEAD);

    // 菜单界面
    lcd_draw_jpg(0, 0, PATH_MENU);

    // 开机播放后台音乐
    system("killall -9 mplayer");
    // system("mplayer -af volume=-33 -softvol -softvol-max 200 -quiet -slave ./test.mp3 -loop 0 &");

    // 打开触摸屏设备
    int ts_x = 0, ts_y = 0;
    struct input_event buf;

    // 无限循环
    while (1)
    {
        read(ts_fd, &buf, sizeof(buf));

        // 当检测到坐标变化时，读取X,Y坐标
        if (buf.type == EV_ABS && buf.code == ABS_X)
        {
            ts_x = buf.value * 800 / 1024;
        }
            
        if (buf.type == EV_ABS && buf.code == ABS_Y)
        {
            ts_y = buf.value * 480 / 600;        
        }

        // 当检测到松开指令时，对所在坐标进行判断
        if(buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
        {
            printf("main(x, y) = (%d, %d)\n", ts_x, ts_y);
            if(ts_x < 400 && ts_y < 240) // 左上，相册
            {
                album(PIC_HEAD);
            }
            
            if (ts_x > 400 && ts_y < 240) // 右上，媒体
            {
                media(MEDIA_HEAD);
            }
            
            if(ts_x < 400 && ts_y > 240) // 左下，资源更新
            {
                SourceUpdate(MEDIA_HEAD, PIC_HEAD);
            }

            if(ts_x > 400 && ts_y > 240) // 右下，幸运抽奖
            {
                game(PIC_HEAD);
            }
        }        
    }
    
    media_quit();
    delete_list_node(MEDIA_HEAD);
    delete_list_node(PIC_HEAD);

    close(ts_fd);
    close(lcd_fd);
    munmap(mmap_fd, 800*480*4);

    UnInit_Font();

	return 0;	
}

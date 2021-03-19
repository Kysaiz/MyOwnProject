#include "../INC/myhead.h"

void *ts_reading(void *arg)
{
    ts_x = 0;
    ts_y = 0;
    TS_X = 0;
    TS_Y = 0;
    while (1)
    {
        read(ts_fd, &buf, sizeof(buf));
        // usleep(10000);
        // 当检测到坐标变化时，读取X,Y坐标
        if (buf.type == EV_ABS && buf.code == ABS_X)
        {
            TS_X = buf.value * 800 / 1024;
        }
            
        if (buf.type == EV_ABS && buf.code == ABS_Y)
        {
            TS_Y = buf.value * 480 / 600;        
        }

        if (buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
        {
            ts_x = TS_X;
            ts_y = TS_Y;
        }
        
    }
}

int main(int argc,char **argv)
{
    lcd_open();
    mmap_lcd();
    touch_open();

    // 字体函数初始化
    Init_Font();

    // 初始化媒体文件、BMP+JPG图片文件的链表
    // 开机从dat文件中读取列表信息到链表中
    MEDIA = list_read("LIST_MEDIA.dat");
    PIC = list_read("LIST_PIC.dat");
    VIDEO = list_read("LIST_VIDEO.dat");
    
    // 从指定的资源文件夹中读取文件
    // dir_new("/Kysaiz/project1/media/", MEDIA_HEAD, PIC_HEAD);
    // dir_new("/Kysaiz/project1/album/", MEDIA_HEAD, PIC_HEAD);

    // 菜单界面
    lcd_draw_jpg(0, 0, PATH_MENU);

    // 开机播放后台音乐
    system("killall -9 mplayer");
    // system("mplayer -af volume=-33 -softvol -softvol-max 200 -quiet -slave ./test.mp3 -loop 0 &");

    // 初始化一个属性变量，并将分离属性加入该变量
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_t tid_ts;
    pthread_create(&tid_ts, &attr, ts_reading, NULL);

    // 无限循环
    while (1)
    {
        // 当检测到松开指令时，对所在坐标进行判断
        if (buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
        {
            printf("main(x, y) = (%d, %d)\n", ts_x, ts_y);
            if(ts_x < 400 && ts_y > 0 && ts_y < 240) // 左上，相册
            {
                album();
            }
            else if (ts_x > 400 && ts_y < 240) // 右上，媒体
            {
                media();
            }
            else if(ts_x < 400 && ts_y > 240) // 左下，资源更新
            {
                SourceUpdate();
            }
            else if(ts_x > 400 && ts_y > 240) // 右下，幸运抽奖
            {
                game();
            }
            memset(&buf, 0, sizeof(buf));
        }        
    }
    
    media_quit();
    delete_list_node(MEDIA);
    delete_list_node(PIC);
    delete_list_node(VIDEO);

    close(ts_fd);
    close(lcd_fd);
    munmap(mmap_fd, 800*480*4);

    UnInit_Font();

	return 0;	
}

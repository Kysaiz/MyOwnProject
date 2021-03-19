#include "../INC/myhead.h"

FILE *video_mp;
_Bool Flag = 1;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void handler_lock()
{
    pthread_mutex_lock(&mutex);
    printf("Lock!\n");
}

void handler_unlock()
{
    pthread_mutex_unlock(&mutex);
    printf("Unlock!\n");
}

// 亮度条
void brightness_bar(int len)
{
    int x, y;
    for (x = 700; x < 750; x++)
    {
        for (y = 100; y < len; y++)
        {
            *(mmap_fd + y * 800 + x) = 0x00FF0000;
        }   
    }

    for (x = 700; x < 750; x++)
    {
        for (y = len; y < 300; y++)
        {
            *(mmap_fd + y * 800 + x) = 0x000000FF;
        }
        
    }    
}

// 视频进度条
void *jdt_display(double perc)
{
    printf("perc = %f\n", perc);
    int x, y;
    for (x = 0; x < (int)perc * 8; x++)
    {
        for (y = 401; y < 410; y++)
        {
            *(mmap_fd + y * 800 + x) = 0x00FFFF00;
        }
    }
    
    for (x = (int)perc * 8; x < 800; x++)
    {
        for (y = 401; y < 410; y++)
        {
            *(mmap_fd + y * 800 + x) = 0x0000FFFF;
        }
        
    }
    
}

// 音量条
void *ylt_display(int x)
{
    int i, y;
    for (i = 520; i < x; i++)
    {
        for (y = 435; y < 450; y++)
        {
            *(mmap_fd + y * 800 + i) = 0x00FF0000;
        }   
    }

    for (i = x; i < 680; i++)
    {
        for (y = 435; y < 450; y++)
        {
            *(mmap_fd + y * 800 + i) = 0x000000FF;
        }
        
    }
    
}

// 打印视频控制界面
void video_menu(_Bool play_flag, int volume)
{
    // 打印到显示屏
    if (play_flag)
    {
        lcd_draw_jpg(0, 410, PATH_VIDEO2);
    }
    else
    {
        lcd_draw_jpg(0, 410, PATH_VIDEO1);
    }
    ylt_display(volume);
}

// 获取当前播放时间和总时间
void video_timeget(double *now, double *len)
{
    char buff[1024];
    system("echo get_time_length > ./tmp/mplayer_fifo1");
    fgets(buff, 1024, video_mp); // 从管道中获得的数据返回    

    char *pos = strrchr(buff, '=');
    pos += 1;
    *len = atof(pos); // 总时间

    bzero(buff, 1024); // 清空用户的缓冲区
    system("echo get_time_pos > ./tmp/mplayer_fifo1"); // 发送命令从播放器中获得当前的百分比时间
    fgets(buff, 1024, video_mp); // 从管道中获得的数据返回

    pos = strrchr(buff, '=');
    pos += 1;
    *now = atof(pos); // 当前进度时间
    printf("now percent||len = %f||%f\n", *now, *len);
}

// 视频进度条
void *video_bar(void *arg)
{
    char buff[1024];
    double len, now = 0;

    system("echo get_time_length > ./tmp/mplayer_fifo1");
    fgets(buff, 1024, video_mp); // 从管道中获得的数据返回
    while (!strstr(buff , "ANS_LENGTH"))
    {
        printf("**%s\n", buff); // 输出得到的数据
        fgets(buff, 1024, video_mp);
    }

    char *pos = strrchr(buff, '=');
    pos += 1;
    len = atof(pos);
    printf("len = %f\n", len);

    // 获取百分比
    while (len - now > 1)
    {
        handler_lock();
        // 获取当前状态
        bzero(buff, 1024); // 清空用户的缓冲区

        system("echo get_time_pos > ./tmp/mplayer_fifo1"); // 发送命令从播放器中获得当前的百分比时间

        fgets(buff, 1024, video_mp); // 从管道中获得的数据返回
        while (!strstr(buff , "ANS_TIME_POSITION"))
        {
            printf("**%s\n", buff); // 输出得到的数据
            fgets(buff, 1024, video_mp);
        }

        printf("buf = %s\n", buff);
        char *pos = strrchr(buff, '=');
        pos += 1;
        now = atof(pos); // 当前进度的百分比
        printf("now percent||len = %f||%f\n", now, len);
        jdt_display((now / len) * 100);
        handler_unlock();

        sleep(1);
    }

    system("killall -9 mplayer");
    Flag = 0;
    printf("This Video is end!\n");
}

// 视频播放
int video(PATH_LIST *VIDEO)
{
    system("killall -9 mplayer");
    int mode = 0;
    
    // 配置播放命令
    char str[200];
    sprintf(str, "mplayer -quiet -slave -input file=./tmp/mplayer_fifo1 -zoom -x 800 -y 400 %s%s &", VIDEO->path, VIDEO->name);
    // sprintf(str, 
    //     "mplayer -quiet -slave -input file=./tmp/mplayer_fifo1 %s\"%s\" &",
    //     VIDEO->path, VIDEO->name);

    // 初始化一个属性变量，并将分离属性加入该变量
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    
    // 用分离属性产生一条新线程，控制视频进度条
    pthread_t tid;
    pthread_create(&tid, &attr, video_bar, NULL);

    // 播放视频
    video_mp = popen(str, "r");
    system("echo \"volume 50 1\" > ./tmp/mplayer_fifo1");

    // 背景显示
    lcd_draw_jpg(0, 410, PATH_VIDEO2);
    ylt_display(600);

    int volume = 600;
    float volume_r = 0, bright = 0;
    _Bool play_flag = 1;
    // 无限循环
    while (Flag)
    {
        if (TS_X > 650 && TS_X < 750 && TS_Y > 100 && TS_Y < 300) // 判断是否为亮度条区域
        if (buf.type == EV_ABS)
        {
            bright = (TS_Y - 200) * 0.5;
            char br[100];
            sprintf(br, "echo \"brightness %d 1\" > ./tmp/mplayer_fifo1", (int)bright);
            system(br);
            brightness_bar(TS_Y);
            TS_Y = 0;
        }

        if (TS_Y > 420 && TS_Y < 460 && TS_X > 520 && TS_X < 680) // 判断是否为音量条区域
        if (buf.type == EV_ABS)
        {
            if (!play_flag)
            {
                handler_unlock(); // 恢复正在运行的进度条线程
                play_flag = 1;
            }
            volume = TS_X;
            volume_r = (TS_X - 520) * 0.625;
            printf("volume is changing!\n");
            char vo[100];
            sprintf(vo, "echo \"volume %d 1\" > ./tmp/mplayer_fifo1", (int)volume_r);
            system(vo); 
        
            video_menu(play_flag, volume);
        }

        if (TS_X > 0 && TS_Y > 390 && TS_Y < 415) // 判断是否为进度条区域
        {
            if (buf.type == EV_ABS)
            {
                if (play_flag)
                {
                    printf("pause!\n");
                    system("echo pause > ./tmp/mplayer_fifo1");
                    handler_lock(); // 暂停正在运行的进度条线程
                    play_flag = 0;

                }
                
                int perc = TS_X / 8;
                jdt_display(perc);
            }

            if (buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
            {
                char buf_seek[100];
                
                printf("seeking!!TS_X = %d, TS_Y = %d\n", TS_X, TS_Y);
                sprintf(buf_seek, "echo \"seek  %d  1\" > ./tmp/mplayer_fifo1", ts_x / 8);
                system(buf_seek);
                
                if (!play_flag)
                {
                    printf("pause!\n");
                    system("echo pause > ./tmp/mplayer_fifo1");
                    handler_unlock(); // 恢复正在运行的进度条线程
                    play_flag = 1;
                }

                video_menu(play_flag, volume);
                TS_X = 0;
                TS_Y = 0;
            }
        }

        // 当检测到松开指令时，对所在坐标进行判断
        if(buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
        {
            printf("video(x, y) = (%d, %d)\n", ts_x, ts_y);
            if (ts_y > 410)
            if (ts_x < 100) // 切换上一个
            {
                //system("killall -9 mplayer");
                mode = 2;
                handler_unlock();
                break;
            }
            else if (ts_x < 200) // 后退5s
            {   
                printf("seek -5!\n");
                system("echo seek -5 > ./tmp/mplayer_fifo1");

                if (!play_flag)
                {
                    handler_unlock(); // 恢复正在运行的进度条线程
                    play_flag = 1;
                }
                video_menu(play_flag, volume);
            }
            else if (ts_x < 300) // 播放/暂停
            {
                if (play_flag)
                {
                    printf("pause!\n");
                    system("echo pause > ./tmp/mplayer_fifo1");
                    handler_lock(); // 暂停正在运行的进度条线程
                    play_flag = 0;
                }
                else
                {
                    printf("continue!\n");
                    system("echo pause > ./tmp/mplayer_fifo1");
                    handler_unlock(); // 恢复正在运行的进度条线程
                    play_flag = 1;
                }

                video_menu(play_flag, volume);
            }
            else if (ts_x < 400) // 快进5s
            {
                double len, now;
                video_timeget(&now, &len);
                if (now + 5 < len)
                {
                    printf("seek +5!\n");
                    system("echo seek +5 > ./tmp/mplayer_fifo1"); 
                    if (!play_flag)
                    {
                        handler_unlock(); // 恢复正在运行的进度条线程
                        play_flag = 1;
                    }
                    video_menu(play_flag, volume);

                }
            }
            else if (ts_x < 500) // 切换下一个
            {
                //system("killall -9 mplayer");
                mode = 1;
                handler_unlock();
                break;
            }
            else if (ts_y > 410 && ts_x > 740) // 返回上层
            {
                //system("killall -9 mplayer");
                mode = 3;
                handler_unlock();
                break;
            }
            ts_x = 0;
            ts_y = 0;
        }
    }

    system("killall -9 mplayer");
    handler_unlock();
    pthread_cancel(tid);
    pclose(video_mp);
    usleep(10000);
    if (Flag)
    {
        return mode;
    }
    else
    {
        Flag = 1;
        sleep(1);
        return 1;
    }
}

// 切换视频
int video_change(PATH_LIST *p)
{
    PATH_LIST *last;
    int m = video(p);

    while (1)
    {
        last = VIDEO->prev;
        if (m == 2) // mode=2时，切换上一个视频
        {
            if (p == VIDEO->next) // 当前正在播放第一个
            {
                p = last;
                printf("LAST ONE\n");
            }
            else
            {
                p = p->prev;
                printf("FRONT ONE\n");
            }
            m = video(p);
        }
        else if (m == 1) // mode=1时，切换下一个视频
        {
            if (p == last)
            {
                p = VIDEO->next;
                printf("FIRST ONE\n");
            }
            else
            {
                p = p->next;
                printf("NEXT ONE\n");
            }
            m = video(p);
        }
        else if (m == 3) // mode=3时，返回上一层
        {
            lcd_draw_jpg(0, 0, PATH_VIDEO0);
            ts_x = 0;
            ts_y = 0;
            return 1;
        }
    }
    
}

// 在列表中选择打开视频
int video_select()
{
    ts_x = 0;
    ts_y = 0;

    if (VIDEO->next == VIDEO)
    {
        show_color(0x00000000);
        Display_characterX(0, 0, "当前没有视频，请进行资源更新！", 0x000000FF, 3);

        // 点击任意处返回
        while (1)
        {
            if (buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
            {
                lcd_draw_jpg(0, 0, PATH_MENU);
                return -2;
            }
        }
    }

    PATH_LIST *playing = VIDEO->next;
    
    int cnt_VIDEO = len_list_node(VIDEO);
    printf("\n\ncnt_VIDEO = %d\n\n", cnt_VIDEO);

    // 背景打印
    lcd_draw_jpg(0, 0, PATH_VIDEO0);

    int pos = pos_list(VIDEO, playing);
    int j = ((pos-1) / 10) * 10;
    printf("j = %d\n", j);
    int page_VIDEO = (cnt_VIDEO / 10) + 1;

    // 打印标题
    char title[100];
    sprintf(title, "视频文件 (%d/%d)", j/10 + 1, page_VIDEO);
    Display_characterX(0, 0, title, 0x000000FF, 1);

    int y = 0;
    PATH_LIST *q;
    // 打印文字
    do
    {
        j++;
        y += 35;
        q = list_getElem(VIDEO, j);
        Display_characterX(0, y, q->name, 0x00000000, 1);
    }while ((j < cnt_VIDEO) && (j % 10 != 0));
    
    j = 0;

    while (1)
    {
        // 当检测到松开指令时，对所在坐标进行判断
        if (buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
        {
            printf("video_select(x, y) = (%d, %d)\n", ts_x, ts_y);
            playing = VIDEO->next;
            // 进行坐标判断
            if (ts_x > 0 && ts_x < 400)
            {
                int num = ts_y / 35;
                if (num <= cnt_VIDEO - j) // 打开视频
                {
                    int i, k;
                    for (k = 1; k < (j / 10); k++)
                    {
                        playing = playing->next;
                    }

                    for (i = 1; i < num; i++)
                    {
                        playing = playing->next;
                    }
                    
                    video_change(playing);
                }
                else if (ts_x > 0) // 列表上一页
                {
                    j -= 10;
                    printf("FRONT PAGE\n");
                }
                else if (ts_x > 200) // 列表下一页
                {
                    j += 10;
                    printf("NEXT PAGE\n");
                }
                
                if (j < 0)
                {
                    j = (cnt_VIDEO / 10) * 10;
                }

                if (j >= cnt_VIDEO || j < 0)
                {
                    j = 0;
                }
                
                printf("j = %d\n", j);

                // 背景打印
                lcd_draw_jpg(0, 0, PATH_VIDEO0);                
                // 打印标题
                sprintf(title, "视频文件 (%d/%d)", j/10 + 1, page_VIDEO);
                Display_characterX(0, 0, title, 0x000000FF, 1);                
                
                int jj = j;
                y = 0;
                do
                {
                    jj++;
                    y += 35;
                    q = list_getElem(VIDEO, jj);
                    Display_characterX(0, y, q->name, 0x00000000, 1);
                }while ((jj < cnt_VIDEO) && (jj % 10 != 0));    
            }
            else if (ts_y > 400 && ts_x > 700) // 返回上层
            {
                lcd_draw_jpg(0, 0, PATH_MEDIA);
                break;
            }
        }
    }
    
    ts_x = 0;
    ts_y = 0;
    return 0;
}
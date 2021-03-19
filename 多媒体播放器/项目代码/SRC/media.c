#include "../INC/myhead.h"

FILE *music_mp;
_Bool Flag1 = 1;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

// 音频噪声消除（不太行）
void *music_dispel(void *volume)
{
    system("echo \"volume 0 1\" > ./tmp/mplayer_fifo1");
    usleep(100000);
    char vo[100];
    sprintf(vo, "echo \"volume %d 1\" > ./tmp/mplayer_fifo1", (int)volume);
    system(vo);
}

// 音频进度条
void *jdt1_display(double perc)
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
void *ylt1_display(int x)
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

// 打印音频控制界面
void music_menu(_Bool play_flag, int volume)
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
    ylt1_display(volume);
}

// 音频进度条
void *music_bar(void *arg)
{
    char buff[1024];
    double len, now = 0;

    system("echo get_time_length > ./tmp/mplayer_fifo1");
    fgets(buff, 1024, music_mp); // 从管道中获得的数据返回
    while (!strstr(buff , "ANS_LENGTH"))
    {
        printf("**%s\n", buff); // 输出得到的数据
        fgets(buff, 1024, music_mp);
    }

    char *pos = strrchr(buff, '=');
    pos += 1;
    len = atof(pos);
    printf("len = %f\n", len);

    // 获取百分比
    while (len - now > 1)
    {
        pthread_mutex_lock(&mutex1);
        // 获取当前状态
        bzero(buff, 1024); // 清空用户的缓冲区

        system("echo get_time_pos > ./tmp/mplayer_fifo1"); // 发送命令从播放器中获得当前的百分比时间

        fgets(buff, 1024, music_mp); // 从管道中获得的数据返回
        while (!strstr(buff , "ANS_TIME_POSITION"))
        {
            printf("**%s\n", buff); // 输出得到的数据
            fgets(buff, 1024, music_mp);
        }

        printf("buf = %s\n", buff);
        char *pos = strrchr(buff, '=');
        pos += 1;
        now = atof(pos); // 当前进度的百分比
        printf("now percent||len = %f||%f\n", now, len);
        jdt1_display((now / len) * 100);

        pthread_mutex_unlock(&mutex1);
        sleep(1);
    }

    system("killall -9 mplayer");
    Flag1 = 0;
    printf("This Music is end!\n");
}

// 音频播放
int music(PATH_LIST *MEDIA)
{
    system("killall -9 mplayer");
    int mode = 0;
    
    // 配置播放命令
    char str[200];
    sprintf(str, 
        "mplayer -quiet -slave -input file=./tmp/mplayer_fifo1 %s\"%s\" &",
        MEDIA->path, MEDIA->name);

    // 初始化一个属性变量，并将分离属性加入该变量
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    
    // 用分离属性产生一条新线程，控制音频进度条
    pthread_t tid;
    pthread_create(&tid, &attr, music_bar, NULL);

    // 播放音频
    music_mp = popen(str, "r");
    system("echo \"volume 50 1\" > ./tmp/mplayer_fifo1");

    // 背景显示
    music_menu(1, 600);

    int volume = 600;
    int volume_r = 0;
    _Bool play_flag = 1;
    // 无限循环
    while (Flag1)
    {
        // 当检测到坐标变化时
        if (TS_Y > 420 && TS_Y < 460 && TS_X > 520 && TS_X < 680) // 判断是否为音量条区域
        if (buf.type == EV_ABS)
        {
            if (!play_flag)
            {
                pthread_mutex_unlock(&mutex1); // 恢复正在运行的音量条线程
                play_flag = 1;
            }

            volume = TS_X;
            volume_r = ((TS_X - 520) * 5)/8;
            printf("volume is changing!\n");
            char vo[100];
            sprintf(vo, "echo \"volume %d 1\" > ./tmp/mplayer_fifo1", (int)volume_r);
            system(vo);
        
            music_menu(play_flag, volume);
        }

        if (TS_X > 0 && TS_Y > 390 && TS_Y < 415) // 判断是否为进度条区域
        {
            if (buf.type == EV_ABS)
            {
                if (play_flag)
                {
                    printf("pause!\n");
                    system("echo pause > ./tmp/mplayer_fifo1");
                    pthread_mutex_lock(&mutex1); // 暂停正在运行的进度条线程
                    play_flag = 0;
                }
                
                int perc = TS_X / 8;
                jdt1_display(perc);
            }

            if (buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0) // 松手时调节进度
            {
                char buf_seek[100];
                
                // 消除噪声
                pthread_t tid_dis;
                pthread_create(&tid_dis, &attr, music_dispel, (void *)volume_r);

                printf("seeking!!TS_X = %d, TS_Y = %d\n", TS_X, TS_Y);
                sprintf(buf_seek, "echo \"seek  %d  1\" > ./tmp/mplayer_fifo1", ts_x / 8);
                system(buf_seek);

                if (!play_flag)
                {
                    printf("pause!\n");
                    system("echo pause > ./tmp/mplayer_fifo1");
                    pthread_mutex_unlock(&mutex1); // 恢复正在运行的进度条线程
                    play_flag = 1;
                }

                music_menu(play_flag, volume);
                TS_X = 0;
                TS_Y = 0;
            }
        }

        // 当检测到松开指令时，对所在坐标进行判断
        if(buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
        {
            printf("music(x, y) = (%d, %d)\n", ts_x, ts_y);
            if (ts_y > 410)
            if (ts_x < 100) // 切换上一个
            {
                //system("killall -9 mplayer");
                mode = 2;
                pthread_mutex_unlock(&mutex1);
                break;
            }
            else if (ts_x < 200) // 后退5s
            {
                // 消除噪声
                pthread_t tid_dis;
                pthread_create(&tid_dis, &attr, music_dispel, (void *)volume_r);

                printf("seek -5!\n");
                system("echo seek -5 > ./tmp/mplayer_fifo1");

                if (!play_flag)
                {
                    pthread_mutex_unlock(&mutex1); // 恢复正在运行的进度条线程
                    play_flag = 1;
                }
                music_menu(play_flag, volume);
            }
            else if (ts_x < 300) // 播放/暂停
            {
                if (play_flag)
                {
                    printf("pause!\n");
                    system("echo pause > ./tmp/mplayer_fifo1");
                    pthread_mutex_lock(&mutex1); // 暂停正在运行的进度条线程
                    play_flag = 0;
                }
                else
                {
                    printf("continue!\n");
                    system("echo pause > ./tmp/mplayer_fifo1");
                    pthread_mutex_unlock(&mutex1); // 恢复正在运行的进度条线程
                    play_flag = 1;
                }
                music_menu(play_flag, volume);

            }
            else if (ts_x < 400) // 快进5s
            {
                // 消除噪声
                pthread_t tid_dis;
                pthread_create(&tid_dis, &attr, music_dispel, (void *)volume_r);

                printf("seek +5!\n");
                system("echo seek +5 > ./tmp/mplayer_fifo1"); 
                if (!play_flag)
                {
                    pthread_mutex_unlock(&mutex1); // 恢复正在运行的进度条线程
                    play_flag = 1;
                }
                music_menu(play_flag, volume);
            }
            else if (ts_x < 500) // 切换下一个
            {
                //system("killall -9 mplayer");
                mode = 1;
                pthread_mutex_unlock(&mutex1);
                break;
            }
            else if (ts_y > 410 && ts_x > 740) // 返回上层
            {
                //system("killall -9 mplayer");
                mode = 3;
                pthread_mutex_unlock(&mutex1);
                break;
            }
            ts_x = 0;
            ts_y = 0;
        }
    }

    system("killall -9 mplayer");
    pthread_mutex_unlock(&mutex1);
    pthread_cancel(tid);
    pclose(music_mp);
    usleep(10000);
    if (Flag1)
    {
        return mode;
    }
    else
    {
        Flag1 = 1;
        sleep(1);
        return 1;
    }
}

// 打印音乐列表
int music_list(PATH_LIST *playing)
{
    // cnt链表元素个数
    int cnt_MEDIA = len_list_node(MEDIA);
    printf("\n\ncnt_MEDIA = %d\n\n", cnt_MEDIA);
 
    // 背景打印
    lcd_draw_jpg(0, 0, PATH_VIDEO0);

    int pos = pos_list(MEDIA, playing);
    int j = ((pos-1) / 10) * 10;
    printf("j = %d\n", j);

    // 打印标题
    char title[100];
    sprintf(title, "音乐文件 (%d/%d)", pos, cnt_MEDIA);
    Display_characterX(0, 0, title, 0x000000FF, 1);

    int y = 0;
    PATH_LIST *q; 
    // 打印文字
    do
    {
        j++;
        y += 35;
        q = list_getElem(MEDIA, j);
        if (playing == q)
        {
            char buf[300];
            sprintf(buf, "☆%s", q->name);
            Display_characterX(0, y, buf, 0x00FF0000, 1);
        }
        else
        {
            Display_characterX(0, y, q->name, 0x00000000, 1);
        }     
    }while ((j < cnt_MEDIA) && (j % 10 != 0));   

    return 0;
}

// 切换音频
int music_change(PATH_LIST *p)
{
    PATH_LIST *last;
    music_list(p);
    int m = music(p);

    while (1)
    {
        last = MEDIA->prev;
        if (m == 2) // mode=2时，切换上一个音频
        {
            if (p == MEDIA->next) // 当前正在播放第一个
            {
                p = last;
                printf("LAST ONE\n");
            }
            else
            {
                p = p->prev;
                printf("FRONT ONE\n");
            }
            music_list(p);
            m = music(p);
        }
        else if (m == 1) // mode=1时，切换下一个音频
        {
            if (p == last)
            {
                p = MEDIA->next;
                printf("FIRST ONE\n");
            }
            else
            {
                p = p->next;
                printf("NEXT ONE\n");
            }
            music_list(p);
            m = music(p);
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

// 在列表中选择打开音频
int music_select()
{
    ts_x = 0;
    ts_y = 0;

    if (MEDIA->next == MEDIA)
    {
        show_color(0x00000000);
        Display_characterX(0, 0, "当前没有音频，请进行资源更新！", 0x000000FF, 3);

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

    PATH_LIST *playing = MEDIA->next;
    
    int cnt_MEDIA = len_list_node(MEDIA);
    printf("\n\ncnt_MEDIA = %d\n\n", cnt_MEDIA);

    // 背景打印
    lcd_draw_jpg(0, 0, PATH_VIDEO0);

    int pos = pos_list(MEDIA, playing);
    int j = ((pos-1) / 10) * 10;
    printf("j = %d\n", j);
    int page_MEDIA = (cnt_MEDIA / 10) + 1;

    // 打印标题
    char title[100];
    sprintf(title, "音乐文件 (%d/%d)", j/10 + 1, page_MEDIA);
    Display_characterX(0, 0, title, 0x000000FF, 1);

    int y = 0;
    PATH_LIST *q;
    // 打印文字
    do
    {
        j++;
        y += 35;
        q = list_getElem(MEDIA, j);
        Display_characterX(0, y, q->name, 0x00000000, 1);
    }while ((j < cnt_MEDIA) && (j % 10 != 0));
    
    j = 0;

    while (1)
    {
        // 当检测到松开指令时，对所在坐标进行判断
        if (buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
        {
            printf("music_select(x, y) = (%d, %d)\n", ts_x, ts_y);
            playing = MEDIA->next;
            // 进行坐标判断
            if (ts_x > 0 && ts_x < 400)
            {
                int num = ts_y / 40;
                if (num <= cnt_MEDIA - j) // 打开音频
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
                    
                    music_change(playing);
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
                    j = (cnt_MEDIA / 10) * 10;
                }

                if (j >= cnt_MEDIA || j < 0)
                {
                    j = 0;
                }
                
                printf("j = %d\n", j);

                // 背景打印
                lcd_draw_jpg(0, 0, PATH_VIDEO0);                
                // 打印标题
                sprintf(title, "音乐文件 (%d/%d)", j/10 + 1, page_MEDIA);
                Display_characterX(0, 0, title, 0x000000FF, 1);                
                
                int jj = j;
                y = 0;
                do
                {
                    jj++;
                    y += 35;
                    q = list_getElem(MEDIA, jj);
                    Display_characterX(0, y, q->name, 0x00000000, 1);
                }while ((jj < cnt_MEDIA) && (jj % 10 != 0));    
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
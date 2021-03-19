#include "../INC/myhead.h"

// 刮刮乐中显示一部分图片
int show_partBG(int x, int y)
{
    int x_start = x - 25;
    int y_start = y - 25;
    int x_end = x + 25;
    int y_end = y + 25;

    // 判断x是否越界
    if (x_start < 0)
    {
        x_start = 0;
    }
    else if (x_end > 800)
    {
        x_end = 800;
    }

    // 判断y是否越界
    if (y_start < 0)
    {
        y_start = 0;
    }
    else if (y_end > 480)
    {
        y_end = 480;
    }
    
    int i, j;
    for (i = y_start; i < y_end; i++)
    {
        for (j = x_start; j < x_end; j++)
        {
            *(mmap_fd+j+i*800) = BG_TMP[i][j];
        }
    }
}

// 刮刮乐模块
int ggl(PATH_LIST *PIC)
{
    int cnt = len_list_node(PIC) - 1;
    printf("PIC COUNT = %d\n", cnt);

    // 随机获取一张图片做背景
    srand((unsigned)time(NULL));
    int r = (rand() % cnt) + 1;
    PATH_LIST *p = list_getElem(PIC, r);

    int type = pic_type(p);
    if (type == 1)
    {
        Display_characterX(300, 200, "Now Loading...", 0x000000FF, 2);
        char BMP_PATH[300];
        sprintf(BMP_PATH, "%s%s", p->path, p->name);     
        show_bmp(BMP_PATH);
    }
    else if (type == 2)
    {
        Display_characterX(300, 200, "Now Loading...", 0x000000FF, 2);
        char JPG_PATH[300];
        sprintf(JPG_PATH, "%s%s", p->path, p->name);
        show_jpg(JPG_PATH);
    }

    // 打印黑屏
    show_color(0x00000000);

    // 当两个flag同时为真时打印一部分图片
    _Bool x_flag = 0, y_flag = 0;

    // 开始刮刮乐
    // 触摸屏参数初始化
    int ts_x = 0, ts_y = 0;
    struct input_event buf;
    while (1)
    {
        read(ts_fd, &buf, sizeof(buf));
        Display_characterX(755, 460, "BACK", 0x000000FF, 1);
        Display_characterX(755, 0, "NEXT", 0x000000FF, 1);

        // 当检测到坐标变化时，读取X,Y坐标
        if (buf.type == EV_ABS)
        {
            if (buf.code == ABS_X)
            {
                ts_x = buf.value * 800 / 1024;
                x_flag = 1;
            }
            else if (buf.code == ABS_Y)
            {
                ts_y = buf.value * 480 / 600;
                y_flag = 1;
            }
            
            if (x_flag && y_flag)
            {
                show_partBG(ts_x, ts_y);
                x_flag = 0;
                y_flag = 0;
            }
        }

        // 当检测到松开指令时，对所在坐标进行判断
        if (buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)        
        {
            printf("ggl(x, y) = (%d, %d)\n", ts_x, ts_y);
            if (ts_y > 460 && ts_x > 780) // 右下角返回上层
            {
                lcd_draw_jpg(0, 0, PATH_GAME); 
                break;
            }

            if (ts_y < 20 && ts_x > 780) // 右上角更换图片
            {
                // 随机获取一张图片做背景
                srand((unsigned)time(NULL));
                r = (rand() % cnt) + 1;
                p = list_getElem(PIC, r);
                type = pic_type(p);
                if (type == 1)
                {
                    Display_characterX(300, 200, "Now Loading...", 0x000000FF, 2);
                    char BMP_PATH[300];
                    sprintf(BMP_PATH, "%s%s", p->path, p->name);     
                    show_bmp(BMP_PATH);
                }
                else if (type == 2)
                {
                    Display_characterX(300, 200, "Now Loading...", 0x000000FF, 2);
                    char JPG_PATH[300];
                    sprintf(JPG_PATH, "%s%s", p->path, p->name);
                    show_jpg(JPG_PATH);
                }        
                
                // 打印黑屏
                show_color(0x00000000);                   
            }
            
            
        }

    }
    
    return 0;
}

// 抽奖模块
int gacha()
{
    // 抽奖模块界面
    lcd_draw_jpg(0, 0, PATH_GACHA);

    // 用flag控制"点击任意处返回"的功能
    _Bool flag = 0;

    // 开始抽奖
    // 触摸屏参数初始化
    int ts_x = 0, ts_y = 0;
    struct input_event buf;
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
        if(flag == 0)
        {
            printf("gacha(x, y) = (%d, %d)\n", ts_x, ts_y);

            if (ts_y > 240) // 下 返回上层
            {
                lcd_draw_jpg(0, 0, PATH_GAME); 
                break;
            }

            if (ts_y < 240) // 上 抽奖
            {
                // 生成随机数
                srand((unsigned)time(NULL));
                int r = rand() % 1000;
                int mode = 4;
                if (r < 10) // 一等奖 10/1000 = 1%
                {
                    mode = 1;
                }
                else if (r < 40) // 二等奖 30/1000 = 3%
                {
                    mode = 2;
                }
                else if (r < 90) // 三等奖 50/1000 = 5%
                {
                    mode = 3;
                }
                
                // 中奖图片地址
                char JPG_PATH[300];
                sprintf(JPG_PATH, "%s%d.jpg", PATH_GACHADIR, mode);

                // 显示中奖图片
                lcd_draw_jpg(0, 0, JPG_PATH);
                flag = 1;
            }
        }
        else
        {
            flag = 0;
            lcd_draw_jpg(0, 0, PATH_GACHA);
        }
    }
    return 0;
}
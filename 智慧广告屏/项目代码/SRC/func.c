#include "../INC/myhead.h"

PATH_LIST *playing = NULL;

// 判断图片类型，BMP返回1，JPG返回2，否则返回0
int pic_type(PATH_LIST *PIC)
{
    char *type = strrchr(PIC->name, '.');
    int len = strlen(type);
    if (!strncmp(type, ".bmp", len))
    {
        return 1;
    }
    else if (!strncmp(type, ".jpg", len))
    {
        return 2;
    }
    return 0;
}

// 相册模块
int album(PATH_LIST *PIC)
{
    struct input_event buf;
    int ts_x, ts_y;
    if (PIC->next == PIC)
    {
        show_color(0x00000000);
        Display_characterX(0, 0, "当前相册没有图片，请进行资源更新！", 0x000000FF, 3);

        // 点击任意处返回
        while (1)
        {
            read(ts_fd, &buf, sizeof(buf));
            if (buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
            {
                lcd_draw_jpg(0, 0, PATH_MENU);
                return -2;
            }
        }
    }

    // 正在显示的图片
    PATH_LIST *p = PIC->next;

    // 最后一张图片
    PATH_LIST *last;

    // 根据图片类型打印第1张图片
    int type = pic_type(p);
    if (type == 1)
    {
        Display_characterX(300, 200, "Now Loading...", 0x000000FF, 2);
        char BMP_PATH[300];
        sprintf(BMP_PATH, "%s%s", p->path, p->name);     
        show_bmp(BMP_PATH);
        BG_print();
    }
    else if (type == 2)
    {
        Display_characterX(300, 200, "Now Loading...", 0x000000FF, 2);
        char JPG_PATH[300];
        sprintf(JPG_PATH, "%s%s", p->path, p->name);
        show_jpg(JPG_PATH);
        BG_print();
    }
    lcd_draw_jpg(345, 454, PATH_ALBUMARROW);

    // arrow_flag = 1打开相册菜单，0关闭相册菜单
    _Bool arrow_flag = 1;

    // list_flag = 1打开列表，0关闭列表
    _Bool list_flag = 0;

    // show_flag = 1打印图片，0不打印图片
    _Bool show_flag = 1;

    while (1)
    {   
        last = PIC->prev;
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
        if (buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
        {
            printf("album(x, y) = (%d, %d)\n", ts_x, ts_y);
            printf("arrow_flag = %d\n", arrow_flag);
            printf("list_flag = %d\n", list_flag);
            
            // 当相册菜单打开时，进行坐标判断
            if (!arrow_flag)
            {
                if (100 < ts_x && ts_x < 220 && ts_y > 430) // 上一张
                {
                    if (p == PIC->next) // 当前正在显示第一张
                    {
                        p = last;
                        printf("LAST ONE\n");
                    }
                    else
                    {
                        p = p->prev;
                        printf("FRONT ONE\n");
                    }

                }
                else if (220 < ts_x && ts_x < 340 && ts_y > 430) // 列表
                {
                    list_flag = !list_flag;
                }
                else if (460 < ts_x && ts_x < 580 && ts_y > 430) // 删除
                {
                    PATH_LIST *tmp_del;
                    tmp_del = delete_list(PIC ,p);
                    if (tmp_del->next == PIC)  // 说明刚刚删除的最后一张
                    {
                        p = PIC->next;  // 让p指回第一张
                    }
                    else // 说明刚才删除的不是最后一张
                    {  
                        p = tmp_del->next;
                    }
                }
                else if (580 < ts_x && ts_x < 700 && ts_y > 430) // 下一张
                {
                    if (p == last) // 当前正在显示最后一张
                    {
                        p = PIC->next;
                        printf("FIRST ONE");
                    }
                    else
                    {
                        p = p->next;
                        printf("NEXT ONE\n");
                    }

                }
                else if (340 < ts_x && ts_x < 460 && ts_y > 430)// 返回上层
                {
                    lcd_draw_jpg(0, 0, PATH_MENU);
                    break;
                }
                else if (ts_y < 429 && ts_y > 404 && ts_x > 345 && ts_x < 455) // 隐藏菜单按钮
                {
                    // 重新打印背景图片
                    BG_print();

                    // 打印菜单箭头
                    lcd_draw_jpg(345, 454, PATH_ALBUMARROW);

                    // 反置arrow_flag
                    arrow_flag = !arrow_flag;
                }
                else // 点击除了以上选项以外的区域 关闭列表
                {
                    list_flag = 0;
                }

                if (PIC->next == PIC) // 如果所有图片都被删除了就返回
                {
                    show_color(0x00000000);
                    Display_characterX(0, 0, "当前相册没有图片，请进行资源更新！", 0x000000FF, 3);
                    // 点击任意处返回
                    while (1)
                    {
                        read(ts_fd, &buf, sizeof(buf));
                        if (buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
                        {
                            lcd_draw_jpg(0, 0, PATH_MENU);
                            return -2;
                        }
                    }
                }
                
                // 只有点击上一张/下一张/删除才重新显示
                if (ts_y > 430)
                if ((100 < ts_x && ts_x < 220) || (580 < ts_x && ts_x < 700) || (460 < ts_x && ts_x < 580))
                {
                    // 根据图片类型打印图片
                    int type = pic_type(p);
                    if (type == 1)
                    {
                        Display_characterX(300, 200, "Now Loading...", 0x000000FF, 2);
                        char BMP_PATH[300];
                        sprintf(BMP_PATH, "%s%s", p->path, p->name);     
                        show_bmp(BMP_PATH);
                        BG_print();
                    }
                    else if (type == 2)
                    {
                        Display_characterX(300, 200, "Now Loading...", 0x000000FF, 2);
                        char JPG_PATH[300];
                        sprintf(JPG_PATH, "%s%s", p->path, p->name);
                        show_jpg(JPG_PATH);
                        BG_print();
                    }
                    
                    // 打印箭头和菜单
                    if (arrow_flag)
                    {
                        lcd_draw_jpg(345, 454, PATH_ALBUMARROW);
                    }
                    else
                    {
                        lcd_draw_jpg(345, 404, PATH_ALBUMARROW);
                        lcd_draw_jpg(100, 429, PATH_ALBUMMENU);
                    }

                }

                // 判断列表是否打开
                if (list_flag)
                {
                    // cnt链表元素个数
                    int cnt_PIC = len_list_node(PIC);

                    // 列表背景打印
                    lcd_draw_jpg(0, 0, PATH_ALBUMLISTBG);

                    // 打印标题
                    int pos = pos_list(PIC, p);
                    int j = ((pos-1) / 10) * 10;
                    int y = 0;
                    char title[50];
                    sprintf(title, "图片文件 (%d/%d)", pos, cnt_PIC);
                    Display_characterX(0, 0, title, 0x000000FF, 1);

                    PATH_LIST *q;
                    // 打印文字
                    do
                    {
                        j++;
                        y += 35;
                        q = list_getElem(PIC, j);
                        if (p == q)
                        {
                            char buf[300];
                            sprintf(buf, "☆%s", q->name);
                            Display_characterX(0, y, buf, 0x00FF0000, 1);
                        }
                        else
                        {
                            Display_characterX(0, y, q->name, 0x00000000, 1);
                        }        
                    }while ((j < cnt_PIC) && (j % 10 != 0));
                }
                else 
                {
                    // 重新打印背景图片
                    BG_print();
                    
                    // 打印箭头和菜单
                    if (arrow_flag)
                    {
                        lcd_draw_jpg(345, 454, PATH_ALBUMARROW);
                    }
                    else
                    {
                        lcd_draw_jpg(345, 404, PATH_ALBUMARROW);
                        lcd_draw_jpg(100, 429, PATH_ALBUMMENU);
                    }
                }

            }
            else if (ts_y > 454 && ts_x > 345 && ts_x < 455) // 打开相册菜单
            {
                if (arrow_flag)
                {
                    // 重新打印背景图片
                    BG_print();

                    // 打印菜单界面
                    lcd_draw_jpg(100, 429, PATH_ALBUMMENU);

                    // 打印菜单on箭头
                    lcd_draw_jpg(345, 404, PATH_ALBUMARROW);

                    // 反置arrow_flag
                    arrow_flag = !arrow_flag;
                }

                // 判断列表是否打开
                if (list_flag)
                {
                    // cnt链表元素个数
                    int cnt_PIC = len_list_node(PIC);

                    // 列表背景打印
                    lcd_draw_jpg(0, 0, PATH_ALBUMLISTBG);

                    // 打印标题
                    int pos = pos_list(PIC, p);
                    int j = ((pos-1) / 10) * 10;
                    int y = 0;
                    char title[50];
                    sprintf(title, "图片文件 (%d/%d)", pos, cnt_PIC);
                    Display_characterX(0, 0, title, 0x000000FF, 1);

                    PATH_LIST *q;
                    // 打印文字
                    do
                    {
                        j++;
                        y += 35;
                        q = list_getElem(PIC, j);
                        if (p == q)
                        {
                            char buf[300];
                            sprintf(buf, "☆%s", q->name);
                            Display_characterX(0, y, buf, 0x00FF0000, 1);
                        }
                        else
                        {
                            Display_characterX(0, y, q->name, 0x00000000, 1);
                        }        
                    }while ((j < cnt_PIC) && (j % 10 != 0));
                }
            }

        }
        
    }

    return 0;
}

// 资源更新模块
int SourceUpdate(PATH_LIST *MEDIA, PATH_LIST *PIC)
{
    lcd_draw_jpg(0, 0, PATH_UPDATE);

    // 触摸屏参数初始化
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
            printf("SourceUpdate(x, y) = (%d, %d)\n", ts_x, ts_y);
            if(ts_y < 96) // 1 读取本地文件到链表中
            {
                list_clear(MEDIA);
                list_clear(PIC);
                dir_new("./media/", MEDIA, PIC);
                dir_new("./album/", MEDIA, PIC);
            }
            else if(ts_y < 192) // 2 从链表保存列表到dat文件中
            {
                list_write(MEDIA, "LIST_MEDIA.dat");
                list_write(PIC, "LIST_PIC.dat");
            }
            else if(ts_y < 288) // 3 当前链表的列表一览
            {
                list_print(PIC, MEDIA);
            }
            else if(ts_y < 384) // 4 从U盘拷贝文件到本地
            {
                copy_from_udisk("/mnt/udisk/");
            }
            else // 5 返回上层
            {
                lcd_draw_jpg(0, 0, PATH_MENU);          
                break;
            }
        }        
    }

    return 0;
}

// 游戏模块
int game(PATH_LIST *PIC)
{
    // 游戏模块界面
    lcd_draw_jpg(0, 0, PATH_GAME);

    // 触摸屏参数初始化
    // int ts_fd = touch_open();
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
            printf("gacha(x, y) = (%d, %d)\n", ts_x, ts_y);
            printf("ts_x = %d\n", ts_x);
            printf("ts_y = %d\n", ts_y);
            if(ts_y < 240) // 上，返回上层
            {
                lcd_draw_jpg(0, 0, PATH_MENU);
                break;                
            }

            if(ts_x < 400 && ts_y > 240) // 左下，抽奖
            {
                gacha();
            }

            if(ts_x > 400 && ts_y > 240) // 右下，刮刮乐
            {
                ggl(PIC);
            }
        }        
    }

    return 0;
}

// 媒体模块
int media(PATH_LIST *MEDIA)
{
    struct input_event buf;
    int ts_x, ts_y;

    if (MEDIA->next == MEDIA)
    {
        show_color(0x00000000);
        Display_characterX(0, 0, "当前无媒体文件，请进行资源更新！", 0x000000FF, 3);
        while (1)
        {
            read(ts_fd, &buf, sizeof(buf));
            if (buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
            {
                lcd_draw_jpg(0, 0, PATH_MENU);
                return -2;
            }
        }
    }

    // 正在播放的媒体文件
    if (playing == NULL)
    {
        playing = MEDIA->next;
        while (playing == NULL || playing == MEDIA)
        {
            playing = playing->next;
        }
    }
    printf("playing %s%s\n", playing->path, playing->name);

    // 最后一个媒体文件
    PATH_LIST *last;

    // cnt链表元素个数
    int cnt_MEDIA = len_list_node(MEDIA);
    printf("\n\ncnt_MEDIA = %d\n\n", cnt_MEDIA);

    // 背景打印
    lcd_draw_jpg(0, 0, PATH_MEDIA1);

    int pos = pos_list(MEDIA, playing);
    int j = ((pos-1) / 10) * 10;
    printf("j = %d\n", j);

    // 打印标题
    char title[100];
    sprintf(title, "媒体文件 (%d/%d)", pos, cnt_MEDIA);
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

    int mode = 1;
    _Bool play_flag = 1;
    char tmp[100];
    while (1)
    {   
        last = MEDIA->prev;
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
        if (buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
        {
            printf("media(x, y) = (%d, %d)\n", ts_x, ts_y);

            // 进行坐标判断
            if (400 < ts_x && ts_x < 600 && ts_y < 160) // 上一个
            {
                if (playing == MEDIA->next) // 当前正在播放第一个
                {
                    playing = last;
                    printf("LAST ONE\n");
                }
                else
                {
                    playing = playing->prev;
                    printf("FRONT ONE\n");
                }
                media_play(playing);
                mode = 1;
            }
            else if (600 < ts_x && ts_x < 800 && ts_y < 160) // 下一个
            {
                if (playing == last) // 当前正在播放最后一个
                {
                    playing = MEDIA->next;
                    printf("FIRST ONE");
                }
                else
                {
                    playing = playing->next;
                    printf("NEXT ONE\n");
                }
                media_play(playing);
                mode = 1;
            }
            else if (400 < ts_x && ts_x < 800 && 160 < ts_y && ts_y < 320) // 暂停 / 继续
            {
                if (mode == 1)
                {
                    media_pause();
                    printf("media_pause()\n");
                    play_flag = 0;
                    mode = 2;
                }
                else if (mode == 2)
                {
                    media_cont();
                    printf("media_cont()\n");
                    play_flag = 1;
                    mode = 1;
                }
                else if (mode == 0)
                {
                    // 随机播放一首歌
                    srand((unsigned)time(NULL));
                    int r = (rand() % cnt_MEDIA) + 1;
                    playing = list_getElem(MEDIA, r);
                    media_play(playing);
                    play_flag = 1;
                    mode = 1;
                }
            }
            else if (400 < ts_x && ts_x < 600 && 320 < ts_y) // 停止
            {
                media_stop();
                printf("media_stop()\n");
                play_flag = 0;
                mode = 0;
            }
            else if (600 < ts_x && ts_x < 800 && 320 < ts_y)// 返回上层
            {
                lcd_draw_jpg(0, 0, PATH_MENU);
                break;
            }

            // 重新打印列表和背景
            // 背景打印
            char mode_buf[300];
            sprintf(mode_buf, "./Image/media_%d.jpg", mode);
            printf("mode_buf = %s\n", mode_buf);
            lcd_draw_jpg(0, 0, mode_buf);

            // 打印标题
            int pos = pos_list(MEDIA, playing);
            int j = ((pos-1) / 10) * 10;
            int y = 0;
            char title[50];
            sprintf(title, "媒体文件 (%d/%d)", pos, cnt_MEDIA);
            Display_characterX(0, 0, title, 0x000000FF, 1);

            PATH_LIST *q; 
            // 打印文字
            do
            {
                j++;
                y += 35;
                q = list_getElem(MEDIA, j);
                if (playing == q && play_flag)
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
        }
            
    }
    
    return 0;
}

// 把BG_TMP的内容重新映射
int BG_print()
{
    int i, j;
    for (i = 0; i < 480; i++)
    {
        for (j = 0; j < 800; j++)
        {
            *(mmap_fd+j+i*800) = BG_TMP[i][j];
        }
        
    }
}

// 屏幕打印颜色
int show_color(int color)
{
	int i;
    for (i = 0; i < 800 * 480; i++)
    {
        *(mmap_fd + i) = color;
    }

	return 0;
}

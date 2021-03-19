#include "../INC/myhead.h"

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
int album()
{
    memset(&buf, 0, sizeof(buf));
    ts_x = 0;
    ts_y = 0;

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
    printf("pic1 = %s\n", p->name);
    printf("type = %d\n", type);
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

    ts_x = 0;
    ts_y = 0;
    return 0;
}

// 资源更新模块
int SourceUpdate()
{
    lcd_draw_jpg(0, 0, PATH_UPDATE);
    memset(&buf, 0, sizeof(buf));
    ts_x = 0;
    ts_y = 0;

    // 无限循环
    while (1)
    {
        // 当检测到松开指令时，对所在坐标进行判断
        if(buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
        {
            printf("SourceUpdate(x, y) = (%d, %d)\n", ts_x, ts_y);
            if(ts_y > 0 && ts_y < 96) // 1 读取本地文件到链表中
            {
                list_clear(MEDIA);
                list_clear(PIC);
                list_clear(VIDEO);
                dir_new("./media/");
                dir_new("./album/");
                dir_new("/Memory/");
            }
            else if(ts_y < 192) // 2 从链表保存列表到dat文件中
            {
                list_write(MEDIA, "LIST_MEDIA.dat");
                list_write(PIC, "LIST_PIC.dat");
                list_write(VIDEO, "LIST_VIDEO.dat");
            }
            else if(ts_y < 288) // 3 当前链表的列表一览
            {
                list_print(PIC, VIDEO);
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

    ts_x = 0;
    ts_y = 0;
    return 0;
}

// 游戏模块
int game()
{
    // 游戏模块界面
    lcd_draw_jpg(0, 0, PATH_GAME);

    // 触摸屏参数初始化
    ts_x = 0;
    ts_y = 0;

    // 无限循环
    while (1)
    {
        // 当检测到松开指令时，对所在坐标进行判断
        if(buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
        {
            printf("gacha(x, y) = (%d, %d)\n", ts_x, ts_y);
            if(ts_y > 0 && ts_y < 240) // 上，返回上层
            {
                lcd_draw_jpg(0, 0, PATH_MENU);
                break;                
            }
            else if(ts_x < 400 && ts_y > 240) // 左下，抽奖
            {
                gacha();
            }
            else if(ts_x > 400 && ts_y > 240) // 右下，刮刮乐
            {
                ggl();
            }
        }        
    }

    ts_x = 0;
    ts_y = 0;
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

// 媒体分支
void media()
{
    // 媒体分支界面
    lcd_draw_jpg(0, 0, PATH_MEDIA);

    ts_x = 0;
    ts_y = 0;

    // 无限循环
    while (1)
    {
        // 当检测到松开指令时，对所在坐标进行判断
        if(buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
        {
            printf("gacha(x, y) = (%d, %d)\n", ts_x, ts_y);
            if(ts_y > 0 && ts_y < 240) // 上，返回上层
            {
                lcd_draw_jpg(0, 0, PATH_MENU);
                break;                
            }

            if(ts_x < 400 && ts_y > 240) // 左下，音频
            {
                music_select();
            }

            if(ts_x > 400 && ts_y > 240) // 右下，视频
            {
                video_select();
            }
        }        
    }

    ts_x = 0;
    ts_y = 0;
}
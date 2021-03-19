#include "../INC/myhead.h"

// �ж�ͼƬ���ͣ�BMP����1��JPG����2�����򷵻�0
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

// ���ģ��
int album()
{
    memset(&buf, 0, sizeof(buf));
    ts_x = 0;
    ts_y = 0;

    if (PIC->next == PIC)
    {
        show_color(0x00000000);
        Display_characterX(0, 0, "��ǰ���û��ͼƬ���������Դ���£�", 0x000000FF, 3);

        // ������⴦����
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

    // ������ʾ��ͼƬ
    PATH_LIST *p = PIC->next;

    // ���һ��ͼƬ
    PATH_LIST *last;

    // ����ͼƬ���ʹ�ӡ��1��ͼƬ
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

    // arrow_flag = 1�����˵���0�ر����˵�
    _Bool arrow_flag = 1;

    // list_flag = 1���б�0�ر��б�
    _Bool list_flag = 0;

    // show_flag = 1��ӡͼƬ��0����ӡͼƬ
    _Bool show_flag = 1;

    while (1)
    {   
        last = PIC->prev;

        // ����⵽�ɿ�ָ��ʱ����������������ж�
        if (buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
        {
            printf("album(x, y) = (%d, %d)\n", ts_x, ts_y);
            printf("arrow_flag = %d\n", arrow_flag);
            printf("list_flag = %d\n", list_flag);
            
            // �����˵���ʱ�����������ж�
            if (!arrow_flag)
            {
                if (100 < ts_x && ts_x < 220 && ts_y > 430) // ��һ��
                {
                    if (p == PIC->next) // ��ǰ������ʾ��һ��
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
                else if (220 < ts_x && ts_x < 340 && ts_y > 430) // �б�
                {
                    list_flag = !list_flag;
                }
                else if (460 < ts_x && ts_x < 580 && ts_y > 430) // ɾ��
                {
                    PATH_LIST *tmp_del;
                    tmp_del = delete_list(PIC ,p);
                    if (tmp_del->next == PIC)  // ˵���ո�ɾ�������һ��
                    {
                        p = PIC->next;  // ��pָ�ص�һ��
                    }
                    else // ˵���ղ�ɾ���Ĳ������һ��
                    {  
                        p = tmp_del->next;
                    }
                }
                else if (580 < ts_x && ts_x < 700 && ts_y > 430) // ��һ��
                {
                    if (p == last) // ��ǰ������ʾ���һ��
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
                else if (340 < ts_x && ts_x < 460 && ts_y > 430)// �����ϲ�
                {
                    lcd_draw_jpg(0, 0, PATH_MENU);
                    break;
                }
                else if (ts_y < 429 && ts_y > 404 && ts_x > 345 && ts_x < 455) // ���ز˵���ť
                {
                    // ���´�ӡ����ͼƬ
                    BG_print();

                    // ��ӡ�˵���ͷ
                    lcd_draw_jpg(345, 454, PATH_ALBUMARROW);

                    // ����arrow_flag
                    arrow_flag = !arrow_flag;
                }
                else // �����������ѡ����������� �ر��б�
                {
                    list_flag = 0;
                }

                if (PIC->next == PIC) // �������ͼƬ����ɾ���˾ͷ���
                {
                    show_color(0x00000000);
                    Display_characterX(0, 0, "��ǰ���û��ͼƬ���������Դ���£�", 0x000000FF, 3);
                    // ������⴦����
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
                
                // ֻ�е����һ��/��һ��/ɾ����������ʾ
                if (ts_y > 430)
                if ((100 < ts_x && ts_x < 220) || (580 < ts_x && ts_x < 700) || (460 < ts_x && ts_x < 580))
                {
                    // ����ͼƬ���ʹ�ӡͼƬ
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
                    
                    // ��ӡ��ͷ�Ͳ˵�
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

                // �ж��б��Ƿ��
                if (list_flag)
                {
                    // cnt����Ԫ�ظ���
                    int cnt_PIC = len_list_node(PIC);

                    // �б�����ӡ
                    lcd_draw_jpg(0, 0, PATH_ALBUMLISTBG);

                    // ��ӡ����
                    int pos = pos_list(PIC, p);
                    int j = ((pos-1) / 10) * 10;
                    int y = 0;
                    char title[50];
                    sprintf(title, "ͼƬ�ļ� (%d/%d)", pos, cnt_PIC);
                    Display_characterX(0, 0, title, 0x000000FF, 1);

                    PATH_LIST *q;
                    // ��ӡ����
                    do
                    {
                        j++;
                        y += 35;
                        q = list_getElem(PIC, j);
                        if (p == q)
                        {
                            char buf[300];
                            sprintf(buf, "��%s", q->name);
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
                    // ���´�ӡ����ͼƬ
                    BG_print();
                    
                    // ��ӡ��ͷ�Ͳ˵�
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
            else if (ts_y > 454 && ts_x > 345 && ts_x < 455) // �����˵�
            {
                if (arrow_flag)
                {
                    // ���´�ӡ����ͼƬ
                    BG_print();

                    // ��ӡ�˵�����
                    lcd_draw_jpg(100, 429, PATH_ALBUMMENU);

                    // ��ӡ�˵�on��ͷ
                    lcd_draw_jpg(345, 404, PATH_ALBUMARROW);

                    // ����arrow_flag
                    arrow_flag = !arrow_flag;
                }

                // �ж��б��Ƿ��
                if (list_flag)
                {
                    // cnt����Ԫ�ظ���
                    int cnt_PIC = len_list_node(PIC);

                    // �б�����ӡ
                    lcd_draw_jpg(0, 0, PATH_ALBUMLISTBG);

                    // ��ӡ����
                    int pos = pos_list(PIC, p);
                    int j = ((pos-1) / 10) * 10;
                    int y = 0;
                    char title[50];
                    sprintf(title, "ͼƬ�ļ� (%d/%d)", pos, cnt_PIC);
                    Display_characterX(0, 0, title, 0x000000FF, 1);

                    PATH_LIST *q;
                    // ��ӡ����
                    do
                    {
                        j++;
                        y += 35;
                        q = list_getElem(PIC, j);
                        if (p == q)
                        {
                            char buf[300];
                            sprintf(buf, "��%s", q->name);
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

// ��Դ����ģ��
int SourceUpdate()
{
    lcd_draw_jpg(0, 0, PATH_UPDATE);
    memset(&buf, 0, sizeof(buf));
    ts_x = 0;
    ts_y = 0;

    // ����ѭ��
    while (1)
    {
        // ����⵽�ɿ�ָ��ʱ����������������ж�
        if(buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
        {
            printf("SourceUpdate(x, y) = (%d, %d)\n", ts_x, ts_y);
            if(ts_y > 0 && ts_y < 96) // 1 ��ȡ�����ļ���������
            {
                list_clear(MEDIA);
                list_clear(PIC);
                list_clear(VIDEO);
                dir_new("./media/");
                dir_new("./album/");
                dir_new("/Memory/");
            }
            else if(ts_y < 192) // 2 ���������б�dat�ļ���
            {
                list_write(MEDIA, "LIST_MEDIA.dat");
                list_write(PIC, "LIST_PIC.dat");
                list_write(VIDEO, "LIST_VIDEO.dat");
            }
            else if(ts_y < 288) // 3 ��ǰ������б�һ��
            {
                list_print(PIC, VIDEO);
            }
            else if(ts_y < 384) // 4 ��U�̿����ļ�������
            {
                copy_from_udisk("/mnt/udisk/");
            }
            else // 5 �����ϲ�
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

// ��Ϸģ��
int game()
{
    // ��Ϸģ�����
    lcd_draw_jpg(0, 0, PATH_GAME);

    // ������������ʼ��
    ts_x = 0;
    ts_y = 0;

    // ����ѭ��
    while (1)
    {
        // ����⵽�ɿ�ָ��ʱ����������������ж�
        if(buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
        {
            printf("gacha(x, y) = (%d, %d)\n", ts_x, ts_y);
            if(ts_y > 0 && ts_y < 240) // �ϣ������ϲ�
            {
                lcd_draw_jpg(0, 0, PATH_MENU);
                break;                
            }
            else if(ts_x < 400 && ts_y > 240) // ���£��齱
            {
                gacha();
            }
            else if(ts_x > 400 && ts_y > 240) // ���£��ι���
            {
                ggl();
            }
        }        
    }

    ts_x = 0;
    ts_y = 0;
    return 0;
}

// ��BG_TMP����������ӳ��
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

// ��Ļ��ӡ��ɫ
int show_color(int color)
{
	int i;
    for (i = 0; i < 800 * 480; i++)
    {
        *(mmap_fd + i) = color;
    }

	return 0;
}

// ý���֧
void media()
{
    // ý���֧����
    lcd_draw_jpg(0, 0, PATH_MEDIA);

    ts_x = 0;
    ts_y = 0;

    // ����ѭ��
    while (1)
    {
        // ����⵽�ɿ�ָ��ʱ����������������ж�
        if(buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
        {
            printf("gacha(x, y) = (%d, %d)\n", ts_x, ts_y);
            if(ts_y > 0 && ts_y < 240) // �ϣ������ϲ�
            {
                lcd_draw_jpg(0, 0, PATH_MENU);
                break;                
            }

            if(ts_x < 400 && ts_y > 240) // ���£���Ƶ
            {
                music_select();
            }

            if(ts_x > 400 && ts_y > 240) // ���£���Ƶ
            {
                video_select();
            }
        }        
    }

    ts_x = 0;
    ts_y = 0;
}
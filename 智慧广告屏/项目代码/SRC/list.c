#include "../INC/myhead.h"

// 创建链表头节点
PATH_LIST *init_list_head()
{
    // 1.为头节点申请空间
    PATH_LIST *head;
    head = malloc(sizeof(PATH_LIST));
    if(head == NULL)
    {
        printf("malloc error!\n");
    }

    // 2.为头节点指针域
    head->next = head;
    head->prev = head;

    return head;
}

// 头插法插入非重复节点
int insert_list_node(PATH_LIST *head, char path[], char name[])
{
    // 0.判断新节点数据域是否有重复
    PATH_LIST *p; 
    for (p = head->next; p != head; p = p->next)
    {
        if (!strncmp(p->name, name, strlen(name)) 
            && !strncmp(p->path, path, strlen(path)))
        {
            printf("REPEAT!\n");
            break;
        }
    }

    // 当p!=head时代表有重复节点
    if (p != head)
    {
        return -1;
    }
    printf("UN REPEAT: ");

    // 1.为头节点申请空间
    PATH_LIST *new1;
    new1 = malloc(sizeof(PATH_LIST));
    if(new1 == NULL)
    {
        printf("malloc error!\n");
    }

    // 2.为新节点赋值
    printf("NEW NODE = %s%s\n", path, name);
    strcpy(new1->path, path);
    strcpy(new1->name, name);

    // 3.让最后一个节点的指针域储存新节点的地址
    new1->next = head;
    PATH_LIST *q = head->prev;
    q->next = new1;
    new1->prev = q;
    head->prev = new1;

    return 0;
}

// 销毁链表
void delete_list_node(PATH_LIST *head)
{
    PATH_LIST *p, *q;
    q = p = head->next;
    while (p != head)
    {
        p = p->next;
        q->prev = NULL;
        q->next = NULL;
        free(q);
        q = p;
    }
}

// 判断链表是否为空，为空返回1，否则返回0
int list_isEmpty(PATH_LIST *head)
{
    if(head->next == head)
    {
        return 1;
    }

    return 0;
}

// 求链表长度
int len_list_node(PATH_LIST *head)
{
    int i = 0;
    PATH_LIST *p = head->next;
    while(p != head)
    {
        i++;
        p = p->next;
    }

    return i;
}

// 获取第x个元素
PATH_LIST *list_getElem(PATH_LIST *head, int x)
{
    PATH_LIST *p = head->next;
    int i = 0;
    while(p != head)
    {
        i++;
        if(i == x)
        {
            return p;
        }
        p = p->next;
    }

    return 0;
}

// 清空链表
int list_clear(PATH_LIST *HEAD)
{
    PATH_LIST *p, *q;
    q = p = HEAD->next;
    while (p != HEAD)
    {
        p = p->next;
        free(q);
        q = p;
    }
    p->next = p->prev = HEAD;

    return 0;
}

// 打印当前链表中的列表信息
int *list_print(PATH_LIST *PIC, PATH_LIST *MEDIA)
{
    // cnt链表元素个数
    int cnt_PIC = len_list_node(PIC);
    int cnt_MEDIA = len_list_node(MEDIA);

    // 背景打印
    lcd_draw_jpg(0, 0, PATH_LISTBG);

    int i = 0, j = 0, y;
    // 打印标题
    char title1[50];
    char title2[50];
    int page_MEDIA = (cnt_MEDIA / 10) + 1;
    int page_PIC = (cnt_PIC / 10) + 1;
    sprintf(title1, "媒体文件 (%d/%d)", i/10 + 1, page_MEDIA);
    sprintf(title2, "图片文件 (%d/%d)", j/10 + 1, page_PIC);
    Display_characterX(0, 0, title1, 0x000000FF, 1);
    Display_characterX(400, 0, title2, 0x000000FF, 1);

    // 触摸屏参数初始化
    int ts_x = 0, ts_y = 0;
    struct input_event buf;


    PATH_LIST *p; 
    // 打印第一次
    y = 0;
    do
    {
        i++;
        y += 35;
        p = list_getElem(MEDIA, i);
        Display_characterX(0, y, p->name, 0x00000000, 1);
    }while ((i < cnt_MEDIA) && (i % 10 != 0));
    y = 0;
    
    do
    {
        j++;
        y += 35;
        p = list_getElem(PIC, j);
        Display_characterX(400, y, p->name, 0x00000000, 1);
    }while ((j < cnt_PIC) && (j % 10 != 0));

    i = 0;
    j = 0;

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
            printf("list_print(x, y) = (%d, %d)\n", ts_x, ts_y);
            if (ts_y > 420) 
            if (ts_x < 160) // MEDIA 上一页
            {
                i -= 10;
                printf("MEDIA FRONT PAGE\n");
            }
            else if (160 < ts_x && ts_x < 320) // MEDIA 下一页
            {
                i += 10;
                printf("MEDIA NEXT PAGE\n");
            }
            else if (480 < ts_x && ts_x < 640) // PIC 上一页
            {
                j -= 10;
                printf("PIC FRONT PAGE\n");
            }
            else if (640 < ts_x && ts_x < 800) // PIC 下一页
            {
                j += 10;
                printf("PIC NEXT PAGE\n");
            }
            else if (320 < ts_x && ts_x < 480)// 返回上层
            { 
                lcd_draw_jpg(0, 0, PATH_UPDATE);
                break;
            }

            // MEDIA链表向上翻页
            if (i < 0)
            {
                i = (cnt_MEDIA / 10) * 10;
            }

            // MEDIA链表指针归零
            if (i >= cnt_MEDIA || i < 0)
            {
                i = 0;
            }
            
            // PIC链表向上翻页
            if (j < 0)
            {
                j = (cnt_PIC / 10) * 10;
            }

            // PIC链表指针归零
            if (j >= cnt_PIC || j < 0)
            {
                j = 0;
            }

            printf("i = %d, j = %d\n", i, j);

            // 背景打印
            lcd_draw_jpg(0, 0, PATH_LISTBG);

            // 打印标题
            sprintf(title1, "媒体文件 (%d/%d)", i/10 + 1, page_MEDIA);
            sprintf(title2, "图片文件 (%d/%d)", j/10 + 1, page_PIC);            
            Display_characterX(0, 0, title1, 0x000000FF, 1);
            Display_characterX(400, 0, title2, 0x000000FF, 1);

            int ii = i;
            y = 0;
            do
            {
                ii++;
                y += 35;
                p = list_getElem(MEDIA, ii);
                Display_characterX(0, y, p->name, 0x00000000, 1);
            }while ((ii < cnt_MEDIA) && (ii % 10 != 0));

            int jj = j;
            y = 0;
            do
            {
                jj++;
                y += 35;
                p = list_getElem(PIC, jj);
                Display_characterX(400, y, p->name, 0x00000000, 1);
            }while ((jj < cnt_PIC) && (jj % 10 != 0));

        }   

    }

    return 0;
}

// 返回元素在链表中的位置，没有返回0
int pos_list(PATH_LIST *HEAD, PATH_LIST *p)
{
    int i = 0;
    PATH_LIST *q = HEAD->next;
    while (p != q && q != HEAD)
    {
        q = q->next;
        i++;
        // printf("q->name = %s\n", q->name);
    }

    if (q != HEAD)
    {
        i += 1;
    }
    
    return i;
}

// 删除链表中的一个节点
PATH_LIST *delete_list(PATH_LIST *head, PATH_LIST *del_p)
{
	PATH_LIST *p;
	PATH_LIST *q;
	
	for (q=head, p=head->next; p!=head; q=p, p=p->next)
	{
		if(p == del_p)
		{
			q->next = p->next;
			p->next->prev = q;
			free(p);
			return q; //返回删除的那个节点的上一个节点。
		}
	}
	
	return NULL;
}
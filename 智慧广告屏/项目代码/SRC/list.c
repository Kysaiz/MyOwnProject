#include "../INC/myhead.h"

// ��������ͷ�ڵ�
PATH_LIST *init_list_head()
{
    // 1.Ϊͷ�ڵ�����ռ�
    PATH_LIST *head;
    head = malloc(sizeof(PATH_LIST));
    if(head == NULL)
    {
        printf("malloc error!\n");
    }

    // 2.Ϊͷ�ڵ�ָ����
    head->next = head;
    head->prev = head;

    return head;
}

// ͷ�巨������ظ��ڵ�
int insert_list_node(PATH_LIST *head, char path[], char name[])
{
    // 0.�ж��½ڵ��������Ƿ����ظ�
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

    // ��p!=headʱ�������ظ��ڵ�
    if (p != head)
    {
        return -1;
    }
    printf("UN REPEAT: ");

    // 1.Ϊͷ�ڵ�����ռ�
    PATH_LIST *new1;
    new1 = malloc(sizeof(PATH_LIST));
    if(new1 == NULL)
    {
        printf("malloc error!\n");
    }

    // 2.Ϊ�½ڵ㸳ֵ
    printf("NEW NODE = %s%s\n", path, name);
    strcpy(new1->path, path);
    strcpy(new1->name, name);

    // 3.�����һ���ڵ��ָ���򴢴��½ڵ�ĵ�ַ
    new1->next = head;
    PATH_LIST *q = head->prev;
    q->next = new1;
    new1->prev = q;
    head->prev = new1;

    return 0;
}

// ��������
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

// �ж������Ƿ�Ϊ�գ�Ϊ�շ���1�����򷵻�0
int list_isEmpty(PATH_LIST *head)
{
    if(head->next == head)
    {
        return 1;
    }

    return 0;
}

// ��������
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

// ��ȡ��x��Ԫ��
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

// �������
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

// ��ӡ��ǰ�����е��б���Ϣ
int *list_print(PATH_LIST *PIC, PATH_LIST *MEDIA)
{
    // cnt����Ԫ�ظ���
    int cnt_PIC = len_list_node(PIC);
    int cnt_MEDIA = len_list_node(MEDIA);

    // ������ӡ
    lcd_draw_jpg(0, 0, PATH_LISTBG);

    int i = 0, j = 0, y;
    // ��ӡ����
    char title1[50];
    char title2[50];
    int page_MEDIA = (cnt_MEDIA / 10) + 1;
    int page_PIC = (cnt_PIC / 10) + 1;
    sprintf(title1, "ý���ļ� (%d/%d)", i/10 + 1, page_MEDIA);
    sprintf(title2, "ͼƬ�ļ� (%d/%d)", j/10 + 1, page_PIC);
    Display_characterX(0, 0, title1, 0x000000FF, 1);
    Display_characterX(400, 0, title2, 0x000000FF, 1);

    // ������������ʼ��
    int ts_x = 0, ts_y = 0;
    struct input_event buf;


    PATH_LIST *p; 
    // ��ӡ��һ��
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

    // ����ѭ��
    while (1)
    {
        read(ts_fd, &buf, sizeof(buf));

        // ����⵽����仯ʱ����ȡX,Y����
        if (buf.type == EV_ABS && buf.code == ABS_X)
        {
            ts_x = buf.value * 800 / 1024;
        }
            
        if (buf.type == EV_ABS && buf.code == ABS_Y)
        {
            ts_y = buf.value * 480 / 600;        
        }

        // ����⵽�ɿ�ָ��ʱ����������������ж�
        if(buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
        {
            printf("list_print(x, y) = (%d, %d)\n", ts_x, ts_y);
            if (ts_y > 420) 
            if (ts_x < 160) // MEDIA ��һҳ
            {
                i -= 10;
                printf("MEDIA FRONT PAGE\n");
            }
            else if (160 < ts_x && ts_x < 320) // MEDIA ��һҳ
            {
                i += 10;
                printf("MEDIA NEXT PAGE\n");
            }
            else if (480 < ts_x && ts_x < 640) // PIC ��һҳ
            {
                j -= 10;
                printf("PIC FRONT PAGE\n");
            }
            else if (640 < ts_x && ts_x < 800) // PIC ��һҳ
            {
                j += 10;
                printf("PIC NEXT PAGE\n");
            }
            else if (320 < ts_x && ts_x < 480)// �����ϲ�
            { 
                lcd_draw_jpg(0, 0, PATH_UPDATE);
                break;
            }

            // MEDIA�������Ϸ�ҳ
            if (i < 0)
            {
                i = (cnt_MEDIA / 10) * 10;
            }

            // MEDIA����ָ�����
            if (i >= cnt_MEDIA || i < 0)
            {
                i = 0;
            }
            
            // PIC�������Ϸ�ҳ
            if (j < 0)
            {
                j = (cnt_PIC / 10) * 10;
            }

            // PIC����ָ�����
            if (j >= cnt_PIC || j < 0)
            {
                j = 0;
            }

            printf("i = %d, j = %d\n", i, j);

            // ������ӡ
            lcd_draw_jpg(0, 0, PATH_LISTBG);

            // ��ӡ����
            sprintf(title1, "ý���ļ� (%d/%d)", i/10 + 1, page_MEDIA);
            sprintf(title2, "ͼƬ�ļ� (%d/%d)", j/10 + 1, page_PIC);            
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

// ����Ԫ���������е�λ�ã�û�з���0
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

// ɾ�������е�һ���ڵ�
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
			return q; //����ɾ�����Ǹ��ڵ����һ���ڵ㡣
		}
	}
	
	return NULL;
}
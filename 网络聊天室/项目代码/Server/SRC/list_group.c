#include "../INC/myhead.h"

// 创建链表头节点
GROUP_MEM *init_listM_head()
{
    // 1.为头节点申请空间
    GROUP_MEM *head;
    head = malloc(sizeof(GROUP_MEM));
    if(head == NULL)
    {
        printf("malloc error!\n");
    }

    // 2.为头节点指针域
    head->next = head;
    head->prev = head;

    return head;
}

LIST_GROUP *init_listG_head()
{
    // 1.为头节点申请空间
    LIST_GROUP *head;
    head = malloc(sizeof(LIST_GROUP));
    if(head == NULL)
    {
        printf("malloc error!\n");
    }

    // 2.为头节点指针域
    head->next = head;
    head->prev = head;

    return head;
}

// 头插法插入节点（创建群聊）
void insert_listG_node(LIST_GROUP *head, char name[50], int num)
{
    // 1.为头节点申请空间
    LIST_GROUP *new1;
    new1 = malloc(sizeof(LIST_GROUP));
    GROUP_MEM *new2;
    new2 = malloc(sizeof(GROUP_MEM));
    if(new1 == NULL)
    {
        printf("malloc error!\n");
    }

    // 2.为新节点赋值
    strncpy(new1->name, name, strlen(name));
    new1->num = num;
    new1->head = init_listM_head();
    new1->sum = 0;

    // 3.让最后一个节点的指针域储存新节点的地址
    new1->next = head;
    LIST_GROUP *p = head->prev;
    p->next = new1;
    new1->prev = p;
    head->prev = new1;

    new2->next = new2;
    new2->prev = new2;
}

// 头插法插入节点（加群）
int insert_listM_node(int num, int fd, int job)
{
    LIST_GROUP *q = HEAD_G;
    for (q = q->next; q != HEAD_G; q = q->next)
    {
        if (q->num == num) // 找到是哪一个群
        {
            q->sum++;
            break;
        }
        
    }

    if (q == HEAD_G && q->next != HEAD_G) // 没有这个群号
    {
        return 2;
    }

    // 给群里其他成员发送入群通知
    GROUP_MEM *t;
    LIST *k = SearchTCP(fd);
    char buf[1024];
    for (t = q->head->next; t != q->head; t = t->next)
    {
        sprintf(buf, "\n入群通知：%s加入群[%s]！\n\n", k->name, q->name);
        send(t->connect_fd, buf, strlen(buf), MSG_NOSIGNAL);
    }
    
    // 1.为头节点申请空间
    GROUP_MEM *new1;
    new1 = malloc(sizeof(GROUP_MEM));
    if(new1 == NULL)
    {
        printf("malloc error!\n");
    }

    // 2.为新节点赋值
    new1->connect_fd = fd;
    new1->job = job;
    
    // 3.让最后一个节点的指针域储存新节点的地址
    new1->next = q->head;
    GROUP_MEM *p = q->head->prev;
    p->next = new1;
    new1->prev = p;
    q->head->prev = new1;

    return 1;
}

// 销毁链表
void delete_listG_node(LIST_GROUP *head)
{
    LIST_GROUP *p, *q;
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

//求链表长度
int len_listG_node(LIST_GROUP *head)
{
    int i = 0;
    LIST_GROUP *p = head->next;
    while(p != head)
    {
        i++;
        p = p->next;
    }

    return i;
}

// 删除链表中的一个节点（删群）
int delete_listG(LIST_GROUP *head, int num, int fd)
{
	LIST_GROUP *p, *g;
	
	for (p = head; p->next != head; p = p->next)
	{
		if(p->next->num == num) // 找到是哪一个群
		{
            GROUP_MEM *q;
            for (q = p->next->head; q->next != p->next->head; q = q->next)
            {
                if (q->next->connect_fd == fd) // 找到是哪一个成员发出的请求
                {
                    if (q->next->job == 2) // 如果是群主则解散群
                    {
                        
                        g = p->next;
                        p->next = g->next;
                        g->next->prev = p;
                        free(g);
                        return 1; //返回删除的那个节点的上一个节点。
                    }
                    else
                    {
                        
                        return 2;
                    }
                    
                }
            }
		}
	}
    
    return 2;
	
}

// 删除链表中的一个节点（退群）
int delete_listM(LIST_GROUP *head, int num, int fd)
{
	LIST_GROUP *p;
	for (p = head->next; p != head; p = p->next)
	{
		if(p->num == num) // 找到是哪一个群
		{
            GROUP_MEM *q, *g;
            for (q = p->head; q->next != p->head; q = q->next)
            {
                if (q->next->connect_fd == fd) // 找到是哪一个成员发出的请求
                {
                    if (q->next->job == 1) // 如果是群主则不能退群
                    {
                        g = q->next;
                        q->next = g->next;
                        g->next->prev = q;
                        p->sum--;

                        GROUP_MEM *t;
                        LIST *k = SearchTCP(fd);
                        char buf[1024];
                        for (t = p->head->next; t != p->head; t = t->next)
                        {
                            sprintf(buf, "\n退群通知：%s退出群[%s]！\n\n", k->name, p->name);
                            send(t->connect_fd, buf, strlen(buf), MSG_NOSIGNAL);
                        }
                        
                        free(g);
                        
                        return 1; 
                    }
                    else
                    {
                        return 2;
                    }
                    
                }
                
            }
		}
	}
	return 0;
}

// 返回元素在链表中的位置，没有返回0
int pos_listG(LIST_GROUP *HEAD, LIST_GROUP *p)
{
    int i = 0;
    LIST_GROUP *q = HEAD->next;
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

#include "../INC/myhead.h"

// 创建链表头节点
LIST *init_list_head()
{
    // 1.为头节点申请空间
    LIST *head;
    head = malloc(sizeof(LIST));
    if(head == NULL)
    {
        printf("malloc error!\n");
    }

    // 2.为头节点指针域
    head->next = head;
    head->prev = head;

    return head;
}

// 头插法插入节点
void insert_list_node(LIST *head, int connect_fd, char ip[30], int port)
{
    // 1.为头节点申请空间
    LIST *new1;
    new1 = malloc(sizeof(LIST));
    if(new1 == NULL)
    {
        printf("malloc error!\n");
    }

    // 2.为新节点赋值
    new1->connect_fd = connect_fd;
    strcpy(new1->ip, ip);
    sprintf(new1->status, "在线");
    new1->port = port;
    sprintf(new1->name, "用户%d", connect_fd);
    bzero(new1->msg, 1024);

    // 3.让最后一个节点的指针域储存新节点的地址
    new1->next = head;
    LIST *p = head->prev;
    p->next = new1;
    new1->prev = p;
    head->prev = new1;
}

// 销毁链表
void delete_list_node(LIST *head)
{
    LIST *p, *q;
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

// 正向遍历链表
void forward_list_node(LIST *head)
{
    LIST *p = head->next;
    printf("正向遍历链表：\n");

    while(p != head)
    {
        printf("%d", p->connect_fd);
        p = p->next;
    }
    printf("\n");
}

//判断链表是否为空，为空返回1，否则返回0
int isEmpty(LIST *head)
{
    if(head->next == head)
    {
        return 1;
    }

    return 0;
}

//求链表长度
int len_list_node(LIST *head)
{
    int i = 0;
    LIST *p = head->next;
    while(p != head)
    {
        i++;
        p = p->next;
    }

    return i;
}

// 删除链表中的一个节点
LIST *delete_list(LIST *head, int k)
{
	LIST *p;
	LIST *q;
	
	for (q=head, p=head->next; p!=head; q=p, p=p->next)
	{
		if(p->connect_fd == k)
		{
			q->next = p->next;
			p->next->prev = q;
			free(p);
			return q; //返回删除的那个节点的上一个节点。
		}
	}
	
	return NULL;
}

// 返回元素在链表中的位置，没有返回0
int pos_list(LIST *HEAD, LIST *p)
{
    int i = 0;
    LIST *q = HEAD->next;
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

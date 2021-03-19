#include "mytypes.h"
#include "myhead.h"

#define BUF_SIZE 20

// 用户广播
void send_ALLmsg(LIST *HEAD, int connect_fd, char *msg)
{
    LIST *q;
    char buf[1024];
    for (q = HEAD->next; HEAD != q; q = q->next)
    {
        if (q->connect_fd != connect_fd)
        {
            sprintf(buf, "客户端%d：%s", connect_fd, msg);
            send(q->connect_fd, buf, strlen(buf), MSG_NOSIGNAL);
        }
    }
    
}

// 服务器广播
void send_Severmsg(LIST *HEAD, char *msg)
{
    LIST *q;
    char buf[1024];
    for (q = HEAD->next; q != HEAD; q = q->next)
    {
        if (strncmp(q->status, "离线", 6))
        {
            sprintf(buf, "服务器：%s", msg);
            send(q->connect_fd, buf, strlen(buf), MSG_NOSIGNAL); 
        }
    }
    
}

// 客户上线
void send_online(LIST *HEAD, int connect_fd)
{
    LIST *q;
    char buf[1024];
    char name[50];
    for (q = HEAD->next; q != HEAD; q = q->next)
    {
        if (q->connect_fd == connect_fd)
        {
            strncpy(name, q->name, strlen(q->name));
            break;
        }
        
    }

    for (q = HEAD->next; q != HEAD; q = q->next)
    {
        if (q->connect_fd != connect_fd && strncmp(q->status, "离线", 6))
        {
            online_print(HEAD, q->connect_fd);
            sprintf(buf, "\n上线通知：%s上线！\n\n", name);
            send(q->connect_fd, buf, strlen(buf), MSG_NOSIGNAL);
        }
    }     
}

// 客户下线
void send_offline(LIST *HEAD, int connect_fd)
{
    LIST *q;
    char buf[1024];
    char name[50];
    for (q = HEAD->next; q != HEAD; q = q->next)
    {
        if (q->connect_fd == connect_fd)
        {
            strncpy(name, q->name, strlen(q->name));
            break;
        }
        
    }

    for (q = HEAD->next; q != HEAD; q = q->next)
    {
        if (q->connect_fd != connect_fd && strncmp(q->status, "离线", 6))
        {
            online_print(HEAD, q->connect_fd);
            sprintf(buf, "\n下线通知：%s下线！\n\n", name);
            send(q->connect_fd, buf, strlen(buf), MSG_NOSIGNAL);
        }
    }   
}

// 向客户端发送当前在线用户列表
void online_print(LIST *HEAD, int connect_fd)
{
    LIST *p;
    char buf[2048];
    sprintf(buf, "\n\t\t========当 前 在 线 用 户========\n\n");
    int i = 1;
    for (p = HEAD->next; p != HEAD; p = p->next)
    {
        if (strncmp(p->status, "隐身", 6) && strncmp(p->status, "离线", 6))
        {
            if (strlen(p->name) < 8)
            {
                sprintf(buf, "%s%d.%s\t\t%s\tIP:%s\tPORT:%d\n", buf, i, p->name, p->status, p->ip, p->port);
            }
            else
            {
                sprintf(buf, "%s%d.%s\t%s\tIP:%s\tPORT:%d\n", buf, i, p->name, p->status, p->ip, p->port);
            }
            i++;
        }
        
    }
    sprintf(buf, "%s\n", buf);
    int ret_send = send(connect_fd, buf, strlen(buf), MSG_NOSIGNAL);
    if (ret_send == -1)
    {
        ChangeStatus(connect_fd, "离线\n\0");
    }
    
    printf("\n向%d发送了列表\n", connect_fd);
}

// 打印群列表
void grouplist_print(LIST_GROUP *HEAD, int connect_fd)
{
    LIST_GROUP *p;
    char buf[2048];
    sprintf(buf, "\n\t\t========当 前 所 有 群 组========\n\n");
    int i = 1;
    for (p = HEAD->next; p != HEAD; p = p->next, i++)
    {
        sprintf(buf, "%s%d.%s\t\t群号:%d\t\t人数:%d\n", 
        buf, i, p->name, p->num, p->sum);
    }

    sprintf(buf, "%s\t\t========您 加 入 的 群 组========\n\n", buf);
    i = 1;
    for (p = HEAD->next; p != HEAD; p = p->next)
    {
        GROUP_MEM *q;
        for (q = p->head->next; q != p->head; q = q->next)
        {
            if (q->connect_fd == connect_fd)
            {
                sprintf(buf, "%s%d.%s\t\t群号:%d\t\t人数:%d\n", buf, i, p->name, p->num, p->sum);
                i++;
                break;
            }
        }
    }
    
    send(connect_fd, buf, strlen(buf), MSG_NOSIGNAL);

    printf("\n向%d发送了群列表\n", connect_fd);
}

// 更改用户名
void ChangeName(int connect_fd, char *buf)
{
    LIST *p = SearchTCP(connect_fd);
    bzero(p->name, sizeof(p->name));
    strncpy(p->name, buf, strlen(buf)-1);

    char tmp[1024];
    sprintf(tmp, "\n改名成功！您当前的用户名是：%s\n", buf);
    send(connect_fd, tmp, strlen(tmp), MSG_NOSIGNAL);
    
}

// 更改用户状态
void ChangeStatus(int connect_fd, char *buf)
{
    LIST *p = SearchTCP(connect_fd);
    _Bool flag;
    
    if (!strncmp(p->status, "隐身", 6) || !strncmp(p->status, "离线", 6))
    {
        flag = 0;
    }
    else
    {
        flag = 1;
    }
    
    
    bzero(p->status, sizeof(p->status));
    strncpy(p->status, buf, strlen(buf)-1);
    
    if (!strncmp(p->status, "隐身", 6) || !strncmp(p->status, "离线", 6))
    {
        
        if (flag)
        {
            
            send_offline(HEAD, connect_fd);
            
        }
    }
    else if (!strncmp(p->status, "离开", 6) || (!strncmp(p->status, "在线", 6)))
    {
        
        if (!flag)
        {
            
            send_online(HEAD, connect_fd);
            
        }
    }

    
    if (strncmp(p->status, "离线", 6))
    {
        char tmp[1024];
        sprintf(tmp, "\n更改成功！您当前的状态是：%s\n", buf);
        send(connect_fd, tmp, strlen(tmp), MSG_NOSIGNAL);
    }
    
}

// 用套接字找到链表位置
LIST *SearchTCP(int connect_fd)
{
    LIST *p;
    for (p = HEAD->next; p != HEAD; p = p->next)
    {
        if (p->connect_fd == connect_fd)
        {
            return p;
        }
    }
}

// 服务器私聊线程
void *Server_AtoB(void *s)
{
    // 获取被请求的人的套接字
    LIST *q = (LIST *)s;
    char *buf = q->msg;

    char g[10];
    int i = 0;
    while (1)
    {
        if (buf[i + 10] != ':')
        {
            g[i] = buf[i + 10];
            i++;
        }
        else
        {
            break;
        }
    }

    int num = atoi(g);
    char msg[1024];
    bzero(msg, 1024);
    sprintf(msg, "\n%s：%s", q->name, &buf[i + 11]);

    LIST *p = HEAD->next;
    for (i = 1; i <= num; p = p->next)
    {
        if (!strncmp(p->status, "在线", 6) || !(strncmp(p->status, "离开", 6)))
        {
            i++;
        }
        
    }
    int connect_fd = p->prev->connect_fd;

    send(connect_fd, msg, strlen(msg), MSG_NOSIGNAL);
    usleep(2000);
}

// 服务器群聊线程
void Server_1toN(char *buf, int fd)
{
    // 初始化一个属性变量，并将分离属性加入该变量
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    char g[10];
    int i = 0;
    while (1)
    {
        if (buf[i] != ':')
        {
            g[i] = buf[i];
            i++;
        }
        else
        {
            break;
        }
    }
    int num = atoi(g);
    
    // 根据群号找到对应的节点q
    LIST_GROUP *q;
    for (q = HEAD_G->next; q != HEAD_G; q = q->next)
    {
        if (q->num == num)
        {
            break;
        }
    }

    LIST *p = SearchTCP(fd);

    char msg[1024];
    bzero(msg, 1024);
    sprintf(msg, "群[%s]%s：%s\n", q->name, p->name, &buf[i + 1]);

    // 向群组q的每一个套接字发消息
    GROUP_MEM *t = q->head;
    for (t = t->next; t != q->head; t = t->next)
    {
        if (t->connect_fd != fd)
        {
            LIST *k = SearchTCP(t->connect_fd);
            if (strncmp(k->status, "离线", 6))
            {
                send(t->connect_fd, msg, strlen(msg), MSG_NOSIGNAL);
            }
            
        }
    }
    usleep(2000);
}

// 文件传输
int Server_File(int cnfd, char *buf)
{
    char buff[1024];
    char new[1024];
    bzero(new, 1024);
    bzero(buff, 1024);
    strncpy(buff, buf, strlen(buf));

    LIST *q = SearchTCP(cnfd);

    char *g = strtok(buff, "|");
    sprintf(new, "%s|%s", g, q->name);
    g = strtok(NULL, "|");
    int num = atoi(g);
    
    g = strtok(NULL, "|");
    int size = atoi(g);
    sprintf(new, "%s|%d|", new, size);

    g = strtok(NULL, "|");
    strncat(new, g, strlen(g));

    LIST *p = HEAD->next;
    int i;
    for (i = 1; i <= num; p = p->next)
    {
        if (!strncmp(p->status, "在线", 6) || !(strncmp(p->status, "离开", 6)))
        {
            i++;
        }
        
    }
    int connect_fd = p->prev->connect_fd;
    send(connect_fd, new, strlen(new), MSG_NOSIGNAL);
    
    // 被传输对象的回复
    char reply[10];
    bzero(reply, 10);
    recv(connect_fd, reply, 10, 0);
    printf("reply = %s", reply);
    if (strncmp("OK", reply, 2))
    {
        send(cnfd, "NO", 2, MSG_NOSIGNAL);
        return 2;
    }
    else
    {
        send(cnfd, "OK", 2, MSG_NOSIGNAL);
    }
    
    printf("\n正在传输文件\n");
    char *filebuf = calloc(1, 20);
    int cnt = size;
    while (1)
    {
        bzero(filebuf, BUF_SIZE);
        if (cnt < BUF_SIZE)
        {
            if (cnt == 0)
            {
                break;
            }
            recv(cnfd, filebuf, cnt, 0);
            send(connect_fd, filebuf, cnt, MSG_NOSIGNAL);
            cnt = 0;
        }
        else
        {
            recv(cnfd, filebuf, BUF_SIZE, 0);
            send(connect_fd, filebuf, BUF_SIZE, MSG_NOSIGNAL);
            cnt = cnt - BUF_SIZE;
        }
        
    }
    printf("传输文件结束\n");
}

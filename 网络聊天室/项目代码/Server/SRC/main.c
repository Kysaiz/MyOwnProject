#include "../INC/myhead.h"

_Bool Q_FLAG = 1;
int K_FLAG = 0;
int connect_fd = 0;
int group_num = 100;

// 服务器广播线程
void *Server_Send(void *sock_fd)
{
    int connect_fd = *(int *)sock_fd;
    char buf[1024];
    while (1)
    {
        bzero(buf, 1024);
        fgets(buf, 1024, stdin);
        send_Severmsg(HEAD, buf);
        if (strstr(buf, "Exit"))
        {
            Q_FLAG = 0;
            break;
        }
        
    }
    
}

// 定时检查在线状态
void timer(int sig)
{
    LIST *q;
    char buf[1024];
    sprintf(buf, "ServerCheck");
    for (q = HEAD->next; q != HEAD; q = q->next)
    {
        if (strncmp(q->status, "离线", 6))
        {
            int ret_send = send(q->connect_fd, buf, strlen(buf), MSG_NOSIGNAL);
            if (ret_send == -1)
            {
                ChangeStatus(q->connect_fd, "离线\n\0");
            }
        }
    }
    int ret_val = alarm(20);
}

// 服务器接收连接 线程例程
void *Server_Accept(void *sock_fd)
{
    int tmp_fd = *(int *)sock_fd;
    while (1)
    {
        // 等待客户端连接
        struct sockaddr_in clean_addr;
        socklen_t addrlen = sizeof(clean_addr);
        int t_fd = accept(tmp_fd, (struct sockaddr *)&clean_addr, &addrlen);
        if (t_fd <= 0)
        {
            perror("等待客户端连接失败");
            break;
        }
        else // 连接成功
        {
            char *buf_ip = inet_ntoa(clean_addr.sin_addr);
            int buf_port = ntohs(clean_addr.sin_port);
            connect_fd = t_fd;
            insert_list_node(HEAD, t_fd, buf_ip, buf_port);
            printf("IP = %s, PORT = %d\n", buf_ip, buf_port);

            send_online(HEAD, t_fd);
        }

    }
}

// 接收来自客户端的数据
void *Server_Msg(void *coo)
{
    // 初始化一个属性变量，并将分离属性加入该变量
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    int connect_fd = *(int*)coo;
    printf("客户端%d连接成功！\n", connect_fd);
    int ret_val, max_fd;
    char buf[1024];

    LIST *p = SearchTCP(connect_fd);

    while (1)
    {
        bzero(&p->msg, 1024);
        ret_val = recv(connect_fd, p->msg, 1024, 0);
        if (ret_val > 0)
        {
            printf("客户端%d：%s", connect_fd, p->msg);
            usleep(1000);
            int len = strlen(p->msg) - 1;

            // 判断用户输入了什么特殊标志
            if (!strncmp("Exit", p->msg, len)) // 用户下线
            {
                K_FLAG = connect_fd;
                break;
            }
            else if (!strncmp("Quit", p->msg, len)) // 关闭服务器
            {
                Q_FLAG = 0;
                K_FLAG = connect_fd;
                break;
            }
            else if (!strncmp("列表", p->msg, len)) // 打印在线用户列表
            {
                online_print(HEAD, connect_fd);
            }
            else if (!strncmp("群列表", p->msg, len)) // 打印群列表
            {
                grouplist_print(HEAD_G, connect_fd);
            }
            else if (!strncmp(p->msg, "请求用户名", 15)) // 用户请求用户名
            {
                send(connect_fd, p->name, strlen(p->name), MSG_NOSIGNAL);
            }
            else if (!strncmp(p->msg, "改名 ", 7)) // 用户改名
            {
                ChangeName(connect_fd, &p->msg[7]);
            }
            else if (!strncmp(p->msg, "请求当前状态", 18))
            {
                send(connect_fd, p->status, strlen(p->status), MSG_NOSIGNAL);
            }
            else if (!strncmp(p->msg, "改状态 ", 10)) // 用户改状态
            {
                ChangeStatus(connect_fd, &p->msg[10]);
            }
            else if (!strncmp(p->msg, "Mode私聊", 10)) // 私聊
            {
                LIST tmp = *p;
                pthread_t tid;
                pthread_create(&tid, &attr, Server_AtoB, &tmp);
            }
            else if (!strncmp(p->msg, "建群 ", 7)) // 建群
            {
                char *buff = &p->msg[7];
                buff[strlen(buff)-1] = '\0'; 
                insert_listG_node(HEAD_G, buff, group_num);
                insert_listM_node(group_num, connect_fd, 2);
                char tmp[1024];
                sprintf(tmp, "您已建立群[%s]，群号为%d\n", buff, group_num);
                send(connect_fd, tmp, strlen(tmp), MSG_NOSIGNAL);
                group_num++;
            }
            else if (!strncmp(p->msg, "加群 ", 7)) // 加群
            {
                char *buff = &p->msg[7];
                buff[strlen(buff)-1] = '\0'; 
                int num = atoi(buff);
                int ret_val = insert_listM_node(num, connect_fd, 1);

                if (ret_val == 1)
                {
                    char tmp[1024];
                    sprintf(tmp, "您已加入群[%s]\n", buff);
                    send(connect_fd, tmp, strlen(tmp), MSG_NOSIGNAL);
                }
                else if (ret_val == 2)
                {
                    char tmp[1024];
                    sprintf(tmp, "找不到该群号[%s]\n", buff);
                    send(connect_fd, tmp, strlen(tmp), MSG_NOSIGNAL);
                }
            }
            else if (!strncmp(p->msg, "退群 ", 7)) // 退群
            {
                char *buff = &p->msg[7];
                buff[strlen(buff)-1] = '\0'; 
                int num = atoi(buff);

                int ret_val = delete_listM(HEAD_G, num, connect_fd);
                if (ret_val == 1)
                {
                    char tmp[1024];
                    sprintf(tmp, "您已退出群[%s]\n", buff);
                    send(connect_fd, tmp, strlen(tmp), MSG_NOSIGNAL);
                }
                else
                {
                    char tmp[1024];
                    sprintf(tmp, "您是群主，您不能退群[%s]\n", buff);
                    send(connect_fd, tmp, strlen(tmp), MSG_NOSIGNAL);
                }
            }
            else if (!strncmp(p->msg, "散群 ", 7)) // 散群
            {
                char *buff = &p->msg[7];
                buff[strlen(buff)-1] = '\0'; 
                int num = atoi(buff);
                
                int ret_val = delete_listG(HEAD_G, num, connect_fd);
                if (ret_val == 1)
                {
                    char tmp[1024];
                    
                    sprintf(tmp, "您已解散群[%s]\n", buff);
                    send(connect_fd, tmp, strlen(tmp), MSG_NOSIGNAL);
                }
                else
                {
                    char tmp[1024];
                    
                    sprintf(tmp, "您不是群主，您不能解散群[%s]\n", buff);
                    send(connect_fd, tmp, strlen(tmp), MSG_NOSIGNAL);
                }
                
                
            }
            else if (!strncmp(p->msg, "发至群", 9)) // 客户发信息到群里
            {
                char *buff = &p->msg[9];
                buff[strlen(buff)-1] = '\0'; 
                Server_1toN(buff, connect_fd);
            }
            else if (!strncmp(p->msg, "Mode离线", 10)) // 客户端下线
            {
                ChangeStatus(connect_fd, &p->msg[4]);
            }
            else if (!strncmp(p->msg, "传文件|", 10)) // 客户端传文件
            {
                int ret = Server_File(connect_fd, p->msg);
            }
        }
        
    }
}

int main(int argc, char const *argv[])
{
    // sigset_t set, oset;
    // sigemptyset(&set);
    // sigemptyset(&oset);
    // sigaddset(&set, SIGPIPE);
    // sigprocmask(SIG_BLOCK, &set, &oset);
    signal(SIGALRM, timer);
    // 1、服务端 创建TCP套接字
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sock_fd)
    {
        perror("服务器端 创建TCP套接字失败");
        return -1;
    }
    perror("创建TCP套接字");
    
    // 2、服务端 绑定IP地址和端口号
    // 定义一个结构体变量
    struct sockaddr_in my_addr;
    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family  = AF_INET; // 选择为IPV4地址
    my_addr.sin_port = htons(atoi(argv[1])); // 配置端口号，注意转为网络字节序
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 设置IP地址并转为网络字节序 

    int ret_val = bind(sock_fd, (struct sockaddr *)&my_addr, sizeof(my_addr));
    if (-1 == ret_val)
    {
        perror("绑定失败");
        return -1;
    }
    perror("绑定IP地址和端口号");
    
    // 3、把套接字设置为监听状态
    ret_val = listen(sock_fd, 2); // LINUX下，设置的最大监听数 +4
    if (-1 == ret_val)
    {
        perror("设置监听失败");
        return -1;
    }
    perror("把套接字设置为监听状态");

    // 初始化一个属性变量，并将分离属性加入该变量
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    // 线程：接收客户端连接
    pthread_t tid;
    pthread_create(&tid, &attr, Server_Accept, &sock_fd);

    // 线程：服务器发送信息
    pthread_t tid1;
    pthread_create(&tid1, &attr, Server_Send, &sock_fd);
    
    // 5、接收客户端的消息
    HEAD = init_list_head();
    HEAD_G = init_listG_head();
    char buf[1024];
    ret_val = 0;
    
    while (Q_FLAG)
    {
        // 如果有新的套接字就创建一条新的线程
        if (connect_fd != 0)
        {
            alarm(30);
            usleep(1000);
            pthread_t tid;
            pthread_create(&tid, &attr, Server_Msg, &HEAD->prev->connect_fd);
            connect_fd = 0;
        }

        if (K_FLAG != 0)
        {
            send_offline(HEAD, K_FLAG);
            delete_list(HEAD, K_FLAG);
            printf("客户端%d退出连接！\n", K_FLAG);
            K_FLAG = 0;
        }
    }

    delete_list_node(HEAD);
    
    // 6、关闭服务端
    close(sock_fd);
    perror("关闭服务端");

    return 0;
}

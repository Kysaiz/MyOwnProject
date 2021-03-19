#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <signal.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include "client_head.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
_Bool Flag = 1;

/*
客户端：
常时显示在线用户列表，自己的用户名、状态
①私聊：选择在线的用户进行私聊
②群聊：可以选择创建、加入某个群聊组，可以收到来自群聊的消息，也可以向群聊组其他成员发消息
③改名：可以修改自己的用户名
④更改当前状态：可以设置在线、隐身、离开等状态
⑤文件传输：1选择传输对象2输入传输文件路径3传输4传输完成后比较文件大小
⑥离线
*/

// 数据接收
void *Msg_resv(void *tmp)
{
    sleep(1);
    int sock_fd = *(int *)tmp;
    int ret_val;
    char *msg = calloc(1, 1024);
    bzero(msg, 1024);
    fd_set set;

    // 3、从服务器接收信息到客户端
    while (Flag)
    {
        FD_ZERO(&set);
        FD_SET(sock_fd, &set);

        select(sock_fd + 1, &set, NULL, NULL, NULL);
        
        bzero(msg, 1024);
        ret_val = recv(sock_fd, msg, 1024, 0);

        if (!strncmp(msg, "服务器：Exit", strlen("服务器：Exit")) || !strncmp(msg, "服务器：Quit", strlen("服务器：Quit")))
        {
            printf("服务器已关闭！\n");
            Flag = 0;
            break;
        }
        else if (!strncmp(msg, "传文件|", 10))
        {
            Client_RecvFile(sock_fd, &msg[10]);
        }
        else if (!strncmp(msg, "ServerCheck", 11))
        {
            
        }
        else if (ret_val > 0)
        {
            printf("%s", msg);
        }           
        
    }
}

int main(int argc, char const *argv[])
{
    // 1、客户端 创建TCP套接字
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sock_fd)
    {
        perror("客户端 创建TCP套接字失败");
        return -1;
    }
    perror("创建TCP套接字");
    
    // 2、客户端 连接 服务端
    // 配置端口号和IP信息
    struct sockaddr_in server_addr;
    int addr_len = sizeof(server_addr);
    // bzero(&server_addr, addr_len);
    server_addr.sin_family  = AF_INET; // 选择为IPV4地址
    server_addr.sin_port = htons(atoi(argv[2])); // 配置端口号，注意转为网络字节序
    // server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 设置IP地址并转为网络字节序 
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    int ret_val = connect(sock_fd, (struct sockaddr *)&server_addr, addr_len);
    if (-1 == ret_val)
    {
        perror("客户端发起连接失败");
        return -1;
    }
    perror("客户端连接服务端");

    char *msg = calloc(1, 1024);
    bzero(msg, 1024);
    
    // 初始化一个属性变量，并将分离属性加入该变量
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);   
    
    pthread_t tid;
    pthread_create(&tid, &attr, Msg_resv, &sock_fd);

    fd_set set;
    int max_fd, mode;

    // 超时时间
    struct timeval time_val;
    time_val.tv_sec = 30;
    time_val.tv_usec = 0;
    int timer = 0;

    while (1)
    {
        if (timer == 6)
        {
            send(sock_fd, "改状态 离开\n\0", 17, 0);
        }

        if (timer >= 6 && timer <= 10)
        {
            printf("\n由于您长时间没有进行操作，将您的状态修改为离开\n");
            printf("请输入任意键继续\n");
        }
        else
        {
            // 向服务器请求打印在线列表
            Client_ListRequest(sock_fd);

            usleep(5000);
            // 打印用户菜单界面
            Client_menu();            
        }

        FD_ZERO(&set); // 清空集合
        FD_SET(STDIN_FILENO, &set); // 添加标准输入到集合中
        max_fd = STDIN_FILENO > sock_fd ? STDIN_FILENO : sock_fd;
        
        int ret_val = select(max_fd+1, &set, NULL, NULL, &time_val);

        if (ret_val == 0)
        {
            time_val.tv_sec = 30;
            timer++;
        }

        if (FD_ISSET(STDIN_FILENO, &set))
        {
            if (timer >= 6)
            {
                send(sock_fd, "改状态 在线\n\0", 17, 0);
            }
            mode = 0;
            scanf("%d", &mode);
            while(getchar() != '\n');

            if (mode == 1) // 私聊
            {
                ClientR_1to1(sock_fd);
            }
            else if (mode == 2) // 群聊
            {
                Client_1toN(sock_fd);
            }
            else if (mode == 3) // 改名
            {
                Client_ChangeName(sock_fd);
            }
            else if (mode == 4) // 状态
            {
                Client_status(sock_fd);
            }
            else if (mode == 5) // 传输文件
            {
                Client_SendFile(sock_fd);
            }
            else if (mode == 6) // 离线
            {
                send(sock_fd, "Mode离线\n\0", 11, 0);
                break;
            }
            else if (mode == 100) // 服务器管理员模式
            {
                
            }
            
            timer = 0;
        }

    }
    
    // 4、关闭客户端
    close(sock_fd);
    perror("关闭客户端");
    free(msg);

    return 0;
}

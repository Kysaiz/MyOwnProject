#ifndef __CLIENT__H
#define __CLIENT__H

#define BUF_SIZE 20

// 向服务器请求打印在线列表，返回值为在线人数
int Client_ListRequest(int sock_fd)
{
    int ret_val = send(sock_fd, "列表\0", strlen("列表\0"), 0);
    char msg[1024];
    bzero(msg, 1024);
    ret_val = recv(sock_fd, msg, 1024, 0);
    if (ret_val > 0)
    {
        printf("%s", msg);
    }

    int num = 0;
    char *buf = msg;
    while(strstr(buf, "\n") != NULL)
    {
        num++;
        buf = strstr(buf, "\n");
        buf += 1;
    }

    return num - 4;
}

// 向服务器请求打印群组列表，返回值群组个数
int Client_ListRequest_G(int sock_fd)
{
    int ret_val = send(sock_fd, "群列表\0", strlen("群列表\0"), 0);
    char msg[1024];
    bzero(msg, 1024);
    ret_val = recv(sock_fd, msg, 1024, 0);
    if (ret_val > 0)
    {
        printf("%s", msg);
    }

    int num = 0;
    char *buf = msg;
    while(strstr(buf, "\n") != NULL)
    {
        num++;
        buf = strstr(buf, "\n");
        buf += 1;
    }

    return num - 4;    
}

// 输入一个int类型
int input_a_int(char s[])
{
    int a;
    int flag = 1;
    
    while (1)
    {
        if (flag == 1)
        {
            printf("\n请%s：", s);
        }
        else
        {
            printf("\n数据有误，请重新%s：", s);
        }

        int ret = scanf("%d", &a);
        
        if(ret == 1 && getchar()=='\n')
        {
            break;
        }

        while(getchar() != '\n');
        flag = 0;
    }
    
    return a;
}

// 打印用户界面
void Client_menu()
{
    usleep(1000);
    printf("\n\t\t========输入数字选择功能========\n\n");
    printf("\t1. 私聊\n");
    printf("\t2. 群聊\n");
    printf("\t3. 改名\n");
    printf("\t4. 当前状态\n");
    printf("\t5. 文件传输\n");
    printf("\t6. 退出\n\n");
}

// 群聊模式
void Client_1toNOK(int num, int sock_fd)
{
    fd_set set;
    int max_fd;
    char *buf = calloc(1, 1024);
    printf("\n\t群聊对象：%d\t输入#0退出\n", num);
    while (1)
    {
        FD_ZERO(&set); // 清空集合
        FD_SET(STDIN_FILENO, &set); // 添加标准输入到集合中
        max_fd = STDIN_FILENO > sock_fd ? STDIN_FILENO : sock_fd;

        select(max_fd +1, &set, NULL, NULL, NULL);

        // 向服务器发送信号
        if (FD_ISSET(STDIN_FILENO, &set))
        {
            bzero(buf, 1024);
            fgets(buf, 1024, stdin);
            if (!strncmp(buf, "#0", 2))
            {
                break;
            }

            char msg[1024];
            sprintf(msg, "发至群%d:%s", num, buf);
            send(sock_fd, msg, strlen(msg), 0);
            
        }
        
    }
    free(buf);    
}

// 群聊模块
void Client_1toN(int sock_fd)
{
    char buf[1024];
    while (1)
    {
        int m = Client_ListRequest_G(sock_fd);
        printf("输入模式：\n\t建群 [群名]\n\t散群 [群号]\n\t加群 [群号]\n\t退群 [群号]\n\t群聊 [群号]\n其他返回\n\n");
        fgets(buf, 1024, stdin);

        if (!strncmp(buf, "建群 ", 7)) // 建群
        {
            send(sock_fd, buf, strlen(buf), 0);
            usleep(2000);

            bzero(buf, 1024);

            recv(sock_fd, buf, 1024, 0);
            printf("%s", buf);
            sleep(2);
        }
        else if (!strncmp(buf, "加群 ", 7)) // 加群
        {
            send(sock_fd, buf, strlen(buf), 0);
            usleep(2000);
            bzero(buf, 1024);

            recv(sock_fd, buf, 1024, 0);
            printf("%s", buf);
            sleep(2);
        }
        else if (!strncmp(buf, "退群 ", 7)) // 退群
        {
            send(sock_fd, buf, strlen(buf), 0);
            usleep(2000);   

            bzero(buf, 1024);

            recv(sock_fd, buf, 1024, 0);
            printf("%s", buf);
            sleep(2);         
        }
        else if (!strncmp(buf, "群聊 ", 7)) // 选择群聊
        {
            char *buff = &buf[7];
            Client_1toNOK(atoi(buff), sock_fd);
        }
        else if (!strncmp(buf, "散群 ", 7)) // 解散群聊
        {
            send(sock_fd, buf, strlen(buf), 0);
            usleep(2000);   

            bzero(buf, 1024);

            recv(sock_fd, buf, 1024, 0);
            printf("%s", buf);  
            sleep(2);
        }
        else
        {
            break;
        }
    }

}

// 私聊模块
void ClientR_1to1(int sock_fd)
{
    // 向服务器请求打印在线用户列表
    int m = Client_ListRequest(sock_fd);
    // 配置信号
    int num = input_a_int("请输入对方的编号，输入0返回");
    if (num > 0 && num <= m)
    {
        fd_set set;
        int max_fd;
        char *buf = calloc(1, 1024);
        printf("\n\t私聊对象：%d\t输入#0退出\n", num);
        while (1)
        {
            FD_ZERO(&set); // 清空集合
            FD_SET(STDIN_FILENO, &set); // 添加标准输入到集合中
            max_fd = STDIN_FILENO > sock_fd ? STDIN_FILENO : sock_fd;

            select(max_fd +1, &set, NULL, NULL, NULL);

            // 向服务器发送信号
            if (FD_ISSET(STDIN_FILENO, &set))
            {
                bzero(buf, 1024);
                fgets(buf, 1024, stdin);
                if (!strncmp(buf, "#0", 2))
                {
                    break;
                }

                char msg[1024];
                sprintf(msg, "Mode私聊%d:%s", num, buf);
                send(sock_fd, msg, strlen(msg), 0);
                
            }
            
        }
        free(buf);
    }
}

// 改名
void Client_ChangeName(int sock_fd)
{
    // 申请缓冲区
    char *msg = calloc(1, 1024);
    char *buf = calloc(1, 1024);
    bzero(msg, 1024);
    bzero(buf, 1024);

    // 获取当前用户名
    int ret_val = send(sock_fd, "请求用户名\0\n", strlen("请求用户名\0\n"), 0); 
    ret_val = recv(sock_fd, buf, 1024, 0);
    printf("当前用户名：%s\n", buf);
    
    // 改名
    printf("请输入修改后的用户名：");
    bzero(buf, 1024);
    fgets(msg, 1024, stdin);
    sprintf(buf, "改名 %s", msg);

    // 向服务器发送改名信号
    ret_val = send(sock_fd, buf, strlen(buf), 0);

    // 释放空间
    free(msg);
    free(buf);

    sleep(2);
}

// 更改当前状态
void Client_status(int sock_fd)
{
    char *msg = calloc(1, 1024);
    char *buf = calloc(1, 1024);
    bzero(msg, 1024);
    bzero(buf, 1024);

    // 获取当前状态
    int ret_val = send(sock_fd, "请求当前状态\n\0", strlen("请求当前状态\n\0"), 0); 
    ret_val = recv(sock_fd, buf, 1024, 0);
    printf("当前状态：%s\n", buf);

    // 改状态
    printf("请输入修改后的状态：\n\t\t在线\n\t\t隐身\n\t\t离开\n\n");
    bzero(buf, 1024);
    fgets(msg, 1024, stdin);
    sprintf(buf, "改状态 %s", msg);

    ret_val = send(sock_fd, buf, strlen(buf), 0);

    // 释放空间
    free(msg);
    free(buf);

    sleep(2);
}

// 文件传输
void Client_SendFile(int sock_fd)
{
    Client_ListRequest(sock_fd);
    printf("输入你要传输的对象编号(#0退出)：");
    char n[10];
    fgets(n, 10, stdin);
    int num;
    if (!strncmp(n, "#0", 2))
    {
        return;
    }
    else
    {
        num = atoi(n);
    }

    while (1)
    {
        char path[1024];
        printf("\n请输入你要传输的文件地址(#0退出)：");
        fgets(path, 1024, stdin);
        if (!strncmp(path, "#0", 2))
        {
            return;
        }
        path[strlen(path)-1] = '\0';

        int fd = open(path, O_RDONLY);
        if (fd > 0)
        {
            char buf[BUF_SIZE];

            struct stat statbuf;
            stat(path, &statbuf);
            int size = statbuf.st_size;

            int cnt = size;
            char msg[1024];
            sprintf(msg, "传文件|%d|%d|%s", num, size, path);
            send(sock_fd, msg, strlen(msg), 0);

            char reply[10];
            recv(sock_fd, reply, 10, 0);
            if (!strncmp("OK", reply, 1))
            {
                printf("对方同意了你的文件传输请求！\n");
                printf("正在发送文件...\n");
                while (1)
                {
                    bzero(buf, BUF_SIZE);

                    if (cnt < BUF_SIZE)
                    {
                        if (cnt == 0)
                        {
                            break;
                        }
                        read(fd, &buf, cnt);
                        send(sock_fd, buf, cnt, 0);
                        cnt = 0;
                    }
                    else
                    {
                        read(fd, &buf, BUF_SIZE);
                        send(sock_fd, buf, BUF_SIZE, 0);
                        cnt = cnt - BUF_SIZE;
                    }
                    
                }
                printf("文件发送成功！\n");
                sleep(1);
            }
            else
            {
                printf("传输文件请求被拒绝！\n");
                sleep(1);
            }
            break;
        }
        else
        {
            printf("文件不存在请重新输入!\n");
        }
        close(fd);
    }
}

// 文件接收
void Client_RecvFile(int sock_fd, char *msg)
{
    // 创建并打开新文件
    char *name1 = calloc(1, 50);
    name1 = strrchr(msg, '/');
    name1++;
    char path_tmp[1024];
    sprintf(path_tmp, "./FileRecv/%s", name1);

    int fd_c = open(path_tmp, O_CREAT | O_EXCL, 0777);
    if (fd_c >= 0)
    {
        close(fd_c);
    }

    int fd = open(path_tmp, O_WRONLY | O_TRUNC, 0777);

    char *g = strtok(msg, "|");

    char name2[50];
    strncpy(name2, g, strlen(g));

    g = strtok(NULL, "|");
    int size = atoi(g);
    
    char buf[BUF_SIZE];
    int cnt = size;

    // 用户选择接受还是拒绝
    printf("\n%s 给你发了一个文件 %s, 大小:%d\n", name2, name1, size);
    printf("\n\tOK：接受，NO：拒绝\n");
    char reply[10];
    fgets(reply, 10, stdin);
    send(sock_fd, reply, strlen(reply), 0);
    if (!strncmp("NO", reply, 2))
    {
        return;
    }
    
    while (cnt > 0)
    {
        bzero(buf, BUF_SIZE);
        if (cnt < BUF_SIZE)
        {
            recv(sock_fd, buf, cnt, 0);
            write(fd, buf, cnt);
            cnt = 0;
        }
        else
        {
            recv(sock_fd, buf, BUF_SIZE, 0);
            write(fd, buf, BUF_SIZE);
            cnt = cnt - BUF_SIZE;
        }
    }

    printf("\n文件接收成功！\n");
    printf("%s 已下载到 %s\n\n", name1, "./FileRecv");
    close(fd);
}

// 客户端下线
void Client_Offline(int sock_fd)
{
    send(sock_fd, "Mode离线\0", 10, 0);
}

#endif
#include <pthread.h>
#ifndef __MY__TYPES__H
#define __MY__TYPES__H

typedef struct list_node            // 每个用户结构体
{
    int connect_fd;                 // 用户的TCP已连接套接字
    char ip[30];                    // 用户的IP
    int port;                       // 用户的端口
    char name[50];                  // 用户名
    char status[10];                // 状态，离线，在线，隐身，离开
    char msg[1024];                 // 该用户发送的消息
	struct list_node *next;         // 后继指针
	struct list_node *prev;         // 前驱指针
}LIST;

typedef struct group_member         // 每个群的成员结构体
{
    int connect_fd;                 // 群成员的TCP套接字
    int job;                        // 职务，1.群员，2管理员
    struct group_member *next;      // 后继指针
    struct group_member *prev;      // 前驱指针
}GROUP_MEM;

typedef struct group_list           // 群结构体
{
    int num;                        // 群号
    char name[50];                  // 群名
    char msg[1024];                 // 群信息缓冲区
    int sum;                        // 群人数
    GROUP_MEM *head;                // 群聊组成员的TCP套接字
    struct group_list *next;        // 后继指针
    struct group_list *prev;        // 前驱指针
}LIST_GROUP;

LIST *HEAD;
LIST_GROUP *HEAD_G;

#endif
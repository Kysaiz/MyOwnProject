#ifndef _FILEPATH_LIST_H
#define _FILEPATH_LIST_H

// 创建链表头节点
PATH_LIST *init_list_head();

// 头插法插入非重复节点
int insert_list_node(PATH_LIST *head, char path[], char name[]);

// 销毁链表
void delete_list_node(PATH_LIST *head);

// 正向遍历链表
void forward_list_node(PATH_LIST *head);

//判断链表是否为空，为空返回1，否则返回0
int list_isEmpty(PATH_LIST *head);

//求链表长度
int len_list_node(PATH_LIST *head);

//获取第x个元素
PATH_LIST *list_getElem(PATH_LIST *head, int x);

// 清空链表
int list_clear(PATH_LIST *HEAD);

// 打印当前链表中的列表信息
int *list_print(PATH_LIST *PIC, PATH_LIST *MEDIA);

// 返回元素在链表中的位置，没有返回0
int pos_list(PATH_LIST *HEAD, PATH_LIST *p);

// 删除链表中的一个节点
PATH_LIST *delete_list(PATH_LIST *head, PATH_LIST *del_p);

#endif
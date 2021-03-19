#ifndef _IOLIST_H_
#define _IOLIST_H_

// 将链表的列表信息写入dat文件
void list_write(PATH_LIST *head, char filename[]);

// 从dat文件中读取列表信息到链表中
PATH_LIST *list_read(char filename[]);

#endif
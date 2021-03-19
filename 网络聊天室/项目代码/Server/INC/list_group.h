#ifndef __LISTG__H
#define __LISTG__H

LIST_GROUP *init_listG_head();
void insert_listG_node(LIST_GROUP *head, char name[50], int num);
int insert_listM_node(int num, int fd, int job);
int delete_listG(LIST_GROUP *head, int num, int fd);
int delete_listM(LIST_GROUP *head, int num, int fd);
void forward_listG_node(LIST_GROUP *head);
int len_listG_node(LIST_GROUP *head);
int pos_listG(LIST_GROUP *HEAD, LIST_GROUP *p);

#endif
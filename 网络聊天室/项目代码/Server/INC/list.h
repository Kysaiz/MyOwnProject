#ifndef __LIST__H
#define __LIST__H

LIST *init_list_head();
void insert_list_node(LIST *head, int connect_fd, char ip[30], int port);
void delete_list_node(LIST *head);
void forward_list_node(LIST *head);
int isEmpty(LIST *head);
int len_list_node(LIST *head);
LIST *delete_list(LIST *head, int k);
int pos_list(LIST *HEAD, LIST *p);

#endif
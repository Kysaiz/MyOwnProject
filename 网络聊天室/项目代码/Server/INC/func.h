#ifndef __MY__FUNC__H
#define __MY__FUNC__H

void send_ALLmsg(LIST *HEAD, int connect_fd, char *msg);
void send_Severmsg(LIST *HEAD, char *msg);
void send_online(LIST *HEAD, int connect_fd);
void send_offline(LIST *HEAD, int connect_fd);
void online_print(LIST *HEAD, int connect_fd);
void ChangeName(int connect_fd, char *buf);
int Online_count(LIST *HEAD);
LIST *SearchTCP(int connect_fd);
int Request_1to1(void *s);
void *Server_AtoB(void *s);
void ChangeStatus(int connect_fd, char *buf);
void *send_1toN_msg(void *p);
void grouplist_print(LIST_GROUP *HEAD, int connect_fd);
void Server_1toN(char *buf, int fd);
int Server_File(int cnfd, char *buf);

#endif
#ifndef _MY_FUNCTION_H
#define _MY_FUNCTION_H

short menu(void);
void stu_write(struct stu_info *head);
struct stu_info *init_list_head();
void init_stu_info(struct stu_info *head, struct stu_info *buf);
void in(struct stu_info *head);
int len_list_node(struct stu_info *head);
int stu_get_info(struct stu_info *head);
void stu_revise_info(struct stu_info *head);
void stu_del_info(struct stu_info *head);
void stu_all_info(struct stu_info *head);
short in_a_short(char s[]);
struct stu_info *init(void);
void delete_list_node(struct stu_info *head);
void pausee();

#endif

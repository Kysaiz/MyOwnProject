#include "main.h"

/*
    1. 假设混元形意门面向社会招揽学员，请设计一个程序来登记报名学员信息。

    要求：
    学员信息的结构体内容包括（学员姓名、学员年纪、学员期望技能等）。
    程序中需要封装函数例如： 
    入学函数（记录学员信息）
    毕业函数（移除学员信息）
    显示函数（输出所有学员信息）
    等......

*/

int main(int argc, char const *argv[])
{
    struct stu_info *head = init();

    short mode = 1;
    // pausee();
    while(mode != 7)
    {
        mode = menu();
        switch (mode)
        {
        case 1:
            stu_all_info(head);
            break;
        case 2:
            len_list_node(head);
            break;
        case 3:
            stu_get_info(head);
            break;
        case 4:
            stu_revise_info(head);
            break;
        case 5:
            in(head);
            break;
        case 6:
            stu_del_info(head);
            break;
        default:
            break;
        }
    }

    delete_list_node(head);
    
    return 0;
}

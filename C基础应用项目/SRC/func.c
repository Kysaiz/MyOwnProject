#include "main.h"

// 菜单界面
short menu(void)
{
    system("clear");
    // clear();

    printf("\t\t\t欢 迎 进 入 学 员 信 息 管 理 系 统");
    printf("\n\n\t1.所有学员的信息"); 
    printf("\n\n\t2.统计学员的人数"); 
    printf("\n\n\t3.查询学员信息"); 
    printf("\n\n\t4.修改学员信息"); 
    printf("\n\n\t5.录入学员信息"); 
    printf("\n\n\t6.删除学员信息"); 
    printf("\n\n\t7.退出系统\n\n"); 
    short mode;
    mode = in_a_short("输入一个数字编号");

    return mode;
}

// 将变更后的学员信息写入文件
void stu_write(struct stu_info *head)
{
    FILE *fp = fopen("../data/master.dat", "w");
    if(fp == NULL)
    {
        printf("open file %s failure. \n", "../data/master.dat");
    }

    struct stu_info *p = head->next;
    while (p != head)
    {
        int wr_cnt = fwrite(p, sizeof(struct stu_info), 1, fp);
        // printf("wr_cnt = %d\n", wr_cnt);
        p = p->next;
    }
    fclose(fp);
}

// 创建链表头节点
struct stu_info *init_list_head()
{
    // 1.为头节点申请空间
    struct stu_info *head;
    head = malloc(sizeof(struct stu_info));
    if(head == NULL)
    {
        printf("malloc error!\n");
    }

    // 2.为头节点指针域
    head->next = head;
    head->prev = head;

    return head;
}

// 读取文件中的学员信息
void init_stu_info(struct stu_info *head, struct stu_info *buf)
{
    // 1.为新节点申请空间
    struct stu_info *new1;
    new1 = malloc(sizeof(struct stu_info));
    if(new1 == NULL)
    {
        printf("malloc error!\n");
    }

    // 2.为新节点赋值
    strcpy(new1->name, buf->name);
    strcpy(new1->date, buf->date);
    strcpy(new1->num, buf->num);
    new1->age = buf->age;
    strcpy(new1->purpose, buf->purpose);

    // 3.让最后一个节点的指针域储存新节点的地址
    new1->next = head;
    struct stu_info *p = head->prev;
    p->next = new1;
    new1->prev = p;
    head->prev = new1;    

}

// 入学函数（录入学员信息）
void in(struct stu_info *head)
{
    system("clear");
    // 1.为新节点申请空间
    struct stu_info *new1;
    new1 = malloc(sizeof(struct stu_info));
    if(new1 == NULL)
    {
        printf("malloc error!\n");
    }

    // 2.为新节点赋值
    char tmp[50];
    short ntmp;
    printf("\t\t 录 入 学 生 信 息 模 块\n");
    
    printf("\n请输入学员姓名：");
    fgets(tmp, 50, stdin);
    strcpy(new1->name, tmp);

    printf("\n请输入学员学号：");
    fgets(tmp, 50, stdin);
    struct stu_info *q = head->next;
    while((strncmp(q->num, tmp, strlen(q->num)) != 0) && (q != head))
    {
        q = q->next;
    }
    while (q != head)
    {
        printf("\n已经存在的学号！请重新输入学员学号：");
        fgets(tmp, 50, stdin);
        q = head->next;
        while((strncmp(q->num, tmp, strlen(q->num)) != 0) && (q != head))
        {
            q = q->next;
        }
    }
    strcpy(new1->num, tmp);

    printf("\n请输入学员的入学日期（建议格式为2020-01-07）：");
    fgets(tmp, 50, stdin);
    strcpy(new1->date, tmp);
    
    ntmp = in_a_short("输入学员的年龄");
    new1->age = ntmp;

    printf("\n请输入学员期望技能：");
    fgets(tmp, 50, stdin);
    strcpy(new1->purpose, tmp);

    // 3.让最后一个节点的指针域储存新节点的地址
    new1->next = head;
    struct stu_info *p = head->prev;
    p->next = new1;
    new1->prev = p;
    head->prev = new1;

    // 4.将新数据写入文件中
    stu_write(head);

    pausee();
}

// 统计学员人数
int len_list_node(struct stu_info *head)
{
    system("clear");
    int i = 0;
    struct stu_info *p = head->next;
    while(p != head)
    {
        i++;
        p = p->next;
    }

    printf("\n\t\t当前系统中存有%d名学员的信息\n", i);
    pausee();
    return i; 
}

// 查询学员信息
int stu_get_info(struct stu_info *head)
{
    system("clear");
    struct stu_info *p = head->next;
    if (p == head) 
    {
        printf("\n\t\t当前没有录入学员信息，请先进行信息录入！\n");
    }
    else
    {
        char mode = 'Y';

        while (mode == 'Y')
        {
            printf("\n\t\t请输入要查询的学员的学号：");
            char buf[50];
            fgets(buf, 50, stdin);

            
            while((strncmp(p->num, buf, strlen(p->num)) != 0) && (p != head))
            {
                p = p->next;
            }
            
            if (p == head)
            {
                printf("\n\t没有查询到该学号，请问要重新输入吗？");
                printf("\n\t输入'Y'表示重新输入，其他表示返回上一级：");
                mode = getchar();
                while (getchar() != '\n');
            }else 
            {
                printf("\n\t\t所查询到的信息如下：\n");
                printf("\t姓名：%s", p->name);
                printf("\t学号：%s", p->num);
                printf("\t入学日期：%s", p->date);
                printf("\t年龄：%hd\n", p->age);
                printf("\t期望技能：%s\n", p->purpose);
                pausee();
                return 1;
            }
        }
    }

    pausee();
    return 0;
}

// 修改学员信息
void stu_revise_info(struct stu_info *head)
{
    system("clear");
    struct stu_info *p = head->next;
    if (p == head) 
    {
        printf("\n\t\t当前没有录入学员信息，请先进行信息录入！\n");
    }
    else
    {
        char mode = 'Y';
        while (mode == 'Y')
        {
            printf("\n\t\t\t请输入要修改的学员的学号：");
            char buf[50];
            fgets(buf, 50, stdin);

            while((strncmp(p->num, buf, strlen(p->num)) != 0) && (p != head))
            {
                p = p->next;
            }
            
            if (p == head)
            {
                printf("\n\t没有查询到该学号，请问要重新输入吗？");
                printf("\n\t输入'Y'表示重新输入，其他表示返回上一级：");
                mode = getchar();
                while (getchar() != '\n');
            }else 
            {
                short mode_c = 1;
                while (0 < mode_c && mode_c < 6)
                {
                    system("clear");
                    printf("\n\t\t所查询到的学员信息如下：\n");
                    printf("\t1.姓名：%s", p->name);
                    printf("\t2.学号：%s", p->num);
                    printf("\t3.入学日期：%s", p->date);
                    printf("\t4.年龄：%hd\n", p->age);
                    printf("\t5.期望技能：%s\n", p->purpose);                
                    mode_c = in_a_short("输入你要修改的项（1~5，其他退出修改模块）");
                    char tmp[50];
                    short n_tmp;
                    switch (mode_c)
                    {
                    case 1:
                        printf("\n\t请输入修改后的姓名：");
                        fgets(tmp, 50, stdin);
                        strcpy(p->name, tmp);
                        break;
                    case 2:
                        printf("\n\t请输入修改后的学号：");
                        fgets(tmp, 50, stdin);
                        strcpy(p->num, tmp);
                        break;
                    case 3:
                        printf("\n\t请输入修改后的入学日期：");
                        fgets(tmp, 50, stdin);
                        strcpy(p->date, tmp);
                        break;
                    case 4:
                        n_tmp = in_a_short("输入修改后的年龄");
                        p->age = n_tmp;
                        break;
                    case 5:
                        printf("\n\t请输入修改后的期望技能：");
                        fgets(tmp, 50, stdin);
                        strcpy(p->purpose, tmp);
                        break;
                    default:
                        break;
                    }
                    if (0 < mode_c && mode_c < 6) 
                    {
                        printf("\n\t\t修改成功！\n");
                        stu_write(head);
                        sleep(1);
                    }
                }
                break;
            }
        }
    }
  
    pausee();
}

// 毕业函数（移除学员信息）
void stu_del_info(struct stu_info *head)
{
    system("clear");
    struct stu_info *p = head->next;
    if (p == head) 
    {
        printf("\n\t\t当前没有录入学员信息，请先进行信息录入！\n");
    }
    else
    {
        char mode = 'Y';

        while (mode == 'Y')
        {
            printf("\n\t\t\t请输入要删除的学员的学号：");
            char buf[50];
            fgets(buf, 50, stdin);

            struct stu_info *p = head->next;
            while((strncmp(p->num, buf, strlen(p->num)) != 0) && (p != head))
            {
                p = p->next;
            }
            
            if (p == head)
            {
                printf("\n\t没有查询到该学号，请问要重新输入吗？");
                printf("\n\t输入'Y'表示重新输入，其他表示返回上一级：");
                mode = getchar();
                while (getchar() != '\n');
            }else 
            {
                printf("\n\t\t所查询到的学员信息如下：\n");
                printf("\t姓名：%s", p->name);
                printf("\t学号：%s", p->num);
                printf("\t入学日期：%s", p->date);
                printf("\t年龄：%hd\n", p->age);
                printf("\t期望技能：%s\n", p->purpose);

                printf("\n\t请问确定要删除吗？");
                printf("\n\t输入'Y'表示确定，其他表示取消并返回上一级：");
                char mode_c;
                mode_c = getchar();
                while (getchar() != '\n');
                if (mode_c == 'Y')
                {
                    struct stu_info *q = p->prev;
                    q->next = p->next; 
                    p->next->prev = q;
                    free(p);
                    printf("\n\n\t\t删除成功！\n");
                    stu_write(head);
                    sleep(1);
                }
                break;
            }
        }

    }

    pausee();
}

// 显示函数（输出所有学员信息）
void stu_all_info(struct stu_info *head)
{
    system("clear");
    struct stu_info *p = head->next;
    if (p == head) 
    {
        printf("\n\t\t当前没有录入学员信息，请先进行信息录入！\n");
    }
    else
    {
        int i;
        for (i = 1; p != head; i++, p = p->next)
        {
            printf("\n\t\t第%d个学员的信息：\n", i);
            printf("\t姓名：%s", p->name);
            printf("\t学号：%s", p->num);
            printf("\t入学日期：%s", p->date);
            printf("\t年龄：%hd\n", p->age);
            printf("\t期望技能：%s\n", p->purpose);
        }
    }

    pausee();
}

// 输入一个short类型
short in_a_short(char s[])
{
    short a;
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

        int ret = scanf("%hd", &a);
        
        if(ret == 1 && getchar()=='\n')
        {
            break;
        }

        while(getchar() != '\n');
        flag = 0;
    }
    
    return a;
}

// 程序开始时读取文件数据
struct stu_info *init(void)
{
    FILE *fp = fopen("../data/master.dat", "a+");
    if (fp == NULL)
    {
        printf("open file %s failure. \n", "../data/master.dat");
    }
    else 
    {
        printf("open file success. \n");
    }

    struct stu_info *head = init_list_head();
    struct stu_info rd_buf;
    short fs_ret;
    int size = sizeof(struct stu_info);
    printf("size = %d\n", size);
    int fr_ret = fread(&rd_buf, size, 1, fp);
    printf("rd_buf = %s\n", rd_buf.name);
    printf("fr_ret = %d\n", fr_ret);
    printf("feof(fp) = %d\n", feof(fp));
    if (!feof(fp))
    {
        init_stu_info(head, &rd_buf);
        fread(&rd_buf, size, 1, fp);
        while (!feof(fp))
        {
            init_stu_info(head, &rd_buf);
            printf("rd_buf = %s\n", rd_buf.name);
            printf("feof(fp) = %d\n", feof(fp));
            fread(&rd_buf, size, 1, fp);
        }
    }
    fclose(fp);

    return head;
}

// 销毁链表
void delete_list_node(struct stu_info *head)
{
    struct stu_info *p, *q;
    q = p = head->next;
    while (p != head)
    {
        p = p->next;
        q->prev = NULL;
        q->next = NULL;
        free(q);
        q = p;
    }
}

// 仿windows下的system("pause");
void pausee()
{
    printf("\nPress any key to continue\n") ;
    struct termios te;
    int ch;
    tcgetattr( STDIN_FILENO,&te);
    te.c_lflag &=~( ICANON|ECHO);
    tcsetattr(STDIN_FILENO,TCSANOW,&te);
    tcflush(STDIN_FILENO,TCIFLUSH);
    fgetc(stdin) ; 
    te.c_lflag |=( ICANON|ECHO);
    tcsetattr(STDIN_FILENO,TCSANOW,&te);
}
#ifndef _MY_TYPES_H
#define _MY_TYPES_H

struct stu_info
{
    char name[50]; // 姓名
    char num[50];  // 学号
    char date[50]; // 入学日期
    char purpose[50]; // 期望技能
    short age;     // 年龄
    struct stu_info *next;
    struct stu_info *prev;
};

#endif
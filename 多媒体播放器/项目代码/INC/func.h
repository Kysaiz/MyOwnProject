#ifndef _MY_FUNC_H
#define _MY_FUNC_H

// 检索DIRNAME中的文件，分类插入到不同链表中
int dir_new(char DIRNAME[]);

// 屏幕打印颜色
int show_color(int color);

// 判断图片类型，BMP返回1，JPG返回2，否则返回0
int pic_type();

// 相册模块
int album();

// 资源更新模块
int SourceUpdate();

// 抽奖模块
int game();

// 音乐模块
int music();

// 把BG_TMP的内容重新映射
int BG_print();

#endif
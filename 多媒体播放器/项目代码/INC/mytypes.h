#ifndef _MYTYPES_H
#define _MYTYPES_H

typedef struct FILE_PATH
{
    char path[50];  // 文件所在地址
    char name[50];  // 文件名称
    struct FILE_PATH *next;
    struct FILE_PATH *prev;
}PATH_LIST;

// 触摸屏、LCD的指示符和LCD内存映射
int ts_fd, lcd_fd, *mmap_fd;

int ts_x, ts_y, TS_X, TS_Y;

struct input_event buf;

PATH_LIST *MEDIA;
PATH_LIST *PIC;
PATH_LIST *VIDEO;

// LCD显示缓存
int BG_TMP[480][800];

#define PATH_GACHADIR       "./gachaPIC/"               // 抽奖图片所在文件夹

#define PATH_MENU           "./Image/menu.jpg"          // 主界面
#define PATH_GACHA          "./Image/gacha.jpg"         // 抽奖界面
#define PATH_GAME           "./Image/game.jpg"          // 游戏界面
#define PATH_UPDATE         "./Image/update.jpg"        // 资源更新界面
#define PATH_LISTBG         "./Image/listbg.jpg"        // 文件一览背景

#define PATH_ALBUMARROW     "./Image/album_arrow.jpg"   // 相册菜单箭头
#define PATH_ALBUMLISTBG    "./Image/album_listbg.jpg"  // 相册列表背景
#define PATH_ALBUMMENU      "./Image/album_menubg.jpg"  // 相册菜单背景

#define PATH_MEDIA          "./Image/media.jpg"         // 媒体分支界面
#define PATH_MEDIA0         "./Image/media_0.jpg"       // 音乐界面_播放
#define PATH_MEDIA1         "./Image/media_1.jpg"       // 音乐界面_暂停
#define PATH_MEDIA2         "./Image/media_2.jpg"       // 音乐界面_继续

#define PATH_UDISKLOAD      "./Image/udisk_load.jpg"    // U盘读取中
#define PATH_UDISKCOMP      "./Image/udisk_comp.jpg"    // U盘读取完成

#define PATH_VIDEO0         "./Image/video_0.jpg"       // 视频列表选择界面
#define PATH_VIDEO1         "./Image/video_1.jpg"       // 暂停播放视频
#define PATH_VIDEO2         "./Image/video_2.jpg"       // 正在播放视频

#endif
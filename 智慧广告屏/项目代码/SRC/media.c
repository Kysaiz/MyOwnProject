#include "../INC/myhead.h"

// 循环播放mplayer
int media_play(PATH_LIST *media)
{
    system("killall -9 mplayer");
    char buf[300];
    sprintf(buf, 
            "mplayer -af volume=-40 -softvol -softvol-max 200 -quiet -slave %s\"%s\" -loop 0 &",
            media->path, media->name);
    printf("buf = %s", buf);
    system(buf);
    return 0;
}

// 暂停mplayer
int media_pause()
{
    system("killall -19 mplayer");
    return 0;
}

// 继续mplayer
int media_cont()
{
    system("killall -18 mplayer");
    return 0;
}

// 停止mplayer
int media_stop()
{
    system("killall -9 mplayer");
    return 0;
}

// mplayer音量(未完成)
int media_volume_down()
{
    system("0\n");
    return 0;
}

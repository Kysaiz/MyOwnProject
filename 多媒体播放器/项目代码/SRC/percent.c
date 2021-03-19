#include "../INC/myhead.h"

int video_percent(FILE *fp)
{
    char buf[1024];

    // 获取百分比
    while (1)
    {
        // 获取当前状态
        bzero(buf, 1024); // 清空用户的缓冲区

        system("echo get_percent_pos > ./tmp/mplayer_fifo1"); // 发送命令从播放器中获得当前的百分比时间

        fgets(buf, 1024, fp); // 从管道中获得的数据返回
        while (!strstr(buf , "ANS_PERCENT_POSITION"))
        {
            printf("**%s\n", buf); // 输出得到的数据

            fgets(buf, 1024, fp);
        }

        printf("buf:%s\n", buf); // 输出得到的数据

        sleep(1);
    }
}

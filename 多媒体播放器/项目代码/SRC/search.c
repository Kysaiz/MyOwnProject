#include "../INC/myhead.h"

// 检索DIRNAME中的文件，分类插入到不同链表中
int dir_new(char DIRNAME[])
{
    DIR *dp = opendir(DIRNAME);
    if (dp == NULL)
    {
        printf("open dir %s failed\n", DIRNAME);
        return -1;
    }

    struct dirent *ep = NULL;
    int cnt_media = 0, cnt_pic = 0;
	while(1)
	{
		ep = readdir(dp);
		if(ep == NULL) //代表读取完毕，结束读取目录
        {
		    break;
        }

		if(ep->d_name[0] == '.') //代表读取到'.'开头的文件，继续读取下一个目录项
        {
            continue;
        }
        // printf("(dirnew)%s\n", ep->d_name);

        char *pos = strrchr(ep->d_name, '.');
        if (pos == NULL)
        {
            continue;
        }
        // printf("%s\n", pos);

        int len = strlen(pos);
        if (!strncmp(pos, ".mp3", len))
        {
            char tmp[100];
            sprintf(tmp, "%s", ep->d_name);
            insert_list_node(MEDIA, DIRNAME, tmp);
            printf("(dirnew)%s\n", ep->d_name);
            cnt_media++;            
        }
        if (!strncmp(pos, ".mp4", len) || !strncmp(pos, ".avi", len))
        {
            char tmp[100];
            sprintf(tmp, "%s", ep->d_name);
            insert_list_node(VIDEO, DIRNAME, tmp);
            printf("(dirnew)%s\n", ep->d_name);
            cnt_media++;
        }
        else if (!strncmp(pos, ".bmp", len) || !strncmp(pos, ".jpg", len))
        {
            char tmp[100];
            sprintf(tmp, "%s", ep->d_name);            
            insert_list_node(PIC, DIRNAME, tmp);
            printf("(dirnew)%s\n", ep->d_name);
            cnt_pic++;
        }
        
	}

    printf("picture COUNT = %d\n", cnt_pic);
    printf("media COUNT = %d\n", cnt_media);

    closedir(dp);
    
    return 0;
}

// 从U盘判读取文件
int copy_from_udisk(char DIRNAME[])
{
    // 显示U盘读取中背景
    show_jpg(PATH_UDISKLOAD);
    BG_print();
    
    DIR *dp = opendir(DIRNAME);
    printf("opendir %s\n", DIRNAME);
    if (dp == NULL)
    {
        printf("open dir %s failed\n", DIRNAME);
        return -1;
    }

    struct dirent *ep = NULL;
    int cnt_media = 0, cnt_pic = 0;
	while(1)
	{
		ep = readdir(dp);
		if(ep == NULL) //代表读取完毕，结束读取目录
        {
		    break;
        }

		if(ep->d_name[0] == '.') //代表读取到'.'开头的文件，继续读取下一个目录项
        {
            continue;
        }
        // printf("%s\n", ep->d_name);

        char *pos = strrchr(ep->d_name, '.');
        if (pos == NULL)
        {
            continue;
        }
        // printf("%s\n", pos);

        int len = strlen(pos);
        if (!strncmp(pos, ".mp3", len))
        {
            // 打印正在读取的文件名字
            BG_print();
            char f[300];
            sprintf(f, "正在复制 %s ……", ep->d_name);
            Display_characterX(0, 220, f, 0x000000FF, 2);

            // COPY到本地
            char tmp[300];
            sprintf(tmp, "cp %s\"%s\" ./media/", DIRNAME, ep->d_name);
            system(tmp);
            printf("%s\n", tmp);
            cnt_media++;
        }
        else if (!strncmp(pos, ".bmp", len) || !strncmp(pos, ".jpg", len))
        {
            // 打印正在读取的文件名字
            BG_print();
            char f[300];
            sprintf(f, "正在复制 %s ……", ep->d_name);
            Display_characterX(0, 220, f, 0x000000FF, 2);

            // COPY到本地
            char tmp[300];
            sprintf(tmp, "cp %s\"%s\" ./album/", DIRNAME, ep->d_name);
            system(tmp);
            printf("%s\n", tmp);
            cnt_pic++;            
        }
        else if (!strncmp(pos, ".mp4", len) || !strncmp(pos, ".avi", len))
        {
            // 打印正在读取的文件名字
            BG_print();
            char f[300];
            sprintf(f, "正在复制 %s ……", ep->d_name);
            Display_characterX(0, 220, f, 0x000000FF, 2);

            // COPY到本地
            char tmp[300];
            sprintf(tmp, "cp %s\"%s\" /Memory/", DIRNAME, ep->d_name);
            system(tmp);
            printf("%s\n", tmp);
            cnt_media++;            
        }
	}

    printf("picture COUNT = %d\n", cnt_pic);
    printf("media COUNT = %d\n", cnt_media);
    
    closedir(dp);

    // 显示U盘读取完成背景
    show_jpg(PATH_UDISKCOMP);
    BG_print();

    // 触摸屏参数初始化
    int ts_x = 0, ts_y = 0;
    struct input_event buf;

    // 点击任意处返回
    while (1)
    {
        read(ts_fd, &buf, sizeof(buf));
        if(buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
        {
            break;
        }
    }

    // 显示上一层背景
    lcd_draw_jpg(0, 0, PATH_UPDATE);

    return 0;    
}
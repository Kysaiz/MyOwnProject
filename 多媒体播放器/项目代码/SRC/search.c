#include "../INC/myhead.h"

// ����DIRNAME�е��ļ���������뵽��ͬ������
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
		if(ep == NULL) //�����ȡ��ϣ�������ȡĿ¼
        {
		    break;
        }

		if(ep->d_name[0] == '.') //�����ȡ��'.'��ͷ���ļ���������ȡ��һ��Ŀ¼��
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

// ��U���ж�ȡ�ļ�
int copy_from_udisk(char DIRNAME[])
{
    // ��ʾU�̶�ȡ�б���
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
		if(ep == NULL) //�����ȡ��ϣ�������ȡĿ¼
        {
		    break;
        }

		if(ep->d_name[0] == '.') //�����ȡ��'.'��ͷ���ļ���������ȡ��һ��Ŀ¼��
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
            // ��ӡ���ڶ�ȡ���ļ�����
            BG_print();
            char f[300];
            sprintf(f, "���ڸ��� %s ����", ep->d_name);
            Display_characterX(0, 220, f, 0x000000FF, 2);

            // COPY������
            char tmp[300];
            sprintf(tmp, "cp %s\"%s\" ./media/", DIRNAME, ep->d_name);
            system(tmp);
            printf("%s\n", tmp);
            cnt_media++;
        }
        else if (!strncmp(pos, ".bmp", len) || !strncmp(pos, ".jpg", len))
        {
            // ��ӡ���ڶ�ȡ���ļ�����
            BG_print();
            char f[300];
            sprintf(f, "���ڸ��� %s ����", ep->d_name);
            Display_characterX(0, 220, f, 0x000000FF, 2);

            // COPY������
            char tmp[300];
            sprintf(tmp, "cp %s\"%s\" ./album/", DIRNAME, ep->d_name);
            system(tmp);
            printf("%s\n", tmp);
            cnt_pic++;            
        }
        else if (!strncmp(pos, ".mp4", len) || !strncmp(pos, ".avi", len))
        {
            // ��ӡ���ڶ�ȡ���ļ�����
            BG_print();
            char f[300];
            sprintf(f, "���ڸ��� %s ����", ep->d_name);
            Display_characterX(0, 220, f, 0x000000FF, 2);

            // COPY������
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

    // ��ʾU�̶�ȡ��ɱ���
    show_jpg(PATH_UDISKCOMP);
    BG_print();

    // ������������ʼ��
    int ts_x = 0, ts_y = 0;
    struct input_event buf;

    // ������⴦����
    while (1)
    {
        read(ts_fd, &buf, sizeof(buf));
        if(buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
        {
            break;
        }
    }

    // ��ʾ��һ�㱳��
    lcd_draw_jpg(0, 0, PATH_UPDATE);

    return 0;    
}
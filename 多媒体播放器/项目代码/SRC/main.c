#include "../INC/myhead.h"

void *ts_reading(void *arg)
{
    ts_x = 0;
    ts_y = 0;
    TS_X = 0;
    TS_Y = 0;
    while (1)
    {
        read(ts_fd, &buf, sizeof(buf));
        // usleep(10000);
        // ����⵽����仯ʱ����ȡX,Y����
        if (buf.type == EV_ABS && buf.code == ABS_X)
        {
            TS_X = buf.value * 800 / 1024;
        }
            
        if (buf.type == EV_ABS && buf.code == ABS_Y)
        {
            TS_Y = buf.value * 480 / 600;        
        }

        if (buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
        {
            ts_x = TS_X;
            ts_y = TS_Y;
        }
        
    }
}

int main(int argc,char **argv)
{
    lcd_open();
    mmap_lcd();
    touch_open();

    // ���庯����ʼ��
    Init_Font();

    // ��ʼ��ý���ļ���BMP+JPGͼƬ�ļ�������
    // ������dat�ļ��ж�ȡ�б���Ϣ��������
    MEDIA = list_read("LIST_MEDIA.dat");
    PIC = list_read("LIST_PIC.dat");
    VIDEO = list_read("LIST_VIDEO.dat");
    
    // ��ָ������Դ�ļ����ж�ȡ�ļ�
    // dir_new("/Kysaiz/project1/media/", MEDIA_HEAD, PIC_HEAD);
    // dir_new("/Kysaiz/project1/album/", MEDIA_HEAD, PIC_HEAD);

    // �˵�����
    lcd_draw_jpg(0, 0, PATH_MENU);

    // �������ź�̨����
    system("killall -9 mplayer");
    // system("mplayer -af volume=-33 -softvol -softvol-max 200 -quiet -slave ./test.mp3 -loop 0 &");

    // ��ʼ��һ�����Ա����������������Լ���ñ���
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_t tid_ts;
    pthread_create(&tid_ts, &attr, ts_reading, NULL);

    // ����ѭ��
    while (1)
    {
        // ����⵽�ɿ�ָ��ʱ����������������ж�
        if (buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
        {
            printf("main(x, y) = (%d, %d)\n", ts_x, ts_y);
            if(ts_x < 400 && ts_y > 0 && ts_y < 240) // ���ϣ����
            {
                album();
            }
            else if (ts_x > 400 && ts_y < 240) // ���ϣ�ý��
            {
                media();
            }
            else if(ts_x < 400 && ts_y > 240) // ���£���Դ����
            {
                SourceUpdate();
            }
            else if(ts_x > 400 && ts_y > 240) // ���£����˳齱
            {
                game();
            }
            memset(&buf, 0, sizeof(buf));
        }        
    }
    
    media_quit();
    delete_list_node(MEDIA);
    delete_list_node(PIC);
    delete_list_node(VIDEO);

    close(ts_fd);
    close(lcd_fd);
    munmap(mmap_fd, 800*480*4);

    UnInit_Font();

	return 0;	
}

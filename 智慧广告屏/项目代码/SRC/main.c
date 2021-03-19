#include "../INC/myhead.h"

int main(int argc,char **argv)
{
    lcd_open();
    mmap_lcd();
    touch_open();

    // ���庯����ʼ��
    Init_Font();

    // ��ʼ��ý���ļ���BMP+JPGͼƬ�ļ�������
    // ������dat�ļ��ж�ȡ�б���Ϣ��������
    PATH_LIST *MEDIA_HEAD = list_read("LIST_MEDIA.dat");
    PATH_LIST *PIC_HEAD = list_read("LIST_PIC.dat");
    
    // ��ָ������Դ�ļ����ж�ȡ�ļ�
    // dir_new("/Kysaiz/project1/media/", MEDIA_HEAD, PIC_HEAD);
    // dir_new("/Kysaiz/project1/album/", MEDIA_HEAD, PIC_HEAD);

    // �˵�����
    lcd_draw_jpg(0, 0, PATH_MENU);

    // �������ź�̨����
    system("killall -9 mplayer");
    // system("mplayer -af volume=-33 -softvol -softvol-max 200 -quiet -slave ./test.mp3 -loop 0 &");

    // �򿪴������豸
    int ts_x = 0, ts_y = 0;
    struct input_event buf;

    // ����ѭ��
    while (1)
    {
        read(ts_fd, &buf, sizeof(buf));

        // ����⵽����仯ʱ����ȡX,Y����
        if (buf.type == EV_ABS && buf.code == ABS_X)
        {
            ts_x = buf.value * 800 / 1024;
        }
            
        if (buf.type == EV_ABS && buf.code == ABS_Y)
        {
            ts_y = buf.value * 480 / 600;        
        }

        // ����⵽�ɿ�ָ��ʱ����������������ж�
        if(buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
        {
            printf("main(x, y) = (%d, %d)\n", ts_x, ts_y);
            if(ts_x < 400 && ts_y < 240) // ���ϣ����
            {
                album(PIC_HEAD);
            }
            
            if (ts_x > 400 && ts_y < 240) // ���ϣ�ý��
            {
                media(MEDIA_HEAD);
            }
            
            if(ts_x < 400 && ts_y > 240) // ���£���Դ����
            {
                SourceUpdate(MEDIA_HEAD, PIC_HEAD);
            }

            if(ts_x > 400 && ts_y > 240) // ���£����˳齱
            {
                game(PIC_HEAD);
            }
        }        
    }
    
    media_quit();
    delete_list_node(MEDIA_HEAD);
    delete_list_node(PIC_HEAD);

    close(ts_fd);
    close(lcd_fd);
    munmap(mmap_fd, 800*480*4);

    UnInit_Font();

	return 0;	
}

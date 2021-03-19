#include "../INC/myhead.h"

// 将链表的列表信息写入dat文件
void list_write(PATH_LIST *head, char filename[])
{
    char nametmp[300];
    sprintf(nametmp, "./DATA/%s", filename);
    FILE *fp = fopen(nametmp, "w");
    if (fp == NULL)
    {
        printf("open file %s failure. \n", nametmp);
    }

    PATH_LIST *p = head->next;
    while (p != head)
    {
        int wr_cnt = fwrite(p, sizeof(PATH_LIST), 1, fp);
        printf("path = %s name = %s\n", p->path, p->name);
        p = p->next;
    }

    printf("list_write %s succeed\n", filename);
    fclose(fp);    
}

// 从dat文件中读取列表信息到链表中
PATH_LIST *list_read(char filename[])
{
    char nametmp[300];
    sprintf(nametmp, "./DATA/%s", filename);
    FILE *fp = fopen(nametmp, "a+");
    if (fp == NULL)
    {
        printf("open file %s failure. \n", nametmp);
    }
    else 
    {
        printf("open file success. \n");
    }

    PATH_LIST *tmp = init_list_head();
    PATH_LIST rd_buf;
    short fs_ret;
    int size = sizeof(PATH_LIST);
    printf("sizeof(PATH_LIST) = %d\n", size);
    fread(&rd_buf, size, 1, fp);
    printf("rd_buf.path = %s rd_buf.name = %s\n", rd_buf.path, rd_buf.name);
    printf("feof(fp) = %d\n", feof(fp));
    if (!feof(fp))
    {
        insert_list_node(tmp, rd_buf.path, rd_buf.name);
        fread(&rd_buf, size, 1, fp);
        while (!feof(fp))
        {
            insert_list_node(tmp, rd_buf.path, rd_buf.name);
            printf("rd_buf.path = %s rd_buf.name = %s\n", rd_buf.path, rd_buf.name);
            printf("feof(fp) = %d\n", feof(fp));            
            fread(&rd_buf, size, 1, fp);
        }
    }
    
    fclose(fp);

    return tmp;
}
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

char *strip(char *s)
{
    int i, len = strlen(s);

    for (i = 0; i < len - 1; i++)
    {
        if (s[i + 1] == 32)
        {        
            int j;
            for (j = i + 1; j < len; j++)
            {
                s[j] = s[j + 1];
            }
            len--;
            i--;
        }
    }
    s[len] = '\0';

    return s;
}

int main(int argc, char const *argv[])
{
    char *s = (char *)malloc(sizeof(char) * 100);

    printf("请输入一个字符串：");
    fgets(s, 100, stdin);
    strip(s);
    printf("去掉空格后结果为：%s", s);

    free(s);
    return 0;
}

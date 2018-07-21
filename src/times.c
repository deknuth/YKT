#include "../inc/main.h"

void setSwipe(int type,int index)
{
    int i;
    for(i=index; i>-1; i--)
    {
        rule[type].swipe[i]++;
        if((rule[type].table[i])>>7)
        {
            printf("i--: %d\n",i);
            break;
        }
    }
    for(i=index; i<86400; i++)
    {
        rule[type].swipe[i]++;
        if((rule[type].table[i])>>7)
        {
            printf("i++: %d\n",i);
            break;
        }
    }
}

int matchTime(unsigned char p_type,int index)
{
    struct tm *p;
    unsigned char week = 0;
    unsigned char num = 0;
    unsigned char pos = 0;
    time_t seconds;

    seconds = time((time_t*)NULL);
    p = localtime(&seconds);         // 取得当地时间
    seconds %= 86400;
    switch(p->tm_wday)
    {
    case 0:
        week = 6;
        break;
    case 1:
        week = 0;
        break;
    case 2:
        week = 1;
        break;
    case 3:
        week = 2;
        break;
    case 4:
        week = 3;
        break;
    case 5:
        week = 4;
        break;
    case 6:
        week = 5;
        break;
    default:
        break;
    }
    seconds += week*86400;
//    printf("seconds: %ld\n",seconds);
    if((rule[p_type].table[seconds]) & 0x01)
    {
        pos = (rule[p_type].table[seconds]) >> 3;
        num = pInfo[index].position[pos];
 //       printf("xxx: %d %d %d\n",pos,num,rule[p_type].swipe[seconds]);
        if(num >= rule[p_type].swipe[seconds])
        {
            m_printf("swipe times overflow!\n");
            return 0;
        }
        else
        {
            //  setSwipe(p_type,seconds);
            pInfo[index].position[pos]++;
            return 1;
        }

    }
    else
        return 0;
}

int setTime(int second)
{
    struct timeval tv;
    struct timezone tz;
    tv.tv_sec = second+E8;
    tv.tv_usec = 0;
    tz.tz_dsttime = 0;
    tz.tz_minuteswest = 0;
    settimeofday(&tv,&tz);
}

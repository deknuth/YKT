#include "../inc/main.h"

long diffTime(struct timeval *x,struct timeval *y)
{
    if (x->tv_sec > y->tv_sec)
        return -1;
    if((x->tv_sec==y->tv_sec) && (x->tv_usec>y->tv_usec))
        return -1;
    return (y->tv_sec - x->tv_sec)*1000000 + (y->tv_usec - x->tv_usec);
}

void byteOrder(void)
{
    union UN
    {
        int i;
        char c;
    }u;
    u.i = 1;

    if (u.c == 1)
        printf("little endian\n");
    else
        printf("big endian\n");
}

int GetBit(int num)			// just user for alarm.c
{
    int i = 0,j = 16;
    while(j--)
    {
        if((num>>(i++)) & 0x01)
            return i-1;
    }
    return -1;
}

#ifdef CAN_CARD
void compleId(const char *src,char* dst)
{
    switch(16-strlen(src))
    {
    case 1:
        snprintf(dst,17,"0%s",src);
        break;
    case 2:
        snprintf(dst,17,"00%s",src);
        break;
    case 3:
        snprintf(dst,17,"000%s",src);
        break;
    case 4:
        snprintf(dst,17,"0000%s",src);
        break;
    case 5:
        snprintf(dst,17,"00000%s",src);
        break;
    case 6:
        snprintf(dst,17,"000000%s",src);
        break;
    case 7:
        snprintf(dst,17,"0000000%s",src);
        break;
    case 8:
        snprintf(dst,17,"00000000%s",src);
        break;
    default:
        snprintf(dst,17,"%s",src);
        break;
    }
}
#elif COM_CARD
void compleId(const char *src,char* dst)
{
    switch(16-strlen(src))
    {
    case 1:
        snprintf(dst,17,"0%02X",src);
        break;
    case 2:
        snprintf(dst,17,"00%02X",src);
        break;
    case 3:
        snprintf(dst,17,"000%02X",src);
        break;
    case 4:
        snprintf(dst,17,"0000%02X",src);
        break;
    case 5:
        snprintf(dst,17,"00000%02X",src);
        break;
    case 6:
        snprintf(dst,17,"000000%02X",src);
        break;
    case 7:
        snprintf(dst,17,"0000000%02X",src);
        break;
    case 8:
        snprintf(dst,17,"00000000%02X",src);
        break;
    default:
        snprintf(dst,17,"%02X",src);
        break;
    }
}
#endif

void m_printf(const char *format,...)
{
#ifdef MDEBUG
    time_t now;
    struct tm  *timenow;
    time(&now);
    timenow = localtime(&now);
    printf("JSON[%02d:%02d:%02d] ",timenow->tm_hour,timenow->tm_min,timenow->tm_sec);
    va_list args;
    va_start(args,format);
    vprintf(format,args);
    va_end(args);
#endif
}

void r_printf(const char *format,...)
{
#ifdef RDEBUG
    time_t now;
    struct tm  *timenow;
    time(&now);
    timenow = localtime(&now);
    printf("ROBOT[%02d:%02d:%02d] ",timenow->tm_hour,timenow->tm_min,timenow->tm_sec);
    va_list args;
    va_start(args,format);
    vprintf(format,args);
    va_end(args);
#endif
}

void u_printf(const char *format,...)
{
#ifdef UDP
    time_t  now;
    struct tm  *timenow;
    time(&now);
    timenow = localtime(&now);
    printf("UDP[%02d:%02d:%02d] ",timenow->tm_hour,timenow->tm_min,timenow->tm_sec);
    va_list args;
    va_start(args,format);
    vprintf(format,args);
    va_end(args);
#endif
}


void w_printf(const char *format,...)
{
#ifdef WDEBUG
    time_t now;
    struct tm  *timenow;
    time(&now);
    timenow = localtime(&now);
    printf("WATCH[%02d:%02d:%02d] ",timenow->tm_hour,timenow->tm_min,timenow->tm_sec);
    va_list args;
    va_start(args,format);
    vprintf(format,args);
    va_end(args);
#endif
}

void s_printf(const char *format,...)
{
#ifdef SQL
    time_t now;
    struct tm  *timenow;
    time(&now);
    timenow = localtime(&now);
    printf("SQL[%02d:%02d:%02d] ",timenow->tm_hour,timenow->tm_min,timenow->tm_sec);
    va_list args;
    va_start(args,format);
    vprintf(format,args);
    va_end(args);
#endif
}


void tprintf(void)
{
    time_t now;
    struct tm  *timenow;
    time(&now);
    timenow = localtime(&now);
    printf("Local time is: %s",asctime(timenow));
}

void gtime(char *date)		// get time
{
    time_t now;
    struct tm *tp;
    now = time(&now);
    tp = localtime(&now);
    memset(date,0x00,15);
    snprintf(date, 15, "%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d", (1900+tp->tm_year),(1+tp->tm_mon), tp->tm_mday, tp->tm_hour, tp->tm_min,tp->tm_sec);
    date[14] = '\0';
}

void gdate(char *date)		// get date
{
    time_t now;
    struct tm *tp;
    now = time(&now);
    tp = localtime(&now);
    memset(date,0x00,16);
    snprintf(date,8, "%4.4d%2.2d%2.2d", (1900+tp->tm_year),(1+tp->tm_mon), tp->tm_mday);
    date[8] = '\0';
}

int _write(int fd,void *buffer,int length)
{
    if(fd < 3)
        return 0;
    int left;
    int wLen;
    char *ptr;
    fd_set fds;
    struct timeval tv={3,0};
    ptr = buffer;
    left = length;
    while(left > 0)
    {
        FD_ZERO(&fds);
        FD_SET(fd,&fds);
        tv.tv_sec = 2;
        switch(select(fd+1,0,&fds,NULL,&tv))
        {
        case -1:
            return 0;
        case 0:
            break;
        default:
            if(FD_ISSET(fd,&fds))
            {
                wLen = write(fd,ptr,left);
                if(wLen <= 0)
                {
                    if(errno == EINTR)
                        wLen = 0;
                    else if(errno == EAGAIN)
                        wLen = 0;
                    else
                    {
                        printf("Write: %s",strerror(errno));
                        return 0;
                    }
                }
                left -= wLen;
                ptr += wLen;
            }
            break;
        }
    }
    return 1;
}

int _m_write(int fd,void *buffer,int length)
{
    if(fd < 3)
        return 0;
    int left;
    int wLen;
    char *ptr;
    fd_set fds;
    struct timeval tv={3,0};
    ptr = buffer;
    left = length;
    while(left > 0)
    {
        FD_ZERO(&fds);
        FD_SET(fd,&fds);
        tv.tv_sec = 2;
        switch(select(fd+1,0,&fds,NULL,&tv))
        {
        case -1:
            return 0;
        case 0:
            break;
        default:
            if(FD_ISSET(fd,&fds))
            {
                wLen = write(fd,ptr,left);
                if(wLen <= 0)
                {
                    if(errno == EINTR)
                        wLen = 0;
                    else if(errno == EAGAIN)
                        wLen = 0;
                    else
                    {
                        printf("Write: %s",strerror(errno));
                        return 0;
                    }
                }
                left -= wLen;
                ptr += wLen;
            }
            break;
        }
    }
    return 1;
}

void StringToHex(unsigned char *dst,const char *src)	// ACII to hex 	only 0-F
{
    int i,j;
    int sLen = strlen(src);
    const unsigned ASCValue[128] = {
            0,0,0,0,0,0,0,0,0,0,		// 0-9
            0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,1,
            2,3,4,5,6,7,8,9,0,0,
            0,0,0,0,0,10,11,12,13,14,
            15,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,10,11,12,
            13,14,15,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0
    };
    for(i = 0,j = 0; i < sLen;)
    {
        dst[j] = ASCValue[(unsigned char)src[i++]];
        dst[j] <<= 4;
        dst[j++] += ASCValue[(unsigned char)src[i++]];
    }
}

int StringToDEC(unsigned char *dst,const char *src)
{
    int i,j = 1;
    i = strlen(src)-1;
    if(i > 3)
        return 0;
    const unsigned ASCValue[128] = {
            0,0,0,0,0,0,0,0,0,0,		// 0-9
            0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,1,
            2,3,4,5,6,7,8,9,0,0,
            0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0
    };
    for(; i >= 0;j--)
    {
        dst[j] = ASCValue[(unsigned char)src[i--]];
        if(i >= 0)
            dst[j] += ASCValue[(unsigned char)src[i--]]*10;
    }
    return 1;
}

void DECToString(char *dst,const unsigned char *src,int sLen)
{
     int  i;
     char szTmp[3] = {0};
     for(i = 0; i < sLen; i++)
     {
         sprintf(szTmp, "%02d",src[i]);
         memcpy(dst, szTmp, 2);
         dst += 2;
     }
}

int raCopy(unsigned char* dst,const unsigned char *src,int len)		// right aligned copy
{
    int i = 0;
    while(*(src+i++) == 0);
    i--;
    if(i >= len)
        return 0;
    else
        memcpy(dst,src+i,len-i);
    return (len-i);
}

int HexToString(char *dst,const unsigned char *src,int sLen)	// hex to ASCII
{
    const char* hexDigits = "0123456789ABCDEF";
    int a,b,j = 0,k = 0;
    char c;
    for(k = 0; k < sLen; k++)
    {
        c = src[k];
//        printf("%x\n",c);
        /*
        if((c & 0x80) > 0)		// c > 128
        {
            a = c & 0x0F;
            b = c & 0xF0;
            dst[j++] = hexDigits[(a+b)/16];
            dst[j++] = hexDigits[a%16];
        }
        */
       // else
        {
            dst[j++] = hexDigits[c%16];
            //dst[j++] = hexDigits[c%16];
        }
    }
    dst[j] = '\0';
    return j;
}

double power(double x, int y)
{
    double i,b;
    b = x;
    if (y == 0)
        return 1;
    for(i = 1; i < y; i++)
        x *= b;
    return x;
}

char *SubLeft(char *dst,char *src, int n)
{
    char *p = src;
    char *q = dst;
    int len = strlen(src);
    if(n > len)
        n = len;
    while(n--)
        *(q++) = *(p++);
    *(q++)='\0';
    return dst;
}

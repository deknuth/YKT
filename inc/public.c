#include "../inc/main.h"
static int rm_dir(char *name);
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

void m_printf(const char *format,...)
{
#ifdef MDEBUG
	time_t now;
	struct tm  *timenow;
	time(&now);
	timenow = localtime(&now);
	printf("APP[%02d:%02d:%02d] ",timenow->tm_hour,timenow->tm_min,timenow->tm_sec);
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
						lprintf(lfd, FATAL,"Write: %s",strerror(errno));
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
						lprintf(lfd, FATAL,"Write: %s",strerror(errno));
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
		if((c & 0x80) > 0)		// c > 128
		{
			a = c & 0x0F;
			b = c & 0xF0;
			dst[j++] = hexDigits[(a+b)/16];
			dst[j++] = hexDigits[a%16];
		}
		else
		{
			dst[j++] = hexDigits[c/16];
			dst[j++] = hexDigits[c%16];
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

double atod(char *temp, int len)		// string to double
{
	int i = 0;
	for(i=0; i<len; i++)
		if(!(isdigit(temp[i]) || temp[i] =='.'))
			break;
	if(i == len)
	{
		i = 0;
		int flag = 0;
		int flag1 = 0;
		char* str;
		double ret = 0.00;
		int llen,j;
		j = 1;
		llen = len;
		if(temp[0] == '-')
		{
			str = ++temp;
			flag1 = 1;
		}
		else
			str = temp;
		if((i = kmp(str,"."))!= -1)
		{
			i--;
			llen--;
		}
		else
			i = len - 1;
		while(llen--)
		{
			if (flag == 0)	//
			{
				if(*str >= '0' && *str <= '9')
					ret += (*str - '0')*power(10,i--);
				else if(*str == 0x2E)
				{
					flag++;
					str++;
				}
				else
					break;
			}
			if (flag == 1)
			{
				if(*str >= '0' && *str <= '9')
					ret += (*str - '0')/power(10,j++);
				else
					break;
			}
			str++;
		}
		if (flag1 == 0)
			return ret;
		else
			return -ret;
	}
	else
		return 0;
}

void dtoa(double n, char s[])		// double to string
{
	char temp[12]={0};
	int i = 0,sign,j = 1,zs,sy,m,l;
	double xs;
	zs = (int)n;
	xs = n - (double)zs;

	if ((sign = zs) < 0.0)
	{
		zs = -zs;
		xs = -xs;
	}
	do
	{
		s[i++] = zs%10 + '0';
	}while ((zs /= 10) >0);
	if(sign < 0)
		s[i++] = '-';
	l = i - 1;
	for(m = 0; m < i; m++)
		temp[m] = s[l--];
	for(m = 0; m < i; m++)
		s[m] = temp[m];
	if(xs > -0.000000001 && xs < 0.000000001)
		s[i++] = '.';
	while(1)
	{
		if(!(xs > -0.000000001 && xs < 0.000000001))
		{
			sy = (int)(power(10, j)*xs);
			s[i++] = sy + '0';
			xs = xs - (double)sy / power(10,j);
			j++;
		}
		else
			break;
	}
	s[i] = '\0';
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

char *SubMid(char *dst,char *src, int n,int m)
{
    char *p = src;
    char *q = dst;
    int len = strlen(src);
    if(n > len)
    	n = len-m;
    if(m < 0)
    	m=0;
    if(m > len)
    	return NULL;
    p += m;
    while(n--)
    	*(q++) = *(p++);
    *(q++)='\0';
    return dst;
}

static int *get_pid(char *key)
{
	if(strlen(key) > 80)
		return 0;
    char buff[80];
    int *pid = NULL;
    FILE *pidred;
    int i = 0;
    char *ret;
    pid = (int *)malloc(8*sizeof(int));
    if (NULL == pid)
    	lprintf(lfd, FATAL,"pid malloc error!");
    memset(pid,0,8*sizeof(int));
    snprintf(buff,sizeof(buff),"pidof %s",key);
    pidred = popen(buff,"r");
    memset(buff,0,80);
    ret = fgets(buff,79,pidred);
    pclose(pidred);
    ret = strtok(buff," ");
    while (ret != NULL)
    {
        pid[i++]=atoi(ret);
        ret =strtok(NULL," ");
    }
    return pid;
}

int IdCrc(unsigned char* id,int len)
{
	int i = 0,j=1;
	int crc = 0;
	for(i=0; i<(len-1); i++)
		crc += (id[i] | id[j++]);
	return crc;
}

int stop_process(char* pName)
{
    int *pid;
    int i = 0;
    if((pid = get_pid(pName)) > 0)
    {
		while (pid[i])
		{
			printf("%d\n",pid[i]);
			kill(pid[i++],10);
		}
    	free(pid);
    	return 1;
    }
    else
    	return  0;
}

void stop(int signo)
{
	int i = 0;
	for(i=0; i<MAXEPOLLSIZE; i++)
	{
		if(rInfo[i].tfd)
			close(rInfo[i].tfd);
		if(wInfo[i].wfd)
			close(wInfo[i].wfd);
		if(mInfo[i].mfd)
			close(mInfo[i].mfd);
	}
	close(psfd);
	close(wsfd);
	close(rsfd);
	close(msfd);
	exit(0);
}

// #c---
/*****************************************************************************
 * 将一个字符的Unicode(UCS-2和UCS-4)编码转换成UTF-8编码.
 *
 * 参数:
 *    unic     字符的Unicode编码值
 *    pOutput  指向输出的用于存储UTF8编码值的缓冲区的指针
 *    eLen  编码长度
 *
 * 返回值:
 *    返回转换后的字符的UTF8编码所占的字节数, 如果出错则返回 0 .
 *
 * 注意:
 *     1. UTF8没有字节序问题, 但是Unicode有字节序要求;
 *        字节序分为大端(Big Endian)和小端(Little Endian)两种;
 *        在Intel处理器中采用小端法表示, 在此采用小端法表示. (低地址存低位)
 ****************************************************************************/

int Unicode2UTF8(unsigned short *unic, unsigned char *utf8,int eLen)
{
    assert(utf8 != NULL);
    while(eLen--)
    {
		if ( *unic <= 0x0000007F )
			// * U-00000000 - U-0000007F:  0xxxxxxx
			*utf8     = (*unic & 0x7F);
		else if (*unic >= 0x00000080 && *unic <= 0x000007FF )
		{
			// * U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
			*(utf8+1) = (*unic & 0x3F) | 0x80;
			*utf8     = ((*unic >> 6) & 0x1F) | 0xC0;
		}
		else if ( *unic >= 0x00000800 && *unic <= 0x0000FFFF )
		{
			// * U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
			*(utf8+2) = (*unic & 0x3F) | 0x80;
			*(utf8+1) = ((*unic >>  6) & 0x3F) | 0x80;
			*utf8     = ((*unic >> 12) & 0x0F) | 0xE0;
		}
		unic++;
		utf8 += 3;
    }
    return 0;
}

int CodeConvert(char **src, size_t *sLen, char **dst, size_t *dLen)
{
	iconv_t cd;
	cd = iconv_open("GBK", "UTF-8");
	if (cd == (iconv_t)-1)
		return 0;
	if (iconv(cd, src, sLen, dst, dLen) == -1)
		return 0;
	iconv_close(cd);
	return 1;
}

int rm_dir(char *name)
{
	DIR *dirp = opendir(name);
	char path[128] = {0};
    if(!dirp)
        return -1;
    struct dirent *dir;
    struct stat st;
    while((dir = readdir(dirp)) != NULL)
    {
		if(strcmp(dir->d_name,".") == 0|| strcmp(dir->d_name,"..") == 0)
			continue;
		snprintf(path,sizeof(path),"%s%c%s",name,'/',dir->d_name);
        if(lstat(path,&st) == -1)
            continue;
        if(S_ISDIR(st.st_mode))
        {
            if(rm_dir(path) == -1)
            {
                closedir(dirp);
                return -1;
            }
            rmdir(path);
        }
        else if(S_ISREG(st.st_mode))
            unlink(path);
        else
            continue;
    }
    closedir(dirp);
    return 0;
}

int FileClear(char *name)
{
    struct stat st;
	char path[128] = { 0 };
	snprintf(path,sizeof(path),"%s%s",CGI_PATH,name);

    if(lstat(path,&st) == -1)
        return -1;
    if(S_ISREG(st.st_mode))		// file
	{
        if(unlink(path) == -1)
            return -1;
    }
    else if(S_ISDIR(st.st_mode))
    {
		if(strcmp(path,".") == 0|| strcmp(path,"..") == 0)
			return -1;
		if(rm_dir(path) == -1)		// delete all the files in dir.
			return -1;
    }
    return 0;
}

// Krasovsky 1940
// a = 6378245.0, 1/f = 298.3
// b = a * (1 - f)
// ee = (a^2 - b^2) / a^2;
const double a = 6378245.0;
const double ee = 0.00669342162296594323;

// World Geodetic System ==> Mars Geodetic System
int outOfChina(CLLocationCoordinate2D coordinate)
{
	if (coordinate.longitude < 72.004 || coordinate.longitude > 137.8347)
		return 1;
	if (coordinate.latitude < 0.8293 || coordinate.latitude > 55.8271)
		return 1;
	return 0;
}

double transformLat(double x, double y)
{
	double ret = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y + 0.2 * sqrt(abs(x));
	ret += (20.0 * sin(6.0 * x * M_PI) + 20.0 * sin(2.0 * x * M_PI)) * 2.0 / 3.0;
	ret += (20.0 * sin(y * M_PI) + 40.0 * sin(y / 3.0 * M_PI)) * 2.0 / 3.0;
	ret += (160.0 * sin(y / 12.0 * M_PI) + 320 * sin(y * M_PI / 30.0)) * 2.0 / 3.0;
	return ret;
}

static double transformLon(double x, double y)
{
	double ret = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * sqrt(abs(x));
	ret += (20.0 * sin(6.0 * x * M_PI) + 20.0 * sin(2.0 * x * M_PI)) * 2.0 / 3.0;
	ret += (20.0 * sin(x * M_PI) + 40.0 * sin(x / 3.0 * M_PI)) * 2.0 / 3.0;
	ret += (150.0 * sin(x / 12.0 * M_PI) + 300.0 * sin(x / 30.0 * M_PI)) * 2.0 / 3.0;
	return ret;
}

// WGS-84 -> GCJ-02
CLLocationCoordinate2D Wgs2Gcj(CLLocationCoordinate2D coordinate)
{
	if (outOfChina(coordinate))
		return coordinate;
	double wgLat = coordinate.latitude;
	double wgLon = coordinate.longitude;
	double dLat = transformLat(wgLon - 105.0, wgLat - 35.0);
	double dLon = transformLon(wgLon - 105.0, wgLat - 35.0);
	double radLat = wgLat / 180.0 * M_PI;
	double magic = sin(radLat);
	magic = 1 - ee * magic * magic;
	double sqrtMagic = sqrt(magic);
	dLat = (dLat * 180.0) / ((a * (1 - ee)) / (magic * sqrtMagic) * M_PI);
	dLon = (dLon * 180.0) / (a / sqrtMagic * cos(radLat) * M_PI);
	CLLocationCoordinate2D c2;
	c2.latitude = wgLat + dLat;
	c2.longitude = wgLon + dLon;
	return c2;
}

// GCJ-02->WGS-84
CLLocationCoordinate2D Gcj2Wgs(CLLocationCoordinate2D coordinate)
{
	if (outOfChina(coordinate))
		return coordinate;
	CLLocationCoordinate2D c2 = Wgs2Gcj(coordinate);
	c2.latitude = 2*coordinate.latitude-c2.latitude;
	c2.longitude = 2*coordinate.longitude-c2.longitude;
	return c2;
}

const double x_M_PI = M_PI * 3000.0 / 180.0;
// GCJ-02 -> BD-09
CLLocationCoordinate2D Gcj2Bd(CLLocationCoordinate2D coordinate)
{
	double x = coordinate.longitude, y = coordinate.latitude;
	double z = sqrt(x * x + y * y) + 0.00002 * sin(y * x_M_PI);
	double theta = atan2(y, x) + 0.000003 * cos(x * x_M_PI);
	CLLocationCoordinate2D c2;
	c2.latitude = z * sin(theta) + 0.006;
	c2.longitude = z * cos(theta) + 0.0065;
	return c2;
}

// BD-09->GCJ-02
CLLocationCoordinate2D Bd2Gcj(CLLocationCoordinate2D coordinate)
{
	double x = coordinate.latitude - 0.0065, y = coordinate.longitude - 0.006;
	double z = sqrt(x * x + y * y) - 0.00002 * sin(y * x_M_PI);
	double theta = atan2(y, x) - 0.000003 * cos(x * x_M_PI);
	CLLocationCoordinate2D c2;
	c2.latitude = z * cos(theta);
	c2.longitude = z * sin(theta);
	return c2;
}

// BD-09->WGS-84
CLLocationCoordinate2D Bd2Wgs(CLLocationCoordinate2D coordinate)
{
	CLLocationCoordinate2D c2;
	c2 = Bd2Gcj(coordinate);
	c2 = Gcj2Wgs(c2);
	return c2;
}


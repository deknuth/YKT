/*
 * main.h
 *
 *  Created on: 2018-6-12
 *      Author: knuth
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/mman.h> 
#include <syscall.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <pthread.h>
#include <sys/stat.h>
#include <locale.h>
#include <semaphore.h>
#include <stdarg.h>
#include <signal.h>
#include <setjmp.h>
#include <time.h>
#include <limits.h>
#include <ctype.h>
#include <float.h>
#include    <termios.h>
#include    <net/if.h>
#include    <linux/can.h>
#include    <linux/can/raw.h>

#include <linux/hidraw.h>
#include <linux/input.h>
#include <errno.h>
#include <dirent.h>
#include <linux/hiddev.h>

#include    "aes.h"
#include    "logic.h"
#include    "base64.h"
#include    "hash.h"
#include    "cjson.h"
#include    "parse.h"
#include    "pool.h"
#include    "log.h"
#include    "times.h"
#include    "db.h"
#include    "channel.h"
#include    "guard.h"
#include    "hal.h"
#include    "mp3.h"
#include    "offline.h"
#define VDEBUG
#define MDEBUG
//#define SQL
#define MAX_CACHE 15000
#define MAX_REC 80000
#define MHI	(MAX_REC<<1)+1			// max hash index
#define COM "/dev/hidraw0"


typedef struct{
    int tfd;        // tcp
    int cfd;        // com
}S_WORK;

extern int _write(int fd,void *buffer,int length);
extern void compleId(const char *src, char* dst);
extern int TcpClient(void);
extern int onlineInit(S_WORK* arg);
extern int offlineInit(void);
extern int HexToString(char *dst,const unsigned char *src,int sLen);

extern long diffTime(struct timeval *x,struct timeval *y);
extern void m_printf(const char *format,...);

extern struct HashItem* idHash;
extern struct HashItem* caHash;
extern tpool_t *pool;
extern ENTER_DBS *enter;

#endif


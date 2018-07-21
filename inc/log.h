#ifndef LOG_H_
#define LOG_H_

#define LOGLINE_MAX 1024
#define DEBUG 1
#define INFO  2
#define WARN  3
#define ERROR 4
#define FATAL 5
#define LOG_TRUNC   1<<0
#define LOG_NODATE  1<<1
#define LOG_NOLF    1<<2
#define LOG_NOLVL   1<<3
#define LOG_DEBUG   1<<4
#define LOG_STDERR  1<<5
#define LOG_NOTID   1<<6
#define	LOG_FILE_PATH   "log/%s.log"
#define LOG_MAX	4096000
#define NANOSECOND 50000000UL

typedef struct logs{
	int fd;
	sem_t sem;
	int flags;
}log_t;

extern log_t *lfd;
extern char LogFile[];

extern int lprintf(log_t *log, unsigned int level, char *fmt, ...);
extern log_t *log_open(int flags);
extern void log_close(log_t *log);
extern void error_log(char* s,char *filename,int row_num);
extern void *LogManage(void *arg);
#endif

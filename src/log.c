#include "../inc/main.h"
static char* last_char_is(const char *s, int c);
static int CreatLogFile(char *path);

int CreatLogFile(char *path)
{
	char buf[14] = {0};
   	time_t now = time(NULL) ;
   	strftime(buf, 100, "%Y%m%d%H%M%S", localtime(&now));
    snprintf(path , 127, LOG_FILE_PATH, buf);
	return 0;
}

char* last_char_is(const char *s, int c)
{
        char *sret;
        if (!s)
            return NULL;
        sret = (char *)s+strlen(s)-1;
        if (sret>=s && *sret == c)
            return sret;
        else
        	return NULL;
}

void chomp(char *s)
{
        char *lc = last_char_is(s,'\n');
        if(lc)
        	*lc = 0;
}

int lprintf(log_t *log, unsigned int level, char *fmt, ...)
{
	int fd;
	int rc;
	va_list ap;
	time_t now;
	char date[50] = {0};
	static char line[LOGLINE_MAX];
	int cnt;
	static char *levels[6] = { "[(bad)] ", "[debug] ", "[info] ", "[warn] ", "[error] ", "[fatal] " };
	if (!log)
		return -1;
	if (!(log->flags & LOG_DEBUG) && level == DEBUG)
		return 0;
	fd = log->fd;
	
	if (!(log->flags & LOG_NODATE))
	{
		now = time(NULL);
		strcpy(date, ctime(&now));
		date[strlen(date) - 1] = ' ';
		date[strlen(date)] = '\0';
	}
	/*
	if (!(log->flags & LOG_NOTID))	//
		sprintf(threadnum, "(TID:%lu) ", pthread_self());
	*/
	cnt = snprintf(line, sizeof(line), "%s%s", log->flags & LOG_NODATE ? ""
			: date, log->flags & LOG_NOLVL ? "" : (level > FATAL ? levels[0]
			: levels[level]));  //, log->flags & LOG_NOTID ? "" : threadnum
	va_start(ap, fmt);
	vsnprintf(line + cnt, sizeof(line) - cnt, fmt, ap); //
	va_end(ap);
	line[sizeof(line) - 1] = '\0';
	if (!(log->flags & LOG_NOLF))
	{
		chomp(line);
        strcpy(line+strlen(line), "\n");
	}
	sem_wait(&log->sem); // 日志信号
	rc = write(fd, line, strlen(line));
	sem_post(&log->sem);
	if (!rc)
		errno = 0;
	return rc;
}

log_t *log_open(int flags)
{
    char fname[128] = {0};
	log_t *log = malloc(sizeof(log_t));
	if (!log)
	{
		fprintf(stderr, "LOG: Unable to malloc()");
		goto log_open_a;
	}
    CreatLogFile(fname);
	log->flags = flags;
	if (!strcmp(fname, "-"))
		log->fd = 2;
	else
	{
        log->fd = open(fname, O_WRONLY | O_CREAT | O_NOCTTY | (flags
                & LOG_TRUNC ? O_TRUNC : O_APPEND), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	}
	if (log->fd == -1)
	{
        fprintf(stderr, "LOG: Opening logfile %s: %s\n", fname, strerror(errno));
		goto log_open_b;
	}
	if (sem_init(&log->sem, 0, 1) == -1)
	{
		fprintf(stderr, "LOG: Could not initialize log semaphore.");
		goto log_open_c;
	}
	return log;

	log_open_c: close(log->fd);
	log_open_b: free(log);
	log_open_a: return NULL;
}

void log_close(log_t *log)
{
	sem_wait(&log->sem);
	sem_destroy(&log->sem);
	close(log->fd);
	free(log);
	return;
}
/*
void *LogManage(void *arg)
{
	struct timespec sleeptm;
	sleeptm.tv_sec = 2;
	sleeptm.tv_nsec = NANOSECOND;
	struct stat *dBuf;
	dBuf = (struct stat *)malloc(sizeof(struct stat));
	while(1)
	{
		memset(dBuf,0x00,sizeof(struct stat));
		if(stat(LogFile,dBuf) == 0)
		{
			if(dBuf->st_size > LOG_MAX)			// change happen
			{
				sem_wait(&lfd->sem);
				close(lfd->fd);
				CreatLogName();
				lfd->fd = open(LogFile, O_WRONLY | O_CREAT | O_NOCTTY | O_TRUNC, 0666);
				if (lfd->fd == -1)
				{
						fprintf(stderr, "LOG: Opening log file %s: %s", LogFile, strerror(errno));
						log_close(lfd);
						break;
				}
				sem_post(&lfd->sem);
			}
		}
		nanosleep(&sleeptm,NULL);
	}
	free(dBuf);
	return (void *)1;
}
*/

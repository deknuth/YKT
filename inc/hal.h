#ifndef HAL_H
#define HAL_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <linux/hiddev.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <linux/hidraw.h>
#include <linux/input.h>
#include <errno.h>

#define CAN_CARD    1
#define CAN "can0"
extern int PortInit(char *path);
extern void readCard(void *arg);
extern int getInfo(int sfd);
extern int canSend(int cfd, unsigned int id, unsigned char *data);
extern void showReports(int fd);
extern void show_all_report(int fd);
extern const unsigned char tab[];
#endif // HAL_H

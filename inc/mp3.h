#ifndef MP3_H
#define MP3_H
#include <sys/soundcard.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include "mad.h"

extern void PlayAudio(void *arg);
#define INVALID 1
#define PASS    2
#define LIMITE  3


#endif

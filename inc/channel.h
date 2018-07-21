#ifndef CHANNEL_H
#define CHANNEL_H
#include "../inc/curl/curl.h"
//#define USE_OPENSSL
#define MAX_CHANNEL_NUM 64
#define INVALID_CONTEXT 3000
#define INVALID_FORMAT  3001
#define EXEC_SUCCESS    3002
#define EXEC_FAILED     3003
#define MAX_OP_ID   64
typedef struct {
    unsigned char c_no;
    unsigned char c_c_status;
    unsigned char c_direct;
    unsigned char c_all_status;
    char acc_id[17];
    char oper_id[MAX_OP_ID];
    unsigned char is_img;       // have img ?
    unsigned int index;
    int tfd;
}S_SWIPE;

typedef struct {
    unsigned char ch_status;
    char ip[2][18];             // 0->in,1->out
}S_CHANNEL;

typedef struct MemoryStruct{
    unsigned char *memory;
    size_t size;
}IMG;

typedef struct cac{
    S_SWIPE *swipe;
    time_t times;
    IMG *img;
    unsigned char cid;
//    struct cac *next;
}IMG_CACHE;


extern void personJudg(void *arg);
unsigned char getImg(IMG *img, S_SWIPE *swipe);
extern int snapshot(S_SWIPE *swipe);
extern S_CHANNEL *pChan;
extern void getOperId(char *str);
extern int writeCache(IMG *img,S_SWIPE *swipe);
extern int swipeUpload(IMG *img,S_SWIPE *swipe);
// extern unsigned char cid[];     //cache id
extern IMG_CACHE *cache;
extern int total;
#endif // CHANNEL_H

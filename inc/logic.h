#ifndef LOGIC_H_
#define LOGIC_H_
#include "../inc/db.h"
#include "../inc/parse.h"

#define VERIFY 0
#define TIMESTAMP 1
#define PUBLISHING 2
#define CLIENT_SUCCESS  1111
/**** user infomation struct ****/
typedef struct pi{
    char id[17];                    // card id
    unsigned char lock;
    unsigned char p_type;           // person type
    unsigned char is_temp;          // is temporary rule ?
    unsigned char position[RULE_NUM];
}S_PINFO;

extern int login(int tfd);
extern void heartbeat(void *arg);
extern void DataProcess(void *arg);
extern int syncTime(int tfd);
extern int matchTRule(DB *table,S_T_COMP *comp,int index);
extern int getORule(DB *table,void* kid,u_int32_t k_size,S_O_RULE *rules);
extern void tcpClose(int tfd);
extern S_PINFO *pInfo;
extern unsigned char id[];
#endif

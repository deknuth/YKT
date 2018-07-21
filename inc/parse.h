#ifndef PARSE_H
#define PARSE_H
#define TYPE_NUM    6
#include    "../inc/cjson.h"
#define T_O_RULE  0
#define T_T_RULE  1
#define T_PERSON  2
#define RULE_NUM    20

typedef struct s_t_rule{
    char id[17];
    unsigned long start;
    unsigned long end;
    unsigned char limit;        // 0->all restrict 1->in allow 2->out allow 3->all allow
    unsigned int asc_times;     // Allowable times of swipe card
    unsigned int rule_id;
}S_T_RULE;                      // 0--temp rules  1-4---formal rules

typedef struct s{
    unsigned char table[86400*7]; // bit0->pass flag bit1-2->dir(0->all restrict 1->in allow 2->out allow 3->all allow) bit3-7->rule number
    unsigned char swipe[86400*7];
    unsigned char position[RULE_NUM];
}S_RULE;              // rules  0-3---formal rules

typedef struct {
    unsigned char type;
    unsigned int rule_id;
    unsigned int school_id;
    unsigned int start;
    unsigned int end;
    unsigned char week;
    unsigned char limit;        // 0->all restrict 1->in allow 2->out allow 3->all allow
    unsigned int  asc_times;    // Allowable times of swipe card
}S_O_RULE;              // rules  0-3---formal rules

typedef struct tcomp{
    char id[17];
    long c_time;              // current time
    long cr_local;            // Card reader location
}S_T_COMP;

typedef struct usr{
    char id[17];            // card id
    unsigned char type;
    unsigned char lock;
}S_USER;

typedef struct {
    int num;
    char in_ip[18];
    char out_ip[18];
}S_PORT;

extern int ruleJson(cJSON * root, int type);
extern int truleProcess(void *data,int tfd);
extern int oruleProcess(void *data,int tfd);
extern int personProcess(void *data,int tfd);
extern int channelProcess(void *data,int tfd);
extern int cacheProcess(void *data,int tfd);
extern int cardManage(S_USER* user,int o_type,unsigned char db);
extern int hashManage(int p_type,S_O_RULE *pOrule,S_O_RULE *pUpdate,unsigned char flag);

extern S_RULE *rule;
#endif // PARSE_H

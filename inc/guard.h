#ifndef GUARD_H
#define GUARD_H
#define CACHE_TIME  20
typedef struct errs{
    unsigned char hal  :1;
    unsigned char tcp  :1;
    unsigned char sync :1;      // sync time
    unsigned char online :1;
    unsigned char can   :1;
    unsigned char  :4;
}S_ERROR;

typedef union{
    unsigned data;
    S_ERROR  err;
}U_ERROR;

/******** application part ****************/
typedef struct et_dbs {
    DB *rt_dbp;                     /* Database temp rule information */
    DB *ro_dbp;
    DB *p_dbp;                      /* Database person information */
    DB *ch_dbp;
    DB *cache_dbp;
//    char *rule_db_name;         /* Name of the rule database */
//    char *person_db_name;       /* Name of the person database */
}ENTER_DBS;

extern int db_start(ENTER_DBS *enter);
extern int db_inert(DB *table,void* kid,u_int32_t k_size,void *idata,size_t idata_size);
extern int db_select_all(DB *table,int(*dataProcess)(void *,int),int tfd);
extern int db_del(DB *table,void* kid,u_int32_t k_size);
extern int delTRule(DB *table,S_T_RULE *rule);
extern int db_del_all(DB *table);
extern void refresh(void);
extern void cacheManage(void);
extern U_ERROR *err;
extern void guard(void);
#endif // GUARD_H

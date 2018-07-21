#include "../inc/main.h"
pthread_t id1,id2,id3;
S_RULE *rule;
S_PINFO *pInfo;
ENTER_DBS *enter;
struct HashItem* idHash;
struct HashItem* caHash;
tpool_t *pool;
log_t *lfd;
S_CHANNEL *pChan;
IMG_CACHE *cache;

int onlineInit(S_WORK *arg)
{
    offlineInit();
    tpool_add_work(pool, DataProcess, arg);
    sleep(1);
    syncTime(arg->tfd);
    sleep(1);
    login(arg->tfd);
    sleep(1);
    tpool_add_work(pool, heartbeat, arg);
    lprintf(lfd,INFO,"Kernel: Online init sucess...");
    return 1;
}

int offlineInit(void)
{
    int i,j;
    memset(pInfo,0x00,sizeof(S_PINFO));
    for(i=0; i<TYPE_NUM; i++)
    {
        for(j=0; j<86400*7; j++)
        {
            rule[i].table[j] = 0;
            rule[i].swipe[j] = 0;
        }
        for(j=0; j<RULE_NUM; j++)
            rule[i].position[j] = 0;
    }
    db_select_all(enter->p_dbp,personProcess,0);
    db_select_all(enter->ro_dbp,oruleProcess,0);
    db_select_all(enter->rt_dbp,truleProcess,0);
    db_select_all(enter->ch_dbp,channelProcess,0);
    return 1;
}

void creatSid(char *cid)
{
    srand((int)time(0));
    int rand_num = rand();
    sprintf(cid,"%016d",rand_num);
}

int main(void)
{
    int i;
    FILE *fp = NULL;
    char cid[24] = {0};
    if(access("cfg/config", F_OK) == 0)
    {
        if((fp=fopen("cfg/config","r+")) != NULL)
        {
            if(fgets(cid, 24, fp) != NULL)
                strcat(id,cid);
            else
            {
                creatSid(cid);
                fwrite(cid,strlen(cid),1,fp);
                memset(id,0x00,24);
                strcat(id,cid);
            }
            fclose(fp);
        }
    }
    else
    {
        creatSid(cid);
        if((fp=fopen("cfg/config","w+")) != NULL)
        {
            fwrite(cid,strlen(cid),1,fp);
            memset(id,0x00,24);
            strcat(id,cid);
            fclose(fp);
        }
    }

    /***openssl curl **********/
    curl_global_init(CURL_GLOBAL_ALL);	// In windows, this will init the winsock stuff
    //   init_locks();
    /**************************/
    cache = (IMG_CACHE *)malloc(sizeof(IMG_CACHE)*MAX_CACHE+1);
    idHash = inithashtable(MHI);
    caHash = inithashtable(MAX_CACHE);
    pInfo = (S_PINFO *)malloc(sizeof(S_PINFO)*MHI);
    err = (U_ERROR *)malloc(sizeof(U_ERROR));
    rule = (S_RULE *)malloc(sizeof(S_RULE)*TYPE_NUM);
    pChan = (S_CHANNEL *)malloc(sizeof(S_CHANNEL)*MAX_CHANNEL_NUM);
    enter = malloc(sizeof(ENTER_DBS));
    for(i=0; i<MAX_CACHE+1; i++)
    {
        cache[i].cid = 255;
        cache[i].times = 0;
    }
    err->data = 0;
    err->err.tcp = 1;
    err->err.online = 1;

    if((lfd = log_open(0)) == 0)
    {
        m_printf("Kernel: Open log file failed!\n");
        goto end;
    }
 //   PlayAudio(1);

    if(db_start(enter) != 0)
    {
        lprintf(lfd,FATAL,"Kernel: DB init failed!");
        goto end;
    }

    pool = tpool_init(25, 15, 1);

    if(pthread_create(&id1,NULL,(void *)guard,(void *)0) != 0)
        lprintf(lfd,FATAL,"Thread: Create guard thread failed!");
    if(pthread_create(&id2,NULL,(void *)refresh,(void *)0) != 0)
        lprintf(lfd,FATAL,"Thread: Create refresh thread failed!");
    if(pthread_create(&id3,NULL,(void *)cacheManage,(void *)0) != 0)
        lprintf(lfd,FATAL,"Thread: Create cache manage thread failed!");

    pthread_join(id1,NULL);
    pthread_join(id2,NULL);
    pthread_join(id3,NULL);
    tpool_destroy(pool, 1);
    curl_global_cleanup();
    //kill_locks();
end:
    FreeHash(idHash);
    FreeHash(caHash);
    free(enter);
    free(pInfo);
    free(err);
    free(rule);
    free(pChan);
    return 0;
}


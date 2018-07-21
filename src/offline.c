#include    "../inc/main.h"

int cacheProcess(void *data,int tfd)
{
    char path[128] = {0};
    S_SWIPE *swipe = data;
    swipe->tfd = tfd;
    snprintf(path,127,"img/%s",swipe->oper_id);
    m_printf("path: %s\n",path);
    FILE *fp = fopen(path, "rb");
    IMG *img = malloc(sizeof(IMG));
    img->size  = 0;
    img->memory = NULL;
    if(fp != NULL)
    {
        fseek(fp, 0, SEEK_END);
        img->size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        img->memory = malloc(img->size + 1);
        fread(img->memory,1,img->size,fp);
        fclose(fp);
    }
    if(swipeUpload(img,swipe))
    {
        db_del(enter->cache_dbp,swipe->oper_id,strlen(swipe->oper_id)+1);
        remove(path);
    }
    if(img->memory != NULL)
        free(img->memory);
    free(img);
}

void offlineManage(void *arg)
{
    int tfd = *(int *)arg;
    db_select_all(enter->cache_dbp,cacheProcess,tfd);
}



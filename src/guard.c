#include    "../inc/main.h"
U_ERROR *err;
void guard(void)
{
    S_WORK cs;
    int count = 0;
#ifdef COM_CARD
    if((cs.cfd = PortInit(COM)) < 1)
    {
        lprintf(lfd,FATAL,"Kernel: Port %s open failed!",COM);
        err->err.hal = 1;
    }
    else
        tpool_add_work(pool, readCard, &cs);
#elif CAN_CARD
    tpool_add_work(pool, readCard, &cs);
#endif
    if((cs.tfd = TcpClient()) > 1)
    {
        err->err.tcp = 0;
        onlineInit(&cs);
    }
    else
    {
        lprintf(lfd,FATAL,"Kernel: Tcp client failed!");
        err->err.tcp = 1;
        offlineInit();
        lprintf(lfd,INFO,"Kernel: offline init sucess...");
    }

    while(1)
    {
        if(err->data)
        {
            if(err->err.tcp)
            {
                if((cs.tfd = TcpClient()) > 1)
                {
                    err->err.tcp = 0;
                    onlineInit(&cs);
                }
                else
                {
                    lprintf(lfd,FATAL,"Kernel: Tcp client failed!");
                    err->err.tcp = 1;
                    sleep(5);
                }
            }
            else if(err->err.online)
            {
                login(cs.tfd);
                sleep(5);
            }

            if(err->err.can)
            {
                err->err.can = 0;
                tpool_add_work(pool, readCard, &cs);
                sleep(2);
            }
        }

        if(count++ == 43200)
        {
            if(err->err.tcp == 0)
                syncTime(cs.tfd);
            count = 0;
        }
        sleep(2);
    }
}

void refresh(void)
{
    time_t times = time(NULL);
    int i=0,j=0;
    while(1)
    {
        if((times%86400) < 10)
        {
            for(i=0; i<TYPE_NUM; i++)
            {
                for(j=0; j<86400*7; j++)
                    rule[i].swipe[j] = 0;       // refresh swipe times
            }
            sleep(86000);
        }
        sleep(4);
    }
}

void cacheManage(void)
{
    int i;
    for(;;)
    {
        time_t times = time(NULL);
        for(i=0; i<MAX_CACHE; i++)
        {
            if(cache[i].cid != 255)
            {
                // printf("CID[%d]: %s\n",i,cache[i].swipe->oper_id);
             //   index = GetHashTablePos(cache[i].swipe->oper_id, caHash, MAX_CACHE);
            //    printf("CID[%d]: %d\n",i,cache[i].cid);
          //      if(index == 0)
           //         printf("W[%d]: %s\n",cache[i].cid,cache[i].swipe->oper_id);
   //             printf("Cache: %p\n",cache[i].img->memory);

                switch(cache[i].cid)
                {
                case 1:
                    s_printf("Free: %d->%p %p\n",i,cache[i].img,cache[i].img->memory);
                    if(cache[i].swipe->is_img && (cache[i].img->size)>0)
                    {
                        free(cache[i].img->memory);
                        free(cache[i].img);
                    }
                    free(cache[i].swipe);
                    cache[i].cid = 255;
                    DelHashTablePos(cache[i].swipe->oper_id, caHash, MAX_CACHE);
                    total--;
                    break;
                case 0:
                    writeCache(cache[i].img,cache[i].swipe);
                    if(cache[i].swipe->is_img && (cache[i].img->size)>0)
                    {
                        free(cache[i].img->memory);
                        free(cache[i].img);
                    }
                    free(cache[i].swipe);
                    cache[i].cid = 255;
                    DelHashTablePos(cache[i].swipe->oper_id, caHash, MAX_CACHE);
                    total--;
                    break;
                case 127:
                    if(times-cache[i].times > CACHE_TIME)       // offline process
                    {
                        printf("time overflow: %s\n",cache[i].swipe->oper_id);
                        writeCache(cache[i].img,cache[i].swipe);
                        if(cache[i].swipe->is_img && (cache[i].img->size)>0)
                        {
                            free(cache[i].img->memory);
                            free(cache[i].img);
                        }
                        free(cache[i].swipe);
                        cache[i].cid = 255;
                        DelHashTablePos(cache[i].swipe->oper_id, caHash, MAX_CACHE);
                        total--;
                    }
                    break;
                default:
                    cache[i].cid = 255;
                    break;
                }
            }
        }
        m_printf("total: %d\n",total);
        sleep(5);
    }
}

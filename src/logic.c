#include "../inc/main.h"
char SCTime[] = {"{\"c\": \"timestamp\"}\n"};
static int dataParse(char* str,int sfd);
const unsigned char key[16] = {"LMUITN25LMUQC436"};
unsigned char id[24];
aes_context aes;

void tcpClose(int tfd)
{
    err->err.tcp = 1;
    err->err.online = 1;
    close(tfd);
}

int syncTime(int tfd)
{
    if(_write(tfd,SCTime,strlen(SCTime)) <= 0)
    {
        err->err.sync = 1;
        tcpClose(tfd);
        return 0;
    }
    else
    {
        err->err.sync = 0;
        return 1;
    }
}

int login(int tfd)
{
    unsigned char *ida;
    char enc[256] = {0};
    char word[256] = {"{\"c\": \"verify\",\"data\": {\"device_id\": \"%s\",\"aes_device_id\": \"%s\",\"type\": 1}}\n"};
    char words[384] = {0};
    int slen = strlen((char *)id);
    int dlen = (slen/16+1)*16;
    aes_setkey_enc(&aes, key, 128);     // set encrypt key
    ida = aes_crypt_ecb(&aes, AES_ENCRYPT, id,slen,dlen);
    base64Encoder(ida,enc,dlen);
    snprintf(words,383,word,id,enc);
    free(ida);
    if(_write(tfd,words,strlen(words)) <= 0)
    {
        tcpClose(tfd);
        return 0;
    }
    else
        return 1;
}

void heartbeat(void *arg)
{
    S_WORK *hb = arg;
    char HB[256] = {"{\"c\": \"heartbeat\",\"data\": {\"packets \": \"i am coming\"}}\n"};
    while(1)
    {
        if(_write(hb->tfd,HB,strlen(HB)) <= 0)
        {
            tcpClose(hb->tfd);
            break;
        }
        sleep(10);
    }
}

#define RSIZE   1024*2048*10
void DataProcess(void *arg)
{
    S_WORK *dp = arg;
    char buf[1024] = { 0 };
    char *array = malloc(sizeof(char)*RSIZE);
    int len = 0;
    int MaxFd,total=0;      // 文件描述符个数
    fd_set readset;
    struct timeval tv;
    memset(array,0x00,sizeof(char)*RSIZE);
    for(;;)
    {
        FD_ZERO( &readset); // 文件描述符置0
        if(dp->tfd >= 0)
            FD_SET(dp->tfd, &readset);
        MaxFd = dp->tfd + 1; // 最大文件描述符数
        tv.tv_sec = 3;
        tv.tv_usec = 0;
        switch(select(MaxFd, &readset, 0, 0, &tv))
        {
        case -1:
            lprintf(lfd,FATAL,"TCP: select error!");
            goto end;
            break;
        case 0:
            break;
        default:
            if(FD_ISSET(dp->tfd, &readset)) // FD_ISSET检测fd是否设置
            {
                len = read(dp->tfd, buf, 1023);
                if(len > 0)
                {
                    total += len;
                    if(total < RSIZE)
                    {
                        if(len == 1023)
                            strcat(array,buf);
                        else
                        {
                            strcat(array,buf);
                            m_printf("array: %s\n\n",array);
                            dataParse(array,dp->tfd);
                            memset(array,0x00,total);
                            total = 0;
                        }
                    }
                    else
                    {
                        memset(array,0x00,total);
                        total = 0;
                    }
                    memset(buf, 0, len);
                }
                else if(len==-1 && errno!=EAGAIN && errno!=EINTR)
                    lprintf(lfd, WARN, "TCP: socket read error!");
                else if(len==0)
                    goto end;
            }
            break;
        }
    }
end:
    lprintf(lfd,FATAL,"TCP: close client!");
    tcpClose(dp->tfd);
    free(array);
}

const char ipset_bk[] = {"{\"cr\":\"%s\",\"data\":{\"operate_id\":\"%s\",\"code\":%d,\"remark\":\"%s\"}}\n"};

int dataParse(char* str,int sfd)
{
    cJSON *root = NULL;
    cJSON *item = NULL;    //cjson对象
    cJSON *object = NULL;
    int res = 0,index;
    char opr_id[MAX_OP_ID] = {0};
    char back[256] = {0};
    int len;
    root = cJSON_Parse(str);
    if(!root)
        lprintf(lfd,WARN,"Json error before: [%s]",cJSON_GetErrorPtr());
    else
    {
        item = cJSON_GetObjectItem(root, "cr");
        if(item != NULL)
        {
            //  printf("cr: %s\n",item->valuestring);
            if(strcmp(item->valuestring,"verify") == 0)
            {
                item = cJSON_GetObjectItem(root, "code");
                if(item != NULL)
                {
                    if(item->valueint == CLIENT_SUCCESS)
                    {
                        db_del_all(enter->rt_dbp);      // delete all temp rule
                        err->err.online = 0;
                        ruleJson(root,1);
                        tpool_add_work(pool, offlineManage, (void *)&sfd);
                    }
                    else
                        err->err.online = 1;
                }
            }
            else if(strcmp(item->valuestring,"timestamp") == 0)
            {
                item = cJSON_GetObjectItem(root, "code");
                //                printf("code: %d\n",item->valueint);
                if(item != NULL)
                {
                    if(item->valueint == 3002)
                    {
                        item = cJSON_GetObjectItem(root, "data");
                        if(item != NULL)
                        {
                            object = cJSON_GetObjectItem(item, "timestamp");
                            if(object != NULL)
                                setTime(object->valueint);
                        }
                    }
                }
            }
            else if(strcmp(item->valuestring,"channel_data") == 0)
            {
                item = cJSON_GetObjectItem(root, "operate_id");
                if(item != NULL)
                {
                    index = GetHashTablePos(item->valuestring, caHash, MAX_CACHE);
                    if(index)
                    {
                        item = cJSON_GetObjectItem(root, "code");
      //                  printf("channel index: %d\n",index);
                        if(item != NULL)
                        {
                            if(item->valueint == 3002)
                                cache[index].cid = 1;
                            else
                                cache[index].cid = 0;
                        }
                        else
                            cache[index].cid = 0;
                    }
                }
            }
        }
        else
        {
            item = cJSON_GetObjectItem(root, "c");
            if(item != NULL)
            {
                if(strcmp(item->valuestring,"publishing") == 0)
                {
                    item = cJSON_GetObjectItem(root, "type");
                    if(item != NULL)
                    {
                        res = ruleJson(root,item->valueint);
                        getOperId(opr_id);
                        if(res)
                            len = snprintf(back,255,ipset_bk,"publishing",opr_id,EXEC_SUCCESS,"exec success!");
                        else
                            len = snprintf(back,255,ipset_bk,"publishing",opr_id,EXEC_FAILED,"exec failed!");
                        m_printf("back: %s\n",back);
                        if(err->err.online == 0)
                            _write(sfd,back,len);
                    }
                }
                else if(strcmp(item->valuestring,"take_photos") == 0)
                {
             //       res = ruleJson(root,0);
                    item = cJSON_GetObjectItem(root, "operate_id");
                    if(item != NULL)
                    {
                        S_SWIPE *swipe = malloc(sizeof(S_SWIPE));
                        snprintf(swipe->oper_id,MAX_OP_ID-1,"%s",item->valuestring);
                        m_printf("opr id : %s\n",swipe->oper_id);
                        item = cJSON_GetObjectItem(root, "data");
                        if(item != NULL)
                        {
                            object = cJSON_GetObjectItem(item, "channel_no");
                            if(object != NULL)
                            {
                                swipe->tfd = sfd;
                                swipe->c_no = object->valueint;
                                object = cJSON_GetObjectItem(item, "channel_direction");
                                if(object != NULL)
                                {
                                    swipe->c_direct = object->valueint;
                                    snapshot(swipe);
                                }
                            }
                        }
                    }
                }
                else if(strcmp(item->valuestring,"cameras") == 0)
                {
                    res = ruleJson(root,0);
                    getOperId(opr_id);
                    if(res)
                        len = snprintf(back,255,ipset_bk,"camera",opr_id,EXEC_SUCCESS,"exec success!");
                    else
                        len = snprintf(back,255,ipset_bk,"camera",opr_id,EXEC_FAILED,"exec failed!");
                    m_printf("back: %s\n",back);
                    if(err->err.online == 0)
                        _write(sfd,back,len);
                }
            }
        }
    }
    return 1;
}

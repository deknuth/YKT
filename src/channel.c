#include "../inc/main.h"
// unsigned char cid[MAX_CACHE+1] = {0};
int total = 0;
const char str[] = {
    "{\
    \"c\": \"channel_data\",\
    \"data\": {\
    \"channel_no\": %d,\
    \"channel_status\": \"%s\",\
    \"channel_direction\": %d,\
    \"channels_status\":\"%s\",\
    \"access_no\": \"%s\",\
    \"operate_id\": \"%s\",\
    \"img\": \"%s\"}}\n"
};

const char snap[] = {
    "{\
    \"cr\": \"take_photos\",\
    \"data\": {\
    \"operate_id\": \"%s\",\
    \"code\": %d,\
    \"remark\": \"%s\",\
    \"img\": \"%s\"}}\n"
};

void getOperId(char *str)
{
    time_t times = time(NULL);
    struct timeval  opr;
    struct timezone tz;
    gettimeofday(&opr, &tz);
    sprintf(str,"%s%ld%ld",id,opr.tv_usec/1000,times);
}

int snapshot(S_SWIPE *swipe)
{
    char *buf = NULL;
    char *enc = NULL;
    unsigned int len = 0;
    unsigned int eLen = 0;
    unsigned char res = 0;
    IMG img;
    img.memory = NULL;
    img.size = 0;
    getImg(&img,swipe);
    if(img.size > 1000)
    {
        len = (img.size)%3;
        eLen = (((img.size + len)/3) << 2);
        enc = malloc(eLen+1);
        buf = malloc(eLen+256);
        m_printf("snapshot img size: %d %d\n",img.size,eLen);
        base64Encoder(img.memory, enc, img.size);
        len = snprintf(buf,eLen+255,snap,swipe->oper_id,EXEC_SUCCESS,"execute success!",enc);
    }
    else
    {
        buf = malloc(512);
        len = snprintf(buf,511,snap,swipe->oper_id,EXEC_FAILED,"execute failed!"," ");
    }
//    m_printf("snapshot json: %s\n",buf);
    if(len > 0)
        res = _write(swipe->tfd,buf,len);
    else
        res = len;
    if(buf != NULL)
        free(buf);
    if(enc != NULL);
    free(enc);
    free(swipe);
    return res;
}

int swipeUpload(IMG *img,S_SWIPE *swipe)
{
    char *buf = NULL;
    char *enc = NULL;
    unsigned int len = 0;
    unsigned int eLen = 0;
    unsigned char res = 0;

    if(img->size > 1000)
    {
        len = (img->size)%3;
        eLen = (((img->size + len)/3) << 2);
        enc = malloc(eLen+1);
        buf = malloc(eLen+512);
        m_printf("img size: %d %d %s\n",img->size,eLen,swipe->oper_id);
 //       m_printf("swipeUpload img point: %p\n",img->memory);
        base64Encoder(img->memory, enc, img->size);
        len = snprintf(buf,eLen+511,str,
                      swipe->c_no,swipe->c_c_status,
                      swipe->c_direct,swipe->c_all_status,
                      swipe->acc_id,swipe->oper_id,enc);
  //      m_printf("json size: %d\n",len);
    }
    else
    {
        buf = malloc(512);
        len = snprintf(buf,511,str,
                      swipe->c_no,swipe->c_c_status,
                      swipe->c_direct,swipe->c_all_status,
                      swipe->acc_id,swipe->oper_id," ");
    }
    if(len > 0)
        res = _write(swipe->tfd,buf,len);
    else
        res = len;
//    m_printf("buf: %s\n",buf);
    if(buf != NULL)
        free(buf);
    if(enc != NULL);
    free(enc);
    return res;
}

int writeCache(IMG *img,S_SWIPE *swipe)
{
    if(swipe->is_img)
    {
        FILE *fp;
        char path[128] = {0};
        snprintf(path,127,"%s%s","img/",swipe->oper_id);
        m_printf("write img path: %s\n",path);
        fp = fopen(path,"w+");
        if(NULL != fp)
        {
            if(fwrite(img->memory,1,img->size,fp) != img->size)
            {
                fclose(fp);
                remove(path);
            }
            fclose(fp);
        }
    }
    db_inert(enter->cache_dbp,swipe->oper_id,strlen(swipe->oper_id)+1,swipe,sizeof(S_SWIPE));
}

int insertCache(IMG *img,S_SWIPE *swipe)
{
    int index;
    index = InsertHash(swipe->oper_id, caHash, MAX_CACHE);
    if(index && (total<MAX_CACHE))
    {
        total++;
        cache[index].cid = 127;
        if(swipe->is_img)
            cache[index].img = img;
        cache[index].times = time(NULL);
        cache[index].swipe = malloc(sizeof(S_SWIPE));
        //        printf("swipe: %d->%p %d\n",index,cache[index].swipe,swipe->is_img);
        memcpy(cache[index].swipe,swipe,sizeof(S_SWIPE));
    }
}

void personJudg(void *arg)
{
    S_SWIPE *swipe = arg;
    if(swipe->c_no < MAX_CHANNEL_NUM)
    {
        struct timeval  x;
        struct timeval  y;

        struct timezone tz;
        unsigned char res = 0;
        unsigned char number = 0;
        unsigned char swip = 0;
        IMG *img = malloc(sizeof(IMG));
        S_T_COMP comp;
        time_t times = time(NULL);
        gettimeofday(&x, &tz);
        if(pInfo[swipe->index].lock)
        {
            m_printf("card lock!\n");
            swip = 0;
            number = 3;
            //tpool_add_work(pool, PlayAudio,(void *)&number);
        }
        else
        {
            if(matchTime(pInfo[swipe->index].p_type,swipe->index))  // find office rule
            {
                m_printf("office rule pass!\n");
                number = 1;
                tpool_add_work(pool, PlayAudio,(void *)&number);
                swip = 1;
            }
            else if(pInfo[swipe->index].is_temp)
            {
                comp.c_time = times-E8;
                snprintf(comp.id,17,"%s",pInfo[swipe->index].id);
                //       printf("tr get time: %ld %s\n",comp.c_time,comp.id);
                if (matchTRule(enter->rt_dbp,&comp,swipe->index) == 1)       // find temp rule
                {
                    m_printf("open door!\n");
                    number = 1;
                    tpool_add_work(pool, PlayAudio,(void *)&number);
                    swip = 1;
                }
                else
                {
                    swip = 0;
                    number = 2;
                    tpool_add_work(pool, PlayAudio,(void *)&number);
                    m_printf("all rule not pass!\n");;
                }
            }
            else
            {
                swip = 0;
                number = 2;
                tpool_add_work(pool, PlayAudio,(void *)&number);
                m_printf("not temp rule!\n");
            }
        }
        snprintf(swipe->acc_id,17,"%s",pInfo[swipe->index].id);
        m_printf("oper id(%d): %s\n",swipe->c_no,swipe->oper_id);
        swipe->c_direct = 0;
        swipe->c_all_status = 0;
        swipe->c_c_status = 0;
        swipe->c_direct = 0;
        img->memory = NULL;
        img->size = 0;

//        if(swip)
        {
            res = getImg(img,swipe);
            if(res)
                swipe->is_img = 1;
            else
                swipe->is_img = 0;
        }
//        else
//            swipe->is_img = 0;
        gettimeofday(&y, &tz);
        m_printf("run time: %ld(us)\n",diffTime(&x,&y));

        if(err->err.online == 0)
        {
//            m_printf("cache img point: %p\n",img->memory);
            swipeUpload(img,swipe);
            insertCache(img,swipe);
        }
        else
            writeCache(img,swipe);
//        if(res && img.size)
            //free(img.memory);
    }
    else
        lprintf(lfd,FATAL,"Channel: Channel number overflow!");
}

static void *myrealloc(void *ptr, size_t size)
{
    if(ptr)
        return realloc(ptr, size);
    else
        return malloc(size);
}

static size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
    size_t realsize = size * nmemb;
    IMG *mem = (IMG *)data;

    mem->memory = myrealloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory)
    {
        memcpy(&(mem->memory[mem->size]), ptr, realsize);
        mem->size += realsize;
        mem->memory[mem->size] = 0;
    }
    //   printf("size: %d\n",mem->size);
    return realsize;
}

const char img_addr[] = {"http://%s/onvifsnapshot/media_service/snapshot?channel=1&subtype=0"};

unsigned char getImg(IMG *img,S_SWIPE *swipe)
{
    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;
    char str[256] = {0};
    headers = curl_slist_append(headers, "Accept: Agent-007");
    curl = curl_easy_init();    // 初始化
    if(curl)
    {
        if(swipe->c_no < MAX_CHANNEL_NUM && swipe->c_direct < 2)
        {
    //        printf("port: %d  %d\n",swipe->c_no,swipe->c_direct);
            snprintf(str,255,img_addr,pChan[swipe->c_no].ip[swipe->c_direct]);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3L);//请求超时时长
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L); //连接超时时长
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);// 改协议头
            curl_easy_setopt(curl, CURLOPT_URL,str);
            m_printf("img add: %s\n",str);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,WriteMemoryCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)img);      // set WriteMemoryCallback *userp

            res = curl_easy_perform(curl);   // 执行
            if (res != 0)
            {
                curl_slist_free_all(headers);
                curl_easy_cleanup(curl);
            }
            return 1;
        }
        else
            return 0;
    }
    else
        return 0;
}

/* we have this global to let the callback get easy access to it */
static pthread_mutex_t *lockarray;
#ifdef USE_OPENSSL
#include <openssl/crypto.h>
static void lock_callback(int mode, int type, char *file, int line)
{
    (void) file;
    (void) line;
    if (mode & CRYPTO_LOCK)
        pthread_mutex_lock(&(lockarray[type]));
    else
        pthread_mutex_unlock(&(lockarray[type]));
}

#if OPENSSL_VERSION_NUMBER >= OPENSSL_VERSION_1_0_0
static void thread_id(CRYPTO_THREADID * id)
{
    id->val = pthread_self();
}
#else
static unsigned long thread_id(void)
{
    unsigned long ret;
    ret = (unsigned long) pthread_self();
    return ret;
}
#endif


void init_locks(void)
{
    int i;
    lockarray = (pthread_mutex_t *) OPENSSL_malloc(CRYPTO_num_locks() * sizeof(pthread_mutex_t));

    for (i = 0; i < CRYPTO_num_locks(); i++)
        pthread_mutex_init(&(lockarray[i]), NULL);

#if OPENSSL_VERSION_NUMBER >= OPENSSL_VERSION_1_0_0
    CRYPTO_THREADID_set_callback(thread_id);
#else
    CRYPTO_set_id_callback((unsigned long (*)()) thread_id);
#endif
    CRYPTO_set_locking_callback((void (*)()) lock_callback);
}

void kill_locks(void)
{
    int i;

    CRYPTO_set_locking_callback(NULL);
    for (i = 0; i < CRYPTO_num_locks(); i++)
        pthread_mutex_destroy(&(lockarray[i]));
    OPENSSL_free(lockarray);
}
#endif


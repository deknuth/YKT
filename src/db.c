#include "../inc/main.h"

void init_DBT(DBT * key, DBT * data)
{
    memset(key, 0, sizeof(DBT));
    memset(data, 0, sizeof(DBT));
}

void db_init(ENTER_DBS *enter)
{
    enter->p_dbp = NULL;
    enter->rt_dbp = NULL;
    enter->ro_dbp = NULL;
}

int channelProcess(void *data,int tfd)
{
    S_PORT *port = data;
    if(port->num < MAX_CHANNEL_NUM)
    {
        memset(pChan[port->num].ip[0],0x00,18);
        memset(pChan[port->num].ip[1],0x00,18);
        snprintf(pChan[port->num].ip[0],17,"%s",port->in_ip);
        snprintf(pChan[port->num].ip[1],17,"%s",port->out_ip);
    }
    m_printf("channel(%d): %s  %s\n",port->num,port->in_ip,port->out_ip);
}

int truleProcess(void *data,int tfd)
{
    S_T_RULE *rule = data;
    s_printf("T rule ID(%d): %s  %ld  %ld  %d  %d\n",rule->rule_id,rule->id,rule->start,rule->end,rule->asc_times,rule->limit);
}

int oruleProcess(void *data,int tfd)
{
    S_O_RULE *rule = data;
    hashManage(rule->type,rule,NULL,1);
    s_printf("ID(%d): %ld  %ld  %d  %d %d\n",rule->rule_id,rule->start,rule->end,rule->asc_times,rule->limit,rule->week);
}

int personProcess(void *data,int tfd)
{
    S_USER *user = data;
    cardManage(user,1,0);
    s_printf("PERSON(%s):  %d  %d \n",user->id,user->lock,user->type);
}

int db_select_all(DB *table,int(*dataProcess)(void *,int),int tfd)
{
    DBC *cursorp;
    DBT key, data;
    init_DBT(&key,&data);
    table->cursor(table, NULL,&cursorp, 0);
    s_printf("table selset:\n");
    while(cursorp->get(cursorp, &key, &data, DB_NEXT) != DB_NOTFOUND)
        dataProcess(data.data,tfd);
    cursorp->close(cursorp);
    return 0;
}

int db_inert(DB *table,void* kid,u_int32_t k_size,void *idata,size_t idata_size)
{
    DBT key, data;
    int ret;
    init_DBT(&key, &data);
    key.data = kid;
    key.size = k_size;
    data.data = idata;
    data.size = idata_size;
    ret = table->put(table, NULL, &key, &data, 0);
    if(ret != 0)
    {
        s_printf("dbp->put ERROR: %s\n", db_strerror(ret));
        return ret;
    }
    table->sync(table, 0);
    return 0;
}

int db_del(DB *table,void* kid,u_int32_t k_size)
{
    DBT key, data;
    int ret;
    init_DBT(&key, &data);
    key.data = kid;
    key.size = k_size;
 //   printf("del id: %s  %d\n",kid,k_size);
    ret = table->del(table, NULL, &key, 0);
    if(ret != 0)
    {
        s_printf("dbp->del ERROR: %s\n", db_strerror(ret));
        return ret;
    }
    table->sync(table, 0);
    return 0;
}

int db_del_all(DB *table)
{
    u_int32_t count;
    table->truncate(table, NULL, &count, 0);    // 删除所有记录，count中返回被删除的记录个数
    table->sync(table, 0);
    s_printf("table del record num: %d\n",count);
    return count;
}

int getORule(DB *table,void* kid,u_int32_t k_size,S_O_RULE *rules)
{
    DBT key, data;
    init_DBT(&key,&data);
    key.data = kid;
    key.size = k_size;
    data.data = rules;
    data.ulen = sizeof(S_O_RULE);
    data.flags = DB_DBT_USERMEM;
    table->get(table, NULL, &key, &data, 0);
    return;
}

int delTRule(DB *table,S_T_RULE *rule)
{
    DBC *cursorp;
    S_T_RULE *info;
    DBT key, data;
    int res,ret;
    init_DBT(&key,&data);

    key.data = rule->id;
    key.size = strlen(rule->id)+1;
    table->cursor(table, NULL,&cursorp, 0);

    ret = cursorp->get(cursorp, &key, &data, DB_SET);       //移动游标到第一条匹配key的数据库记录
    if(ret == 0)
    {
        while(ret != DB_NOTFOUND)
        {
            info = data.data;
            if(info->rule_id == rule->rule_id)
            {
                cursorp->del(cursorp, 0);
                s_printf("T rule del: %d\n",info->rule_id);
                res = 1;
                break;
            }
            ret = cursorp->get(cursorp, &key, &data, DB_NEXT_DUP);
        }
    }
    cursorp->close(cursorp);
    table->sync(table, 0);
    if(ret != 0)
        return 0;
    return res;
}

int matchTRule(DB *table,S_T_COMP *comp,int index)   // temp rule check
{
    DBC *cursorp;
    S_T_RULE *info;
    DBT key, data;
    S_T_RULE replace;
    int ret,res=0;
    init_DBT(&key,&data);

    key.data = comp->id;
    key.size = strlen(comp->id)+1;
 //   printf("key.data:%s %d\n",key.data,key.size);

    table->cursor(table, NULL,&cursorp, 0);

    ret = cursorp->get(cursorp, &key, &data, DB_SET);       //移动游标到第一条匹配key的数据库记录
    if(ret != 0) // not record
        pInfo[index].is_temp = 0;
    else
    {
        while(ret != DB_NOTFOUND)
        {
            info = data.data;

            s_printf("t rule select: %d %d %ld %ld %d\n",
                   info->rule_id,
                   info->asc_times,
                   info->end,
                   info->start,
                   info->limit
                   );

            if(((comp->c_time) > (info->end)) || (info->asc_times)<=0)
            {
                cursorp->del(cursorp, 0);
//        /        s_printf("del this temp rule record!\n");
            }
            else if((comp->c_time) > (info->start))
            {
                   // if((info->limit) == (comp->cr_local))     // direction
                    {
                        memcpy(&replace,info,sizeof(S_T_RULE));
                        replace.asc_times--;
                        data.data = &replace;
                        data.size = sizeof(S_T_RULE);
                        s_printf("replace: %d %d\n",replace.rule_id,replace.asc_times);
                        cursorp->put(cursorp, &key, &data, DB_CURRENT);
                        res = 1;
                        break;
                    }
            }
            ret = cursorp->get(cursorp, &key, &data, DB_NEXT_DUP);
        }
    }
    table->sync(table, 0);
    cursorp->close(cursorp);
    return res;
}

int db_open(DB **dbpp, const char *f_db,DB_ENV *dbenv,unsigned char flag)
{
    DB *dbp;
    u_int32_t flags;
    int ret;

    ret = db_create(&dbp, dbenv, 0);
    if (ret != 0)
    {
        lprintf(lfd,FATAL,"DB: %s\n!",db_strerror(ret));
        return ret;
    }
    *dbpp = dbp;
    if(flag)
        dbp->set_flags(dbp, DB_DUP);
    dbp->set_errpfx(dbp, "db.c");

    flags = DB_CREATE;// | DB_THREAD;

    ret = dbp->open(dbp, NULL, f_db, NULL, DB_BTREE, flags, 0664);
    if (ret != 0)
    {
        lprintf(lfd,FATAL,"DB: Database '%s' open failed!",f_db);
        return ret;
    }
    return 0;
}

int db_setup(ENTER_DBS *enter,DB_ENV *dbenv)
{
    int ret;
    u_int32_t flags;

    ret = db_env_create(&dbenv, 0);
    if(ret != 0)
    {
        lprintf(lfd,FATAL,"DB: db env create ERROR: %s\n!",db_strerror(ret));
        return ret;
    }
    flags = DB_CREATE | DB_INIT_MPOOL ;//| DB_INIT_CDB | DB_THREAD;


    ret = dbenv->open(dbenv, "db", flags, 0);
    if (ret != 0)
    {
        lprintf(lfd,FATAL,"DB: db env open ERROR: %s\n!",db_strerror(ret));
        return ret;
    }

    ret = db_open(&(enter->p_dbp),"person",dbenv,0);
    if(ret != 0)
    {
        lprintf(lfd,FATAL,"DB: person open  ERROR: %s\n!",db_strerror(ret));
        return ret;
    }

    ret = db_open(&(enter->rt_dbp),"trule",dbenv,1);
    if(ret != 0)
    {
        lprintf(lfd,FATAL,"DB: temp rule table open  ERROR: %s\n!",db_strerror(ret));
        return ret;
    }

    ret = db_open(&(enter->ro_dbp),"orule",dbenv,0);
    if(ret != 0)
    {
        lprintf(lfd,FATAL,"DB: office rule table open  ERROR: %s\n!",db_strerror(ret));
        return ret;
    }

    ret = db_open(&(enter->ch_dbp),"channel",dbenv,0);
    if(ret != 0)
    {
        lprintf(lfd,FATAL,"DB: channel table open  ERROR: %s\n!",db_strerror(ret));
        return ret;
    }

    ret = db_open(&(enter->cache_dbp),"cache",dbenv,0);
    if(ret != 0)
    {
        lprintf(lfd,FATAL,"DB: cache table open  ERROR: %s\n!",db_strerror(ret));
        return ret;
    }
    return ret;
}

int db_close(ENTER_DBS *enter,DB_ENV *dbenv)
{
    int ret;
    if(dbenv != NULL)
        dbenv->close(dbenv, 0);
    if(enter->p_dbp != NULL)
    {
        ret = enter->p_dbp->close(enter->p_dbp, 0);
        if (ret != 0)
            lprintf(lfd,FATAL,"DB: close person database failed: %s!",db_strerror(ret));
    }
    if(enter->rt_dbp != NULL)
    {
        ret = enter->rt_dbp->close(enter->rt_dbp, 0);
        if (ret != 0)
            lprintf(lfd,FATAL,"DB: close temp rule database failed: %s",db_strerror(ret));
    }
    lprintf(lfd,INFO,"DB: databases closed.");
    return 0;
}

int db_start(ENTER_DBS *enter)
{
    int ret;
    DB_ENV *dbenv;
    db_init(enter);
    ret = db_setup(enter,dbenv);
    if(ret != 0)
    {
        lprintf(lfd,FATAL,"DB: Error opening databases!");
        db_close(enter,dbenv);
        return ret;
    }
    return ret;
}


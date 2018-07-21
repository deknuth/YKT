#include "../inc/main.h"

unsigned char getWeek(char* str)
{
    int len = strlen(str),i;
    unsigned char week = 0;
    for(i=0; i<len; i+=2)
    {
        switch(str[i]-48)
        {
        case 1:
            week |= 0x01;
            break;
        case 2:
            week |= 0x02;
            break;
        case 3:
            week |= 0x04;
            break;
        case 4:
            week |= 0x08;
            break;
        case 5:
            week |= 0x10;
            break;
        case 6:
            week |= 0x20;
            break;
        case 7:
            week |= 0x40;
            break;
        default:
            break;
        }
    }
  //  printf("week: %d\n",week);
    return week;
}

/*****************
 * o_type->
        1：初始
        2：增加
        3：更新
        4：删除
****************/

int cardManage(S_USER* user,int o_type,unsigned char db)
{
    int index,res=1;
    index = GetHashTablePos(user->id,idHash,MHI);
//    printf("user->id: %s\n",user->id);
    if(o_type != 4)
    {
        if(index == 0)
          index = InsertHash(user->id,idHash,MHI);
        snprintf(pInfo[index].id,17,"%s",user->id);
//        printf("index: %d\n",index);
        pInfo[index].lock = user->lock;
        pInfo[index].p_type = user->type;
        if(db)
        {
            if(db_inert(enter->p_dbp,user->id,strlen(user->id)+1,user,sizeof(S_USER)) != 0)
            {
                lprintf(lfd,FATAL,"DB: insert person id failed!");
                res = 0;
            }
        }
    }
    else        // delete record
    {
        if(index > 0)           // have record
        {
            memset(pInfo[index].id,0x00,17);
            pInfo[index].lock = 0;
            pInfo[index].p_type = 0;
            memset(pInfo[index].position,0x00,RULE_NUM);
            if(db_del(enter->p_dbp,user->id,strlen(user->id)+1) != 0)
            {
                lprintf(lfd,FATAL,"DB: del person id failed!");
                res = 0;
            }
        }
    }
    return res;
}

/**********************
 * r_type->rule type
 * 0---temp
 * 1---office
 * o_type->opretion type
 *      1：初始
        2：增加
        3：更新
        4：删除
 * p_type->person type
 * *******************/

int getNullPos(int type)
{
    int i = 0;
    for(i=0; i<RULE_NUM ;i++)
    {
        if(rule[type].position[i] == 0)
            return i;
    }
}

int hashManage(int p_type,S_O_RULE *pOrule,S_O_RULE *pUpdate,unsigned char flag)
{
    unsigned char i,pos,bit=1;
    int k,end,start,temp;
    switch (flag)
    {
    case 0:     // delete
        for(i=0;i<7;i++)
        {
            if(((pOrule->week)>>i)&0x01)
            {
                temp = i*86400;
                start = temp + pOrule->start;
                end = temp + pOrule->end;
                if((end-start) > 0 && end < temp+86400)
                {
                    if(bit)
                    {
                        pos = (rule[p_type].table[start])>>3;
                        m_printf("delete pos: %d\n",pos);
                        rule[p_type].position[pos] = 0;
                        bit = 0;
                    }
                    for(k=start; k<end; k++)
                    {
                        rule[p_type].table[k] = 0;
                        rule[p_type].swipe[k] = 0;
                    }
                }
            }
        }
        break;
    case 1:         // insert
        for(i=0;i<7;i++)
        {
            if(((pOrule->week)>>i)&0x01)
            {
                temp = i*86400;
                start = temp + pOrule->start;
                end = temp + pOrule->end;
                if((end-start) > 0 && end < temp+86400)
                {
                    if(bit)
                    {
                        pos = getNullPos(p_type);
                        rule[p_type].position[pos] = 1;
                        bit = 0;
                    }
        //            printf("se: %d %d %d\n",start,end,pOrule->week);
                    for(k=start; k<end; k++)
                    {
                        rule[p_type].table[k] |= 0x1;
                        rule[p_type].table[k] |= ((pOrule->limit)<<1);
                        rule[p_type].table[k] |= (pos<<3);
                        rule[p_type].swipe[k] = pOrule->asc_times;
                    }
                }
            }
        }
        break;
    case 2:     // update
        for(i=0;i<7;i++)        // delete original
        {
            if(((pOrule->week)>>i)&0x01)
            {
                temp = i*86400;
                start = temp + pOrule->start;
                end = temp + pOrule->end;
                if((end-start) > 0 && end < temp+86400)
                {
                    if(bit)
                    {
                        pos = (rule[p_type].table[start])>>3;
                        m_printf("delete pos: %d\n",pos);
                        rule[pOrule->type].position[pos] = 0;
                        bit = 0;
                    }
                    for(k=start; k<end; k++)
                    {
                        rule[pOrule->type].table[k] = 0;
                        rule[pOrule->type].swipe[k] = 0;
                    }
                }
            }
        }
        for(i=0;i<7;i++)    // insert current
        {
            if(((pUpdate->week)>>i)&0x01)
            {
                temp = i*86400;
                start = temp + pUpdate->start;
                end = temp + pUpdate->end;
                if((end-start) > 0 && end < temp+86400)
                {
                    if(bit)
                    {
                        m_printf("insert pos: %d\n",pos);
                        pos = getNullPos(p_type);
                        rule[p_type].position[pos] = 1;
                        bit = 0;
                    }
//                    printf("se: %d %d %d\n",start,end,pUpdate->week);
                    for(k=start; k<end; k++)
                    {
                        rule[p_type].table[k] |= 0x1;
                        rule[p_type].table[k] |= ((pUpdate->limit)<<1);
                        rule[p_type].table[k] |= (pos<<3);
                        rule[p_type].swipe[k] = pUpdate->asc_times;
                    }
                }
            }
        }
        break;
    default:
        break;
    }
}

int ruleManage(void* info,int p_type,int o_type,int r_type)
{
    S_O_RULE *pOrule;
    S_O_RULE data;
    S_T_RULE *pTrule;
    int res = 1;
    s_printf("O P R type: %d %d %d\n",o_type,p_type,r_type);
    switch(r_type)
    {
    case 1:         // office rule
        pOrule = info;
        switch(o_type)
        {
        case 1:
        case 2:
            s_printf("o rule insert!\n");
            s_printf("o insert ID(%d): %ld  %ld  %d  %d %d\n",pOrule->rule_id,pOrule->start,pOrule->end,pOrule->asc_times,pOrule->limit,pOrule->week);
            hashManage(p_type,pOrule,NULL,1);
            if(db_inert(enter->ro_dbp,&(pOrule->rule_id),sizeof(int), pOrule,sizeof(S_O_RULE)) != 0)
            {
                res = 0;
                lprintf(lfd,FATAL,"DB: insert office rule failed!");
            }
            break;
        case 3:
            s_printf("o rule update!\n");
            getORule(enter->ro_dbp,&(pOrule->rule_id),sizeof(int),&data);
            s_printf("o check ID(%d): %d  %d  %d\n",data.rule_id,data.start,data.end,data.week);
            hashManage(p_type,&data,pOrule,2);
            if(db_inert(enter->ro_dbp,&(pOrule->rule_id),sizeof(int), pOrule,sizeof(S_O_RULE)) != 0)
            {
                res = 0;
                lprintf(lfd,FATAL,"DB: insert office rule failed!");
            }
            break;
        case 4:     // delete
            hashManage(p_type,pOrule,NULL,0);
            if(db_del(enter->ro_dbp,&(pOrule->rule_id),sizeof(int)) != 0)
            {
                res = 0;
                lprintf(lfd,FATAL,"DB: delete office rule failed!");
            }
            break;
        default:
            break;
        }
        break;
    case 0:     // temp rule
        pTrule = info;
        if(o_type != 4)
        {
            s_printf("insert tr time: %ld %ld\n",pTrule->start,pTrule->end);
            if(db_inert(enter->rt_dbp,pTrule->id,strlen(pTrule->id)+1, pTrule,sizeof(S_T_RULE)) != 0)
            {
                res = 0;
                lprintf(lfd,FATAL,"DB: insert temp rule failed!");
            }
        }
        else
        {
            if(delTRule(enter->rt_dbp,pTrule) == 0)
            {
                lprintf(lfd,FATAL,"DB: delete temp rule failed!");
                res = 0;
            }
        }
        break;
    default:
        break;
    }
    return res;
}

int arrayParse(cJSON *root,int i,cJSON *item,int o_type)
{
    int j,size,p_type,index;
    unsigned char week;
    char id[17] = {0};
    int ret = 0;
    int res = 0;
    unsigned char ch_no;
    S_T_RULE t_rule;
    S_USER user;
    S_O_RULE o_rule;
    cJSON *object,*element;

    item = cJSON_GetArrayItem(root, i);
    size = cJSON_GetArraySize(item);
   // printf("array(%s)\n", item->string);
    //  printf("%s\n", cJSON_Print(item));
    //  printf("%d\n", cJSON_GetArraySize(item));

    if(strcmp(item->string,"person")==0)
    {
        for (j=0; j<size; j++)
        {
            object = cJSON_GetArrayItem(item,j);
            element = cJSON_GetObjectItem(object,"access_no");
            if(element != NULL)
            {
                memset(&user,0x00,sizeof(user));
                snprintf(user.id,17,"%s",element->valuestring);
                element = cJSON_GetObjectItem(object,"is_lock");
                if(element != NULL)
                {
                    user.lock = element->valueint;
                    element = cJSON_GetObjectItem(object,"person_type");
                    if(element != NULL)
                    {
                        user.type = element->valueint;
                        res = cardManage(&user,o_type,1);
                    }
                }
            }
        }
    }
    else if(strcmp(item->string,"official_rule") == 0)
    {
        for (j=0; j<size; j++)
        {
            object = cJSON_GetArrayItem(item,j);

            element = cJSON_GetObjectItem(object,"person_type");

            if(element != NULL)
            {
                m_printf("person_type: %d\n",element->valueint);
                p_type = element->valueint;
                o_rule.type = p_type;
                if(p_type<TYPE_NUM && p_type>-1)
                {
                    element = cJSON_GetObjectItem(object,"week");
                    if(element != NULL)
                    {
                        week = getWeek(element->valuestring);
                        o_rule.week = week;
                    }

                    element = cJSON_GetObjectItem(object,"start_time");
                    if(element != NULL)
                    {
                        o_rule.start = element->valueint;
                        element = cJSON_GetObjectItem(object,"end_time");
                        if(element != NULL)
                            o_rule.end = element->valueint;
                    }

                    element = cJSON_GetObjectItem(object,"direction");
                    if(element != NULL)
                        o_rule.limit = element->valueint;

                    element = cJSON_GetObjectItem(object,"run_times");
                    if(element != NULL)
                        o_rule.asc_times = element->valueint;

                    element = cJSON_GetObjectItem(object,"rule_id");
                    if(element != NULL)
                        o_rule.rule_id = element->valueint;
                    res = ruleManage(&o_rule,p_type,o_type,1);
                }
            }
        }
    }
    else if(strcmp(item->string,"temp_rule") == 0)
    {
        //printf("%s\n", cJSON_Print(item));
        for (j=0; j<size; j++)
        {
            object = cJSON_GetArrayItem(item,j);

            element = cJSON_GetObjectItem(object,"start_time");
            if(element != NULL)
                t_rule.start = element->valueint;
            //printf("start_time: %d\n",element->valueint);

            element = cJSON_GetObjectItem(object,"end_time");
            if(element != NULL)
                t_rule.end = element->valueint;
            //          printf("end_time: %d\n",element->valueint);

            element = cJSON_GetObjectItem(object,"direction");
            if(element != NULL)
                t_rule.limit = element->valueint;
            //                  printf("direction: %d\n",element->valueint);

            element = cJSON_GetObjectItem(object,"run_times");
            if(element != NULL)
                t_rule.asc_times = element->valueint;
            //                    printf("run_times: %d\n",element->valueint);

            element = cJSON_GetObjectItem(object,"rule_id");
            if(element != NULL)
            {
                t_rule.rule_id = element->valueint;
                element = cJSON_GetObjectItem(object,"access_no");
                if(element != NULL)
                {
                    while(1)
                    {
                        memset(id,0x00,17);
                        ret =  sscanf(element->valuestring,"%16[^,]",id);
                        if(ret == -1)
                        {
                            res = 0;
                            break;
                        }
                        if((index=GetHashTablePos(id,idHash,MHI)) > 0)
                            pInfo[index].is_temp = 1;
                        m_printf("trule id: %s index: %d %d\n",id,index,o_type);
                        element->valuestring += strlen(id)+1;
                        snprintf(t_rule.id,17,"%s",id);
                        res = ruleManage(&t_rule,0,o_type,0);
                     //   sleep(1);
                    }
                }
            }
        }
    }
    else if(strcmp(item->string,"cameras") == 0)
    {
        S_PORT port;
        for(j=0; j<size; j++)
        {
            object = cJSON_GetArrayItem(item,j);
            element = cJSON_GetObjectItem(object,"channel_no");
            if(element != NULL)
            {
                m_printf("channel no: %d\n",element->valueint);
                port.num = element->valueint;
                if(port.num < MAX_CHANNEL_NUM)
                {
                    element = cJSON_GetObjectItem(object,"ip_in");
                    if(element != NULL)
                    {
                        sprintf(pChan[port.num].ip[0],"%s",element->valuestring);
                        memset(port.in_ip,0x00,18);
                        strncat(port.in_ip,element->valuestring,17);
                        m_printf("in ip: %s\n",port.in_ip);
                    }
                    element = cJSON_GetObjectItem(object,"ip_out");
                    if(element != NULL)
                    {
                        sprintf(pChan[port.num].ip[1],"%s",element->valuestring);
                        memset(port.out_ip,0x00,18);
                        strncat(port.out_ip,element->valuestring,17);
                        m_printf("out ip: %s\n",element->valuestring);
                    }
                    if(db_inert(enter->ch_dbp, &(port.num), sizeof(int), &port, sizeof(S_PORT)) != 0)
                        lprintf(lfd,FATAL,"DB: insert channel info failed!");
                    else
                        res = 1;
                }
            }
        }
    }
    else if(strcmp(item->string,"take_photos") == 0)
    {
        for (j=0; j<size; j++)
        {
            object = cJSON_GetArrayItem(item,j);
            element = cJSON_GetObjectItem(object,"channel_no");
            if(element != NULL)
            {
                ch_no = object->valueint;
                if(ch_no < MAX_CHANNEL_NUM)
                {
                    element = cJSON_GetObjectItem(object, "ip_in");
                    if(element != NULL)
                    {
                        memset(pChan[ch_no].ip[0],0x00,18);
                        snprintf(pChan[ch_no].ip[0],17,"%s",element->valuestring);
                    }
                    element = cJSON_GetObjectItem(object, "ip_out");
                    if(element != NULL)
                    {
                        memset(pChan[ch_no].ip[1],0x00,18);
                        snprintf(pChan[ch_no].ip[1],17,"%s",element->valuestring);
                    }
                    m_printf("%s %s\n",pChan[ch_no].ip[0],pChan[ch_no].ip[1]);
                    res = 1;
                }
                else
                    res = 0;
            }
        }
    }
    return res;
}

int ruleJson(cJSON *root,int type)
{
    int i,res=1;
    cJSON *item;
    for(i=0; i<cJSON_GetArraySize(root); i++)   //遍历最外层json键值对
    {
        item = cJSON_GetArrayItem(root, i);
        switch(item->type)
        {
        case cJSON_Object:
            ruleJson(item,type);
            break;
        case cJSON_Number:
         //   printf("cJSON_Number\n");
         //   printf("number(%s)->", item->string);
         //   printf("%d\n",item->valueint);
            break;
        case cJSON_String:
       //     printf("cJSON_String\n");
       //     printf("string(%s)->", item->string);
       //     printf("%s\n",item->valuestring);
            break;
        case cJSON_Array:
       //     printf("cJSON_Array\n");
            res = arrayParse(root,i,item,type);
            break;
        default:
            break;
        }
    }
    return res;
}


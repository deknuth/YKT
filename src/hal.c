#include "../inc/main.h"
#define BAUDRATE B115200

#define debugnum(data,len,prefix)\
{\
    unsigned int i;   \
    for (i=0;i < len;i++)\
    {\
        if(prefix) \
            printf("0x%02x ",data[i]);\
        else  \
            printf("%02x ",data[i]);\
    }\
}

#define CARD_USB
int PortInit(char *path)
{
    int fd;
#ifdef CARD_COM

    struct termios newtio;
    fd = open(path, O_RDWR|O_NOCTTY|O_NDELAY);
    if (fd == -1)
    {
        lprintf(lfd,FATAL,"HAL: %s can't open!",path);
        return 0;
    }
    tcgetattr(fd,&newtio);
    bzero(&newtio,sizeof(newtio));
    //setting c_cflag
    newtio.c_cflag &= ~CSIZE;
    newtio.c_cflag |= (CLOCAL|CREAD);
    newtio.c_cflag &= ~PARENB;
    newtio.c_cflag &= ~PARODD;
    newtio.c_cflag &= ~CSTOPB;
    newtio.c_cflag |= CS8;
    newtio.c_oflag |= OPOST;
    newtio.c_oflag &= ~ONLCR;
    newtio.c_cflag &= ~HUPCL;
    newtio.c_lflag &= ~IEXTEN;
    newtio.c_lflag &= ~ECHOK;
    newtio.c_lflag &= ~ECHOCTL;
    newtio.c_lflag &= ~ECHOKE;
    newtio.c_iflag &= ~INPCK;
    newtio.c_iflag |= IGNBRK;
    newtio.c_iflag &= ~ICRNL;
    newtio.c_iflag &=~ (IXON|IXOFF|IXANY);
    cfsetispeed(&newtio,BAUDRATE);
    cfsetospeed(&newtio,BAUDRATE);
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd,TCSANOW,&newtio);
#else

    struct hiddev_devinfo dinfo;
    fd = open(path, O_RDWR);
    if (fd == -1)
    {
        fprintf(stderr, "open %s failure\n", path);
        return;
    }
    m_printf("%s\n", path);
    int version;
    if (ioctl(fd, HIDIOCGVERSION, &version) < 0)
        perror("HIDIOCGVERSION");
    else
    {
        //printf("HIDIOCGVERSION: %d.%d\n", (version >> 16) & 0xFFFF,version & 0xFFFF);
        if (version != HID_VERSION)
            ;//printf("WARNING: version does not match compile-time version\n");
    }

    if (ioctl(fd, HIDIOCGDEVINFO, &dinfo) < 0)
        ;//perror("HIDIOCGDEVINFO");
    else
    {
        /*
        printf("HIDIOCGDEVINFO: bustype=%d busnum=%d devnum=%d ifnum=%d\n"
                "\tvendor=0x%04hx product=0x%04hx version=0x%04hx\n"
                "\tnum_applications=%d\n", dinfo.bustype, dinfo.busnum,
                dinfo.devnum, dinfo.ifnum, dinfo.vendor, dinfo.product,
                dinfo.version, dinfo.num_applications);
                */
    }

    char name[100] = {0};
    if (ioctl(fd, HIDIOCGNAME(99), name) < 0)
        perror("HIDIOCGNAME");
    else
    {
        name[99] = 0;
        //printf("HIDIOCGNAME: %s\n", name);
    }

    show_all_report(fd);
#endif
    return fd;
}

int canSend(int cfd, unsigned int id, unsigned char *data)
{
    struct can_frame frame;
    frame.can_id = id;
    frame.can_dlc = 8;
    memcpy(frame.data,data,8);
    if(write(cfd, &frame, sizeof(frame)) < 0)
    {
        lprintf(lfd,FATAL,"CAN: send failed!");
        close(cfd);
        err->err.can = 1;
        return 0;
    }
    return 1;
}

#ifdef COM_CARD
void readCard(void *arg)
{
    int number = 0;
    int MaxFd;      // 文件描述符个数
    S_WORK *rc = arg;
    fd_set readset;
    struct timeval tv;
    struct hiddev_event ev[64];
    unsigned int i,j=0,index;
    int rd = 0;
    char oid[64] = {0};
    char cid[18] = {0};

    setbuf(stdout,NULL);

    FD_ZERO(&readset);
    do{
        FD_ZERO(&readset); // 文件描述符置0
        if (rc->cfd >= 0)
            FD_SET(rc->cfd, &readset);
        MaxFd = rc->cfd + 1; // 最大文件描述符数
        tv.tv_sec = 0;
        tv.tv_usec = 200000;
        switch (select(MaxFd, &readset, 0, 0, &tv))
        {
        case -1:
            perror("select error");
            rc->cfd = -1;
            break;
        case 0:
            continue;
        default:
            if(FD_ISSET(rc->cfd, &readset)) // FD_ISSET检测fd是否设置
            {
                rd = read(rc->cfd, ev, sizeof(ev));
                if (rd < sizeof(ev[0]))
                {
                    if (rd < 0)
                    {
                        err->err.hal = 1;
                        perror("card read err");
                        break;
                    }
                }
                for (i = 0; i < rd / sizeof(ev[0]); i++)
                {
                    if(ev[i].hid && (ev[i].hid)!=0x280000)
                    {
        //                printf("%c:%d\n",((ev[i].hid)>>16),((ev[i].hid)>>16));
                        if(((ev[i].hid)>>16)<45)
                            oid[j++] = tab[((ev[i].hid)>>16)];
                        else
                            oid[j++] = ((ev[i].hid)>>16);
                    }
                    else if((ev[i].hid)==0x280000)
                    {
                        compleId(oid,cid);
                        printf("READ ID: %s\n",cid);
                        index = GetHashTablePos(cid,idHash,MHI);
                   //     printf("ID INDEX: %d\n",index);
                        if(index > 0)
                        {
                            S_SWIPE swipe;
                            swipe.index = index;
                            swipe.c_no = 1;
                            swipe.tfd = rc->tfd;
                            tpool_add_work(pool, personJudg,&swipe);
                        }
                        else
                        {
                            number = 0;
                            tpool_add_work(pool, PlayAudio,(void *)&number);
                            printf("Invalid card!\n");
                        }
                        memset(oid, 0x00, rd);
                        memset(cid, 0x00, 17);
                        j = 0;
                    }
                }
            }
        }
    }while(rc->cfd >= 0);
    close(rc->cfd);
}

#elif CAN_CARD

void readCard(void *arg)
{
    S_WORK *rc = arg;
    int len,MaxFd,status,index,number;
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_frame frame;
    struct can_filter rfilter[1];
    struct timeval tv;
    fd_set readset;
    char cid[18] = {0};
    char oid[18] = {0};
    status = system("sh can.sh");
    if(WIFEXITED(status))
    {
        if(0 == WEXITSTATUS(status))
            lprintf(lfd,INFO,"system: can.sh exec succeed!");
        else
        {
            if(127 == WEXITSTATUS(status))
                lprintf(lfd,FATAL,"system: command not found!");
            else
               lprintf(lfd,FATAL,"system: command failed (%s)!",strerror(WEXITSTATUS(status)));
        }
    }
    else
        lprintf(lfd,FATAL,"system: subprocess exit failed!");
    sleep(1);

    if((rc->cfd = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
    {
        lprintf(lfd,FATAL,"CAN: create socket failed!");
        goto end;
    }

    strcpy(ifr.ifr_name, CAN);
    ioctl(rc->cfd, SIOCGIFINDEX, &ifr);
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if(bind(rc->cfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        lprintf(lfd,FATAL,"CAN: bind can device failed!");
        close(rc->cfd);
        goto end;
    }

    rfilter[0].can_id = 0x1F;
    rfilter[0].can_mask = 0;//CAN_SFF_MASK;
    if(setsockopt(rc->cfd, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter)) < 0)
    {
        lprintf(lfd,FATAL,"CAN: set receiving filter error!");
        close(rc->cfd);
        goto end;
    }

    if(fcntl(rc->cfd, F_SETFL, fcntl(rc->cfd, F_GETFD, 0)|O_NONBLOCK) == -1)
        goto end;
    err->err.can = 0;
    for(;;)
    {
        FD_ZERO(&readset);
        if(rc->cfd >= 0)
            FD_SET(rc->cfd, &readset);
        MaxFd = rc->cfd + 1;
        tv.tv_sec = 3;
        tv.tv_usec = 0;
        switch(select(MaxFd, &readset, 0, 0, &tv))
        {
        case -1:
            lprintf(lfd,FATAL,"CAN: select error!");
            goto end;
            break;
        case 0:
            break;
        default:
            if(FD_ISSET(rc->cfd, &readset))
            {
                len = read(rc->cfd, &frame, sizeof(frame));
                if(len > 0)
                {
                    HexToString(oid,frame.data,frame.can_dlc);
                    compleId(oid,cid);
                    index = GetHashTablePos(cid,idHash,MHI);
                    if(index > 0)
                    {
                        S_SWIPE *swipe = malloc(sizeof(S_SWIPE));
                        swipe->index = index;
                        swipe->c_no = ((frame.can_id) & 0xFF);
                        swipe->tfd = rc->tfd;
                        getOperId(swipe->oper_id);
                        m_printf("READ ID(%d): %s\n",swipe->c_no,cid);
                        tpool_add_work(pool, personJudg, swipe);
                    }
                    else
                    {
                        number = 0;
                        tpool_add_work(pool, PlayAudio,(void *)&number);
                        m_printf("Invalid card!\n");
                    }
                    memset(cid, 0x00, 17);
                }
                else
                {
                    if(errno == EAGAIN)
                        lprintf(lfd, WARN, "CAN: can read error!");
                    else
                        goto end;
                }
            }
            break;
        }
    }
end:
    close(rc->cfd);
    err->err.can = 1;
}
#endif

void showReports(int fd)
{
    struct hiddev_report_info rinfo;
    struct hiddev_field_info finfo;
    struct hiddev_usage_ref uref;
    unsigned int i, j;
    int ret;

    rinfo.report_type = 1;
    rinfo.report_id = HID_REPORT_ID_FIRST;
    ret = ioctl(fd, HIDIOCGREPORTINFO, &rinfo);
    while (ret >= 0)
    {
       // printf("HIDIOCGREPORTINFO: report_id=0x%X (%u fields)\n",rinfo.report_id, rinfo.num_fields);
        for (i = 0; i < rinfo.num_fields; i++)
        {
            finfo.report_type = rinfo.report_type;
            finfo.report_id = rinfo.report_id;
            finfo.field_index = i;
            ioctl(fd, HIDIOCGFIELDINFO, &finfo);

          /*  printf(
                    "HIDIOCGFIELDINFO: field_index=%u maxusage=%u flags=0x%X\n"
                            "\tphysical=0x%X logical=0x%X application=0x%X\n"
                            "\tlogical_minimum=%d,maximum=%d physical_minimum=%d,maximum=%d\n",
                    finfo.field_index, finfo.maxusage, finfo.flags,
                    finfo.physical, finfo.logical, finfo.application,
                    finfo.logical_minimum, finfo.logical_maximum,
                    finfo.physical_minimum, finfo.physical_maximum);
*/
            for (j = 0; j < finfo.maxusage; j++)
            {
                uref.report_type = finfo.report_type;
                uref.report_id = finfo.report_id;
                uref.field_index = i;
                uref.usage_index = j;
                ioctl(fd, HIDIOCGUCODE, &uref);
                ioctl(fd, HIDIOCGUSAGE, &uref);
/*
                printf(" >> usage_index=%u usage_code=0x%X () value=%d\n",
                        uref.usage_index, uref.usage_code,
                        //    controlName(uref.usage_code),
                        uref.value);*/

            }
        }
   //     printf("\n");

        rinfo.report_id |= HID_REPORT_ID_NEXT;
        ret = ioctl(fd, HIDIOCGREPORTINFO, &rinfo);
    }
}

void show_all_report(int fd)
{
    struct hiddev_report_info rinfo;
    struct hiddev_field_info finfo;
    struct hiddev_usage_ref uref;
    unsigned int rtype, i, j;
    char *rtype_str;

    for (rtype = HID_REPORT_TYPE_MIN; rtype <= HID_REPORT_TYPE_MAX; rtype++)
    {
        switch (rtype)
        {
        case HID_REPORT_TYPE_INPUT:
            rtype_str = "Input";
            break;
        case HID_REPORT_TYPE_OUTPUT:
            rtype_str = "Output";
            break;
        case HID_REPORT_TYPE_FEATURE:
            rtype_str = "Feature";
            break;
        default:
            rtype_str = "Unknown";
            break;
        }
 //       fprintf(stdout, "Reports of type %s (%d):\n", rtype_str, rtype);
        rinfo.report_type = rtype;
        rinfo.report_id = HID_REPORT_ID_FIRST;
        while (ioctl(fd, HIDIOCGREPORTINFO, &rinfo) >= 0)
        {
//            fprintf(stdout, " Report id: %d (%d fields)\n", rinfo.report_id, rinfo.num_fields);
            for (i = 0; i < rinfo.num_fields; i++)
            {
                memset(&finfo, 0, sizeof(finfo));
                finfo.report_type = rinfo.report_type;
                finfo.report_id = rinfo.report_id;
                finfo.field_index = i;
                ioctl(fd, HIDIOCGFIELDINFO, &finfo);
                /*
                fprintf(stdout, " Field: %d: app: %04x phys %04x "
                        "flags %x (%d usages) unit %x exp %d\n", i,
                        finfo.application, finfo.physical, finfo.flags,
                        finfo.maxusage, finfo.unit, finfo.unit_exponent);
                        */
                memset(&uref, 0, sizeof(uref));
                for (j = 0; j < finfo.maxusage; j++)
                {
                    uref.report_type = finfo.report_type;
                    uref.report_id = finfo.report_id;
                    uref.field_index = i;
                    uref.usage_index = j;
                    ioctl(fd, HIDIOCGUCODE, &uref);
                    ioctl(fd, HIDIOCGUSAGE, &uref);
                //    fprintf(stdout, " Usage: %04x val %d\n", uref.usage_code,uref.value);
                }
            }
            rinfo.report_id |= HID_REPORT_ID_NEXT;
        }
    }
    // if (!run_as_daemon)
    fprintf(stdout, "Waiting for events ... (interrupt to exit)\n");
}

int getInfo(int sfd)
{
    while(1)
    {
        showReports(sfd);
    }
}

const unsigned char tab[45] =
{
    0,0,0,0,65,66,67,68,69,70,
    71,72,73,74,75,76,77,78,79,80,
    81,82,83,84,85,86,87,88,89,90,
    49,50,51,52,53,54,55,56,57,48,
    42,42,42,42,32
};


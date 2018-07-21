/****************** CLIENT CODE ****************/
#include "../inc/main.h"
/******* setnonblocking - 设置句柄为非阻塞方式 *******/
int setnonblocking(int sockfd)
{
    struct timeval timeout = { 3, 0 };
    //    int mw_optval = 1;
    int nRecvBuf = 512*1024;		//	512K
    int nSendBuf = 512*1024;		//	512K
    //   setsockopt(sockfd,SOL_SOCKET,SO_REUSEPORT,(char *)&mw_optval,sizeof(mw_optval));
    //   setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(char *)&mw_optval,sizeof(mw_optval)); 	// 设置端口多重邦定
    setsockopt(sockfd,SOL_SOCKET,SO_SNDTIMEO,(char *)&timeout,sizeof(timeout));
    setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout,sizeof(timeout));
    setsockopt(sockfd,SOL_SOCKET,SO_RCVBUF,(const char*)&nRecvBuf,sizeof(int));
    setsockopt(sockfd,SOL_SOCKET,SO_SNDBUF,(const char*)&nSendBuf,sizeof(int));
    if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)|O_NONBLOCK) == -1)
        return 0;
    return 1;
}

#define TIME_OUT_TIME 5
int TcpClient(void)
{
    int cfd;
    struct sockaddr_in sAddr;
    socklen_t addr_size;
    int error=0, len;
    len = sizeof(int);
    struct timeval tm;
    fd_set set;
    int ret = 1;

    cfd = socket(PF_INET, SOCK_STREAM, 0);
    setnonblocking(cfd);		// 设置非阻塞模式

    sAddr.sin_family = AF_INET;
    sAddr.sin_port = htons(8088);
    sAddr.sin_addr.s_addr = inet_addr("120.78.144.255");
    memset(sAddr.sin_zero, '\0', sizeof(sAddr.sin_zero));
    addr_size = sizeof(sAddr);
    if(connect(cfd, (struct sockaddr *) &sAddr, addr_size) == -1)
    {
        tm.tv_sec = TIME_OUT_TIME;
        tm.tv_usec = 0;
        FD_ZERO(&set);
        FD_SET(cfd, &set);
        if(select(cfd+1, NULL, &set, NULL, &tm) > 0)
        {
            getsockopt(cfd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
            if(error == 0)
                ret = 1;
            else
                ret = 0;
        }
        else
            ret = 0;
    }
    if(!ret)
    {
        close(cfd);
        return 0;
    }
    return cfd;
}

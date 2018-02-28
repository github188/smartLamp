#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include "log.h"
#include "netinf.h"
#include <sys/msg.h>
#include <fcntl.h>


/********************************************************************
* 函数名:   ConnectServer
* 功能:     用来建立一个tcp客户端socket
* 参数说明：serverip    服务器IP地址或主机名
*           serverport  服务器端口
*           nonblock  整数型是否非阻塞
* 作者:     zhangwei
* 日期:     2017-04-06
*********************************************************************/
int ConnectServer(char * serverip, int serverport, int nonblock)
{
    int    sockfd = 0;
    int    ret = 0;
    
    struct sockaddr_in    sin = { 0 };
    struct hostent *he = NULL;
    char   szLogInfo[1024] = { 0 };

    he = gethostbyname(serverip);
    if(NULL == he)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "gethostbyname failed! serverip[%s]", serverip);
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == sockfd)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "socket create failed! errno=%d, errmsg[%s]", errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
        return -2;
    }

    ioctl(sockfd, FIONBIO, &nonblock);  //non-block or not  1: non-block  0:block

    memset((char*)&sin, 0, sizeof(struct sockaddr_in));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(serverport);
    sin.sin_addr = *((struct in_addr *)he->h_addr);

    ret = connect(sockfd, (struct sockaddr *)&sin, sizeof(sin));
    if(-1 == ret)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "connect failed! errno=%d, errmsg[%s]", errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
	 	printf("%s\n", szLogInfo);
        close(sockfd);
        return -3;
    }

    return sockfd;
}

//add by tianyu
int Send_New(int sock, char * buf, size_t size, int flag, int timeout)
{
	char   szLogInfo[1024] = { 0 };
	int    ret = 0;
	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "enter Send_New");
    WRITELOG(LOG_INFO, szLogInfo); 
	
    //ret = write(sock, buf, size);
	ret = send(sock, buf, size, 0);
    if(-1 == ret)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "write error! errno=%d,errmsg[%s]", errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
        return -2;
    }

	if (ret < size)
	{
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "write finished, but nSend(%d) < buflen(%d)!", ret, size);
        WRITELOG(LOG_WARN, szLogInfo);
    }
	else
	{
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "write finished, sendlen=%d, buflen=%d", ret, size);
        WRITELOG(LOG_INFO, szLogInfo);
	}
	return 0;
}

/********************************************************************
* 函数名:   Send
* 功能:     用来通过一个socket在指定时间内发送数据
* 参数说明：sock    整数型socket
*           buf     待发送的内容
*           size    要发送的大小
*           flag    发送选项
*           timeout 超时时间值
* 作者:     zhangwei
* 日期:     2017-04-06
*********************************************************************/
int Send(int sock, char * buf, size_t size, int flag, int timeout)
{
    int    i = 0;
    int    ret = 0;
    int    intretry = 0;
    char   szLogInfo[1024] = { 0 };
    struct timeval tival = { 0 };
    fd_set writefds = { 0 };
    int maxfds = 0;
    int nonblock = 0;

    tival.tv_sec = timeout;
    tival.tv_usec = 0;

    FD_ZERO(&writefds);

    if(sock > 0)
    {
        ioctl(sock, FIONBIO, &nonblock);  //non-block or not
        FD_SET(sock, &writefds);
        maxfds = ((sock > maxfds) ? sock : maxfds);
    }
    else
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Send socket:%d error!", sock);
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }

    ret = select(maxfds + 1, NULL, &writefds, NULL, &tival);
    if(ret <= 0)
    {
        if(ret < 0)
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Send socket:%d select() error! return:%d, errno=%d, errmsg[%s]", sock, ret, errno, strerror(errno));
            WRITELOG(LOG_ERROR, szLogInfo);
	     close(sock);
            return -2;
        }
        else
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Send socket:%d select timeout(%d)!", sock, timeout);
            WRITELOG(LOG_ERROR, szLogInfo);
	     return 0;
        }
    }
    if(!(FD_ISSET(sock, &writefds)))
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Send socket:%d not in writefds!", sock);
        WRITELOG(LOG_ERROR, szLogInfo);
        close(sock);
        return -3;
    }

    while(i < size)
    {
        ret = send(sock, buf + i, size - i, flag);
		
        if(ret <= 0)
        {
             if  ((errno != EAGAIN) && (errno != EWOULDBLOCK) && ( errno != EINTR ))	
	      {	
	           snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Write.\n");	
		   WRITELOG(LOG_ERROR, szLogInfo);	
		   close(sock);
		   return -4;//返回错误，要关闭SOCKET				
	     }				
	     else		
	     {
	             snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Write EAGAIN.\n");		
		      WRITELOG(LOG_INFO, szLogInfo);					
		     continue;//add by tang bisheng				
	    }
           /*
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Send socket:%d send() error! return:%d, errno=%d, errmsg[%s]", sock, ret, errno, strerror(errno));
            WRITELOG(LOG_ERROR, szLogInfo);
            if (EINTR == errno)
            {
               if(intretry < 3)
               {
                   intretry++;
		     Sleep(100);
                   continue;
               }
               else
               {
                   snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Send socket:%d send() error!EINTR 10 times!", sock);
                   WRITELOG(LOG_ERROR, szLogInfo);
               }
            }
            close(sock);
            return -4;
            */
        }
        else
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "The below information is send function[172 line]");
            WRITELOG(LOG_INFO, szLogInfo);
            dumpInfo(buf + i, ret);
            i += ret;
        }
    }
    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Send socket:%d send() OK! %d/%d bytes sent!", sock, i, size);
    WRITELOG(LOG_INFO, szLogInfo);
    return i;
}


//add by tianyu
int Recv_New(int sock, char * buf, size_t size, int flag, int timeout)
{
		int i = 0, ret = 0, intretry = 0;
		char   szLogInfo[1024] = { 0 };

		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "enter Recv_New. socket[%d] buf[%p] size[%d]", 
														sock, buf, size);
    	WRITELOG(LOG_INFO, szLogInfo); 
		
		if (sock <= 0)
		{
			snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Recv socket:%d error!", sock);
			WRITELOG(LOG_ERROR, szLogInfo);
			return -1;
		}
		
		ret = recv(sock, buf, size, 0);
		//ret = read(sock, buf, size);
		if (ret < 0)
   		{
        	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "read failed! errno=%d, errmsg[%s]", errno, strerror(errno));
        	WRITELOG(LOG_ERROR, szLogInfo);
        	return -1;
    	}
    	else if (0 == ret)
    	{
        	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "server close the connection! sockfd=%d", sock);
        	WRITELOG(LOG_ERROR, szLogInfo);
        	return -2;
    	}

		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Recv socket:%d recv() OK! %d/%d bytes received!", sock, ret, size);
    	WRITELOG(LOG_INFO, szLogInfo);
		return ret;
}

int setnonblock(int sockfd)
{
    if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK) == -1)
    {
        return -1;
    }
    
    return 0;
}

int read_safe(int fd, char *buf, int size)
{
	int read_size = 0;
	int ret;
	char   szLogInfo[1024] = { 0 };
	while (read_size < size) 
	{
		ret = read(fd, buf + read_size, size - read_size);
		/*
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "******after read: ret[%d] errno[%d] errmsg[%s]!******",
												ret, errno, strerror(errno));
   		WRITELOG(LOG_INFO, szLogInfo);
   		*/
		if (ret < 0) 
		{
			/*
			snprintf(szLogInfo, sizeof(szLogInfo) - 1, "******read error errno[%d] errmsg[%s]!******",
												errno, strerror(errno));
   			WRITELOG(LOG_ERROR, szLogInfo);
			*/
			if (errno == EINTR) 
			{
				continue;
			}
			else if (errno == EAGAIN)
			{
				break;
			}
			snprintf(szLogInfo, sizeof(szLogInfo) - 1, "******read error: return ******", 
												errno, strerror(errno));
   			WRITELOG(LOG_ERROR, szLogInfo);
			return -1;
		}
		if (ret == 0)
		{
			snprintf(szLogInfo, sizeof(szLogInfo) - 1, "******server close socket ******"
													);
   			WRITELOG(LOG_ERROR, szLogInfo);
			return -1;
		}
		read_size += ret;
	}
	return read_size;
}

int write_safe(int fd, char *buf, int size)
{
	int written_size = 0;
    int ret;
	char   szLogInfo[1024] = { 0 };
    while (written_size < size) 
	{
        ret = write(fd, buf + written_size, size - written_size);
        if (ret < 0) 
		{
            if (errno == EINTR) 
			{
					continue;
			}
			else if (errno == EAGAIN)
			{
				break;
			}
			snprintf(szLogInfo, sizeof(szLogInfo) - 1, "******write error errno[%d] errmsg[%s]!******",
												errno, strerror(errno));
   			WRITELOG(LOG_ERROR, szLogInfo); 
			return -1;
        }
		if (ret == 0)
		{
			snprintf(szLogInfo, sizeof(szLogInfo) - 1, "******server close socket******");
   			WRITELOG(LOG_ERROR, szLogInfo); 
		}
        written_size += ret;
    }
    return written_size;
}


/********************************************************************
* 函数名:   Recv
* 功能:     用来从一个socket在指定时间内读取数据
* 参数说明：sock    整数型socket
*           buf     接收数据的缓冲区
*           size    要接收数据的大小
*           flag    接收选项
*           timeout 超时时间值
* 作者:     zhangwei
* 日期:     2017-04-06
*********************************************************************/
int Recv(int sock, char * buf, size_t size, int flag, int timeout)
{
    int i = 0, ret = 0, intretry = 0;
    char   szLogInfo[1024] = { 0 };
    struct timeval tival;
    fd_set readfds;
    int maxfds = 0;
    int nonblock = 0;

    tival.tv_sec = timeout;
    tival.tv_usec = 0;

    FD_ZERO(&readfds);

    if(sock > 0)
    {
        ioctl(sock, FIONBIO, &nonblock);  //non-block or not
        FD_SET(sock, &readfds);
        maxfds = ((sock > maxfds) ? sock : maxfds);
    }
    else
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Recv socket:%d error!", sock);
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }

    if(0 == timeout)
    {
        ret = select(maxfds + 1, &readfds, NULL, NULL, NULL);
    }
    else
    {
        ret = select(maxfds + 1, &readfds, NULL, NULL, &tival);
    }
    if(ret <= 0)
    {
        if(ret < 0)
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Recv socket:%d select() error! return:%d, errno=%d, errmsg[%s]", sock, ret, errno, strerror(errno));
            WRITELOG(LOG_ERROR, szLogInfo);
	     close(sock);
            return -2;
        }
        else
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Recv socket:%d select timeout(%d)!", sock, timeout);
            WRITELOG(LOG_ERROR, szLogInfo);
	     return 0;
        }
    }
    if(!(FD_ISSET(sock, &readfds)))
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Recv socket:%d not in readfds!", sock);
        WRITELOG(LOG_ERROR, szLogInfo);
        close(sock);
        return -3;
    }
    while(i < size)
    {
        ret = recv(sock, buf + i, size - i, flag);
        if(ret <= 0)
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Recv socket:%d recv() error! return:%d, errno=%d, errmsg[%s]", sock, ret, errno, strerror(errno));
            WRITELOG(LOG_ERROR, szLogInfo);
            if(errno == EINTR)
            {
                if(intretry < 3)
                {
                    intretry++;
			Sleep(100);
                    continue;
                }
                else
                {
                     snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Recv socket:%d recv() error! EINTR 10 times!", sock);
                }
            }
            close(sock);
            return -4;
        }
        else if(ret < size - i)
        {
            i += ret;
            break;
        }
	 else
        {
            i += ret;
        }
    }
    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Recv socket:%d recv() OK! %d/%d bytes received!", sock, i, size);
    WRITELOG(LOG_INFO, szLogInfo);
    return i;
}



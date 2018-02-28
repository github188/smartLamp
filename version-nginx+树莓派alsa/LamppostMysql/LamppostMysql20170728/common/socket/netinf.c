#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/msg.h>
#include <errno.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>

#include "../log/log.h"
#include "netinf.h"
#include "netinfo.h"
#include "../../chargepilesys/global/global.h"


static int g_epollfd = 0;
static pthread_t g_recvtid = 0;
static pthread_t g_sendtid = 0;


//add by tianyu
pthread_t g_io_thread = 0;
int g_pipe[2] = {-1, -1};
pthread_mutex_t g_pipe_mutex;

int g_recv_msqid = -1;
int g_send_msqid = -1;

void dumpInfo1(unsigned char *info, int length)
{
	char log_str_buf[2048] = {0};
	int index = 0;
	int j = 0;
	char buffer[1024] = {0};
	memset(buffer, 0, 1024);
	for (index = 0; index < length; index++, j+=2)
	{
		sprintf(&buffer[j],"%02x",info[index]);
	}
	snprintf(log_str_buf, sizeof(log_str_buf)-1, "dump info length = %d, %s\n", length, buffer);
	WRITELOG(LOG_INFO, log_str_buf);
}


int setnonblock(int sockfd)
{
    if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK) == -1)
    {
        return -1;
    }
    
    return 0;
}

/********************************************************************
* 函数名:   OpenServer
* 功能:     用来建立一个tcp服务端socket
* 参数说明：port        整数型监听端口号
*           total       整数型监听个数
*           sendbuflen  整数型发送缓冲区大小
*           recvbuflen  整数型接收缓冲区大小
*           blockORnot  整数型是否阻塞
*           reuseORnot  整数型是否端口重用
* 作者:     zhangwei
* 日期:     2017-04-06
*********************************************************************/
int OpenServer(int port, int total, int sendbuflen, int recvbuflen, int blockORnot, int reuseORnot)
{
    int    index    = 0;
    int    listenfd = 0;
    int    ret      = 0;
    struct epoll_event    ev;
	//add by tianyu
	struct epoll_event    ev1;
    struct sockaddr_in    localaddr;
    char   szLogInfo[1024] = { 0 };

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "create socket failed! errno=%d, errmsg[%s]", errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }

    ret = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuseORnot, sizeof(int));
    if(ret < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "setsockopt: reuse failed! errno=%d, errmsg[%s]", errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
        return -2;
    }

    ret = setsockopt(listenfd, SOL_SOCKET, SO_RCVBUF, &recvbuflen, sizeof(int));
    if ( ret < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "setsockopt: recvbuf failed! errno=%d, errmsg[%s]", errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
        return -3;
    }

    ret = setsockopt(listenfd, SOL_SOCKET, SO_SNDBUF, &sendbuflen, sizeof(int));
    if (ret < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "setsockopt: sendbuf failed! errno=%d, errmsg[%s]", errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
        return -4;
    }

    ioctl(listenfd, FIONBIO, &blockORnot);/*block or not*/

    localaddr.sin_family = AF_INET;
    localaddr.sin_port = htons(port);
    localaddr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(localaddr.sin_zero), 8);

    ret = bind(listenfd, (struct sockaddr *)&localaddr, sizeof(struct sockaddr));
    if(ret < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "bind failed! errno=%d, errmsg[%s]", errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
        close(listenfd);
        return -5;
    }
    ret = listen(listenfd, total);
    if(ret < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "listen failed! errno=%d, errmsg[%s]", errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
        close(listenfd);
        return -6;
    }
    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "opened on port %d(%d) OK, socket(%d), buf(%d:%d)!", port, total, listenfd, sendbuflen, recvbuflen);
    WRITELOG(LOG_INFO, szLogInfo);

    g_epollfd = epoll_create(MAXEPOLLSIZE);
    if(-1 == g_epollfd)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "epoll_create failed! errno=%d, errmsg[%s]", errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
        close(listenfd);
        return -7;
    }

    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = listenfd;
    ret = epoll_ctl(g_epollfd, EPOLL_CTL_ADD, listenfd, &ev);
    if(-1 == ret)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "epoll_ctl: add failed! errno=%d, errmsg[%s]", errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
        return -8;
    }

    // socket接收消息队列
    g_recv_msqid = msgget(SOCK_RECV_MSGKEY, IPC_CREAT | 0666);
    if(g_recv_msqid < 0)
    { 
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "create socket recv msq failed! errno=%d, errmsg[%s]", errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
        return -10;
    }
#if 0	
    // socket接收线程
    ret = pthread_create(&g_recvtid, NULL, recv_entry, (void*)(long)listenfd);
    if(0 != ret)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "create socket recv thread failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -9;
    }
    
    // socket发送消息队列
    g_send_msqid = msgget(SOCK_SEND_MSGKEY, IPC_CREAT | 0666);
    if(g_send_msqid < 0)
    { 
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "create socket send msq failed! errno=%d, errmsg[%s]", errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
        return -12;
    }
    // socket发送线程
    ret = pthread_create(&g_sendtid, NULL, send_entry, (void*)NULL);
    if(0 != ret)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "create socket send thread failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -11;
    }
#else
	//多个线程同时向pipe写数据时，ET模式epoll同一时间可能只触发一次
	//每次读取长度只有一个结构体大小，可能会漏读消息
	//将模式改为LT
	//ev1.events = EPOLLIN | EPOLLET;
	ev1.events = EPOLLIN;
    ev1.data.fd = g_pipe[0];
    ret = epoll_ctl(g_epollfd, EPOLL_CTL_ADD, g_pipe[0], &ev1);
    if(-1 == ret)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "epoll_ctl: add failed! errno=%d, errmsg[%s]", errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
        return -9;
    }
	//socket 处理线程
	ret = pthread_create(&g_io_thread, NULL, io_proc, (void*)(long)listenfd);
	if (0 != ret)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "create socket io thread failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -10;
	}
#endif    
    return 0;
}

void *recv_entry(void *arg)
{
    int    ret      = 0;
    int    listenfd = (long)arg;
    int    connfd   = 0;
    int    nConnFds    = 0;
    int    i = 0;
    int    curfds = 1;
    struct sockaddr_in    clientaddr;
    socklen_t socklen = sizeof(struct sockaddr_in);
    struct epoll_event    ev;
    struct epoll_event    events[MAXEPOLLSIZE];
    char   szLogInfo[1024] = { 0 };

    while(1)
    {
        nConnFds = epoll_wait(g_epollfd, events, MAXEPOLLSIZE, -1);
        if(-1 == nConnFds)
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "epoll_wait failed! errno=%d, errmsg[%s]", errno, strerror(errno));
            WRITELOG(LOG_ERROR, szLogInfo);
            continue;
        }

        for(i = 0; i < nConnFds; i++)
        {
            if(events[i].data.fd == listenfd)
            {
                if(curfds > MAXEPOLLSIZE)
                {
                    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "too many connection, more than %d", MAXEPOLLSIZE);
                    WRITELOG(LOG_ERROR, szLogInfo);
                    continue;
                }
                connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &socklen);
                if(-1 == connfd)
                {
                    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "accept failed! errno=%d, errmsg[%s]", errno, strerror(errno));
                    WRITELOG(LOG_ERROR, szLogInfo);
                    continue;
                }

                ret = setnonblock(connfd);
                if(-1 == ret)
                {
                    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "setnonblock failed! connfd=%d", connfd);
                    WRITELOG(LOG_ERROR, szLogInfo);
                }
				
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = connfd;
                ret = epoll_ctl(g_epollfd, EPOLL_CTL_ADD, connfd, &ev);
                if(-1 == ret)
                {
                    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "epoll_ctl: add failed! connfd=%d", connfd);
                    WRITELOG(LOG_ERROR, szLogInfo);
                    close(connfd);
                    continue;
                }

                curfds++;
            }
            else
            {
                ret = recvproc(events[i].data.fd);
                if(0 != ret)
                {
                    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "recvproc failed! ret=%d", ret);
                    WRITELOG(LOG_ERROR, szLogInfo);
                    //if (-2 == ret)   // 对端关闭连接
					if ( -1 == ret || -2 == ret ) //modify by tianyu
					{
                        epoll_ctl(g_epollfd, EPOLL_CTL_DEL, events[i].data.fd, &ev);
                        close(events[i].data.fd);

						// 从hash表中删除数据
						delete_net_fd(events[i].data.fd);
						
                        curfds--;
                    }
                }
            }
        }
    }

    close(g_epollfd);
    close(listenfd);
    
    return NULL;
}

void *send_entry(void *arg)
{
    int ret    = 0;
    char   szLogInfo[1024] = { 0 };
    
    while(1)
    {
        ret = sendproc();
        if(0 != ret)
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "sendproc failed! ret=%d", ret);
            WRITELOG(LOG_ERROR, szLogInfo);
            sleep(0);
            continue;
        }
    }
    
    return;
}

int recvproc(int fd)
{
    int ret   = 0;
    int nread = 0;
	int total = 0;
    t_buf buf = {0};
    t_mymsg    mymsg = { 0 };
    char   szLogInfo[1024] = { 0 };

    buf.fd = fd;
    
    // nread = recv(buf.fd, buf.buf, MAXLINE, 0);//读取客户端socket流
    nread = read(buf.fd, buf.buf, MAXLINE);//读取客户端socket流
    if (nread < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "read failed! errno=%d, errmsg[%s]", errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
    else if (0 == nread)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "client close the connection! sockfd=%d", fd);
        WRITELOG(LOG_ERROR, szLogInfo);
        return -2;
    }
    buf.len = nread;

	/*
    nread = 0;
	total = 0;
	while (1)
	{
	    nread = read(buf.fd, buf.buf+nread, MAXLINE);//读取客户端socket流
	    if (nread < 0)
        {
            if ( ( errno != EWOULDBLOCK ) && ( errno != EAGAIN ) && ( errno != EINTR ) && (errno != EINPROGRESS))
            {
                snprintf(szLogInfo, sizeof(szLogInfo)-1, "read failed errno=%d,errinfo=%s.",errno,strerror(errno));				
                WRITELOG(LOG_INFO, szLogInfo);
				close(buf.fd);
				return -1;				
            }
			else
			{
                snprintf(szLogInfo, sizeof(szLogInfo)-1, "read errno=%d,errinfo=%s.",errno,strerror(errno));				
                WRITELOG(LOG_INFO, szLogInfo);

			    nread = total;

				Sleep(100);
			    continue;
			}
        }
		
	    total += nread;

		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "total=%d", total);
        WRITELOG(LOG_INFO, szLogInfo);
		
        if (total < 20)
        {
            Sleep(100);
            continue;
        }
        else
        {
            break;
        }
	}

	buf.len = total;
	*/

    mymsg.mtype = 1;
    memcpy(mymsg.mtext, &buf, min(sizeof(int) + sizeof(int) + buf.len, MAXLINE));

    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "The below information is recvproc function[376 line]");
    WRITELOG(LOG_INFO, szLogInfo); 
    dumpInfo1(buf.buf, buf.len); /////

    //打印mymsg.mtype 值
    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "mymsg.mtype= %d   in recvproc............................",mymsg.mtype);
    WRITELOG(LOG_INFO, szLogInfo); 

    ret = msgsnd(g_recv_msqid, &mymsg, min(sizeof(int) + sizeof(int) + buf.len, MAXLINE), IPC_NOWAIT);
    if(ret < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "msgsnd failed! errno=%d, errmsg[%s]", errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
        return -3;
    }
    
    return 0;
}

int sendproc()
{
    int nRecv    = 0;
    int nSend    = 0;
    t_mymsg    mymsg = { 0 };
    t_buf     *buf   = NULL;
    char   szLogInfo[1024] = { 0 };
    
    nRecv = msgrcv(g_send_msqid, &mymsg, MAXLINE, 0, 0); // 阻塞接收
    if(nRecv < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "msgrcv failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }

    buf = (t_buf *)mymsg.mtext;
    
    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "*****begin write msg. fd[%d]*******", buf->fd);
    WRITELOG(LOG_INFO, szLogInfo); 
    dumpInfo1(buf->buf, buf->len); /////
	
    nSend = write(buf->fd, buf->buf, buf->len);
    if(-1 == nSend)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "write error! errno=%d,errmsg[%s] fd[%d]", errno, strerror(errno), buf->fd);
        WRITELOG(LOG_ERROR, szLogInfo);
        return -2;
    }

	if (nSend < buf->len)
	{
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "write finished, but nSend(%d) < buflen(%d)! fd[%d]", nSend, buf->len, buf->fd);
        WRITELOG(LOG_WARN, szLogInfo);
    }
	else
	{
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "write finished, sendlen=%d, buflen=%d fd[%d]", nSend, buf->len, buf->fd);
        WRITELOG(LOG_INFO, szLogInfo);
	}
    
    return 0;
}


//add by tianyu

int read_safe(int fd, char *buf, int size)
{
	int read_size = 0;
	int ret;
	char   szLogInfo[1024] = { 0 };
	while (read_size < size) 
	{
		ret = read(fd, buf + read_size, size - read_size);
		if (ret < 0) 
		{
			if (errno == EINTR) 
			{
				continue;
			}
			else if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
			{
				break;
			}
			snprintf(szLogInfo, sizeof(szLogInfo) - 1, "******read error errno[%d] errmsg[%s]!******",
												errno, strerror(errno));
   			WRITELOG(LOG_ERROR, szLogInfo);
			return -1;
		}
		if (ret == 0)
		{
			snprintf(szLogInfo, sizeof(szLogInfo) - 1, "******client close socket!******"
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
			else if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
			{
				break;
			}
			snprintf(szLogInfo, sizeof(szLogInfo) - 1, "******write error. ret[%d] errno[%d] errmsg[%s]!******",
												ret, errno, strerror(errno));
   			WRITELOG(LOG_ERROR, szLogInfo); 
			return -1;
        }
		if (ret == 0)
		{
			snprintf(szLogInfo, sizeof(szLogInfo) - 1, "******client close socket!******"
												);
   			WRITELOG(LOG_ERROR, szLogInfo);
			return -1;
		}
        written_size += ret;
    }
    return written_size;
}

int read_proc(int fd)
{
	int read_len = 0;
	int ret;
	char   szLogInfo[1024] = { 0 };
	t_buf *pbuf = NULL;
    t_mymsg    mymsg = { 0 };

	pbuf = (t_buf *)mymsg.mtext;
	pbuf->fd = fd;
	ret = read_safe(fd, pbuf->buf, MAXLINE);
	if (ret < 0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "******read socket error!******");
   		WRITELOG(LOG_ERROR, szLogInfo); 
		return -1;
	}
	pbuf->len = ret;

	mymsg.mtype = 1;
    
    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "The below information is recvproc function[376 line]");
    WRITELOG(LOG_INFO, szLogInfo); 
    //dumpInfo1(buf.buf, buf.len); /////

    //打印mymsg.mtype 值
    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "mymsg.mtype= %d   in recvproc............................",mymsg.mtype);
    WRITELOG(LOG_INFO, szLogInfo); 

    ret = msgsnd(g_recv_msqid, &mymsg, min(sizeof(int) + sizeof(int) + pbuf->len, MAXLINE), IPC_NOWAIT);
    if(ret < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "msgsnd failed! errno=%d, errmsg[%s]", errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
        return -3;
    }
	
	return 0;
}

int send_proc(int fd, int * cur_fds)
{
	t_mymsg m_msg = {0};
	int ret;
	struct epoll_event	  ev;
	char   szLogInfo[1024] = { 0 };
	ret = read_safe(fd, (char *)&m_msg, sizeof(t_mymsg));
	//ret = read_safe(fd, (char *)&m_msg, 1024*4);
	if (ret<0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "******read from pipe error!******");
   		WRITELOG(LOG_ERROR, szLogInfo); 
		return -2;
	}
	t_buf * buf   = NULL;
	buf = (t_buf *)m_msg.mtext;
    
    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "*****begin write msg. fd[%d]*******", buf->fd);
    WRITELOG(LOG_INFO, szLogInfo); 
    //dumpInfo1(buf->buf, buf->len); 
	
	// TODO: 检测fd可写再发送

	ret = write_safe(buf->fd, buf->buf, buf->len);
	if (ret<0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "******send msg error! ret[%d]******", ret);
   		WRITELOG(LOG_ERROR, szLogInfo); 
#if 0
		epoll_ctl(g_epollfd, EPOLL_CTL_DEL, buf->fd, &ev);
		close(buf->fd);
	
		// 从hash表中删除数据
		delete_net_fd(buf->fd);					
		(*cur_fds)--;
#endif
		return -1;
	}
	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "******finsih send msg! send_len[%d] buf_len[%d]******", 
													ret, buf->len);
   	WRITELOG(LOG_INFO, szLogInfo);
	return 0;
}


void * io_proc(void * arg)
{
	int    ret		= 0;
	int    listenfd = (long)arg;
	int    connfd	= 0;
	int    nConnFds    = 0;
	int    i = 0;
	int    curfds = 1;
	int    act_fd = 0;	
	struct sockaddr_in	  clientaddr;
	socklen_t socklen = sizeof(struct sockaddr_in);
	struct epoll_event	  ev;
	struct epoll_event	  events[MAXEPOLLSIZE];
	char   szLogInfo[1024] = { 0 };
	
	while(1)
	{
		nConnFds = epoll_wait(g_epollfd, events, MAXEPOLLSIZE, 5000);
		if(-1 == nConnFds)
		{
			snprintf(szLogInfo, sizeof(szLogInfo) - 1, "epoll_wait failed! errno=%d, errmsg[%s]", errno, strerror(errno));
			WRITELOG(LOG_ERROR, szLogInfo);
			continue;
		}

		if (0 == nConnFds)
		{
			//超时5s
			continue;
		}
		for(i = 0; i < nConnFds; i++)
		{
			act_fd = events[i].data.fd;
			if (events[i].events & EPOLLERR)
			{
				epoll_ctl(g_epollfd, EPOLL_CTL_DEL, act_fd, &ev);
				close(act_fd);
				// 从hash表中删除数据
				delete_net_fd(act_fd);			
				curfds--;
				continue;
			}
			if( act_fd == listenfd ) //有新的连接
			{
				if(curfds > MAXEPOLLSIZE)
				{
					snprintf(szLogInfo, sizeof(szLogInfo) - 1, "too many connection, more than %d", MAXEPOLLSIZE);
					WRITELOG(LOG_ERROR, szLogInfo);
					continue;
				}
				connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &socklen);
				if(-1 == connfd)
				{
					snprintf(szLogInfo, sizeof(szLogInfo) - 1, "accept failed! errno=%d, errmsg[%s]", errno, strerror(errno));
					WRITELOG(LOG_ERROR, szLogInfo);
					continue;
				}
				
				ret = setnonblock(connfd);
				if(-1 == ret)
				{
					snprintf(szLogInfo, sizeof(szLogInfo) - 1, "setnonblock failed! connfd=%d", connfd);
					WRITELOG(LOG_ERROR, szLogInfo);
					close(connfd);
					continue;
				}
				
				ev.events = EPOLLIN | EPOLLET | EPOLLERR;
				ev.data.fd = connfd;
				ret = epoll_ctl(g_epollfd, EPOLL_CTL_ADD, connfd, &ev);
				if(-1 == ret)
				{
					snprintf(szLogInfo, sizeof(szLogInfo) - 1, "epoll_ctl: add failed! connfd=%d", connfd);
					WRITELOG(LOG_ERROR, szLogInfo);
					close(connfd);
					continue;
				}
	
				curfds++;
			}
			else if (events[i].events & EPOLLIN)
			{
				if (act_fd == g_pipe[0]) //有数据需要发送给集中器
				{
					ret = send_proc(act_fd, &curfds);
				}
				else	// 集中器有数据返回
				{
					ret = read_proc(act_fd);
					if(0 != ret)
					{
						snprintf(szLogInfo, sizeof(szLogInfo) - 1, "recvproc failed! ret=%d", ret);
						WRITELOG(LOG_ERROR, szLogInfo);
						if ( -1 == ret ) 
						{
							epoll_ctl(g_epollfd, EPOLL_CTL_DEL, act_fd, &ev);
							close(act_fd);
							// 从hash表中删除数据
							delete_net_fd(act_fd);					
							curfds--;
						}
					}
				}
			}
		}
	}
	close(g_epollfd);
	close(listenfd);
		

}


/********************************************************************
* 函数名:   ConnectServer
* 功能:     用来建立一个tcp客户端socket
* 参数说明：serverip    服务器IP地址或主机名
*           serverport  服务器端口
*           blockORnot  整数型是否阻塞
* 作者:     zhangwei
* 日期:     2017-04-06
*********************************************************************/
int ConnectServer(char *serverip, int serverport, int blockORnot)
{
    int    sockfd = 0;
    int    ret = 0;
    
    struct sockaddr_in    sin = { 0 };
    struct hostent *he = NULL;
    char   szLogInfo[1024] = { 0 };

	if (serverip == NULL)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "ConnectServer:serverip is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -4;
    }

    he = gethostbyname(serverip);
    if (NULL == he)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "gethostbyname failed! serverip[%s]", serverip);
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sockfd)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "socket create failed! errno=%d, errmsg[%s]", errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
        return -2;
    }

    ioctl(sockfd, FIONBIO, &blockORnot);  //block or not

    memset((char*)&sin, 0, sizeof(struct sockaddr_in));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(serverport);
    sin.sin_addr = *((struct in_addr *)he->h_addr);

    ret = connect(sockfd, (struct sockaddr *)&sin, sizeof(sin));
    if (-1 == ret)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "connect failed! errno=%d, errmsg[%s]", errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
        close(sockfd);
        return -3;
    }

    return sockfd;
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
int Send(int sock, char *buf, size_t size, int flag, int timeout)
{
    int    i = 0;
    int    ret = 0;
    int    intretry = 0;
    char   szLogInfo[1024] = { 0 };
    struct timeval tival = { 0 };
    fd_set writefds = { 0 };
    int maxfds = 0;

	if (buf == NULL)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Send:buf is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -5;
    }

    tival.tv_sec = timeout;
    tival.tv_usec = 0;

    FD_ZERO(&writefds);

    if(sock > 0)
    {
        FD_SET(sock, &writefds);
        maxfds=((sock > maxfds)?sock:maxfds);
    }
    else
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Send socket:%d error! return:-2", sock);
        WRITELOG(LOG_ERROR, szLogInfo);
        return -2;
    }

    ret = select(maxfds + 1, NULL, &writefds, NULL, &tival);
    if(ret <= 0)
    {
        if(ret < 0)
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Send socket:%d select() error! return:%d, errno=%d, errmsg[%s]", sock, ret, errno, strerror(errno));
            WRITELOG(LOG_ERROR, szLogInfo);
        }
        else
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Send socket:%d select timeout(%d)!", sock, timeout);
            WRITELOG(LOG_ERROR, szLogInfo);
        }
        close(sock);
        return -3;
    }
	
    if(!(FD_ISSET(sock, &writefds)))
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Send socket:%d not in writefds!", sock);
        WRITELOG(LOG_ERROR, szLogInfo);
        close(sock);
        return -4;
    }

    while(i < size)
    {
        ret = send(sock, buf + i, size - i, flag);
        if(ret <= 0)
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Send socket:%d send() error! return:%d, errno=%d, errmsg[%s]", sock, ret, errno, strerror(errno));
            WRITELOG(LOG_ERROR, szLogInfo);
            if (EINTR == errno || EAGAIN == errno)
            {
               if(intretry < 10)
               {
                   intretry++;
				   Sleep(10);
                   continue;
               }
               else
               {
                   snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Send socket:%d send() error!EINTR 10 times!", sock);
                   WRITELOG(LOG_ERROR, szLogInfo);
               }
            }
            close(sock);
            return -1;
        }
        else
        {
            i += ret;
        }
    }
    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Send socket:%d send() OK! %d/%d bytes sent!", sock, i, size);
    WRITELOG(LOG_INFO, szLogInfo);
    return i;
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
int Recv(int sock, char *buf, size_t size, int flag, int timeout)
{
    int i = 0, ret = 0, intretry = 0;
    char   szLogInfo[1024] = { 0 };
    struct timeval tival;
    fd_set readfds;
    int maxfds = 0;
	
	if (buf == NULL)
    {
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Recv:buf is NULL!");
		WRITELOG(LOG_ERROR, szLogInfo);
		return -5;
	}

    tival.tv_sec = timeout;
    tival.tv_usec = 0;

    FD_ZERO(&readfds);

    if(sock > 0)
    {
        FD_SET(sock, &readfds);
        maxfds=((sock > maxfds)?sock:maxfds);
    }
    else
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Recv socket:%d error! return:-2", sock);
        WRITELOG(LOG_ERROR, szLogInfo);
        return -2;
    }

    ret = select(maxfds + 1, &readfds, NULL, NULL, &tival);
    if (ret <= 0)
    {
        if(ret < 0)
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Recv socket:%d select() error! return:%d, errno=%d, errmsg[%s]", sock, ret, errno, strerror(errno));
            WRITELOG(LOG_ERROR, szLogInfo);
        }
        else
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Recv socket:%d select timeout(%d)!", sock, timeout);
            WRITELOG(LOG_ERROR, szLogInfo);
        }
        close(sock);
        return -3;
    }
    if(!(FD_ISSET(sock, &readfds)))
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Recv socket:%d not in readfds!", sock);
        WRITELOG(LOG_ERROR, szLogInfo);
        close(sock);
        return -4;
    }
	
    while(i < size)
    {
        ret = recv(sock, buf + i, size - i, flag);
        if(ret <= 0)
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Recv socket:%d recv() error! return:%d, errno=%d, errmsg[%s]", sock, ret, errno, strerror(errno));
            WRITELOG(LOG_ERROR, szLogInfo);
            if (errno == EINTR || errno == EAGAIN)
            {
                if(intretry < 10)
                {
                    intretry++;
					Sleep(10);
                    continue;
                }
                else
                {
                    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Recv socket:%d recv() error! EINTR 10 times!", sock);
					WRITELOG(LOG_ERROR, szLogInfo);
                }
            }
            close(sock);
            return -1;
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


//add by tianyu 设置超时机制
int msg_recv_nob(int msg_id, t_mymsg *msg, int size, long type)
{
	char szLogInfo[1024] = { 0 };
	int ret = 0;
	int cnt = 0;
	while(1)
	{
		ret = msgrcv(msg_id, msg, size, type, IPC_NOWAIT);
		if (ret < 0)
		{
			if ( errno == ENOMSG ) // current no msg
			{
				snprintf(szLogInfo, sizeof(szLogInfo) - 1, "*********msgrcv no msg! cnt[%d]", cnt);
        		WRITELOG(LOG_ERROR, szLogInfo);
				if (cnt >= 60)  //超时时间暂时设置为60s
				{
					snprintf(szLogInfo, sizeof(szLogInfo) - 1, "*********msgrcv timeout! cnt[%d]", cnt);
        			WRITELOG(LOG_ERROR, szLogInfo);
					return -2; 
				}
				sleep(1);  
				cnt++;
				continue;
			}
			snprintf(szLogInfo, sizeof(szLogInfo) - 1, "******** msgrcv failed! errno[%d], errmsg[%s]", errno, strerror(errno));
        	WRITELOG(LOG_ERROR, szLogInfo);
			ret = -1;
			break;
		}
		break;
	}
	return ret;
}




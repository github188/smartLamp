#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <signal.h>
#include "global.h"
#include "log.h"
#include "serv.h"
#include "envparams_proc.h"
#include "lamp_proc.h"
#include "link_chk.h"
#include "lockandsensors.h"
#include "lock_proc.h"
#include "music_proc.h"
//add by tianyu
#include <unistd.h>


t_service g_serv_func[] =
{
    {AFN_CONFIRM_DENY, FN_DI, terminal_linkchk_ack, NULL},
    
    {AFN_CTRL_CMD, FN_ONOFF_SWITCH, lamp_onoff_req, lamp_onoff_ack},
    {AFN_CTRL_CMD, FN_IMME_DIM, lamp_imme_dim_req, lamp_imme_dim_ack},
    {AFN_CTRL_CMD, FN_ONOFF_LOCK, parklock_onoff_req, parklock_onoff_ack},
    {AFN_CTRL_CMD, FN_ONOFF_LOCK_NEW, parklock_onoff_req, parklock_onoff_ack_new},
    {AFN_CTRL_CMD, FN_PLAYSTART_MUSIC, ent_playstart_req, NULL},
    {AFN_CTRL_CMD, FN_PLAYCTRL_MUSIC, ent_playctrl_req, ent_playctrl_ack},
    {AFN_CTRL_CMD, FN_SETVOLUME_MUSIC, ent_volumeset_req, ent_volumeset_ack},
    {AFN_CTRL_CMD, FN_GETVOLUME_MUSIC, ent_volumeget_req, ent_volumeget_ack},
    
    {AFN_TYPE1_DATA_REQ, FN_DIM_VAL, lamp_dim_val_req, lamp_dim_val_ack},
    {AFN_TYPE1_DATA_REQ, FN_SWITCH_IMME_INFO, lamp_switch_imme_info_req, lamp_switch_imme_info_ack},

    {AFN_TYPE1_DATA_REQ, FN_ENV_T, env_temperature_req, env_temperature_ack},
    {AFN_TYPE1_DATA_REQ, FN_ENV_H, env_humidity_req, env_humidity_ack},
    {AFN_TYPE1_DATA_REQ, FN_ENV_WP, env_windpower_req, env_windpower_ack},
    {AFN_TYPE1_DATA_REQ, FN_ENV_WP, env_winddirection_req, env_winddirection_ack},
    {AFN_TYPE1_DATA_REQ, FN_ENV_SR, env_sunshineradiation_req, env_sunshineradiation_ack},
    {AFN_TYPE1_DATA_REQ, FN_ENV_ALL, env_all_req, env_all_ack},
    
    {AFN_TYPE1_DATA_REQ, FN_LOCK_STATUS, parklock_status_req, parklock_status_ack},
    {AFN_TYPE1_DATA_REQ, FN_LOCK_STATUS_NEW, parklock_status_req, parklock_status_ack_new},
    {AFN_TYPE1_DATA_REQ, FN_PLAY_LIST, ent_playlist_req, ent_playlist_ack},
};

void *recv_entry(void *arg)//接收来自服务器端数据
{
    int nRecv     = 0;
    int iRet      = 0;
	int flag 	  = 0;
    t_mymsg    mymsg = { 0 };   //消息队列中被读取的数据；mymsg.mtext=length(4) + frame
    t_buf     *ptBbuf = NULL;   // 定义的一个数据缓存区
    char   szLogInfo[512] = { 0 };

    mymsg.mtype = 1;
    ptBbuf = (t_buf *)mymsg.mtext;
    while(1)
    {
    	//add by tianyu
    	memset(ptBbuf, 0, sizeof(mymsg.mtext));
		pthread_mutex_lock(&g_socket_mutex);
		if (-1 == g_sockfd)
			flag = 1;
		pthread_mutex_unlock(&g_socket_mutex);
		
        if(flag == 1)
        {
        	flag = 0;
            Sleep(1000);
            continue;
        }
		
        //nRecv = Recv(g_sockfd, ptBbuf->buf, MAX_BUF_LEN - sizeof(int), 0, 0);// 阻塞接收;
		nRecv = Recv_New(g_sockfd,ptBbuf->buf,MAX_BUF_LEN - sizeof(int),0,0);
		if(nRecv < 0)
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Recv failed! nRecv=%d", nRecv);
            WRITELOG(LOG_ERROR, szLogInfo);
            Sleep(1000);
            continue;
        }
        ptBbuf->len = nRecv;
        
        iRet = msgsnd(g_recv_msqid, &mymsg, nRecv + sizeof(int), IPC_NOWAIT);
        if(iRet < 0)
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "msgsnd failed! errno=%d, errmsg[%s]", errno, strerror(errno));
            WRITELOG(LOG_ERROR, szLogInfo);
            //Sleep(1000);
            continue;
        }
    }
    
    return NULL;
}

void *send_entry(void *arg)//向服务器端发送数据
{
    int nRet     = 0;
	int flag 	 = 0;
    t_mymsg mymsg = { 0 };//消息队列中被读取的数据；mymsg.mtext=length + frame
    t_buf *ptBuf  = NULL; // 定义的一个数据缓存区
    char   szLogInfo[512] = { 0 };
    
    while(1)
    {
        nRet = msgrcv(g_send_msqid, &mymsg, MAX_BUF_LEN, 0, 0); // 阻塞发送;从消息队列中读取数据，为了发送该数据
        if(nRet < 0)
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "msgrcv failed! errno=%d, errmsg[%s]", errno, strerror(errno));
            WRITELOG(LOG_ERROR, szLogInfo);
            Sleep(1000);
            continue;
        }
        printf("recv from send msgq: %d\n", nRet);

        ptBuf = (t_buf *)mymsg.mtext;//ptBuf 指向要发送的数据
                                    //ptBuf->len:帧的长度ptBuf->buf:帧的内容
                                    
        //printf("In the send_entry [mymsg address]=0x%x,[mymsg.mtext]=0x%x,[ptBuf]=0x%x",&mymsg,&mymsg.mtext,ptBuf);
        //snprintf(szLogInfo, sizeof(szLogInfo) - 1, "In the send_entry [mymsg address]=0x%x,[mymsg.mtext]=0x%x,[ptBuf]=0x%x",&mymsg,&mymsg.mtext,ptBuf);
        //WRITELOG(LOG_ERROR, szLogInfo);

        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "The below information is send_entry function[109 line]");
        WRITELOG(LOG_INFO, szLogInfo);
		dumpInfo(ptBuf->buf, ptBuf->len);
		
        //nRet = Send(g_sockfd, ptBuf->buf, ptBuf->len + 1, 1);//发送数据

		//add by tianyu
		pthread_mutex_lock(&g_socket_mutex);
		if ( g_sockfd == -1 )
			flag = 1;
		pthread_mutex_unlock(&g_socket_mutex);
		
        if (flag == 1)
        {
        	flag = 0;
			Sleep(1000);
			continue;	//丢弃当前消息
		}
		
		nRet = Send_New(g_sockfd, ptBuf->buf, ptBuf->len + 1, 0, 0);
		if(nRet < 0)
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Send failed! nRet=%d", nRet);
            WRITELOG(LOG_ERROR, szLogInfo);
            //Sleep(1000);
            continue;
        }
		printf("ptBuf->len=%d\n", ptBuf->len);
    }
    
    return NULL;
}

void *service_entry(void *arg)
{
    int i = 0;
    int nRet = 0;
    t_mymsg mymsg = { 0 };
    t_buf  *ptBuf = NULL;
    //t_buf   buf = { 0 };
    t_3761frame frame = { 0 };
    t_3761frame rsp_frame = {0};
    char   szLogInfo[512] = { 0 };

	struct msqid_ds m_ds;
	int index = (int)arg; 

    printf("i= %p; nRet = %p; mymsg = %p; mymsg.mtext= %p; ptBuf = %p",&i,&nRet,&mymsg,mymsg.mtext,&ptBuf);
    while(1)
    {
        nRet = msgrcv(g_recv_msqid, &mymsg, MAX_BUF_LEN, 0, 0); // 阻塞接收
        if(nRet < 0)
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "msgrcv failed! errno=%d, errmsg[%s]", errno, strerror(errno));
            WRITELOG(LOG_ERROR, szLogInfo);
            Sleep(1000);
            continue;
        }

		memset(&m_ds, 0, sizeof(m_ds));
		msgctl(g_recv_msqid, IPC_STAT, &m_ds);
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "func: service_entry. after msgrcv, msg_number[%u] cur_bytes[%u] index[%d] thread_id[%lu]", 
											m_ds.msg_qnum, m_ds.msg_cbytes, index, pthread_self());
		WRITELOG(LOG_ERROR, szLogInfo);
        
        ptBuf = (t_buf *)mymsg.mtext;
        printf("ptBuf = 0x%x\n", ptBuf);
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "msgrcv success.");
        WRITELOG(LOG_INFO, szLogInfo);
        printf("%s\n", szLogInfo);

        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "The below information is service_entry function[157 line]");
        WRITELOG(LOG_INFO, szLogInfo);
        //dumpInfo(ptBuf->buf, ptBuf->len);

        // 解析3761帧
        nRet = parse_3761frame(ptBuf->buf, ptBuf->len, &frame);
        if(nRet <= 0)
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "parse_3761frame failed!");
            WRITELOG(LOG_ERROR, szLogInfo);
            Sleep(1000);
            continue;
        }
        
        // 业务逻辑处理
        for(i = 0; i < sizeof(g_serv_func) / sizeof(g_serv_func[0]); i++)
        {
            printf("ZW: i=%d num=%d\n", i, sizeof(g_serv_func) / sizeof(g_serv_func[0]));
            if((frame.user_data.afn == g_serv_func[i].afn) 
               && (frame.user_data.fn[0] == (1 << ((g_serv_func[i].fn - 1) % 8)))
               && (frame.user_data.fn[1] == (g_serv_func[i].fn - 1) / 8))
            {
                if(NULL != g_serv_func[i].serv_proc_req)
                {
                    //printf("ZW: Enter g_serv_func[i].serv_proc_req\n");
                    //printf("GC: Enter g_serv_func[%d].serv_proc_req\n",i);
                    nRet = g_serv_func[i].serv_proc_req(&frame, ptBuf);
                    if(nRet != 0)
                    {
                        
                        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_proc_req[%d].serv_proc_func failed!", i);
                        WRITELOG(LOG_ERROR, szLogInfo);
                        printf("szLogInfo = %s\n",szLogInfo);
                        Sleep(1000);
                        //continue;
                        //modify by tianyu
                        break;
                    }
                }
                
                //printf("GC: g_serv_func[%d].serv_proc_ack = 0x%x\n",i,g_serv_func[i].serv_proc_ack);
                if(NULL != g_serv_func[i].serv_proc_ack)
                {
                    nRet = g_serv_func[i].serv_proc_ack(ptBuf, &frame);
                    //printf("GC: 1 ptBuf = 0x%x\n",ptBuf);
                    //printf("GC: 1 ptBuf = 0x%x\n",&ptBuf);
                    //printf("GC: Enter g_serv_func[%d].serv_proc_ack\n",i);
                    if(nRet != 0)
                    {
                        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_proc_ack[%d].serv_proc_func failed!", i);
                        WRITELOG(LOG_ERROR, szLogInfo);
                        Sleep(1000);
                        //continue;
                         //modify by tianyu
                        break;
                    }

                    nRet = pack_3761frame(&frame, ptBuf->buf, MAX_BUF_LEN);//nRet:为帧的长度                                                                           

                    if(nRet <= 0)
                    {
                        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "pack_3761frame failed!");
                        WRITELOG(LOG_ERROR, szLogInfo);
                        Sleep(1000);
                        //continue;
						//modify by tianyu
                        break;
                    }
                    ptBuf->len = nRet;//把帧的长度写入帧中
                                     //(所以消息队列中的帧数据是帧长度+帧数据)
                                     //也就是t_mymsg结构体的mtext部分

                    printf("GC: Enter msgsnd\n");
                    printf("GC: In service_entry ptBuf->len = %d\n",ptBuf->len);
                   // printf("GC: In service_entry ptBuf->buf = %s\n",ptBuf->buf);
                    
                    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "The below information is in serv.c [232 line]");
                    WRITELOG(LOG_INFO, szLogInfo);
	                //dumpInfo(mymsg.mtext, ptBuf->len + sizeof(int));
	                
	                //memcpy(mymsg.mtext, &ptBuf, MAXLINE);  //GC added it
	                #if 0
                    nRet = msgsnd(g_send_msqid, &mymsg, ptBuf->len+sizeof(int), IPC_NOWAIT);
                    printf("GC: Enter msgsnd = %d;mymsg.mtype = %d................\n",i,mymsg.mtype);
                    if(nRet < 0)
                    {
                        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "msgsnd failed! errno=%d, errmsg[%s]", errno, strerror(errno));
                        WRITELOG(LOG_ERROR, szLogInfo);
                        Sleep(1000);
                        //continue;
                        //modify by tianyu
                        break;
                    }
					#else
					pthread_mutex_lock(&g_pipe_mutex);
					nRet = write_safe(g_pipe[1],&mymsg,sizeof(t_mymsg));
					pthread_mutex_unlock(&g_pipe_mutex);
					if (nRet == -1)
					{
						// TODO: write pipe error. 异常情况处理
						snprintf(szLogInfo, sizeof(szLogInfo) - 1, "write pipe failed! errno=%d, errmsg[%s]", errno, strerror(errno));
                        WRITELOG(LOG_ERROR, szLogInfo);
						sleep(1);
						break;
					}
					#endif
                }
				break;
            }
        }
    }
    
    return NULL;
}

//add by tianyu
void * io_proc(void * arg)
{
	
	struct timeval tv;
	int read_fd = g_pipe[0];
	int maxfd = 0;
	int ret;
	fd_set read_fds;
	char   szLogInfo[512] = { 0 };
	t_mymsg m_msg = {0};
	t_buf *ptBuf  = NULL; 
	ptBuf = (t_buf *)m_msg.mtext;
	
	while(1)
	{
		// 与服务器建立连接:g_sockfd
		g_sockfd = ConnectServer(g_acServerIp, g_usServerPort, 0);
		if (g_sockfd <= 0)
		{
			snprintf(szLogInfo, sizeof(szLogInfo) - 1, "create socket errno=%d, errmsg[%s]", errno, strerror(errno));
        	WRITELOG(LOG_ERROR, szLogInfo);
			sleep(5);
			continue;
		}
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "connected server. fd[%d]", g_sockfd);
        WRITELOG(LOG_INFO, szLogInfo);
		
		ret = setnonblock(g_sockfd);
		if (ret == -1)
		{
			snprintf(szLogInfo, sizeof(szLogInfo) - 1, "set socket nonblock failed! errno=%d, errmsg[%s]", errno, strerror(errno));
        	WRITELOG(LOG_ERROR, szLogInfo);
			close(g_sockfd);
			g_sockfd = -1;
			sleep(5);
			continue;
		}
		//发送登录请求
		ret = terminal_send_login_req(g_sockfd);
		if (ret == -1)
		{
			snprintf(szLogInfo, sizeof(szLogInfo) - 1, "terminal_send_login_req failed! errno=%d, errmsg[%s]", errno, strerror(errno));
        	WRITELOG(LOG_ERROR, szLogInfo);
			close(g_sockfd);
			g_sockfd = -1;
			sleep(1);
			continue;
		}
		
		maxfd = (read_fd > g_sockfd)? read_fd : g_sockfd;
		
		do
		{
			FD_ZERO(&read_fds);
			FD_SET(read_fd, &read_fds);
			FD_SET(g_sockfd, &read_fds);
			tv.tv_sec = 5;
			tv.tv_usec = 0;
			ret = select(maxfd+1, &read_fds, NULL, NULL, &tv);
			if (ret ==0 )
			{
				snprintf(szLogInfo, sizeof(szLogInfo) - 1, "select timeout errno=%d, errmsg[%s]", errno, strerror(errno));
        		WRITELOG(LOG_ERROR, szLogInfo);
				continue;
			}
			if (ret == -1)
			{
				snprintf(szLogInfo, sizeof(szLogInfo) - 1, "create select failed! errno=%d, errmsg[%s]", errno, strerror(errno));
        		WRITELOG(LOG_ERROR, szLogInfo);
				close(g_sockfd);
				g_sockfd = -1;
				sleep(1);
				break;
			}
			if (FD_ISSET(read_fd, &read_fds))	//pipe有数据，需发送
			{
				memset(&m_msg, 0, sizeof(t_mymsg));
				ret = read_safe(read_fd,(char *)&m_msg, sizeof(t_mymsg));
				if (ret == -1)
				{
					snprintf(szLogInfo, sizeof(szLogInfo) - 1, "read pipe errno=%d, errmsg[%s]", errno, strerror(errno));
        			WRITELOG(LOG_ERROR, szLogInfo);
					// TODO: pipe read error 异常处理
					sleep(1);
					continue;
				}
				// TODO: 判断是否是重连消息.
				if (m_msg.mtype == SMART_LAMP_LOGIN)
				{
					snprintf(szLogInfo, sizeof(szLogInfo) - 1, "recv longin req: start to reconnect");
        			WRITELOG(LOG_INFO, szLogInfo);
					close(g_sockfd);
					g_sockfd = -1;
					sleep(1);
					break;
				}
				ret = write_safe(g_sockfd, ptBuf->buf, ptBuf->len);
				if (ret == -1)
				{
					snprintf(szLogInfo, sizeof(szLogInfo) - 1, "socket write msq failed! errno=%d, errmsg[%s]", errno, strerror(errno));
        			WRITELOG(LOG_ERROR, szLogInfo);
					close(g_sockfd);
					g_sockfd = -1;
					sleep(1);
					break;
				}
			}
			if (FD_ISSET(g_sockfd, &read_fds)) //服务端有数据到达
			{
				memset(&m_msg, 0, sizeof(t_mymsg));
				ret = read_safe(g_sockfd, ptBuf->buf, MAX_BUF_LEN - sizeof(int));
				/*
				snprintf(szLogInfo, sizeof(szLogInfo) - 1, "aaa: finish read socket: ret[%d] do not sleep----", ret);
        		      WRITELOG(LOG_INFO, szLogInfo);
				*/
				if (ret == -1)
				{
					snprintf(szLogInfo, sizeof(szLogInfo) - 1, "socket read msq failed! errno=%d, errmsg[%s]", errno, strerror(errno));
        			WRITELOG(LOG_ERROR, szLogInfo);
					close(g_sockfd);
					g_sockfd = -1;
					sleep(1);
					break;
				}
				ptBuf->len = ret;
				m_msg.mtype = 1;
				//test
				//sleep(3);
				ret = msgsnd(g_recv_msqid, &m_msg, ret + sizeof(int), IPC_NOWAIT);
        		if(ret < 0)
       			{
            		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "msgsnd msg queue failed! errno=%d, errmsg[%s]", errno, strerror(errno));
            		WRITELOG(LOG_ERROR, szLogInfo);
            		sleep(1);
            		continue;
        		}
			}
			
		}while(g_sockfd > 0);
	}
	
	
}

void *eviroment_collect(void *arg)
{
time_t last_time=0;
time_t current_time=0;
char   szLogInfo[512] = { 0 };
while(1)
{
	current_time=time(NULL);

	if(current_time-last_time>5&&g_EviromentSensorsOrNot>0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "begin  closeBuzzer");
        WRITELOG(LOG_INFO, szLogInfo);
		CloseBuzzer();
 		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "finish closeBuzzer");
        WRITELOG(LOG_INFO, szLogInfo);
    	sleep(1);
		g_iRetVal = init_serial(); 
		Sleep(5);
        Get_All(&_t_get_all_envirparam_ack);
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "close  g_serial_fd[%d]", g_serial_fd);
        WRITELOG(LOG_INFO, szLogInfo);
		close(g_serial_fd);
		g_serial_fd = -1;
		last_time=current_time;
	}
	else
	{
		Sleep(1000);
		continue;
	}	
}
return NULL;
}




int main(int argc, char *argv[])
{
    int    iRet  = 0;
    int    index = 0;
    char   szLogInfo[512] = { 0 };
    time_t  old_time = 0;
    time_t  new_time = 0;

   StartLog("smartlamppost.ini", "smartlamppost.log");
  //loseBuzzer();
   /* // 连接主站服务器
    g_sockfd = ConnectServer(g_acServerIp, g_usServerPort, 0);
    if(g_sockfd < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "ConnectServer failed! g_sockfd=%d", g_sockfd);
        WRITELOG(LOG_ERROR, szLogInfo);
	 printf("%s\n", szLogInfo);
        exit(g_sockfd);
    }

    printf("connect to %s, %d, sockfd=%d\n", g_acServerIp, g_usServerPort, g_sockfd);
*/
	//add by tianyu
	signal(SIGPIPE, SIG_IGN);
    pthread_mutex_init(&g_socket_mutex, NULL); 
	pthread_mutex_init(&g_music_mutex, NULL);
	pthread_mutex_init(&g_pipe_mutex, NULL);
    
    // 创建socket接收消息队列
    g_recv_msqid = msgget(SOCK_RECV_MSGKEY, IPC_CREAT | 0666);//创建一个队列
    if(g_recv_msqid < 0)
    { 
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "create socket recv msq failed! errno=%d, errmsg[%s]", errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
        exit(-1);
    }
#if 0	
    // 创建socket接收线程
    iRet = pthread_create(&g_recvtid, NULL, recv_entry, (void*)NULL);
    if(0 != iRet)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "create socket recv thread failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        Sleep(1000);
        exit(iRet);
    }

    // 创建socket发送消息队列
    g_send_msqid = msgget(SOCK_SEND_MSGKEY, IPC_CREAT | 0666);//创建一个队列
    if(g_send_msqid < 0)
    { 
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "create socket send msq failed! errno=%d, errmsg[%s]", errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
        Sleep(1000);
        exit(-1);
    }
    // 创建socket发送线程
    iRet = pthread_create(&g_sendtid, NULL, send_entry, (void*)NULL);
    if(0 != iRet)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "create socket send thread failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        Sleep(1000);
        exit(iRet);
    }
#else
//modify by tianyu
	iRet = pipe(g_pipe);
	if (iRet == -1)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "create pipe failed! iRet=%d, quit...", iRet);
        WRITELOG(LOG_ERROR, szLogInfo);
		sleep(1);
		exit(iRet);
	}
	// 创建socket发送线程
    iRet = pthread_create(&g_sendtid, NULL, io_proc, (void*)NULL);
    if(0 != iRet)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "create socket io thread failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        Sleep(1000);
        exit(iRet);
    }
#endif

    // 创建业务线程
    for(index = 0; index < g_uiServThreadNum; index++)
    {
        iRet = pthread_create(&g_aServTid[index], NULL, service_entry, (void*)(long)index);
        if(0 != iRet)
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "create service thread failed! index=%d, iRet=%d, quit...", index, iRet);
            WRITELOG(LOG_ERROR, szLogInfo);
            Sleep(1000);
            exit(iRet);
        }
    }


		// 创建环境参数采集线程
	   iRet = pthread_create(&g_eviromentid, NULL, eviroment_collect, (void*)NULL);

	//add by tianyu
	old_time = time(NULL);
	
    while(1)
    {
    	//modify by tianyu: 由于登录操作交由socket线程，防止初始去重新登录
        new_time = time(NULL);
		//old_time = new_time = time(NULL);
        
        // 与服务器建立连接并登录服务器
        if((0 == g_uiLoginSuccess) && (new_time - old_time > g_uiLoginCycle))
        {
            iRet = terminal_login_req();
			g_uiHeartbeatNoAckCnt=0;
			//add by tianyu: 延迟重新登录时间, 10 次后恢复初始值
			if (g_uiLoginNoAckCnt++ > 10)
				g_uiLoginNoAckCnt = 1;
			g_uiLoginCycle = (g_uiLoginNoAckCnt+1)*10;
            if(iRet != 0)
            {
                snprintf(szLogInfo, sizeof(szLogInfo) - 1, "send login request failed! iRet=%d", iRet);
                WRITELOG(LOG_ERROR, szLogInfo);
                printf("%s\n", szLogInfo);
            }
			
            old_time = new_time;
        }
        
        // 发心跳消息给服务器
        if((1 == g_uiLoginSuccess)
           && (new_time - old_time > g_uiHeartbeatCycle)
           && (g_uiHeartbeatNoAckCnt < 3))
        {
    	    iRet = terminal_heartbeat_req();
    	    if(iRet != 0)
            {
                snprintf(szLogInfo, sizeof(szLogInfo) - 1, "send heartbeat request failed! iRet=%d", iRet);
                WRITELOG(LOG_ERROR, szLogInfo);
                printf("%s\n", szLogInfo);
            }

            old_time = new_time;
            g_uiHeartbeatNoAckCnt++;
    	}
    	
    	// 重连主站服务器
    	else if((1 == g_uiLoginSuccess) && (new_time - old_time > g_uiHeartbeatCycle))
    	{
    	    g_uiLoginSuccess = 0;
    	}
    	
        Sleep(1000);
    }
	
	//add by tianyu
	pthread_mutex_destroy(&g_pipe_mutex);
	pthread_mutex_destroy(&g_music_mutex);
	pthread_mutex_destroy(&g_socket_mutex);  
	
    return 0;
}


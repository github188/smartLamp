#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <time.h>
#include "log.h"
#include "frame.h"
#include "global.h"
#include "serv.h"
#include "link_chk.h"
#include "lockandsensors.h"


int parse_diconfirmdeny(char *data, int len, t_di_confirm_deny_list *di_confirm_deny_list)
{
    int i = 0;
    int num = 0;

    if (data == NULL || di_confirm_deny_list == NULL)
    {
        return -1;
    }
	
    num = len / sizeof(t_di_confirm_deny);
    for(i = 0; i < num; i++)
    {
        memcpy(di_confirm_deny_list->confirm_deny[i].di, data + i * sizeof(t_di_confirm_deny), 4);
        memcpy(di_confirm_deny_list->confirm_deny[i].dev_no, data + i * sizeof(t_di_confirm_deny) + 4, 2);
        memcpy(&di_confirm_deny_list->confirm_deny[i].err, data + i * sizeof(t_di_confirm_deny) + 6, 1);
    }
    di_confirm_deny_list->num = num;

    return 0;
}

int pack_diconfirmdeny(t_di_confirm_deny_list *di_confirm_deny_list, char *data, int len)
{
    int i = 0;

    if ((data == NULL)
        || (di_confirm_deny_list == NULL)
        || (len < (di_confirm_deny_list->num * sizeof(t_di_confirm_deny))))
    {
        return -1;
    }
    
    for(i = 0; i < di_confirm_deny_list->num; i++)
    {
        memcpy(data + i * sizeof(t_di_confirm_deny), di_confirm_deny_list->confirm_deny[i].di, 4);
        memcpy(data + i * sizeof(t_di_confirm_deny) + 4, di_confirm_deny_list->confirm_deny[i].dev_no, 2);
        memcpy(data + i * sizeof(t_di_confirm_deny) + 6, &di_confirm_deny_list->confirm_deny[i].err, 1);
    }

    return di_confirm_deny_list->num * sizeof(t_di_confirm_deny);
}

//add by tianyu
int terminal_send_login_req(int fd)
{	
	int iRet =0, buf_len = 0;
	char send_buf[MAX_BUF_LEN] = {0};
	t_3761frame frame= {0};
	char   szLogInfo[512] = { 0 };
	memcpy(frame.addr, collector_id, MAX_ADDR_LEN);
    frame.user_data.afn = AFN_LINK_CHK;
    frame.user_data.fn[0] = 1 << ((FN_LOGIN- 1) % 8);
    frame.user_data.fn[1] = (FN_LOGIN - 1) / 8;
    frame.user_data.len = 0;
   // memcpy(frame.user_data.data, buf->buf, buf->len);
	
	buf_len = pack_3761frame(&frame, send_buf, MAX_BUF_LEN);
    if(buf_len <= 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "login require pack_3761frame failed! iRet=%d", buf_len);
        WRITELOG(LOG_ERROR, szLogInfo);
   		return -1;
    }
	iRet = write_safe(fd, send_buf, buf_len);
	if (iRet == -1)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "terminal_send_login_req: write  socket  failed! errno[%d] errmsg[%s]", errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
   		return -1;
	}
	
	return 0;
}


int terminal_login_req()
{
	int iRet =0;
	t_3761frame frame= {0};
	//char data[MAX_BUF_LEN]={0};
	t_mymsg mymsg = { 0 };
    t_buf  *ptBuf = NULL;
	//t_buf buf= {0};
	char   szLogInfo[512] = { 0 };

    printf("Enter to terminal_login_req...\n");
#if 0	
	//add by tianyu
	pthread_mutex_lock(&g_socket_mutex);
	
    if(g_sockfd > 0)
    {
        //close(g_sockfd);
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "terminal_login_req: *******begin close socket: socket[%d]*******",
        												g_sockfd);
        WRITELOG(LOG_ERROR, szLogInfo);
        shutdown(g_sockfd, SHUT_RDWR);
        g_sockfd = -1;
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "terminal_login_req: *******finish close socket*******");
        WRITELOG(LOG_ERROR, szLogInfo);
    }
    g_sockfd = ConnectServer(g_acServerIp, g_usServerPort, 0);

	//add by tianyu
	pthread_mutex_unlock(&g_socket_mutex);
	
    if(g_sockfd < 0)
    {	
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "ConnectServer failed! g_sockfd=%d", g_sockfd);
        WRITELOG(LOG_ERROR, szLogInfo);
        printf("%s\n", szLogInfo);
        return -1;
    }

	 mymsg.mtype = 1;

	 ptBuf = (t_buf *)(mymsg.mtext);
	
	memcpy(frame.addr, collector_id, MAX_ADDR_LEN);
    frame.user_data.afn = AFN_LINK_CHK;
    frame.user_data.fn[0] = 1 << ((FN_LOGIN- 1) % 8);
    frame.user_data.fn[1] = (FN_LOGIN - 1) / 8;
    frame.user_data.len = 0;
   // memcpy(frame.user_data.data, buf->buf, buf->len);
	
	iRet = pack_3761frame(&frame, ptBuf->buf, MAX_BUF_LEN);
    if(iRet <= 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "login require pack_3761frame failed! iRet=%d", iRet);
        WRITELOG(LOG_ERROR, szLogInfo);
		printf("%s\n", szLogInfo);
   		return -2;
    }
	
	ptBuf->len= iRet;

	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "The below information is in terminal_login_req function[102 line]");
    WRITELOG(LOG_INFO, szLogInfo);
	dumpInfo(mymsg.mtext, ptBuf->len + sizeof(int));
	
#else
	//modify by tianyu 登录服务器由socket线程, 设置登录消息类型

	mymsg.mtype = SMART_LAMP_LOGIN;
	
#endif
			
	
#if 0
	iRet = msgsnd(g_send_msqid, &mymsg, ptBuf->len + sizeof(int), IPC_NOWAIT);
                    if(iRet < 0)
                    {
                        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "msgsnd failed! msqid=%d, len=%d,errno=%d, errmsg[%s]", g_send_msqid, ptBuf->len, errno, strerror(errno));
                        WRITELOG(LOG_ERROR, szLogInfo);
			            printf("%s\n", szLogInfo);
			            return -3;
                    }
#else
	pthread_mutex_lock(&g_pipe_mutex);
	iRet = write_safe(g_pipe[1],&mymsg,sizeof(t_mymsg));
	pthread_mutex_unlock(&g_pipe_mutex);
	if (iRet == -1)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "write pipe failed! errno=%d, errmsg[%s]", errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
		return -3;
	}
#endif		
	return 0;
}

int terminal_linkchk_ack(t_3761frame *frame, t_buf *buf)
{
    int iRet =0;
    char   szLogInfo[512] = { 0 };
    t_di_confirm_deny_list login_ack_di_confirm_deny_list = { 0 };
    const char login_di[4] = {0x00, 0x00, 0x00, 0x01};
    const char heartbear_di[4] = {0x00, 0x00, 0x00, 0x04};

    //printf("Enter terminal_linkchk_ack...\n");
    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "The below information is terminal_linkchk_ack function[127 line]");
    WRITELOG(LOG_INFO, szLogInfo);
    //dumpInfo(frame->user_data.data, frame->user_data.len);
    
    iRet = parse_diconfirmdeny(frame->user_data.data,frame->user_data.len, &login_ack_di_confirm_deny_list);
    if(iRet < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Parse confirm deny-failed in the terminal link check ack!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
    
    printf("Begin to memcmp...\n");
    
    if(0 == memcmp(login_ack_di_confirm_deny_list.confirm_deny[0].di, login_di, 4))
    {
        terminal_login_ack(&login_ack_di_confirm_deny_list);
    }
    else if(0 == memcmp(login_ack_di_confirm_deny_list.confirm_deny[0].di, heartbear_di, 4))
    {
        terminal_heartbeat_ack(&login_ack_di_confirm_deny_list);
    }
    else
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Recv unknown confirm-deny DI in the terminal link check ack!");
        WRITELOG(LOG_ERROR, szLogInfo);
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "The below information is terminal_linkchk_ack function[153 line]");
        WRITELOG(LOG_INFO, szLogInfo);
        //dumpInfo(login_ack_di_confirm_deny_list.confirm_deny[0].di, 4);
        return -1;
    }

    return 0;    
}

int terminal_login_ack(t_di_confirm_deny_list *pdi_confirm_deny_list)
{
	int iRet =0;
	char   szLogInfo[512] = { 0 };
    //time_t  cur_time = 0;
	//t_di_confirm_deny_list login_ack_di_confirm_deny_list;
	//parse_diconfirmdeny(char *data, int len, t_di_confirm_deny_list *di_confirm_deny_list)

	//iRet = parse_diconfirmdeny(frame->user_data.data,frame->user_data.len, &login_ack_di_confirm_deny_list);
	//if(iRet < 0)
    //                {
    //                    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Parse confirm deny  failed in the terminal login ack!");
    //                    WRITELOG(LOG_ERROR, szLogInfo);
	//		return -1;
    //                }

	printf("Enter to terminal_login_ack...\n");
	
	if(pdi_confirm_deny_list->confirm_deny[0].err == 0 || pdi_confirm_deny_list->confirm_deny[0].err == 2)
	{
		g_uiLoginSuccess = 1;
		g_uiLoginNoAckCnt = 0;
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Login is success!");
        WRITELOG(LOG_INFO, szLogInfo);
		if (pdi_confirm_deny_list->confirm_deny[0].err == 2)
		{
			/*********************************************
			发送音乐列表给服务端
			*********************************************/
			ent_playlist_ack_db();
		}
	}
	else
	{
		g_uiLoginSuccess = 0;
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Login is fail!");
        WRITELOG(LOG_INFO, szLogInfo);
	}
	
    return 0;
}

// 心跳请求包含发送所有环境参数
int terminal_heartbeat_req()
{
    int iRet =0;
	t_3761frame frame= {0};
	t_mymsg mymsg = { 0 };
    t_buf  *ptBuf = NULL;
	char   szLogInfo[512] = { 0 };



    //环境参数获取
    //    memset(buf,0,sizeof(t_buf));
      //  CloseBuzzer();
        //OpenBuzzer();
      /*  if (g_EviromentSensorsOrNot>0)
        {Get_All(&_t_get_all_envirparam_ack);
		g_EviromentSensorsOrNot=3;
		
		}
		*/
			 
    printf("Enter to terminal_heartbeat_req...\n");
    mymsg.mtype = 1;
	ptBuf = (t_buf *)(mymsg.mtext);
	
	memcpy(frame.addr, collector_id, MAX_ADDR_LEN);
    frame.user_data.afn = AFN_LINK_CHK;
    frame.user_data.fn[0] = 1 << ((FN_HEARTBEAT- 1) % 8);
    frame.user_data.fn[1] = (FN_HEARTBEAT - 1) / 8;


	if (_t_get_all_envirparam_ack.retcode==-1)  {
      g_EviromentSensorsOrNot=g_EviromentSensorsOrNot-1;
	  frame.user_data.len = 0;
      printf("Enter to _t_get_all_envirparam_ack=-1...\n");}
   else
   	{g_EviromentSensorsOrNot=20;
   	frame.user_data.len= sizeof(_t_get_all_envirparam_ack);
   memcpy(frame.user_data.data, &_t_get_all_envirparam_ack,frame.user_data.len);
	}
  
	
	iRet = pack_3761frame(&frame, ptBuf->buf, MAX_BUF_LEN);
    if(iRet <= 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "heartbeat require pack_3761frame failed! iRet=%d", iRet);
        WRITELOG(LOG_ERROR, szLogInfo);
        printf("%s\n", szLogInfo);
        return -1;
    }
	
	ptBuf->len= iRet;

	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "The below information is in terminal_heartbeat_req function[221 line]");
    WRITELOG(LOG_INFO, szLogInfo);
	//dumpInfo(mymsg.mtext, ptBuf->len + sizeof(int));
#if 0
	iRet = msgsnd(g_send_msqid, &mymsg, ptBuf->len + sizeof(int), IPC_NOWAIT);
    if(iRet < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "msgsnd failed! msqid=%d, len=%d,errno=%d, errmsg[%s]", g_send_msqid, ptBuf->len, errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
        printf("%s\n", szLogInfo);
        return -2;
    }
#else
	pthread_mutex_lock(&g_pipe_mutex);
	iRet = write_safe(g_pipe[1],&mymsg,sizeof(t_mymsg));
	pthread_mutex_unlock(&g_pipe_mutex);
	if (iRet == -1)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "write pipe failed! errno=%d, errmsg[%s]", errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
		return -2;
	}
#endif	
                    
    return 0;
}
int terminal_heartbeat_ack(t_di_confirm_deny_list *pdi_confirm_deny_list)
{
    int iRet =0;
	char   szLogInfo[512] = { 0 };

	printf("Enter to terminal_heartbeat_ack...\n");
	g_uiHeartbeatNoAckCnt = 0;
	if(pdi_confirm_deny_list->confirm_deny[0].err==0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Heartbeat is success!");
        WRITELOG(LOG_INFO, szLogInfo);
	}
	else
	{
		g_uiLoginSuccess = 0;
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Heartbeat is fail!");
        WRITELOG(LOG_INFO, szLogInfo);
	}
	
    return 0;
}



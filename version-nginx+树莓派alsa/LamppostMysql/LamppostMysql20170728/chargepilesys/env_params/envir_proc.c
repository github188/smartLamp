/*****************************************************************************
Copyright: 2017-2026, Chongqing Optical&Electronical Information Institute.
File name: music_proc.c
Description: control to play music etc.
Author: zhangwei
Version: v1.0.0
Date: 2017-05-09
History: 
-----------------------------------------
date          author        description
-----------------------------------------
2017-05-09    zhangwei      created

*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "gdmysql.h"
#include "log.h"
#include "cJSON.h"
#include "frame.h"
#include "netinfo.h"
#include "netinf.h"
#include "global.h"
#include "jsondata.h"
#include "servdata.h"
#include "upserv.h"
#include "envir_proc.h"
#include "uuid.h"
#include "md5.h"
#include "config.h"

extern Config_T g_Conf;

/*************************************************
Function: serv_get_envir_param
Description: 获取指定的环境参数，例如温度，湿度，照度，风向，风速等等某一项
Calls: 
Called By: down_service_entry
Table Accessed: 
Table Updated:  
Input:  index--线程索引号
        in --前端发来的json格式的字符串请求数据
        len--out缓存长度
Output: out--发送给前端的json格式的字符串响应数据
Return: 0--success; 其它--failure
Others: 无
*************************************************/
int serv_get_envir_param(int serial, const char *in, unsigned char *out, int len)
{    
	int ret 			=	0;
	t_getenvirparam_req envir_param_req = { 0 };
	char szLogInfo[1024] = { 0 };
	t_3761frame frame	 = { 0 };
	t_net_node *netnode  = NULL;
	t_mymsg     mymsg	 = { 0 };
	int nRecv			 =	0;		
	t_buf *pbuf 		 = NULL;
	int    nReqType      = -1;
	t_msg_header header = {0};
	if (in == NULL || out == NULL)
	{
	    snprintf(szLogInfo, sizeof(szLogInfo)-1, "serv_get_envir_param: input parameter(s) is NULL!");
		WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_get_envir_param: input parameter(s) is NULL!");
		return -1;
	}
	
	// json解析
	
	ret = demux_getenvparam_req(in, &envir_param_req);
	if(MUXDEMUX_ERR == ret)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_get_envir_param: demux_getenvparam_req failed!");
	    WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_get_envir_param: demux_getenvparam_req failed!");
		return -1;
	}
/*
	//打印envparam_req 结构体 的内容
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, " in serv_get_envir_param envir_param_req adress = 0x%x",&envir_param_req);
        WRITELOG(LOG_INFO, szLogInfo);
    //打印envir_param_req.nReqType 地址 结构体 的内容
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, " in serv_get_envir_param envir_param_req.nReqType adress = 0x%x",&envir_param_req.nReqType);
        WRITELOG(LOG_INFO, szLogInfo);
    //打印envir_param_req.nReqType结构体 的内容
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "envir_param_req.nReqType = %d",envir_param_req.nReqType);
        WRITELOG(LOG_INFO, szLogInfo);
        //打印envir_param_req.collector_id结构体 的内容
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "envir_param_req.collector_id = %s",envir_param_req.collector_id);
        WRITELOG(LOG_INFO, szLogInfo);
 */           
	nReqType = envir_param_req.nReqType;
		
	// frame封装
	memset(&frame, 0, sizeof(t_3761frame));
	memcpy(frame.addr, envir_param_req.collector_id, min(sizeof(frame.addr), sizeof(envir_param_req.collector_id)));
	frame.user_data.afn = AFN_TYPE1_DATA_REQ;
	switch(nReqType)
	{
	case 1: //温度	
		{
	        frame.user_data.fn[0] = (1 << ((FN_ENV_T - 1) % 8));
		    frame.user_data.fn[1] = (FN_ENV_T - 1) / 8;
		    break;
		}
	case 2: //湿度
		{
	        frame.user_data.fn[0] = (1 << ((FN_ENV_H - 1) % 8));
		    frame.user_data.fn[1] = (FN_ENV_H - 1) / 8;
		    break;
		}
	case 3: //照度
		{
	        frame.user_data.fn[0] = (1 << ((FN_ENV_SR - 1) % 8));
		    frame.user_data.fn[1] = (FN_ENV_SR - 1) / 8;
		    break;
		}
	case 4: //风向
		{
	        frame.user_data.fn[0] = (1 << ((FN_ENV_WD - 1) % 8));
		    frame.user_data.fn[1] = ((FN_ENV_WD - 1) / 8);
		    break;
		}
	case 5: //风速
	    {
	        frame.user_data.fn[0] = (1 << ((FN_ENV_WP - 1) % 8));
		    frame.user_data.fn[1] = (FN_ENV_WP - 1) / 8;
		    break;
		}
	default:
		{
			snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_get_envir_param: nReqType=%d is invalid!",nReqType);
	        WRITELOG(LOG_ERROR, szLogInfo);
			strcpy(out, "serv_get_envir_param:  invalid reqtype!");
		    return -1;
		}
	}		
	frame.user_data.len = sizeof(int) + sizeof(t_msg_header);
	sprintf(header.serial, "%012d", serial);
	memcpy(frame.user_data.data, &header, sizeof(t_msg_header));
    memcpy(frame.user_data.data + sizeof(t_msg_header), (char *)&nReqType, sizeof(int));
	pbuf = (t_buf *)mymsg.mtext;
	ret = pack_3761frame(&frame, pbuf->buf, MAXLINE);
	//buf.len = MAXLINE;
    pbuf->len = ret;
		
	// 通过集中器ID在hash表中查找socket
	netnode = find_net(envir_param_req.collector_id);
	if (NULL == netnode)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_get_envir_param: exec find_net failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_get_envir_param: find_net failed!");
        return -1;
    }
	pbuf->fd = netnode->netinfo.fd;
	
	// 发送到socket发送消息队列
	mymsg.mtype = 1;
	
	//modify by tianyu: 使用pipe
	pthread_mutex_lock(&g_pipe_mutex);
	ret = write_safe(g_pipe[1], (char *)&mymsg, sizeof(t_mymsg));
	pthread_mutex_unlock(&g_pipe_mutex);
	if (ret < 0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_get_envir_param:write pipe failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_get_envir_param: write pipe failed!");
        return -1;
	}
 
	return 0;
}

/*************************************************
Function: serv_get_all_envir_param
Description: 获取所有环境参数，包括温度，湿度，照度，风向，风速这些环境参数
Calls: 
Called By: down_service_entry
Table Accessed: 
Table Updated:  
Input:  index--线程索引号
        in --前端发来的json格式的字符串请求数据
        len--out缓存长度
Output: out--发送给前端的json格式的字符串响应数据
Return: 0--success; 其它--failure
Others: 无
*************************************************/
int serv_get_all_envir_param(int serial, const char *in, unsigned char *out, int len)
{
    int ret 			=	0;
	t_get_all_envirparam_req envir_param_req = { 0 };
	char szLogInfo[1024] = { 0 };
	t_3761frame frame	 = { 0 };
	t_net_node *netnode  = NULL;
	t_mymsg     mymsg	 = { 0 };
	int    nRecv	     =	0;		
	t_buf *pbuf 		 = NULL;
	int    nReqType      = -1;
	t_msg_header header = {0};
	if (in == NULL || out == NULL)
	{
	    snprintf(szLogInfo, sizeof(szLogInfo)-1, "serv_get_all_envir_param: input parameter(s) is NULL!");
		WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_get_all_envir_param: input parameter(s) is NULL!");
		return -1;
	}
		
	// json解析
	ret = demux_get_all_envparam_req(in, &envir_param_req);
	if(MUXDEMUX_ERR == ret)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "demux_get_all_envparam_req failed!");
		WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_get_all_envir_param: demux_get_all_envparam_req failed!");
		return -1;
	}
	nReqType = envir_param_req.nReqType;
	if (nReqType != 6)
	{
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "nReqType=%d is invalid,so error!",nReqType);
		WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_get_all_envir_param: invalid ReqType");
		return -1;
	}
			
	// frame封装
	memset(&frame, 0, sizeof(t_3761frame));
	memcpy(frame.addr, envir_param_req.collector_id, min(sizeof(frame.addr), sizeof(envir_param_req.collector_id)));
	frame.user_data.afn = AFN_TYPE1_DATA_REQ;	
	frame.user_data.fn[0] = (1 << ((FN_ENV_ALL - 1) % 8));
	frame.user_data.fn[1] = (FN_ENV_ALL - 1) / 8;
	frame.user_data.len = sizeof(int) + sizeof(t_msg_header);
	sprintf(header.serial, "%012d", serial);
	memcpy(frame.user_data.data, &header, sizeof(t_msg_header));
	memcpy(frame.user_data.data + sizeof(t_msg_header), (char *)&nReqType, sizeof(int));
	pbuf = (t_buf *)mymsg.mtext;
	ret = pack_3761frame(&frame, pbuf->buf, MAXLINE);
	pbuf->len = ret;
			
	// 通过集中器ID在hash表中查找socket
	netnode = find_net(envir_param_req.collector_id);
	if (NULL == netnode)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_get_all_envir_param: exec find_net failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_get_all_envir_param: find_net failed!");
        return -1;
    }
	pbuf->fd = netnode->netinfo.fd;
		
	// 发送到socket发送消息队列
	mymsg.mtype = 1;
	
	//modify by tianyu: 使用pipe
	pthread_mutex_lock(&g_pipe_mutex);
	ret = write_safe(g_pipe[1], (char *)&mymsg, sizeof(t_mymsg));
	pthread_mutex_unlock(&g_pipe_mutex);
	if (ret < 0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_get_all_envir_param:write pipe failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_get_all_envir_param: write pipe failed!");
        return -1;
	}
	return 0;
}

/*
int serv_getANDsend_all_envirparam_to_mysql(const char *in)
{
    int ret = 0;
    t_get_all_envirparam_req envir_param_req = { 0 };
    t_get_all_envirparam_ack envir_param_ack = { 0 };
    char szLogInfo[512] = { 0 };
    t_3761frame frame    = { 0 };
    t_buf buf            = { 0 };
    t_net_node *netnode  = NULL;
    t_mymsg     mymsg    = { 0 };
    int    nRecv         =  0;      
    t_buf *pbuf          = NULL;
    int    nReqType      = -1;
    
        if (in == NULL)
        {
            snprintf(szLogInfo, sizeof(szLogInfo)-1, "serv_getANDsend_all_envirparam_to_mysql: input parameter(s) is NULL!");
            WRITELOG(LOG_ERROR, szLogInfo);
            return -1;
        }
            
        // json解析
        ret = demux_get_all_envparam_req(in, &envir_param_req);
        if(MUXDEMUX_ERR == ret)
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "demux_get_all_envparam_req failed!");
            WRITELOG(LOG_ERROR, szLogInfo);
            return -1;
        }
        nReqType = envir_param_req.nReqType;
        if (nReqType != 6)
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "nReqType=%d is invalid,so error!",nReqType);
            WRITELOG(LOG_ERROR, szLogInfo);
            return -1;
        }
                
        // frame封装
        memset(&frame, 0, sizeof(t_3761frame));
        memcpy(frame.addr, envir_param_req.collector_id, min(sizeof(frame.addr), sizeof(envir_param_req.collector_id)));
        frame.user_data.afn = AFN_TYPE1_DATA_REQ;   
        frame.user_data.fn[0] = (1 << ((FN_ENV_ALL - 1) % 8));
        frame.user_data.fn[1] = (FN_ENV_ALL - 1) / 8;
        frame.user_data.len = sizeof(int);
        memcpy(frame.user_data.data, (char *)&nReqType, sizeof(int));
        ret = pack_3761frame(&frame, buf.buf, MAXLINE);
        buf.len = ret;
                
        // 通过集中器ID在hash表中查找socket
        netnode = find_net(envir_param_req.collector_id);
        buf.fd = netnode->netinfo.fd;
            
        // 发送到socket发送消息队列
        mymsg.mtype = 1;
        memcpy(mymsg.mtext, &buf, MAXLINE);
        ret = msgsnd(g_send_msqid, &mymsg, MAXLINE, IPC_NOWAIT);
        if(ret < 0)
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_get_all_envir_param: msgsnd failed!");
            WRITELOG(LOG_ERROR, szLogInfo);
            return -1;
        }
            
        // 阻塞方式从业务消息队列接收mtype为fd的消息，即对应的响应消息
        nRecv = msgrcv(g_serv_msqid, &mymsg, MAXLINE, netnode->netinfo.fd, 0); // 阻塞接收
        if(nRecv < 0)
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_get_all_envir_param: msgrcv failed!");
            WRITELOG(LOG_ERROR, szLogInfo);
            return -1;
        }

        ////gui chen codes
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "The below information is in serv_get_envir_param envir_proc.c [200 line]");
            WRITELOG(LOG_INFO, szLogInfo);
            dumpInfo(mymsg.mtext, sizeof(envir_param_ack));
        
            memcpy(&envir_param_ack,mymsg.mtext,sizeof(t_get_all_envirparam_ack)); 
    
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, 
                "serv_get_all_envir_param: retcode=%d,retmsg=%s,szFengSuValue=%s,szFengXiangValue=%s,szShiDuVlaue=%s,szWenduValue=%s,szZhaoDuVlaue=%s",
                envir_param_ack.retcode,envir_param_ack.retmsg,envir_param_ack.szFengSuValue,envir_param_ack.szFengXiangValue,
                envir_param_ack.szShiDuVlaue,envir_param_ack.szWenduValue,envir_param_ack.szZhaoDuVlaue);
        WRITELOG(LOG_INFO, szLogInfo);
        
        return 0;
}
*/

int QueryUserInfoFromDB(UserLogInReq_T *req, UserLogInRsp_T *rsp)
{
	MYSQL     *conn_ptr = NULL;
    MYSQL_RES  *result;
	MYSQL_ROW  row;

	char  szSql[512]      = {0};
	int   iRet            = 0;
	int  iRows 			  = 0;
	int index 			  = 0;
	int i;
	char  szLogInfo[1024] = {0};
	char mingwen[33] = {0};
	unsigned char decrypt[16] = {0};
	char jiami[33] = {0};
	char on_line[2] = {0};
	if (req == NULL || rsp == NULL)
	{
	    snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryUserInfoFromDB: input parameter(s) is NULL!");
		WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
	}

	//初始化并连接到数据库
	initmysql();
    conn_ptr = ConnectMySql(g_Conf.tDBInfo.szIPAddr, g_Conf.tDBInfo.szUserName, g_Conf.tDBInfo.szPassword, g_Conf.tDBInfo.szDBName, g_Conf.tDBInfo.iDBPort);   // 数据库信息根据配置项修改
    if ( !conn_ptr )
    {
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryUserInfoFromDB: exec ConnectMySql failed!");
		WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(rsp->szRetMsg, "ConnectMySql failed!");
		rsp->iRetCode = -1;
        return 0;
    }
	sprintf(szSql, "select * from user_verify where user_name = '%s'", req->user_name);
	iRet = mysql_real_query(conn_ptr, szSql, strlen(szSql));
	if ( iRet != 0 )
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryUserInfoFromDB: exec mysql_real_query failed! errno[%d] errmsg[%s]", mysql_errno(conn_ptr), mysql_error(conn_ptr));
		WRITELOG(LOG_ERROR, szLogInfo);
		CloseMySql(conn_ptr);
		strcpy(rsp->szRetMsg, "query database failed!");
		rsp->iRetCode = -1;
        return 0;
	}
	result = mysql_store_result(conn_ptr);
	if (result == NULL)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryUserInfoFromDB: exec mysql_store_result failed! errno[%d] errmsg[%s]", mysql_errno(conn_ptr), mysql_error(conn_ptr));
		WRITELOG(LOG_ERROR, szLogInfo);
		CloseMySql(conn_ptr);
		strcpy(rsp->szRetMsg, "query database failed!");
		rsp->iRetCode = -1;
        return 0;
	}
	iRows = mysql_num_rows(result);
	if (iRows== 0)
	{
		strcpy(rsp->szRetMsg, "user unauthorized!");
		rsp->iRetCode = -1;
		DoFreeMySqlResult(result);
		CloseMySql(conn_ptr);
		return 0;
	}
	row = mysql_fetch_row(result);
    //数据库中只有一条数据
	memcpy(mingwen, row[1], sizeof(mingwen)-1); //密码
	memcpy(rsp->accessToken, row[2], sizeof(rsp->accessToken)-1);
	memcpy(on_line, row[3], sizeof(on_line)-1);	
	DoFreeMySqlResult(result);
	MD5_CTX md5;
	MD5Init (&md5);
	MD5Update (&md5, (unsigned char *)mingwen, strlen (mingwen));
	MD5Final (&md5, decrypt);
	for (i = 0; i < 16; i++)
    {
        sprintf(jiami+i*2, "%02x", decrypt[i]);
    }
	snprintf(szLogInfo, sizeof(szLogInfo)-1, "trans_jiami[%s] local_jiami[%s]", req->password, jiami);
	WRITELOG(LOG_INFO, szLogInfo);
	if (strcmp(req->password, jiami))
	{
		strcpy(rsp->szRetMsg, "password error!");
		rsp->iRetCode = -1;
		memset(rsp->accessToken, 0, sizeof(rsp->accessToken));
		CloseMySql(conn_ptr);
		return 0;
	}
	if (on_line[0] == '1')
	{
		strcpy(rsp->szRetMsg, "user has already logged in!");
		rsp->iRetCode = -1;
		memset(rsp->accessToken, 0, sizeof(rsp->accessToken));
		CloseMySql(conn_ptr);
		return 0;
	}	
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "update user_verify set on_line = '1' where user_name = '%s'", req->user_name);
	iRet = mysql_real_query(conn_ptr, szSql, strlen(szSql));
	if ( iRet != 0 )
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryUserInfoFromDB: exec mysql_real_query failed! errno[%d] errmsg[%s]", mysql_errno(conn_ptr), mysql_error(conn_ptr));
		WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(rsp->szRetMsg, "update logging status failed!");
		rsp->iRetCode = -1;
		memset(rsp->accessToken, 0, sizeof(rsp->accessToken));
		CloseMySql(conn_ptr);
        return 0;
	}
	rsp->iRetCode = 0;
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "select * from collector_user where user_name = '%s'", req->user_name);
	iRet = mysql_real_query(conn_ptr, szSql, strlen(szSql));
	if ( iRet != 0 )
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryUserInfoFromDB: exec mysql_real_query failed! errno[%d] errmsg[%s]", mysql_errno(conn_ptr), mysql_error(conn_ptr));
		WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(rsp->szRetMsg, "query collector failed!");
		CloseMySql(conn_ptr);
        return 0;
	}
	strcpy(rsp->szRetMsg, "log in success!");
	result = mysql_store_result(conn_ptr);
	if (result == NULL)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryUserInfoFromDB: exec mysql_store_result failed! errno[%d] errmsg[%s]", mysql_errno(conn_ptr), mysql_error(conn_ptr));
		WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(rsp->szRetMsg, "query collector failed!");
		rsp->iRetCode = -1;
		CloseMySql(conn_ptr);
        return -1;
	}
	iRows = mysql_num_rows(result);
	if (iRows == 0)
	{
		rsp->collector_nums = 0;
		DoFreeMySqlResult(result);
		CloseMySql(conn_ptr);
		return 0;
	}
	while( row = mysql_fetch_row(result) )
	{
		memcpy(rsp->collector[index], row[0], sizeof(rsp->collector[0]) - 1);
		index++;
	}
	rsp->collector_nums = index;
	DoFreeMySqlResult(result);
	CloseMySql(conn_ptr);
	return 0;
}


int UpdateUserInfoFromDB(UserLogOutReq_T *req, UserLogOutRsp_T *rsp)
{
	MYSQL     *conn_ptr = NULL;
    MYSQL_RES  *result;
	MYSQL_ROW  row;

	char  szSql[512]      = {0};
	char uuid[37] = {0};
	int   iRet            = 0;
	int  iRows 			  = 0;
	int index 			  = 0;
	int i;
	char  szLogInfo[1024] = {0};
	char on_line[2] = {0};
	if (req == NULL || rsp == NULL)
	{
	    snprintf(szLogInfo, sizeof(szLogInfo)-1, "UpdateUserInfoFromDB: input parameter(s) is NULL!");
		WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
	}

	//初始化并连接到数据库
	initmysql();
    conn_ptr = ConnectMySql(g_Conf.tDBInfo.szIPAddr, g_Conf.tDBInfo.szUserName, g_Conf.tDBInfo.szPassword, g_Conf.tDBInfo.szDBName, g_Conf.tDBInfo.iDBPort);   // 数据库信息根据配置项修改
    if ( !conn_ptr )
    {
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryUserInfoFromDB: exec ConnectMySql failed!");
		WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(rsp->szRetMsg, "ConnectMySql failed!");
		rsp->iRetCode = -1;
        return 0;
    }
	sprintf(szSql, "select * from user_verify where token = '%s'", req->accessToken);
	iRet = mysql_real_query(conn_ptr, szSql, strlen(szSql));
	if ( iRet != 0 )
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "UpdateUserInfoFromDB: exec mysql_real_query failed! errno[%d] errmsg[%s]", mysql_errno(conn_ptr), mysql_error(conn_ptr));
		WRITELOG(LOG_ERROR, szLogInfo);
		CloseMySql(conn_ptr);
		strcpy(rsp->szRetMsg, "query database failed!");
		rsp->iRetCode = -1;
        return 0;
	}
	
	result = mysql_store_result(conn_ptr);
	if (result == NULL)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "UpdateUserInfoFromDB: exec mysql_store_result failed! errno[%d] errmsg[%s]", mysql_errno(conn_ptr), mysql_error(conn_ptr));
		WRITELOG(LOG_ERROR, szLogInfo);
		CloseMySql(conn_ptr);
		strcpy(rsp->szRetMsg, "query database failed!");
		rsp->iRetCode = -1;
        return 0;
	}
	iRows = mysql_num_rows(result);
	if (iRows== 0)
	{
		strcpy(rsp->szRetMsg, "token unauthorized!");
		rsp->iRetCode = -1;
		DoFreeMySqlResult(result);
		CloseMySql(conn_ptr);
		return 0;
	}
	row = mysql_fetch_row(result);
    //数据库中只有一条数据
	memcpy(on_line, row[3], sizeof(on_line)-1);	
	DoFreeMySqlResult(result);
	if (on_line[0] == '0')
	{
		strcpy(rsp->szRetMsg, "user has not log in!");
		rsp->iRetCode = -1;
		CloseMySql(conn_ptr);
		return 0;
	}
	random_uuid(uuid);
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "update user_verify set token = '%s', on_line = '0' where token = '%s'", 
											uuid, req->accessToken);
	iRet = mysql_real_query(conn_ptr, szSql, strlen(szSql));
	if ( iRet != 0 )
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "UpdateUserInfoFromDB: exec mysql_real_query failed! sql[%s]errno[%d] errmsg[%s]", 
													szSql, mysql_errno(conn_ptr), mysql_error(conn_ptr));
		WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(rsp->szRetMsg, "update logging status failed!");
		rsp->iRetCode = -1;
		CloseMySql(conn_ptr);
        return 0;
	}
	rsp->iRetCode = 0;
	strcpy(rsp->szRetMsg, "user log out success!");
	CloseMySql(conn_ptr);
	return 0;
}


int QueryEnvInfoFromDB(GetEnvReq_T *req, GetEnvRsp_T *rsp)
{
	MYSQL     *conn_ptr = NULL;
    MYSQL_RES  *result;
	MYSQL_ROW  row;

	char  szSql[512]      = {0};
	int   iRet            = 0;
	int  iRows 			  = 0;
	int index 			  = 0;
	int i;
	char  szLogInfo[1024] = {0};
	char on_line[2] = {0};
	char user_name[33] = {0};
	if (req == NULL || rsp == NULL)
	{
	    snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryEnvInfoFromDB: input parameter(s) is NULL!");
		WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
	}

	//初始化并连接到数据库
	initmysql();
    conn_ptr = ConnectMySql(g_Conf.tDBInfo.szIPAddr, g_Conf.tDBInfo.szUserName, g_Conf.tDBInfo.szPassword, g_Conf.tDBInfo.szDBName, g_Conf.tDBInfo.iDBPort);   // 数据库信息根据配置项修改
    if ( !conn_ptr )
    {
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryEnvInfoFromDB: exec ConnectMySql failed!");
		WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(rsp->szRetMsg, "ConnectMySql failed!");
		rsp->iRetCode = -1;
        return 0;
    }
	sprintf(szSql, "select * from user_verify where token = '%s'", req->accessToken);
	iRet = mysql_real_query(conn_ptr, szSql, strlen(szSql));
	if ( iRet != 0 )
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryEnvInfoFromDB: exec mysql_real_query failed! errno[%d] errmsg[%s]", mysql_errno(conn_ptr), mysql_error(conn_ptr));
		WRITELOG(LOG_ERROR, szLogInfo);
		CloseMySql(conn_ptr);
		strcpy(rsp->szRetMsg, "query database failed!");
		rsp->iRetCode = -1;
        return 0;
	}
	result = mysql_store_result(conn_ptr);
	if (result == NULL)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryEnvInfoFromDB: exec mysql_store_result failed! errno[%d] errmsg[%s]", mysql_errno(conn_ptr), mysql_error(conn_ptr));
		WRITELOG(LOG_ERROR, szLogInfo);
		CloseMySql(conn_ptr);
		strcpy(rsp->szRetMsg, "query database failed!");
		rsp->iRetCode = -1;
        return 0;
	}
	iRows = mysql_num_rows(result);
	if (iRows== 0)
	{
		strcpy(rsp->szRetMsg, "token unauthorized!");
		rsp->iRetCode = -1;
		DoFreeMySqlResult(result);
		CloseMySql(conn_ptr);
		return 0;
	}
	row = mysql_fetch_row(result);
    //数据库中只有一条数据
    memcpy(user_name, row[0], sizeof(user_name)-1);
	memcpy(on_line, row[3], sizeof(on_line)-1);	
	DoFreeMySqlResult(result);
	if (on_line[0] == '0')
	{
		strcpy(rsp->szRetMsg, "user has not log in!");
		rsp->iRetCode = -1;
		CloseMySql(conn_ptr);
		return 0;
	}

	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "select * from collector_user where collector_id = '%s' and user_name = '%s'", 
																req->collector_id, user_name);
	iRet = mysql_real_query(conn_ptr, szSql, strlen(szSql));
	if ( iRet != 0 )
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryEnvInfoFromDB: exec mysql_real_query failed! errno[%d] errmsg[%s]", mysql_errno(conn_ptr), mysql_error(conn_ptr));
		WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(rsp->szRetMsg, "query database failed!");
		rsp->iRetCode = -1;
		CloseMySql(conn_ptr);
        return 0;
	}
    result = mysql_store_result(conn_ptr);
	if (result == NULL)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryEnvInfoFromDB: exec mysql_store_result failed! errno[%d] errmsg[%s]", mysql_errno(conn_ptr), mysql_error(conn_ptr));
		WRITELOG(LOG_ERROR, szLogInfo);
		CloseMySql(conn_ptr);
		strcpy(rsp->szRetMsg, "query database failed!");
		rsp->iRetCode = -1;
        return 0;
	}
	iRows = mysql_num_rows(result);
	if (iRows== 0)
	{
		strcpy(rsp->szRetMsg, "collector_id invalid!");
		rsp->iRetCode = -1;
		DoFreeMySqlResult(result);
		CloseMySql(conn_ptr);
		return 0;
	}
	DoFreeMySqlResult(result);
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "select * from EvirParamUpdate where collector_id = '%s'", req->collector_id);
	iRet = mysql_real_query(conn_ptr, szSql, strlen(szSql));
	if ( iRet != 0 )
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryEnvInfoFromDB: exec mysql_real_query failed! errno[%d] errmsg[%s]", mysql_errno(conn_ptr), mysql_error(conn_ptr));
		WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(rsp->szRetMsg, "query database failed!");
		rsp->iRetCode = -1;
		CloseMySql(conn_ptr);
        return 0;
	}
	result = mysql_store_result(conn_ptr);
	if (result == NULL)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryEnvInfoFromDB: exec mysql_store_result failed! errno[%d] errmsg[%s]", mysql_errno(conn_ptr), mysql_error(conn_ptr));
		WRITELOG(LOG_ERROR, szLogInfo);
		CloseMySql(conn_ptr);
		strcpy(rsp->szRetMsg, "query database failed!");
		rsp->iRetCode = -1;
        return 0;
	}
	iRows = mysql_num_rows(result);
	if (iRows== 0)
	{
		strcpy(rsp->szRetMsg, "no environment parameter of the lamppost!");
		rsp->iRetCode = -1;
		DoFreeMySqlResult(result);
		CloseMySql(conn_ptr);
		return 0;
	}
	row = mysql_fetch_row(result);
    //数据库中只有一条数据
	strncpy(rsp->Temperature, row[1], sizeof(rsp->Temperature)-1);
	strncpy(rsp->Humidity, row[2], sizeof(rsp->Humidity)-1);
	strncpy(rsp->Illumination, row[3], sizeof(rsp->Illumination)-1);
	strncpy(rsp->WindDirection, row[4], sizeof(rsp->WindDirection)-1);
	strncpy(rsp->WindSpeed, row[5], sizeof(rsp->WindSpeed)-1);
	DoFreeMySqlResult(result);
	rsp->iRetCode = 0;
	strcpy(rsp->szRetMsg, "query environment parameter success!");
	CloseMySql(conn_ptr);
	return 0;
}


int QueryEnvInfoFromDB_noVerify(GetEnvReqNoVerify_T *req, GetEnvRsp_T *rsp)
{
	MYSQL     *conn_ptr = NULL;
    MYSQL_RES  *result;
	MYSQL_ROW  row;

	char  szSql[512]      = {0};
	int   iRet            = 0;
	int  iRows 			  = 0;
	int index 			  = 0;
	int i;
	char  szLogInfo[1024] = {0};
	char on_line[2] = {0};
	char user_name[33] = {0};
	if (req == NULL || rsp == NULL)
	{
	    snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryEnvInfoFromDB: input parameter(s) is NULL!");
		WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
	}

	//初始化并连接到数据库
	initmysql();
    conn_ptr = ConnectMySql(g_Conf.tDBInfo.szIPAddr, g_Conf.tDBInfo.szUserName, g_Conf.tDBInfo.szPassword, g_Conf.tDBInfo.szDBName, g_Conf.tDBInfo.iDBPort);   // 数据库信息根据配置项修改
    if ( !conn_ptr )
    {
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryEnvInfoFromDB: exec ConnectMySql failed!");
		WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(rsp->szRetMsg, "ConnectMySql failed!");
		rsp->iRetCode = -1;
        return 0;
    }
	
	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "select * from EvirParamUpdate where collector_id = '%s'", req->collector_id);
	iRet = mysql_real_query(conn_ptr, szSql, strlen(szSql));
	if ( iRet != 0 )
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryEnvInfoFromDB: exec mysql_real_query failed! errno[%d] errmsg[%s]", mysql_errno(conn_ptr), mysql_error(conn_ptr));
		WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(rsp->szRetMsg, "query database failed!");
		rsp->iRetCode = -1;
		CloseMySql(conn_ptr);
        return 0;
	}
	result = mysql_store_result(conn_ptr);
	if (result == NULL)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryEnvInfoFromDB: exec mysql_store_result failed! errno[%d] errmsg[%s]", mysql_errno(conn_ptr), mysql_error(conn_ptr));
		WRITELOG(LOG_ERROR, szLogInfo);
		CloseMySql(conn_ptr);
		strcpy(rsp->szRetMsg, "query database failed!");
		rsp->iRetCode = -1;
        return 0;
	}
	iRows = mysql_num_rows(result);
	if (iRows== 0)
	{
		strcpy(rsp->szRetMsg, "no environment parameter of the lamppost!");
		rsp->iRetCode = -1;
		DoFreeMySqlResult(result);
		CloseMySql(conn_ptr);
		return 0;
	}
	row = mysql_fetch_row(result);
    //数据库中只有一条数据
    strncpy(rsp->Temperature, row[1], sizeof(rsp->Temperature)-1);
	strncpy(rsp->Humidity, row[2], sizeof(rsp->Humidity)-1);
	strncpy(rsp->Illumination, row[3], sizeof(rsp->Illumination)-1);
	strncpy(rsp->WindDirection, row[4], sizeof(rsp->WindDirection)-1);
	strncpy(rsp->WindSpeed, row[5], sizeof(rsp->WindSpeed)-1);
	DoFreeMySqlResult(result);
	rsp->iRetCode = 0;
	strcpy(rsp->szRetMsg, "query environment parameter success!");
	CloseMySql(conn_ptr);
	return 0;
}


int serv_user_login(int serial, const char *in, unsigned char *out, int len)
{
	int ret             =   0;
	UserLogInReq_T log_in_req = {0};
	UserLogInRsp_T log_in_rsp = {0};
    char szLogInfo[1024] = { 0 };
  	if (NULL == in || NULL == out)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_user_login:NULL == in || NULL == out!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_user_login: NULL == in || NULL == out!");
        return -1;
    }
	// json解析
    ret = demux_user_log_in_req(in, &log_in_req);
    if(MUXDEMUX_ERR == ret)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_user_login:demux_user_log_in_req failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "demux_user_log_in_req failed!");
        return -1;
    }
	ret = QueryUserInfoFromDB(&log_in_req, &log_in_rsp);
	if (ret != 0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_user_login:exec QueryUserInfoFromDB failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "exec QueryUserInfoFromDB failed!");
        return -1;
	}
	ret =  mux_user_log_in_ack(&log_in_rsp, out, len);
	if (MUXDEMUX_ERR == ret)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_user_login: exec mux_user_log_in_ack failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "mux_user_log_in_ack failed!");
        return -1;
    }
	return 0;
}

int serv_user_logout(int serial, const char *in, unsigned char *out, int len)
{

	//验证token, 更新on_line状态，更新token
	int ret             =   0;
	UserLogOutReq_T log_out_req = {0};
	UserLogOutRsp_T log_out_rsp = {0};
    char szLogInfo[1024] = { 0 };
  	if (NULL == in || NULL == out)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_user_logout:NULL == in || NULL == out!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_user_logout: NULL == in || NULL == out!");
        return -1;
    }
	// json解析
    ret = demux_user_log_out_req(in, &log_out_req);
    if(MUXDEMUX_ERR == ret)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_user_logout:demux_user_log_out_req failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "demux_user_log_out_req failed!");
        return -1;
    }
	ret = UpdateUserInfoFromDB(&log_out_req, &log_out_rsp);
	if (ret != 0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_user_logout:exec QueryUserInfoFromDB failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "exec QueryUserInfoFromDB failed!");
        return -1;
	}
	ret =  mux_user_log_out_ack(&log_out_rsp, out, len);
	if (MUXDEMUX_ERR == ret)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_user_logout: exec mux_user_log_in_ack failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "mux_user_log_in_ack failed!");
        return -1;
    }
	return 0;
}

int serv_get_env_values(int serial, const char *in, unsigned char *out, int len)
{
	//验证token，验证集中器id,  查询环境参数
	int ret             =   0;
	GetEnvReq_T get_env_req = {0};
	GetEnvRsp_T get_env_rsp = {0};
    char szLogInfo[1024] = { 0 };
  	if (NULL == in || NULL == out)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_get_env_values:NULL == in || NULL == out!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_get_env_values: NULL == in || NULL == out!");
        return -1;
    }
	// json解析
    ret = demux_get_env_req(in, &get_env_req);
    if(MUXDEMUX_ERR == ret)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_get_env_values:demux_get_env_req failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "demux_get_env_req failed!");
        return -1;
    }
	ret = QueryEnvInfoFromDB(&get_env_req, &get_env_rsp);
	if (ret != 0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_get_env_values:exec QueryEnvInfoFromDB failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "exec QueryEnvInfoFromDB failed!");
        return -1;
	}
	ret =  mux_get_env_ack(&get_env_rsp, out, len);
	if (MUXDEMUX_ERR == ret)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_get_env_values: exec mux_get_env_ack failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "mux_get_env_ack failed!");
        return -1;
    }
	return 0;
}

int serv_get_env_values_no_verify(int serial, const char *in, unsigned char *out, int len)
{
	//  查询环境参数
	int ret             =   0;
	GetEnvReqNoVerify_T get_env_req = {0};
	GetEnvRsp_T get_env_rsp = {0};
    char szLogInfo[1024] = { 0 };
  	if (NULL == in || NULL == out)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_get_env_values:NULL == in || NULL == out!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_get_env_values: NULL == in || NULL == out!");
        return -1;
    }
	// json解析
    ret = demux_get_env_req_no_verify(in, &get_env_req);
    if(MUXDEMUX_ERR == ret)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_get_env_values:demux_get_env_req failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "demux_get_env_req failed!");
        return -1;
    }
	ret = QueryEnvInfoFromDB_noVerify(&get_env_req, &get_env_rsp);
	if (ret != 0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_get_env_values:exec QueryEnvInfoFromDB failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "exec QueryEnvInfoFromDB failed!");
        return -1;
	}
	ret =  mux_get_env_ack(&get_env_rsp, out, len);
	if (MUXDEMUX_ERR == ret)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_get_env_values: exec mux_get_env_ack failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "mux_get_env_ack failed!");
        return -1;
    }
	return 0;
}




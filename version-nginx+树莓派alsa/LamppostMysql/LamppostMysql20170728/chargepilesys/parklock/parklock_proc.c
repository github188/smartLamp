/*****************************************************************************
Copyright: 2017-2026, Chongqing Optical&Electronical Information Institute.
File name: chargepile_proc.c
Description: maitain,manage the information on parking-locks
Author: zhangwei
Version: v1.0.0
Date: 2017-04-10
History: 
-----------------------------------------
date          author        description
-----------------------------------------
2017-04-10    zhangwei      created

*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "parklock_proc.h"
#include "config.h"


extern Config_T g_Conf;


/*************************************************
Function: serv_add_parklock
Description: 增加车位锁处理
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
int serv_add_parklock(int serial, const char *in, unsigned char *out, int len)
{
	AddLockReq_T  tAddLock      = {0};
	DealLockRSP_T tDealLockRSP  = {0};

	int    iFuncRetVal     = 0;
	int    iDBRetVal       = 0;
	int    iBodyLen        = 0;
	char   szBodyData[500] = {0};
	char   szData[1000]    = {0};
    char   szLogInfo[1024] = {0};

    if (in == NULL || out == NULL)
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "serv_add_parklock: input parameter(s) is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);

		return -1;
    }

	// json解析
    iFuncRetVal = demux_addparklock_req(in, &tAddLock);
    if (MUXDEMUX_ERR == iFuncRetVal)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_add_parklock: exec demux_addparklock_req failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }

	snprintf(szLogInfo, sizeof(szLogInfo)-1, "serv_add_parklock: CollectorID=%s, ChargepileID=%s, ParklockID=%s, Name=%s, Longitude=%s, Latitude=%s, DetailAddress=%s",
	                      tAddLock.szCollectorID, tAddLock.szChargepileID, tAddLock.szParklockID, tAddLock.szName, tAddLock.szLongitude, tAddLock.szLatitude, tAddLock.szDetailAddress);
    WRITELOG(LOG_INFO, szLogInfo);

	// 将车位锁信息插入到数据库中
	iFuncRetVal = InsertLockInfoIntoDB(&tAddLock, &iDBRetVal);
	if (iFuncRetVal != 0)
	{
	    snprintf(szLogInfo, sizeof(szLogInfo)-1, "serv_add_parklock: exec InsertLockInfoIntoDB failed!");
		WRITELOG(LOG_ERROR, szLogInfo);
		iDBRetVal = 3;   // 3-服务器错误
	}
	else
	{
	    snprintf(szLogInfo, sizeof(szLogInfo)-1, "serv_add_parklock: exec InsertLockInfoIntoDB successfully, DBRetVal=%d!", iDBRetVal);
		WRITELOG(LOG_INFO, szLogInfo);
	}

	// 向请求端返回响应消息
	memset(&tDealLockRSP, 0x00, sizeof(tDealLockRSP));
	tDealLockRSP.iRetCode = iDBRetVal;
	if (iDBRetVal == 0)
	{
		strncpy(tDealLockRSP.szRetMsg, "Add parklock successfully!", strlen("Add parklock successfully!"));
	}
	else if (iDBRetVal == 1)
	{
		strncpy(tDealLockRSP.szRetMsg, "Lockid repeatted!", strlen("Lockid repeatted!"));
	}
	else if (iDBRetVal == 2)
	{
		strncpy(tDealLockRSP.szRetMsg, "Lockid has not existed!", strlen("Lockid has not existed!"));
	}
	else if (iDBRetVal == 3)
	{
		strncpy(tDealLockRSP.szRetMsg, "Server error!", strlen("Server error!"));
	}
	else if (iDBRetVal == 99)
	{
		strncpy(tDealLockRSP.szRetMsg, "Other error!", strlen("Other error!"));
	}

    // 组装响应消息
    //json封装
    iFuncRetVal = mux_parklock_ack(&tDealLockRSP, out, len);
	if (MUXDEMUX_ERR == iFuncRetVal)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_add_parklock: exec mux_parklock_ack failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
    /*
	iFuncRetVal = DealLockRspMsg(&tDealLockRSP, szBodyData, sizeof(szBodyData)-1);
	if (iFuncRetVal != 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "serv_add_parklock: exec DealLockRspMsg failed! Ret=%d", iFuncRetVal);
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }

	strncpy(out, szBodyData, strlen(szBodyData));
	out[len - 1] = '\0';
	*/
	
    return 0;
}

/*************************************************
Function: serv_del_parklock
Description: 删除车位锁处理
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
int serv_del_parklock(int serial, const char *in, unsigned char *out, int len)
{
	DelLockReq_T  tDelLock	   = {0};
	DealLockRSP_T tDealLockRSP = {0};
	
	int    iFuncRetVal	   = 0;
	int    iDBRetVal	   = 0;
	int    iBodyLen 	   = 0;
	char   szBodyData[500] = {0};
	char   szData[1000]    = {0};
	char   szLogInfo[1024] = {0};
	
	if (in == NULL || out == NULL)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "serv_del_parklock: input parameter(s) is NULL!");
		WRITELOG(LOG_ERROR, szLogInfo);
	
		return -1;
	}

	// json解析
    iFuncRetVal = demux_delparklock_req(in, &tDelLock);
    if (MUXDEMUX_ERR == iFuncRetVal)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_del_parklock: exec demux_delparklock_req failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
		
	snprintf(szLogInfo, sizeof(szLogInfo)-1, "serv_del_parklock: ParklockID=%s", tDelLock.szParklockID);
	WRITELOG(LOG_INFO, szLogInfo);
	
	// 删除数据库中的车位锁信息
	iFuncRetVal = DeleteLockInfoInDB(&tDelLock, &iDBRetVal);
	if (iFuncRetVal != 0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "serv_del_parklock: exec DeleteLockInfoInDB failed!");
		WRITELOG(LOG_ERROR, szLogInfo);
		iDBRetVal = 3;	 // 3-服务器错误
	}
	else
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "serv_del_parklock: exec DeleteLockInfoInDB successfully, DBRetVal=%d!", iDBRetVal);
		WRITELOG(LOG_INFO, szLogInfo);
	}
	
	// 向请求端返回响应消息
	memset(&tDealLockRSP, 0x00, sizeof(tDealLockRSP));
	tDealLockRSP.iRetCode = iDBRetVal;
	if (iDBRetVal == 0)
	{
		strncpy(tDealLockRSP.szRetMsg, "Delete parklock successfully!", strlen("Delete parklock successfully!"));
	}
	else if (iDBRetVal == 1)
	{
		strncpy(tDealLockRSP.szRetMsg, "Lockid repeatted!", strlen("Lockid repeatted!"));
	}
	else if (iDBRetVal == 2)
	{
		strncpy(tDealLockRSP.szRetMsg, "Lockid has not existed!", strlen("Lockid has not existed!"));
	}
	else if (iDBRetVal == 3)
	{
		strncpy(tDealLockRSP.szRetMsg, "Server error!", strlen("Server error!"));
	}
	else if (iDBRetVal == 99)
	{
		strncpy(tDealLockRSP.szRetMsg, "Other error!", strlen("Other error!"));
	}
	
	// 组装响应消息
    //json封装
    iFuncRetVal = mux_parklock_ack(&tDealLockRSP, out, len);
	if (MUXDEMUX_ERR == iFuncRetVal)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_del_parklock: exec mux_parklock_ack failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
	
	return 0;
}

/*************************************************
Function: serv_modify_parklock
Description: 修改车位锁处理
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
int serv_modify_parklock(int serial, const char *in, unsigned char *out, int len)
{
	ModifyLockReq_T  tModLock	= {0};
	DealLockRSP_T tDealLockRSP	= {0};
	
	int    iFuncRetVal	   = 0;
	int    iDBRetVal	   = 0;
	int    iBodyLen 	   = 0;
	char   szBodyData[500] = {0};
	char   szData[1000]    = {0};
	char   szLogInfo[1024] = {0};
	
	if (in == NULL || out == NULL)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "serv_modify_parklock: input parameter(s) is NULL!");
		WRITELOG(LOG_ERROR, szLogInfo);
	
		return -1;
	}
	
	// json解析
    iFuncRetVal = demux_modparklock_req(in, &tModLock);
    if (MUXDEMUX_ERR == iFuncRetVal)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_del_parklock: exec demux_modparklock_req failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
	
	snprintf(szLogInfo, sizeof(szLogInfo)-1, "serv_modify_parklock: CollectorID=%s, ChargepileID=%s, ParklockID=%s, Name=%s, Longitude=%s, Latitude=%s, DetailAddress=%s",
							  tModLock.szCollectorID, tModLock.szChargepileID, tModLock.szParklockID, tModLock.szName, tModLock.szLongitude, tModLock.szLatitude, tModLock.szDetailAddress);
	WRITELOG(LOG_INFO, szLogInfo);
	
	// 在数据库中修改车位锁信息
	iFuncRetVal = ModifyLockInfoInDB(&tModLock, &iDBRetVal);
	if (iFuncRetVal != 0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "serv_modify_parklock: exec ModifyLockInfoInDB failed!");
		WRITELOG(LOG_ERROR, szLogInfo);
		iDBRetVal = 3;	 // 3-服务器错误
	}
	else
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "serv_modify_parklock: exec ModifyLockInfoInDB successfully, DBRetVal=%d!", iDBRetVal);
		WRITELOG(LOG_INFO, szLogInfo);
	}
	
	// 向请求端返回响应消息
	memset(&tDealLockRSP, 0x00, sizeof(tDealLockRSP));
	tDealLockRSP.iRetCode = iDBRetVal;
	if (iDBRetVal == 0)
	{
		strncpy(tDealLockRSP.szRetMsg, "Modify parklock info successfully!", strlen("Modify parklock info successfully!"));
	}
	else if (iDBRetVal == 1)
	{
		strncpy(tDealLockRSP.szRetMsg, "Lockid repeatted!", strlen("Lockid repeatted!"));
	}
	else if (iDBRetVal == 2)
	{
		strncpy(tDealLockRSP.szRetMsg, "Lockid has not existed!", strlen("Lockid has not existed!"));
	}
	else if (iDBRetVal == 3)
	{
		strncpy(tDealLockRSP.szRetMsg, "Server error!", strlen("Server error!"));
	}
	else if (iDBRetVal == 99)
	{
		strncpy(tDealLockRSP.szRetMsg, "Other error!", strlen("Other error!"));
	}
	
	// 组装响应消息
    //json封装
    iFuncRetVal = mux_parklock_ack(&tDealLockRSP, out, len);
	if (MUXDEMUX_ERR == iFuncRetVal)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_modify_parklock: exec mux_parklock_ack failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
		
	return 0;
}


/*************************************************
Function: serv_queryid_parklock
Description: 通过设备ID查询车位锁处理
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
int serv_queryid_parklock(int serial, const char *in, unsigned char *out, int len)
{
	QueryLockReq_T tQueryLock	 = {0};
	QueryLockRSP_T tQueryLockRSP = {0};

	int    iFuncRetVal		= 0;
	int    iDBRetVal		= 0;
	int    iBodyLen 		= 0;
	char   szBodyData[1024] = {0};
	char   szData[1024] 	= {0};
	char   szLogInfo[1024]	= {0};
	
	if (in == NULL || out == NULL)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "serv_queryid_parklock: input parameter(s) is NULL!");
		WRITELOG(LOG_ERROR, szLogInfo);
	
		return -1;
	}
	
    // json解析
    iFuncRetVal = demux_queryidparklock_req(in, &tQueryLock);
    if (MUXDEMUX_ERR == iFuncRetVal)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_queryid_parklock: exec demux_queryidparklock_req failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
	
	snprintf(szLogInfo, sizeof(szLogInfo)-1, "serv_queryid_parklock: ParklockID=%s", tQueryLock.szParklockID);
	WRITELOG(LOG_INFO, szLogInfo);
	
	// 查询数据库中的车位锁信息
	iFuncRetVal = QueryLockInfoFromDB(&tQueryLock, &tQueryLockRSP);
	if (iFuncRetVal != 0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "serv_queryid_parklock: exec QueryLockInfoFromDB failed!");
		WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
	}
	else
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "serv_queryid_parklock: exec QueryLockInfoFromDB successfully, DBRetVal=%d!", tQueryLockRSP.iRetCode);
		WRITELOG(LOG_INFO, szLogInfo);
	}
	
	// 组装响应消息
    //json封装
    iFuncRetVal = mux_queryidparklock_ack(&tQueryLockRSP, out, len);
	if (MUXDEMUX_ERR == iFuncRetVal)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_queryid_parklock: exec mux_queryidparklock_ack failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
		
	return 0;
}


/*************************************************
Function: serv_querygis_parklock
Description: 通过地理位置信息查询车位锁处理
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
int serv_querygis_parklock(int serial, const char *in, unsigned char *out, int len)
{
    QueryLockGISReq_T tQueryLockGIS = {0};

	int    iFuncRetVal      = 0;
	int    iDBRetVal        = 0;
	int    iBodyLen         = 0;
	char   szBodyData[10000]= {0};
	char   szData[1024]     = {0};
    char   szLogInfo[1024]  = {0};

    // json解析
    iFuncRetVal = demux_querygisparklock_req(in, &tQueryLockGIS);
    if (MUXDEMUX_ERR == iFuncRetVal)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_querygis_parklock: exec demux_querygisparklock_req failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
	
	snprintf(szLogInfo, sizeof(szLogInfo)-1, "serv_querygis_parklock: StartLongitude=%s, EndLongitude=%s, StartLatitude=%s, EndLatitude=%s", 
		  tQueryLockGIS.szStartLongitude, tQueryLockGIS.szEndLongitude, tQueryLockGIS.szStartLatitude, tQueryLockGIS.szEndLatitude);
    WRITELOG(LOG_INFO, szLogInfo);

	// 查询数据库中的车位锁GIS信息,并组装json响应消息
	iFuncRetVal = QueryLockInfoGISFromDB(&tQueryLockGIS, szBodyData, sizeof(szBodyData)-1);
	if (iFuncRetVal != 0)
	{
	    snprintf(szLogInfo, sizeof(szLogInfo)-1, "serv_querygis_parklock: exec QueryLockInfoGISFromDB failed!");
		WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
	}
	else
	{
	    snprintf(szLogInfo, sizeof(szLogInfo)-1, "serv_querygis_parklock: exec QueryLockInfoGISFromDB successfully!");
		WRITELOG(LOG_INFO, szLogInfo);
	}

	strncpy(out, szBodyData, strlen(szBodyData));
	out[len - 1] = '\0';
	
    return 0;
}


/*************************************************
Function: serv_control_parklock
Description: 控制车位锁的开关
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
int serv_control_parklock(int serial, const char *in, unsigned char *out, int len)
{
	ControlLockReq_T  tControlLock = {0};

    /*
	int    iBodyLen        = 0;
	char   szBodyData[500] = {0};
	char   szData[1000]    = {0};
	*/
    char   szLogInfo[1024] = {0};

	t_3761frame tFrame     = {0};
    t_net_node *ptNetnode  = NULL;
    t_mymsg     tMymsg     = {0};
    int         iRetVal    =  0;
	int         iRetCode   = 0;      //0成功，1 id重复，2 id不存在，3服务器错误, 99其他
    t_di_confirm_deny_list di_confirm_deny_list = { 0 };
    t_buf      *pbuf       = NULL;
    int Ret =0;
	long msgFlag = 0;
	t_msg_header header = {0};
    // json解析
    iRetVal = demux_controlparklock_req(in, &tControlLock);
    if (MUXDEMUX_ERR == iRetVal)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_control_parklock: exec demux_controlparklock_req failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "demux_controlparklock_req failed!");
        return -1;
    }
	
	snprintf(szLogInfo, sizeof(szLogInfo)-1, "serv_control_parklock: CollectorID=%s, ChargepileID=%s, ParklockID=%s, LockCtrol=%d",
	                      tControlLock.szCollectorID, tControlLock.szChargepileID, tControlLock.szParklockID, tControlLock.iLockCtrol);
    WRITELOG(LOG_INFO, szLogInfo);

	// frame封装
    memset(&tFrame, 0, sizeof(t_3761frame));
    memcpy(tFrame.addr, tControlLock.szCollectorID, min(sizeof(tFrame.addr), sizeof(tControlLock.szCollectorID)));
    tFrame.user_data.afn = AFN_CTRL_CMD;
    tFrame.user_data.fn[0] = (1 << ((FN_ONOFF_LOCK - 1) % 8));
    tFrame.user_data.fn[1] = (FN_ONOFF_LOCK - 1) / 8;
	/*添加消息头:消息序列号*/
    tFrame.user_data.len = sizeof(int) + sizeof(t_msg_header);
	sprintf(header.serial, "%012d", serial);
	memcpy(tFrame.user_data.data, &header, sizeof(t_msg_header));
    snprintf(tFrame.user_data.data + sizeof(t_msg_header), sizeof(tFrame.user_data.data)-1-sizeof(t_msg_header), "%d", tControlLock.iLockCtrol);
	pbuf = (t_buf *)tMymsg.mtext;
	Ret = pack_3761frame(&tFrame, pbuf->buf, MAXLINE);
    pbuf->len = Ret;

    // 通过集中器ID在hash表中查找socket
    ptNetnode = find_net(tControlLock.szCollectorID);
	if (NULL == ptNetnode)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_control_parklock: exec find_net failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_control_parklock: find_net failed!");
        return -1;
    }
	
    pbuf->fd = ptNetnode->netinfo.fd;

    // 发送到socket发送消息队列
    tMymsg.mtype = 1;
   
	//modify by tianyu: 使用pipe
	pthread_mutex_lock(&g_pipe_mutex);
	iRetVal = write_safe(g_pipe[1], (char *)&tMymsg, sizeof(t_mymsg));
	pthread_mutex_unlock(&g_pipe_mutex);
	if (iRetVal < 0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_control_parklock :write pipe failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_control_parklock: write pip failed!");
        return -1;
	}

    return 0;
}


/*************************************************
Function: serv_realtimequery_parklock
Description: 控制车位锁的开关
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
int serv_realtimequery_parklock(int serial, const char *in, unsigned char *out, int len)
{
	RealTimeQueryLockReq_T  tRealTimeQueryLock    = {0};
    char   szLogInfo[1024]  = {0};
	t_3761frame tFrame      = {0};
    t_net_node *ptNetnode   = NULL;
    t_mymsg     tMymsg      = {0};
    int         iRetVal     = 0;
	int         iRetCode    = 0;      //0成功，1 id重复，2 id不存在，3服务器错误, 99其他
    t_buf      *pbuf        = NULL;
	long msgFlag = 0;
	t_msg_header header = {0};
    // json解析
    iRetVal = demux_realtimequeryparklock_req(in, &tRealTimeQueryLock);
    if (MUXDEMUX_ERR == iRetVal)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_realtimequery_parklock: exec demux_realtimequeryparklock_req failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "demux_realtimequeryparklock_req failed!");
        return -1;
    }
	
	snprintf(szLogInfo, sizeof(szLogInfo)-1, "serv_realtimequery_parklock: ParklockID=%s", tRealTimeQueryLock.szParklockID);
    WRITELOG(LOG_INFO, szLogInfo);

	// frame封装
    memset(&tFrame, 0x00, sizeof(t_3761frame));
	memcpy(tFrame.addr, tRealTimeQueryLock.szCollectorID, min(sizeof(tFrame.addr), sizeof(tRealTimeQueryLock.szCollectorID)));
    tFrame.user_data.afn = AFN_TYPE1_DATA_REQ;    // 一类数据
    tFrame.user_data.fn[0] = (1 << ((FN_LOCK_STATUS - 1) % 8));
    tFrame.user_data.fn[1] = (FN_LOCK_STATUS - 1) / 8;
    tFrame.user_data.len = strlen(tRealTimeQueryLock.szParklockID) + sizeof(t_msg_header);
	sprintf(header.serial, "%012d", serial);
	memcpy(tFrame.user_data.data, &header, sizeof(t_msg_header));
    snprintf(tFrame.user_data.data + sizeof(t_msg_header), sizeof(tFrame.user_data.data)-1-sizeof(t_msg_header), "%s", tRealTimeQueryLock.szParklockID);
	pbuf = (t_buf *)tMymsg.mtext;
	iRetVal= pack_3761frame(&tFrame, pbuf->buf, MAXLINE);
    pbuf->len = iRetVal;

    // 通过集中器ID在hash表中查找socket
    ptNetnode = find_net(tRealTimeQueryLock.szCollectorID);
    if (NULL == ptNetnode)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_realtimequery_parklock: exec find_net failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_realtimequery_parklock: find_net failed!");
        return -1;
    }
	
    pbuf->fd = ptNetnode->netinfo.fd;

    // 发送到socket发送消息队列
    tMymsg.mtype = 1;
    
	//modify by tianyu: 使用pipe
	pthread_mutex_lock(&g_pipe_mutex);
	iRetVal = write_safe(g_pipe[1], (char *)&tMymsg, sizeof(t_mymsg));
	pthread_mutex_unlock(&g_pipe_mutex);
	if (iRetVal < 0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_realtimequery_parklock :write pipe failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_realtimequery_parklock: write pipe failed!");
        return -1;
	}

    return 0;
}



/*************************************************
Function: InsertLockInfoIntoDB
Description: 将车位锁插入数据库中
Calls: 
Called By: 
Table Accessed: 
Table Updated:  
Input:  ptAddLock --车位锁信息结构体
Output: piRetVal--数据库返回信息
Return: 0--success; 其它--failure
Others: 无
*************************************************/
int InsertLockInfoIntoDB(AddLockReq_T *ptAddLock, int *piRetVal)
{  
    MYSQL     *conn_ptr = NULL;
    MYSQL_RES  result;
	MYSQL_ROW  row;

	char  szSql[512]      = {0};
	int   iRet            = 0;
	int   iPosFlag        = 0;
	int   iRowCount       = 0;
	int   iFieldCount     = 0;
	char  szLogInfo[1024] = {0};
	
	if (ptAddLock == NULL || piRetVal == NULL)
	{
	    snprintf(szLogInfo, sizeof(szLogInfo)-1, "InsertLockInfoIntoDB: input parameter(s) is NULL!");
		WRITELOG(LOG_ERROR, szLogInfo);
	
		return -1;
	}

	//初始化并连接到数据库
	initmysql();
    conn_ptr = ConnectMySql(g_Conf.tDBInfo.szIPAddr, g_Conf.tDBInfo.szUserName, g_Conf.tDBInfo.szPassword, g_Conf.tDBInfo.szDBName, g_Conf.tDBInfo.iDBPort);   // 数据库信息根据配置项修改
    if (!conn_ptr)
    {
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "InsertLockInfoIntoDB: exec ConnectMySql failed!");
		WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }

    //执行存储过程
    memset(szSql, 0x00, sizeof(szSql));
	snprintf(szSql, sizeof(szSql)-1, "call pr_addparklock('%s','%s','%s','%s','%s','%s','%s',@ret)", 
		ptAddLock->szParklockID, ptAddLock->szChargepileID, ptAddLock->szCollectorID, ptAddLock->szName, ptAddLock->szLongitude, ptAddLock->szLatitude, ptAddLock->szDetailAddress);
	iRet = ExecuteProcdure(conn_ptr, szSql, strlen(szSql), 1);
    if (iRet != 0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "InsertLockInfoIntoDB: exec ExecuteProcdure failed! iRet=%d", iRet);
		WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
	}
    else
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "InsertLockInfoIntoDB: exec ExecuteProcdure successfully!");
		WRITELOG(LOG_INFO, szLogInfo);
	}
	
	//查询存储过程返回的数据
    memset(szSql, 0x00, sizeof(szSql));
	snprintf(szSql,sizeof(szSql)-1,"select @ret");
	iRet = ExecuteSqlQuery(conn_ptr,szSql,strlen(szSql),1,&result,&iFieldCount,&iRowCount);
    if (iRet != 0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "InsertLockInfoIntoDB: exec ExecuteSqlQuery select failed! iRet=%d", iRet);
		WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
	}
    else
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "InsertLockInfoIntoDB: select success,FieldCount=%d,RowCount=%d", iFieldCount, iRowCount);
		WRITELOG(LOG_INFO, szLogInfo);
	}
    // Fetch数据
	while (row = DoFetchResult(&result))
	{
        for (iPosFlag = 0; iPosFlag < iFieldCount; iPosFlag ++)
		{
			snprintf(szLogInfo, sizeof(szLogInfo)-1, "InsertLockInfoIntoDB: row[%d]=%s", iPosFlag, row[iPosFlag]);
		    WRITELOG(LOG_INFO, szLogInfo);

			*piRetVal = atoi(row[iPosFlag]);   // 数据库执行之后的返回值
		}
	}
	
    //一定要释放结果集，否则有内存泄露
    // DoFreeMySqlResult(&result);
    //-------------------------------------------------------------------------------
    //关闭数据库
    CloseMySql(conn_ptr);
	
    return 0;
}


/*************************************************
Function: DeleteLockInfoInDB
Description: 将删除数据库中的车位锁信息
Calls: 
Called By: 
Table Accessed: 
Table Updated:  
Input:  ptDelLock --车位锁信息结构体
Output: piRetVal--数据库返回信息
Return: 0--success; 其它--failure
Others: 无
*************************************************/
int DeleteLockInfoInDB(DelLockReq_T *ptDelLock, int *piRetVal)
{  
    MYSQL     *conn_ptr = NULL;
    MYSQL_RES  result;
	MYSQL_ROW  row;

	char  szSql[512]      = {0};
	int   iRet            = 0;
	int   iPosFlag        = 0;
	int   iRowCount       = 0;
	int   iFieldCount     = 0;
	char  szLogInfo[1024] = {0};
	
	if (ptDelLock == NULL || piRetVal == NULL)
	{
	    snprintf(szLogInfo, sizeof(szLogInfo)-1, "DeleteLockInfoInDB: input parameter(s) is NULL!");
		WRITELOG(LOG_ERROR, szLogInfo);
	
		return -1;
	}

	//初始化并连接到数据库
	initmysql();
    conn_ptr = ConnectMySql(g_Conf.tDBInfo.szIPAddr, g_Conf.tDBInfo.szUserName, g_Conf.tDBInfo.szPassword, g_Conf.tDBInfo.szDBName, g_Conf.tDBInfo.iDBPort);   // 数据库信息根据配置项修改
    if (!conn_ptr)
    {
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "DeleteLockInfoInDB: exec ConnectMySql failed!");
		WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }

    //执行存储过程
    memset(szSql, 0x00, sizeof(szSql));
	snprintf(szSql, sizeof(szSql)-1, "call pr_delparklock('%s',@ret)", ptDelLock->szParklockID);
	iRet = ExecuteProcdure(conn_ptr, szSql, strlen(szSql), 1);
    if (iRet != 0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "DeleteLockInfoInDB: exec ExecuteProcdure failed! iRet=%d", iRet);
		WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
	}
    else
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "DeleteLockInfoInDB: exec ExecuteProcdure successfully!");
		WRITELOG(LOG_INFO, szLogInfo);
	}
	
	//查询存储过程返回的数据
    memset(szSql, 0x00, sizeof(szSql));
	snprintf(szSql,sizeof(szSql)-1,"select @ret");
	iRet = ExecuteSqlQuery(conn_ptr,szSql,strlen(szSql),1,&result,&iFieldCount,&iRowCount);
    if (iRet != 0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "DeleteLockInfoInDB: exec ExecuteSqlQuery select failed! iRet=%d", iRet);
		WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
	}
    else
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "DeleteLockInfoInDB: select success,FieldCount=%d,RowCount=%d", iFieldCount, iRowCount);
		WRITELOG(LOG_INFO, szLogInfo);
	}
    // Fetch数据
	while (row = DoFetchResult(&result))
	{
        for (iPosFlag = 0; iPosFlag < iFieldCount; iPosFlag ++)
		{
			snprintf(szLogInfo, sizeof(szLogInfo)-1, "DeleteLockInfoInDB: row[%d]=%s", iPosFlag, row[iPosFlag]);
		    WRITELOG(LOG_INFO, szLogInfo);

			*piRetVal = atoi(row[iPosFlag]);   // 数据库执行之后的返回值
		}
	}
	
    //一定要释放结果集，否则有内存泄露
    // DoFreeMySqlResult(&result);
    //-------------------------------------------------------------------------------
    //关闭数据库
    CloseMySql(conn_ptr);
	
    return 0;
}


/*************************************************
Function: ModifyLockInfoInDB
Description: 在数据库中修改车位锁信息
Calls: 
Called By: 
Table Accessed: 
Table Updated:  
Input:  ptModLock --车位锁信息结构体
Output: piRetVal--数据库返回信息
Return: 0--success; 其它--failure
Others: 无
*************************************************/
int ModifyLockInfoInDB(ModifyLockReq_T *ptModLock, int *piRetVal)
{  
    MYSQL     *conn_ptr = NULL;
    MYSQL_RES  result;
	MYSQL_ROW  row;

	char  szSql[512]      = {0};
	int   iRet            = 0;
	int   iPosFlag        = 0;
	int   iRowCount       = 0;
	int   iFieldCount     = 0;
	char  szLogInfo[1024] = {0};

	if (ptModLock == NULL || piRetVal == NULL)
	{
	    snprintf(szLogInfo, sizeof(szLogInfo)-1, "ModifyLockInfoInDB: input parameter(s) is NULL!");
		WRITELOG(LOG_ERROR, szLogInfo);
	
		return -1;
	}

	//初始化并连接到数据库
	initmysql();
    conn_ptr = ConnectMySql(g_Conf.tDBInfo.szIPAddr, g_Conf.tDBInfo.szUserName, g_Conf.tDBInfo.szPassword, g_Conf.tDBInfo.szDBName, g_Conf.tDBInfo.iDBPort);   // 数据库信息根据配置项修改
    if (!conn_ptr)
    {
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "ModifyLockInfoInDB: exec ConnectMySql failed!");
		WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }

	//执行存储过程
    memset(szSql, 0x00, sizeof(szSql));
	snprintf(szSql, sizeof(szSql)-1, "call pr_modifyparklock('%s','%s','%s','%s','%s',@ret)", 
		 ptModLock->szParklockID, ptModLock->szName, ptModLock->szLongitude, ptModLock->szLatitude, ptModLock->szDetailAddress);
	iRet = ExecuteProcdure(conn_ptr, szSql, strlen(szSql), 1);
    if (iRet != 0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "ModifyLockInfoInDB: exec ExecuteProcdure failed! iRet=%d", iRet);
		WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
	}
    else
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "ModifyLockInfoInDB: exec ExecuteProcdure successfully!");
		WRITELOG(LOG_INFO, szLogInfo);
	}

	//查询存储过程返回的数据
    memset(szSql, 0x00, sizeof(szSql));
	snprintf(szSql,sizeof(szSql)-1,"select @ret");
	iRet = ExecuteSqlQuery(conn_ptr,szSql,strlen(szSql),1,&result,&iFieldCount,&iRowCount);
    if (iRet != 0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "ModifyLockInfoInDB: exec ExecuteSqlQuery select failed! iRet=%d", iRet);
		WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
	}
    else
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "ModifyLockInfoInDB: select success,FieldCount=%d,RowCount=%d", iFieldCount, iRowCount);
		WRITELOG(LOG_INFO, szLogInfo);
	}

	// Fetch数据
	while (row = DoFetchResult(&result))
	{
        for (iPosFlag = 0; iPosFlag < iFieldCount; iPosFlag ++)
		{
			snprintf(szLogInfo, sizeof(szLogInfo)-1, "ModifyLockInfoInDB: row[%d]=%s", iPosFlag, row[iPosFlag]);
		    WRITELOG(LOG_INFO, szLogInfo);

			*piRetVal = atoi(row[iPosFlag]);   // 数据库执行之后的返回值
		}
	}
	
    //一定要释放结果集，否则有内存泄露
    // DoFreeMySqlResult(&result);
    //-------------------------------------------------------------------------------
    //关闭数据库
    CloseMySql(conn_ptr);

    return 0;
}


/*************************************************
Function: QueryLockInfoFromDB
Description: 从数据库中获取车位锁信息
Calls: 
Called By: 
Table Accessed: 
Table Updated:  
Input:  ptQueryLock --车位锁请求信息结构体
Output: ptQueryLockRSP --车位锁响应信息结构体
Return: 0--success; 其它--failure
Others: 无
*************************************************/
int QueryLockInfoFromDB(QueryLockReq_T *ptQueryLock, QueryLockRSP_T *ptQueryLockRSP)
{  
    MYSQL     *conn_ptr = NULL;
    MYSQL_RES  result;
	MYSQL_ROW  row;

	char  szSql[512]      = {0};
	int   iRet            = 0;
	// int   iPosFlag        = 0;
	int   iRowCount       = 0;
	int   iFieldCount     = 0;
	char  szLogInfo[1024] = {0};

	if (ptQueryLock == NULL || ptQueryLockRSP == NULL)
	{
	    snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryLockInfoFromDB: input parameter(s) is NULL!");
		WRITELOG(LOG_ERROR, szLogInfo);
	
		return -1;
	}

	//初始化并连接到数据库
	initmysql();
    conn_ptr = ConnectMySql(g_Conf.tDBInfo.szIPAddr, g_Conf.tDBInfo.szUserName, g_Conf.tDBInfo.szPassword, g_Conf.tDBInfo.szDBName, g_Conf.tDBInfo.iDBPort);   // 数据库信息根据配置项修改
    if (!conn_ptr)
    {
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryLockInfoFromDB: exec ConnectMySql failed!");
		WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }

	//执行存储过程
    memset(szSql, 0x00, sizeof(szSql));
	snprintf(szSql, sizeof(szSql)-1, "call pr_queryparklock('%s',@ret1,@ret2,@ret3,@ret4,@ret5,@ret6,@ret7,@ret8,@ret9)", ptQueryLock->szParklockID);
	iRet = ExecuteProcdure(conn_ptr, szSql, strlen(szSql), 1);
    if (iRet != 0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryLockInfoFromDB: exec ExecuteProcdure failed! iRet=%d", iRet);
		WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
	}
    else
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryLockInfoFromDB: exec ExecuteProcdure successfully!");
		WRITELOG(LOG_INFO, szLogInfo);
	}

	
	//查询存储过程返回的数据
	memset(szSql, 0x00, sizeof(szSql));
	snprintf(szSql,sizeof(szSql)-1,"select @ret1,@ret2,@ret3,@ret4,@ret5,@ret6,@ret7,@ret8,@ret9");
	iRet = ExecuteSqlQuery(conn_ptr,szSql,strlen(szSql),1,&result,&iFieldCount,&iRowCount);
	if (iRet != 0)
	{
	    snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryLockInfoFromDB: exec ExecuteSqlQuery select failed! iRet=%d", iRet);
		WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
	}
	else
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryLockInfoFromDB: select success,FieldCount=%d,RowCount=%d", iFieldCount, iRowCount);
		WRITELOG(LOG_INFO, szLogInfo);
	}

	// Fetch数据
	while (row = DoFetchResult(&result))
	{
	    /*
             for (iPosFlag = 0; iPosFlag < iFieldCount; iPosFlag ++)
		{
			snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryLockInfoFromDB: row[%d]=%s", iPosFlag, row[iPosFlag]);
		    WRITELOG(LOG_INFO, szLogInfo);

			*piRetVal = atoi(row[iPosFlag]);   // 数据库执行之后的返回值
		}
		*/

		/*
		      out out_collectorid     varchar(30),
             	out out_chargepileid    varchar(30),
             	out out_parklockid      varchar(30),
             	out out_lockname        varchar(64),
             	out out_longtitude      varchar(20),
             	out out_latitude        varchar(20),
             	out out_address         varchar(256),
             	out out_lockstatus      int,
                  out out_retval          int
		*/

		memcpy(ptQueryLockRSP->szCollectorID,   row[0], sizeof(ptQueryLockRSP->szCollectorID)-1);
		memcpy(ptQueryLockRSP->szChargepileID,  row[1], sizeof(ptQueryLockRSP->szChargepileID)-1);
		memcpy(ptQueryLockRSP->szParklockID,    row[2], sizeof(ptQueryLockRSP->szParklockID)-1);
        memcpy(ptQueryLockRSP->szName,          row[3], sizeof(ptQueryLockRSP->szName)-1);
		memcpy(ptQueryLockRSP->szLongitude,     row[4], sizeof(ptQueryLockRSP->szLongitude)-1);
		memcpy(ptQueryLockRSP->szLatitude,      row[5], sizeof(ptQueryLockRSP->szLatitude)-1);
		memcpy(ptQueryLockRSP->szDetailAddress, row[6], sizeof(ptQueryLockRSP->szDetailAddress)-1);
		ptQueryLockRSP->iLockStatus = atoi(row[7]);
		ptQueryLockRSP->iRetCode = atoi(row[8]);
	}

	if (ptQueryLockRSP->iRetCode == 0)
	{
		strncpy(ptQueryLockRSP->szRetMsg, "Query lock info successfully!", strlen("Query lock info successfully!"));
	}
	else if (ptQueryLockRSP->iRetCode == 1)
	{
		strncpy(ptQueryLockRSP->szRetMsg, "Lockid repeatted!", strlen("Lockid repeatted!"));
	}
	else if (ptQueryLockRSP->iRetCode == 2)
	{
		strncpy(ptQueryLockRSP->szRetMsg, "Lockid has not existed!", strlen("Lockid has not existed!"));
	}
	else if (ptQueryLockRSP->iRetCode == 3)
	{
		strncpy(ptQueryLockRSP->szRetMsg, "Server error!", strlen("Server error!"));
	}
	else if (ptQueryLockRSP->iRetCode == 99)
	{
		strncpy(ptQueryLockRSP->szRetMsg, "Other error!", strlen("Other error!"));
	}
	
    //一定要释放结果集，否则有内存泄露
    // DoFreeMySqlResult(&result);
    //-------------------------------------------------------------------------------
    //关闭数据库
    CloseMySql(conn_ptr);

	return 0;
}


/*************************************************
Function: QueryLockInfoGISFromDB
Description: 从数据库中获取车位锁信息(GIS)
Calls: 
Called By: 
Table Accessed: 
Table Updated:  
Input:  ptQueryLockGIS --车位锁请求信息结构体(GIS)
          json_len--json消息长度
Output: json_content--json消息内容
Return: 0--success; 其它--failure
Others: 无
*************************************************/
int QueryLockInfoGISFromDB(QueryLockGISReq_T *ptQueryLockGIS, char *json_content, int json_len)
{  
    MYSQL     *conn_ptr = NULL;
    MYSQL_RES  result;
	MYSQL_ROW  row;

	cJSON *root         = NULL;
    cJSON *json_level1  = NULL;
    cJSON *json_level2  = NULL;
    char  *out          = NULL;

	char  szSql[512]      = {0};
	int   iRet            = 0;
	int   iPosFlag        = 0;
	int   iLockInfoNum    = 0;
	int   iRowCount       = 0;
	int   iFieldCount     = 0;
	int   iRetCode        = 0;      //0成功，1无数据, 2数据库执行失败
	char  szRetMsg[256]   = {0};
	char  szLogInfo[1024] = {0};
	
	QueryLockGISRSP_T tQueryLockGISRSP[Max_ParkLock_Num] = {0};
	

	if (ptQueryLockGIS == NULL || json_content == NULL)
	{
	    snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryLockInfoGISFromDB: input parameter(s) is NULL!");
		WRITELOG(LOG_ERROR, szLogInfo);
	
		return -1;
	}

	//初始化并连接到数据库
	initmysql();
    conn_ptr = ConnectMySql(g_Conf.tDBInfo.szIPAddr, g_Conf.tDBInfo.szUserName, g_Conf.tDBInfo.szPassword, g_Conf.tDBInfo.szDBName, g_Conf.tDBInfo.iDBPort);   // 数据库信息根据配置项修改
    if (!conn_ptr)
    {
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryLockInfoGISFromDB: exec ConnectMySql failed!");
		WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }

	//执行SQL语句
    memset(szSql, 0x00, sizeof(szSql));
	snprintf(szSql, sizeof(szSql)-1, "select c.collectorid,b.pileid,a.lockid,a.lockname,a.longtitude,a.latitude,a.address,a.lockstatus from tb_lockbaseinfo a, tb_lockpilerelate b, tb_pilecollectorrelate c where (a.longtitude >= '%s' && a.longtitude <= '%s') and (a.latitude >= '%s' && a.latitude <= '%s') and (a.lockid = b.lockid) and (b.pileid = c.pileid)", 
		  ptQueryLockGIS->szStartLongitude, ptQueryLockGIS->szEndLongitude, ptQueryLockGIS->szStartLatitude, ptQueryLockGIS->szEndLatitude);
	iRet = ExecuteSqlQuery(conn_ptr,szSql,strlen(szSql),1,&result,&iFieldCount,&iRowCount);
    if (iRet != 0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryLockInfoGISFromDB: exec ExecuteSqlQuery to select failed!");
		WRITELOG(LOG_ERROR, szLogInfo);
		iRetCode = 2;
	}
    else
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryLockInfoGISFromDB: select success,FieldCount=%d,RowCount=%d", iFieldCount, iRowCount);
		WRITELOG(LOG_INFO, szLogInfo);

		
		if (iRowCount == 0) 	 // 没有查询到数据，直接返回iRetCode为1
		{
		    iRetCode = 1;
		}
		else
		{
		    // Fetch数据
			while (row = DoFetchResult(&result))
			{
				/*
				for(i = 0; i < nFieldCount; i++)
				{
					printf("%s ", row[i] ? row[i] : "NULL");
				}
				*/
				memcpy(tQueryLockGISRSP[iPosFlag].szCollectorID,   row[0], sizeof(tQueryLockGISRSP[iPosFlag].szCollectorID)-1);
				memcpy(tQueryLockGISRSP[iPosFlag].szChargepileID,  row[1], sizeof(tQueryLockGISRSP[iPosFlag].szChargepileID)-1);
                memcpy(tQueryLockGISRSP[iPosFlag].szParklockID,    row[2], sizeof(tQueryLockGISRSP[iPosFlag].szParklockID)-1);
				memcpy(tQueryLockGISRSP[iPosFlag].szName,          row[3], sizeof(tQueryLockGISRSP[iPosFlag].szName)-1);
				memcpy(tQueryLockGISRSP[iPosFlag].szLongitude,     row[4], sizeof(tQueryLockGISRSP[iPosFlag].szLongitude)-1);
				memcpy(tQueryLockGISRSP[iPosFlag].szLatitude,      row[5], sizeof(tQueryLockGISRSP[iPosFlag].szLatitude)-1);
				memcpy(tQueryLockGISRSP[iPosFlag].szDetailAddress, row[6], sizeof(tQueryLockGISRSP[iPosFlag].szDetailAddress)-1);

				tQueryLockGISRSP[iPosFlag].iLockStatus = atoi(row[7]);

				iPosFlag ++;

				if (iPosFlag >= Max_ParkLock_Num)   // 超过最大个数,则只返回Max_ParkLock_Num个车位锁信息
				{
				    break;
				}
			}

			iRetCode = 0;
    	}
	}

	iLockInfoNum = iPosFlag;

	//一定要释放结果集，否则有内存泄露
    // DoFreeMySqlResult(&result);
	//关闭数据库
    CloseMySql(conn_ptr);

	if (iRetCode == 0)
	{
		strncpy(szRetMsg, "Operate success!", strlen("Operate success!"));
	}
	else if (iRetCode == 1)
	{
		strncpy(szRetMsg, "No data!", strlen("No data!"));
	}
	else if (iRetCode == 2)
	{
		strncpy(szRetMsg, "DB exec failed!", strlen("DB exec failed!"));
	}

	// 组装JSON消息
	//--------------------
	root = cJSON_CreateObject();
    if (NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryLockInfoGISFromDB: exec cJSON_CreateObject to get root failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		
        return -1;
    }

	cJSON_AddNumberToObject(root, "retCode", iRetCode); // retCode
    
    cJSON_AddStringToObject(root, "retMsg", szRetMsg); // retMsg

	json_level1 = cJSON_CreateArray();
    if(NULL == json_level1)
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryLockInfoGISFromDB: exec cJSON_CreateArray to get json_level1 failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		
        cJSON_Delete(root);
        return -1;
    }
	cJSON_AddItemToObject(root, "chargepiles", json_level1);

	// 逐个将车位锁信息加入json消息中
	for (iPosFlag = 0; iPosFlag < iLockInfoNum; iPosFlag ++)
	{
    	json_level2 = cJSON_CreateObject();
    	if(NULL == json_level2)
    	{
    	    snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryLockInfoGISFromDB: exec cJSON_CreateObject(%d) to get json_level2 failed!", iPosFlag);
            WRITELOG(LOG_ERROR, szLogInfo);
    		
    	    cJSON_Delete(root);
            return -1;
    	}
    	cJSON_AddItemToArray(json_level1, json_level2);

		// collector_id
    	if (0 == strlen(tQueryLockGISRSP[iPosFlag].szCollectorID))
    	{
    	    cJSON_AddNullToObject(json_level2, "collector_id");
    	}
    	else
    	{
    	    cJSON_AddStringToObject(json_level2, "collector_id", tQueryLockGISRSP[iPosFlag].szCollectorID);
    	}

		// chargepile_id
    	if (0 == strlen(tQueryLockGISRSP[iPosFlag].szChargepileID))
    	{
    	    cJSON_AddNullToObject(json_level2, "chargepile_id");
    	}
    	else
    	{
    	    cJSON_AddStringToObject(json_level2, "chargepile_id", tQueryLockGISRSP[iPosFlag].szChargepileID);
    	}

		// parklock_id
    	if (0 == strlen(tQueryLockGISRSP[iPosFlag].szParklockID))
    	{
    	    cJSON_AddNullToObject(json_level2, "parklock_id");
    	}
    	else
    	{
    	    cJSON_AddStringToObject(json_level2, "parklock_id", tQueryLockGISRSP[iPosFlag].szParklockID);
    	}

		// name
    	if (0 == strlen(tQueryLockGISRSP[iPosFlag].szName))
    	{
    	    cJSON_AddNullToObject(json_level2, "name");
    	}
    	else
    	{
    	    cJSON_AddStringToObject(json_level2, "name", tQueryLockGISRSP[iPosFlag].szName);
    	}

		// longitude
    	if (0 == strlen(tQueryLockGISRSP[iPosFlag].szLongitude))
    	{
    	    cJSON_AddNullToObject(json_level2, "longitude");
    	}
    	else
    	{
    	    cJSON_AddStringToObject(json_level2, "longitude", tQueryLockGISRSP[iPosFlag].szLongitude);
    	}

		// latitude
    	if (0 == strlen(tQueryLockGISRSP[iPosFlag].szLatitude))
    	{
    	    cJSON_AddNullToObject(json_level2, "latitude");
    	}
    	else
    	{
    	    cJSON_AddStringToObject(json_level2, "latitude", tQueryLockGISRSP[iPosFlag].szLatitude);
    	}

		// detail_address
    	if (0 == strlen(tQueryLockGISRSP[iPosFlag].szDetailAddress))
    	{
    	    cJSON_AddNullToObject(json_level2, "detail_address");
    	}
    	else
    	{
    	    cJSON_AddStringToObject(json_level2, "detail_address", tQueryLockGISRSP[iPosFlag].szDetailAddress);
    	}

		// lock_status
    	if (0xffffffff == tQueryLockGISRSP[iPosFlag].iLockStatus)
    	{
    	    cJSON_AddNullToObject(json_level1, "lock_status");
    	}
    	else
    	{
    	    cJSON_AddNumberToObject(json_level1, "lock_status", tQueryLockGISRSP[iPosFlag].iLockStatus);
    	}
	}

	//--------------------

    out=cJSON_Print(root);
	strncpy(json_content, out, json_len - 1);
	json_content[json_len - 1] = '\0';

	cJSON_Delete(root);
	free(out);
	
	return 0;
}


/*************************************************
Function: UpdateLockInfoInDB
Description: 在数据库中更新车位锁状态
Calls: 
Called By: 
Table Accessed: 
Table Updated:  
Input:  ptModLock --车位锁信息结构体
Output: piRetVal--数据库返回信息
Return: 0--success; 其它--failure
Others: 无
*************************************************/
int UpdateLockInfoInDB(char *pszParklockID, int iLockStatus, int *piRetVal)
{  
    MYSQL     *conn_ptr = NULL;
    MYSQL_RES  result;
	MYSQL_ROW  row;

	char  szSql[512]      = {0};
	int   iRet            = 0;
	int   iPosFlag        = 0;
	int   iRowCount       = 0;
	int   iFieldCount     = 0;
	char  szLogInfo[1024] = {0};

	if (pszParklockID == NULL || piRetVal == NULL)
	{
	    snprintf(szLogInfo, sizeof(szLogInfo)-1, "UpdateLockInfoInDB: input parameter(s) is NULL!");
		WRITELOG(LOG_ERROR, szLogInfo);
	
		return -1;
	}

	//初始化并连接到数据库
	initmysql();
    conn_ptr = ConnectMySql(g_Conf.tDBInfo.szIPAddr, g_Conf.tDBInfo.szUserName, g_Conf.tDBInfo.szPassword, g_Conf.tDBInfo.szDBName, g_Conf.tDBInfo.iDBPort);   // 数据库信息根据配置项修改
    if (!conn_ptr)
    {
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "UpdateLockInfoInDB: exec ConnectMySql failed!");
		WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }

	//执行存储过程
    memset(szSql, 0x00, sizeof(szSql));
	snprintf(szSql, sizeof(szSql)-1, "call pr_updateparklockinfo('%s',%d,@ret)", pszParklockID, iLockStatus);
	iRet = ExecuteProcdure(conn_ptr, szSql, strlen(szSql), 1);
    if (iRet != 0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "UpdateLockInfoInDB: exec ExecuteProcdure failed! iRet=%d", iRet);
		WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
	}
    else
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "UpdateLockInfoInDB: exec ExecuteProcdure successfully!");
		WRITELOG(LOG_INFO, szLogInfo);
	}

	//查询存储过程返回的数据
    memset(szSql, 0x00, sizeof(szSql));
	snprintf(szSql,sizeof(szSql)-1,"select @ret");
	iRet = ExecuteSqlQuery(conn_ptr,szSql,strlen(szSql),1,&result,&iFieldCount,&iRowCount);
    if (iRet != 0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "UpdateLockInfoInDB: exec ExecuteSqlQuery select failed! iRet=%d", iRet);
		WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
	}
    else
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "UpdateLockInfoInDB: select success,FieldCount=%d,RowCount=%d", iFieldCount, iRowCount);
		WRITELOG(LOG_INFO, szLogInfo);
	}

	// Fetch数据
	while (row = DoFetchResult(&result))
	{
        for (iPosFlag = 0; iPosFlag < iFieldCount; iPosFlag ++)
		{
			snprintf(szLogInfo, sizeof(szLogInfo)-1, "UpdateLockInfoInDB: row[%d]=%s", iPosFlag, row[iPosFlag]);
		    WRITELOG(LOG_INFO, szLogInfo);

			*piRetVal = atoi(row[iPosFlag]);   // 数据库执行之后的返回值
		}
	}
	
    //一定要释放结果集，否则有内存泄露
    // DoFreeMySqlResult(&result);
	//-------------------------------------------------------------------------------
    //关闭数据库
    CloseMySql(conn_ptr);

    return 0;
}



/*************************************************
Function: DealLockRspMsg
Description: 组装json响应消息体(增删改使用)
Calls: 
Called By: 
Table Accessed: 
Table Updated:  
Input:  ptDealLockRSP --响应消息结构体
           json_content--json消息内容
           json_len--json消息长度
Output: 
Return: 0--success; 其它--failure
Others: 无
*************************************************/
/*
int DealLockRspMsg(DealLockRSP_T *ptDealLockRSP, char *json_content, int json_len)
{
    cJSON *root         = NULL;
    char  *out          = NULL;

	char szLogInfo[256] = {0};

	// 判断函数参数是否合法
	if (ptDealLockRSP == NULL || json_content == NULL)
	{
	    snprintf(szLogInfo, sizeof(szLogInfo)-1, "DealLockRspMsg: ptDealLockRSP or json_content is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
	
	    return -1;
	}
    
    root = cJSON_CreateObject();
    if(NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "DealLockRspMsg: exec cJSON_CreateObject to get root failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		
        return -2;
    }
	
    cJSON_AddNumberToObject(root, "retCode", ptDealLockRSP->iRetCode); // retCode
    
    cJSON_AddStringToObject(root, "retMsg", ptDealLockRSP->szRetMsg); // retMsg
	
	out=cJSON_Print(root);
	strncpy(json_content, out, json_len - 1);
	json_content[json_len - 1] = '\0';

	cJSON_Delete(root);
	free(out);

	return 0;
}
*/


/*************************************************
Function: DealQueryLockInfoRspMsg
Description: 组装json响应消息体(查询车位锁使用)
Calls: 
Called By: 
Table Accessed: 
Table Updated:  
Input:  ptDealLockRSP --响应消息结构体
           json_content--json消息内容
           json_len--json消息长度
Output: 
Return: 0--success; 其它--failure
Others: 无
*************************************************/
int DealQueryLockInfoRspMsg(QueryLockRSP_T *ptQueryLockRSP, char *json_content, int json_len)
{
    cJSON *root         = NULL;
    cJSON *json_level1  = NULL;
    cJSON *json_level2  = NULL;
    char  *out          = NULL;

	char szLogInfo[256] = {0};

	// 判断函数参数是否合法
	if (ptQueryLockRSP == NULL || json_content == NULL)
	{
	    snprintf(szLogInfo, sizeof(szLogInfo)-1, "DealQueryLockInfoRspMsg: ptQueryLockRSP or json_content is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
	
	    return -1;
	}
    
    root = cJSON_CreateObject();
    if (NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "DealQueryLockInfoRspMsg: exec cJSON_CreateObject to get root failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		
        return -1;
    }
	
    cJSON_AddNumberToObject(root, "retCode", ptQueryLockRSP->iRetCode); // retCode
    
    cJSON_AddStringToObject(root, "retMsg", ptQueryLockRSP->szRetMsg); // retMsg

	json_level1 = cJSON_CreateObject();
    if (NULL == json_level1)
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "DealQueryLockInfoRspMsg: exec cJSON_CreateObject to get json_level1 failed!");
        WRITELOG(LOG_ERROR, szLogInfo);

		cJSON_Delete(root);
		
        return -1;
    }

	cJSON_AddItemToObject(root, "chargepile", json_level1);

    // collector_id
	if (0 == strlen(ptQueryLockRSP->szCollectorID))
	{
	    cJSON_AddNullToObject(json_level1, "collector_id");
	}
	else
	{
	    cJSON_AddStringToObject(json_level1, "collector_id", ptQueryLockRSP->szCollectorID);
	}

	// chargepile_id
	if (0 == strlen(ptQueryLockRSP->szChargepileID))
	{
	    cJSON_AddNullToObject(json_level1, "chargepile_id");
	}
	else
	{
	    cJSON_AddStringToObject(json_level1, "chargepile_id", ptQueryLockRSP->szChargepileID);
	}

	// parklock_id
	if (0 == strlen(ptQueryLockRSP->szParklockID))
	{
	    cJSON_AddNullToObject(json_level1, "parklock_id");
	}
	else
	{
	    cJSON_AddStringToObject(json_level1, "parklock_id", ptQueryLockRSP->szParklockID);
	}

	// name
	if (0 == strlen(ptQueryLockRSP->szName))
	{
	    cJSON_AddNullToObject(json_level1, "name");
	}
	else
	{
	    cJSON_AddStringToObject(json_level1, "name", ptQueryLockRSP->szName);
	}

	// longitude
	if (0 == strlen(ptQueryLockRSP->szLongitude))
	{
	    cJSON_AddNullToObject(json_level1, "longitude");
	}
	else
	{
	    cJSON_AddStringToObject(json_level1, "longitude", ptQueryLockRSP->szLongitude);
	}

	// latitude
	if (0 == strlen(ptQueryLockRSP->szLatitude))
	{
	    cJSON_AddNullToObject(json_level1, "latitude");
	}
	else
	{
	    cJSON_AddStringToObject(json_level1, "latitude", ptQueryLockRSP->szLatitude);
	}

	// detail_address
	if (0 == strlen(ptQueryLockRSP->szDetailAddress))
	{
	    cJSON_AddNullToObject(json_level1, "detail_address");
	}
	else
	{
	    cJSON_AddStringToObject(json_level1, "detail_address", ptQueryLockRSP->szDetailAddress);
	}

	// lock_status
	if (0xffffffff == ptQueryLockRSP->iLockStatus)
	{
	    cJSON_AddNullToObject(json_level1, "lock_status");
	}
	else
	{
	    cJSON_AddNumberToObject(json_level1, "lock_status", ptQueryLockRSP->iLockStatus);
	}
	
	out=cJSON_Print(root);
	strncpy(json_content, out, json_len - 1);
	json_content[json_len - 1] = '\0';

	cJSON_Delete(root);
	free(out);

	return 0;
}

int verify_token_and_collector(char token[64], char collector_id[32])
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

	//初始化并连接到数据库
	initmysql();
    conn_ptr = ConnectMySql(g_Conf.tDBInfo.szIPAddr, g_Conf.tDBInfo.szUserName, g_Conf.tDBInfo.szPassword, g_Conf.tDBInfo.szDBName, g_Conf.tDBInfo.iDBPort);   // 数据库信息根据配置项修改
    if ( !conn_ptr )
    {
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "verify_token_and_collector: exec ConnectMySql failed!");
		WRITELOG(LOG_ERROR, szLogInfo);
        return 1;
    }
	sprintf(szSql, "select * from user_verify where token = '%s'", token);
	iRet = mysql_real_query(conn_ptr, szSql, strlen(szSql));
	if ( iRet != 0 )
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "verify_token_and_collector: exec mysql_real_query failed! errno[%d] errmsg[%s]", mysql_errno(conn_ptr), mysql_error(conn_ptr));
		WRITELOG(LOG_ERROR, szLogInfo);
		CloseMySql(conn_ptr);
        return 2;
	}
	result = mysql_store_result(conn_ptr);
	if (result == NULL)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "verify_token_and_collector: exec mysql_store_result failed! errno[%d] errmsg[%s]", mysql_errno(conn_ptr), mysql_error(conn_ptr));
		WRITELOG(LOG_ERROR, szLogInfo);
		CloseMySql(conn_ptr);
        return 2;
	}
	iRows = mysql_num_rows(result);
	if (iRows== 0)
	{
		DoFreeMySqlResult(result);
		CloseMySql(conn_ptr);
		return 3;
	}
	row = mysql_fetch_row(result);
    //数据库中只有一条数据
    memcpy(user_name, row[0], sizeof(user_name)-1);
	memcpy(on_line, row[3], sizeof(on_line)-1);	
	DoFreeMySqlResult(result);
	if (on_line[0] == '0')
	{
		CloseMySql(conn_ptr);
		return 4;
	}

	memset(szSql, 0, sizeof(szSql));
	sprintf(szSql, "select * from collector_user where collector_id = '%s' and user_name = '%s'", 
																collector_id, user_name);
	iRet = mysql_real_query(conn_ptr, szSql, strlen(szSql));
	if ( iRet != 0 )
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "verify_token_and_collector: exec mysql_real_query failed! errno[%d] errmsg[%s]", mysql_errno(conn_ptr), mysql_error(conn_ptr));
		WRITELOG(LOG_ERROR, szLogInfo);
		CloseMySql(conn_ptr);
        return 3;
	}
    result = mysql_store_result(conn_ptr);
	if (result == NULL)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "verify_token_and_collector: exec mysql_store_result failed! errno[%d] errmsg[%s]", mysql_errno(conn_ptr), mysql_error(conn_ptr));
		WRITELOG(LOG_ERROR, szLogInfo);
		CloseMySql(conn_ptr);
        return 3;
	}
	iRows = mysql_num_rows(result);
	if (iRows== 0)
	{
		DoFreeMySqlResult(result);
		CloseMySql(conn_ptr);
		return 5;
	}
	DoFreeMySqlResult(result);
	CloseMySql(conn_ptr);
	return 0;
}

int serv_query_parklock_status(int serial, const char *in, unsigned char *out, int len)
{
	QueryLockStatusReq_T  query_req   = {0};
	QueryLockStatusRsp_T  query_rsp = {0};
    char   szLogInfo[1024]  = {0};
	t_3761frame tFrame      = {0};
    t_net_node *ptNetnode   = NULL;
    t_mymsg     tMymsg      = {0};
    int         iRetVal     = 0;
	int         iRetCode    = 0;     
    t_buf      *pbuf        = NULL;
	long msgFlag = 0;
	t_msg_header header = {0};
	char parkLockId[32] = "xxxxxx"; //目前暂时没用，使用默认值
    // json解析
    iRetVal = demux_query_lock_status_req(in, &query_req);
    if (MUXDEMUX_ERR == iRetVal)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_query_parklock_status: exec demux_query_lock_status_req failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "demux_query_lock_status_req failed!");
        return -1;
    }
	//验证token 与集中器id
	iRetVal = verify_token_and_collector(query_req.accessToken, query_req.collector_id);
	if (iRetVal != 0)
	{
		query_rsp.iRetCode = -1;
		if (iRetVal == 1)
		{
			strcpy(query_rsp.szRetMsg, "connect database failed!");
		}else if (iRetVal == 2)
		{
			strcpy(query_rsp.szRetMsg, "query database failed!");
		}else if (iRetVal == 3)
		{
			strcpy(query_rsp.szRetMsg, "token unauthorized!");
		}else if (iRetVal == 4)
		{
			strcpy(query_rsp.szRetMsg, "user has not log in!");
		}else if (iRetVal == 5)
		{
			strcpy(query_rsp.szRetMsg, "collector_id invalid!");
		}
		
		iRetCode = mux_query_lock_status_ack(&query_rsp, out, len);
		if (MUXDEMUX_ERR == iRetCode)
    	{
        	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_query_parklock_status: exec mux_query_lock_status_ack failed!");
        	WRITELOG(LOG_ERROR, szLogInfo);
    	}
		return -1;
	}
	// frame封装
    memset(&tFrame, 0x00, sizeof(t_3761frame));
	memcpy(tFrame.addr, query_req.collector_id, min(sizeof(tFrame.addr), sizeof(query_req.collector_id)));
    tFrame.user_data.afn = AFN_TYPE1_DATA_REQ;    // 一类数据
    tFrame.user_data.fn[0] = (1 << ((FN_LOCK_STATUS_NEW - 1) % 8));
    tFrame.user_data.fn[1] = (FN_LOCK_STATUS_NEW - 1) / 8;
    tFrame.user_data.len = strlen(parkLockId) + sizeof(t_msg_header);
	sprintf(header.serial, "%012d", serial);
	memcpy(tFrame.user_data.data, &header, sizeof(t_msg_header));
    snprintf(tFrame.user_data.data + sizeof(t_msg_header), sizeof(tFrame.user_data.data)-1-sizeof(t_msg_header), "%s", parkLockId);
	pbuf = (t_buf *)tMymsg.mtext;
	iRetVal= pack_3761frame(&tFrame, pbuf->buf, MAXLINE);
    pbuf->len = iRetVal;

    // 通过集中器ID在hash表中查找socket
    ptNetnode = find_net(query_req.collector_id);
    if (NULL == ptNetnode)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_query_parklock_status: exec find_net failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_query_parklock_status: find_net failed!");
        return -1;
    }
	
    pbuf->fd = ptNetnode->netinfo.fd;

    // 发送到socket发送消息队列
    tMymsg.mtype = 1;
    
	//modify by tianyu: 使用pipe
	pthread_mutex_lock(&g_pipe_mutex);
	iRetVal = write_safe(g_pipe[1], (char *)&tMymsg, sizeof(t_mymsg));
	pthread_mutex_unlock(&g_pipe_mutex);
	if (iRetVal < 0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_query_parklock_status :write pipe failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_query_parklock_status: write pipe failed!");
        return -1;
	}

    return 0;
}


int serv_control_parklock_dev(int serial, const char *in, unsigned char *out, int len)
{
	CtlLockReq_T ctl_req = {0};
	CtlLockRsp_T ctl_rsp = {0};
    char   szLogInfo[1024] = {0};

	t_3761frame tFrame     = {0};
    t_net_node *ptNetnode  = NULL;
    t_mymsg     tMymsg     = {0};
    int         iRetVal    =  0;
	int         iRetCode   = 0;      
    t_buf      *pbuf       = NULL;
    int Ret =0;
	long msgFlag = 0;
	t_msg_header header = {0};
    // json解析
    iRetVal = demux_control_lock_req(in, &ctl_req);
    if (MUXDEMUX_ERR == iRetVal)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_control_parklock_dev: exec demux_control_lock_req failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "demux_control_lock_req failed!");
        return -1;
    }
	//验证token 与集中器id
	iRetVal = verify_token_and_collector(ctl_req.accessToken, ctl_req.collector_id);
	if (iRetVal != 0)
	{
		ctl_rsp.iRetCode = -1;
		if (iRetVal == 1)
		{
			strcpy(ctl_rsp.szRetMsg, "connect database failed!");
		}else if (iRetVal == 2)
		{
			strcpy(ctl_rsp.szRetMsg, "query database failed!");
		}else if (iRetVal == 3)
		{
			strcpy(ctl_rsp.szRetMsg, "token unauthorized!");
		}else if (iRetVal == 4)
		{
			strcpy(ctl_rsp.szRetMsg, "user has not log in!");
		}else if (iRetVal == 5)
		{
			strcpy(ctl_rsp.szRetMsg, "collector_id invalid!");
		}
		
		iRetCode = mux_control_lock_ack(&ctl_rsp, out, len);
		if (MUXDEMUX_ERR == iRetCode)
    	{
        	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_control_parklock_dev: exec mux_control_lock_ack failed!");
        	WRITELOG(LOG_ERROR, szLogInfo);
    	}
		return -1;
	}

	// frame封装
    memset(&tFrame, 0, sizeof(t_3761frame));
    memcpy(tFrame.addr, ctl_req.collector_id, min(sizeof(tFrame.addr), sizeof(ctl_req.collector_id)));
    tFrame.user_data.afn = AFN_CTRL_CMD;
    tFrame.user_data.fn[0] = (1 << ((FN_ONOFF_LOCK_NEW - 1) % 8));
    tFrame.user_data.fn[1] = (FN_ONOFF_LOCK_NEW - 1) / 8;
	/*添加消息头:消息序列号*/
    tFrame.user_data.len = sizeof(int) + sizeof(t_msg_header);
	sprintf(header.serial, "%012d", serial);
	memcpy(tFrame.user_data.data, &header, sizeof(t_msg_header));
    snprintf(tFrame.user_data.data + sizeof(t_msg_header), sizeof(tFrame.user_data.data)-1-sizeof(t_msg_header), "%d", ctl_req.lock_control);
	pbuf = (t_buf *)tMymsg.mtext;
	Ret = pack_3761frame(&tFrame, pbuf->buf, MAXLINE);
    pbuf->len = Ret;

    // 通过集中器ID在hash表中查找socket
    ptNetnode = find_net(ctl_req.collector_id);
	if (NULL == ptNetnode)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_control_parklock_dev: exec find_net failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_control_parklock_dev: find_net failed!");
        return -1;
    }
	
    pbuf->fd = ptNetnode->netinfo.fd;

    // 发送到socket发送消息队列
    tMymsg.mtype = 1;
   
	//modify by tianyu: 使用pipe
	pthread_mutex_lock(&g_pipe_mutex);
	iRetVal = write_safe(g_pipe[1], (char *)&tMymsg, sizeof(t_mymsg));
	pthread_mutex_unlock(&g_pipe_mutex);
	if (iRetVal < 0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_control_parklock_dev :write pipe failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_control_parklock_dev: write pip failed!");
        return -1;
	}

    return 0;
}




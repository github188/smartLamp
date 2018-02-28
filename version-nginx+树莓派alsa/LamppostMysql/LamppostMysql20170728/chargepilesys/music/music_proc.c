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
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "log.h"
#include "cJSON.h"
#include "frame.h"
#include "netinfo.h"
#include "netinf.h"
#include "global.h"
#include "jsondata.h"
#include "servdata.h"
#include "upserv.h"
#include "music_proc.h"
#include "gdmysql.h"
#include "config.h"

extern Config_T g_Conf;

/*************************************************
Function: serv_getplaylist_music
Description: 获取集中器本地存储的音乐文件列表
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
int serv_getplaylist_music(int serial, const char *in, unsigned char *out, int len)
{
    int ret             =   0;
    t_playlist_req playlist_req = { 0 };
    char szLogInfo[1024] = { 0 };
    t_3761frame frame   = { 0 };
    t_net_node *netnode = NULL;
    t_mymsg    mymsg    = { 0 };
    int nRecv           =   0;
    t_buf *pbuf         = NULL;
    t_play_list  play_list = { 0 };
	long msgFlag = 0;
	t_msg_header header = {0};
	if (NULL == in || NULL == out)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_getplaylist_music:NULL == in || NULL == out!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_getplaylist_music: NULL == in || NULL == out!");
        return -1;
    }
    
    // json解析
    ret = demux_playlist_req(in, &playlist_req);
    if(MUXDEMUX_ERR == ret)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_getplaylist_music:demux_playlist_req failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "demux_playlist_req failed!");
        return -1;
    }
    
    // frame封装
    memset(&frame, 0, sizeof(t_3761frame));
	memcpy(frame.addr, playlist_req.collector_id, min(sizeof(frame.addr), sizeof(playlist_req.collector_id)));
    frame.user_data.afn = AFN_TYPE1_DATA_REQ;
    frame.user_data.fn[0] = 1 << ((FN_PLAY_LIST - 1) % 8);
    frame.user_data.fn[1] = (FN_PLAY_LIST - 1) / 8;
    frame.user_data.len = sizeof(t_msg_header);
	sprintf(header.serial, "%012d", serial);
	memcpy(frame.user_data.data, &header, sizeof(t_msg_header));
	pbuf = (t_buf *)mymsg.mtext;
    ret = pack_3761frame(&frame, pbuf->buf, MAXLINE);
    if(ret < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_getplaylist_music: exec pack_3761frame failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_getplaylist_music: pack_3761frame failed!");
        return -1;
    }
    pbuf->len = ret;
    
    // 通过集中器ID在hash表中查找socket
    netnode = find_net(playlist_req.collector_id);
	if (NULL == netnode)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_playstart_music: exec find_net failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_getplaylist_music: find_net failed!");
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
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_getplaylist_music:write pipe failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_getplaylist_music: write pipe failed!");
        return -1;
	}
    return 0;
}

int QueryMusicNumFromDB(char *collector_id)
{
	int cnt = 0;
	MYSQL     *conn_ptr = NULL;
    MYSQL_RES  *result;
	int iRet = 0;
	MYSQL_ROW  row;
	char  szSql[512]      = {0};
	char szLogInfo[1024] = { 0 };
	initmysql();
    conn_ptr = ConnectMySql(g_Conf.tDBInfo.szIPAddr, g_Conf.tDBInfo.szUserName, g_Conf.tDBInfo.szPassword, g_Conf.tDBInfo.szDBName, g_Conf.tDBInfo.iDBPort);   // 数据库信息根据配置项修改
    if ( !conn_ptr )
    {
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryMusicNumFromDB: exec ConnectMySql failed!");
		WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
	sprintf(szSql, "select * from SLP_musiclist where collector_id = '%s'", collector_id);
	iRet = mysql_real_query(conn_ptr, szSql, strlen(szSql));
	if ( iRet != 0 )
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryMusicNumFromDB: exec mysql_real_query failed! errno[%d] errmsg[%s]", mysql_errno(conn_ptr), mysql_error(conn_ptr));
		WRITELOG(LOG_ERROR, szLogInfo);
		CloseMySql(conn_ptr);
        return -1;
	}
	result = mysql_store_result(conn_ptr);
	if (result == NULL)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryMusicNumFromDB: exec mysql_store_result failed! errno[%d] errmsg[%s]", mysql_errno(conn_ptr), mysql_error(conn_ptr));
		WRITELOG(LOG_ERROR, szLogInfo);
		CloseMySql(conn_ptr);
        return -1;
	}
	cnt = mysql_num_rows(result);
	DoFreeMySqlResult(result);
	CloseMySql(conn_ptr);
	return cnt;
}

int QueryMusicListFromDB(t_playlist_req *req, t_playlist_ack_db *rsp)
{
	MYSQL     *conn_ptr = NULL;
    MYSQL_RES  *result;
	MYSQL_ROW  row;

	char  szSql[512]      = {0};
	int   iRet            = 0;
	int  iRows 			  = 0;
	int index 			  = 0;
	char  szLogInfo[1024] = {0};
	if (req == NULL || rsp == NULL)
	{
	    snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryMusicListFromDB: input parameter(s) is NULL!");
		WRITELOG(LOG_ERROR, szLogInfo);
	
		return -1;
	}

	//初始化并连接到数据库
	initmysql();
    conn_ptr = ConnectMySql(g_Conf.tDBInfo.szIPAddr, g_Conf.tDBInfo.szUserName, g_Conf.tDBInfo.szPassword, g_Conf.tDBInfo.szDBName, g_Conf.tDBInfo.iDBPort);   // 数据库信息根据配置项修改
    if ( !conn_ptr )
    {
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryMusicListFromDB: exec ConnectMySql failed!");
		WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
	sprintf(szSql, "select * from SLP_musiclist where collector_id = '%s' order by music_id", req->collector_id);
	iRet = mysql_real_query(conn_ptr, szSql, strlen(szSql));
	if ( iRet != 0 )
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryMusicListFromDB: exec mysql_real_query failed! errno[%d] errmsg[%s]", mysql_errno(conn_ptr), mysql_error(conn_ptr));
		WRITELOG(LOG_ERROR, szLogInfo);
		CloseMySql(conn_ptr);
        return -1;
	}
	result = mysql_store_result(conn_ptr);
	if (result == NULL)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryMusicListFromDB: exec mysql_store_result failed! errno[%d] errmsg[%s]", mysql_errno(conn_ptr), mysql_error(conn_ptr));
		WRITELOG(LOG_ERROR, szLogInfo);
		CloseMySql(conn_ptr);
        return -1;
	}
	rsp->retcode = 0;
	iRows = mysql_num_rows(result);
	if (iRows== 0)
	{
		strcpy(rsp->retmsg, "Success, but music list is NULL!");
		rsp->filenum = 0;
		DoFreeMySqlResult(result);
		CloseMySql(conn_ptr);
		return 0;
	}
	rsp->filenum = iRows;
	strcpy(rsp->retmsg, "Success");
	while( row = mysql_fetch_row(result) )
	{
		rsp->filelist[index].music_id = atoi(row[1]);
		memcpy(rsp->filelist[index].music_name, row[2], sizeof(rsp->filelist[index].music_name) - 1);
		memcpy(rsp->filelist[index].music_length, row[3], sizeof(rsp->filelist[index].music_length) - 1);
		index++;
	}
	DoFreeMySqlResult(result);
	CloseMySql(conn_ptr);
	return 0;
}

int SelectNumFromDB(MYSQL	*conn_ptr, char *sql, int len)
{
	int iRet = 0;
	char  szLogInfo[1024] = {0};
	MYSQL_RES  *result;
	iRet = mysql_real_query(conn_ptr, sql, len);
	if ( iRet != 0 )
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "SelectNumFromDB: exec mysql_real_query failed! errno[%d] errmsg[%s]", mysql_errno(conn_ptr), mysql_error(conn_ptr));
		WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
	}
	result = mysql_store_result(conn_ptr);
	if (result == NULL)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "SelectNumFromDB: exec mysql_store_result failed! errno[%d] errmsg[%s]", mysql_errno(conn_ptr), mysql_error(conn_ptr));
		WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
	}
	iRet = mysql_num_rows(result);
	DoFreeMySqlResult(result);
	return iRet;
}

int parseFileListToDB(char *collector_id, char *fileList, int listLen)
{
	/********************************
	格式为1.wav,2.wav,3.wav...
	*********************************/
	char *pos = fileList;
	char *pos1 = fileList;
	char tmp[80] = {0};
	char file_name[64] = {0};
	char file_len[16] = {0};
	char  szSql[1024]      = {0};
	char  szLogInfo[1024] = {0};
	int index = 0;
	int len = 0;
	int cnt = 0;
	int flag = 0;
	MYSQL     *conn_ptr = NULL;
	MYSQL_RES  *result;
	if (listLen <= 0)
		return -1;
	//初始化并连接到数据库
	initmysql();
    conn_ptr = ConnectMySql(g_Conf.tDBInfo.szIPAddr, g_Conf.tDBInfo.szUserName, g_Conf.tDBInfo.szPassword, g_Conf.tDBInfo.szDBName, g_Conf.tDBInfo.iDBPort);   // 数据库信息根据配置项修改
    if ( !conn_ptr )
    {
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "QueryMusicListFromDB: exec ConnectMySql failed!");
		WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
	sprintf(szSql, "select * from SLP_musiclist where collector_id = '%s'", collector_id);
	cnt = SelectNumFromDB(conn_ptr, szSql, strlen(szSql));
	if (cnt <0)
	{
		CloseMySql(conn_ptr);
		return -1;
	}
		
	while(pos != NULL && pos1 != NULL)
	{
		pos1 = strchr(pos, ',');
		if(pos1 == NULL)
			break;
		memset(tmp, 0, sizeof(tmp));
		memset(file_name, 0, sizeof(file_name));
		memset(file_len, 0, sizeof(file_len));
		memset(szSql, 0, sizeof(szSql));
		memcpy(tmp, pos, (int)(pos1 - pos));
		sscanf(tmp, "%s %s", file_name, file_len);
		sprintf(szSql, "select * from SLP_musiclist where collector_id = '%s' and music_name = '%s'", collector_id, file_name);
		flag = SelectNumFromDB(conn_ptr, szSql, strlen(szSql));
		if (flag != 0)
			continue;
		memset(szSql, 0, sizeof(szSql));
		sprintf(szSql,"insert into SLP_musiclist values('%s', %d, '%s', '%s')", collector_id, ++cnt, file_name, file_len);
		mysql_real_query(conn_ptr, szSql, strlen(szSql));
		pos = pos1+1;
	}	
	CloseMySql(conn_ptr);
	return 0;
}

int serv_getplaylist_music_DB(int serial, const char *in, unsigned char *out, int len)
{
	int ret             =   0;
    t_playlist_req playlist_req = { 0 };
	t_playlist_ack_db playlist_ack = { 0 };
    char szLogInfo[1024] = { 0 };
  	if (NULL == in || NULL == out)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_getplaylist_music:NULL == in || NULL == out!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_getplaylist_music: NULL == in || NULL == out!");
        return -1;
    }
	// json解析
    ret = demux_playlist_req(in, &playlist_req);
    if(MUXDEMUX_ERR == ret)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_getplaylist_music:demux_playlist_req failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "demux_playlist_req failed!");
        return -1;
    }
	ret = QueryMusicListFromDB(&playlist_req, &playlist_ack);
	if (ret != 0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_getplaylist_music:exec QueryMusicListFromDB failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "QueryMusicListFromDB failed!");
        return -1;
	}
	ret =  mux_querymusiclist_ack_db(&playlist_ack, out, len);
	if (MUXDEMUX_ERR == ret)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_queryid_parklock: exec mux_queryidparklock_ack failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "mux_querymusiclist_ack_db failed!");
        return -1;
    }
	return 0;
}


/*************************************************
Function: serv_playstart_music
Description: 集中器上的启动播放
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
int serv_playstart_music(int serial, const char *in, unsigned char *out, int len)
{
    int ret             =   0;
    t_playstart_req playstart_req = { 0 };
    char szLogInfo[1024] = { 0 };
    t_3761frame frame   = { 0 };
    t_net_node *netnode = NULL;
    t_mymsg    mymsg    = { 0 };
    int nRecv           =   0;
    t_buf *pbuf          = NULL;
	//add by tianyu
	long msgFlag = 0;
	t_msg_header header = {0};
	if (NULL == in || NULL == out)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_playstart_music:NULL == in || NULL == out!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_playstart_music:NULL == in || NULL == out!");
        return -1;
    }
    
    // json解析
    ret = demux_playstart_req(in, &playstart_req);
    if(MUXDEMUX_ERR == ret)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_playstart_music:demux_playstart_req failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_playstart_music:demux_playstart_req failed!");
        return -1;
    }
    
    // frame封装
    memset(&frame, 0, sizeof(t_3761frame));
    memcpy(frame.addr, playstart_req.collector_id, min(sizeof(frame.addr), sizeof(playstart_req.collector_id)));
    frame.user_data.afn = AFN_CTRL_CMD;
    frame.user_data.fn[0] = (1 << ((FN_PLAYSTART_MUSIC - 1) % 8));
    frame.user_data.fn[1] = (FN_PLAYSTART_MUSIC - 1) / 8;
    frame.user_data.len = strlen(playstart_req.filename) + sizeof(t_msg_header);
	sprintf(header.serial, "%012d", serial);
	memcpy(frame.user_data.data, &header, sizeof(t_msg_header));
    memcpy(frame.user_data.data + sizeof(t_msg_header), playstart_req.filename, min(MAX_DATAUNIT_LEN - sizeof(t_msg_header), 
													strlen(playstart_req.filename)));
	pbuf = (t_buf *)mymsg.mtext;
	ret = pack_3761frame(&frame, pbuf->buf, MAXLINE);
    if(ret < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_playstart_music: exec pack_3761frame failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_playstart_music:pack_3761frame failed!");
        return -1;
    }
    pbuf->len = ret;
    
    // 通过集中器ID在hash表中查找socket
    netnode = find_net(playstart_req.collector_id);
    if (NULL == netnode)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_playstart_music: exec find_net failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_playstart_music:find_net failed!");
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
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_playstart_music:write pipe failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_playstart_music:write pipe failed!");
        return -1;
	}
  
    return 0;
}

/*************************************************
Function: serv_playctrl_music
Description: 集中器上的音乐播放控制
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
int serv_playctrl_music(int serial, const char *in, unsigned char *out, int len)
{
    int ret             =   0;
    t_playctrl_req playctrl_req = { 0 };
    char szLogInfo[1024] = { 0 };
    t_3761frame frame   = { 0 };
    t_net_node *netnode = NULL;
    t_mymsg    mymsg    = { 0 };
    int nRecv           =   0;
    t_buf *pbuf          = NULL;
	long msgFlag = 0;
    t_msg_header header = {0};
    // json解析
    ret = demux_playctrl_req(in, &playctrl_req);
    if(MUXDEMUX_ERR == ret)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "demux_playctrl_req failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "demux_playctrl_req failed!");
        return -1;
    }
    
    // frame封装
    memset(&frame, 0, sizeof(t_3761frame));
    memcpy(frame.addr, playctrl_req.collector_id, min(sizeof(frame.addr), sizeof(playctrl_req.collector_id)));
    frame.user_data.afn = AFN_CTRL_CMD;
    frame.user_data.fn[0] = 1 << ((FN_PLAYCTRL_MUSIC - 1) % 8);
    frame.user_data.fn[1] = (FN_PLAYCTRL_MUSIC - 1) / 8;
    frame.user_data.len = sizeof(playctrl_req.command) + sizeof(t_msg_header);
	sprintf(header.serial, "%012d", serial);
	memcpy(frame.user_data.data, &header, sizeof(t_msg_header));
    memcpy(frame.user_data.data + sizeof(t_msg_header), &playctrl_req.command, min(MAX_DATAUNIT_LEN - sizeof(t_msg_header), sizeof(playctrl_req.command)));
	pbuf = (t_buf *)mymsg.mtext;
	ret = pack_3761frame(&frame, pbuf->buf, MAXLINE);
    if(ret < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_playctrl_music: exec pack_3761frame failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_playctrl_music:pack_3761frame failed!");
        return -1;
    }
    pbuf->len = ret;
    
    // 通过集中器ID在hash表中查找socket
    netnode = find_net(playctrl_req.collector_id);
    if (NULL == netnode)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_playctrl_music: exec find_net failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_playctrl_music:find_net failed!");
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
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_playctrl_music:write pipe failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_playctrl_music:write pipe failed!");
        return -1;
	}

    //打印mymsg.mtype 和netnode->netinfo.fd的值
    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "mymsg.mtype= %d;netnode->netinfo.fd = %d;buf.fd = %d",mymsg.mtype,netnode->netinfo.fd,pbuf->fd);
    WRITELOG(LOG_INFO, szLogInfo);  
   
    return 0;
}


/*************************************************
Function: serv_setvolume_music
Description: 集中器上的播放音量设置
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
int serv_setvolume_music(int serial, const char *in, unsigned char *out, int len)
{
    int ret             =   0;
    t_setvolume_req setvolume_req = { 0 };
    char szLogInfo[1024] = { 0 };
    t_3761frame frame   = { 0 };
    t_net_node *netnode = NULL;
    t_mymsg    mymsg    = { 0 };
    int nRecv           =   0;
    t_buf *pbuf          = NULL;
	long msgFlag = 0;
    t_msg_header header = {0};
    // json解析
    ret = demux_setvolume_req(in, &setvolume_req);
    if(MUXDEMUX_ERR == ret)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "demux_setvolume_req failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "demux_setvolume_req failed!");
        return -1;
    }
    
    // frame封装
    memset(&frame, 0, sizeof(t_3761frame));
    memcpy(frame.addr, setvolume_req.collector_id, min(sizeof(frame.addr), sizeof(setvolume_req.collector_id)));
    frame.user_data.afn = AFN_CTRL_CMD;
    frame.user_data.fn[0] = 1 << ((FN_SETVOLUME_MUSIC - 1) % 8);
    frame.user_data.fn[1] = (FN_SETVOLUME_MUSIC - 1) / 8;
    frame.user_data.len = sizeof(setvolume_req.volume) + sizeof(t_msg_header);
	sprintf(header.serial, "%012d", serial);
	memcpy(frame.user_data.data, &header, sizeof(t_msg_header));
    memcpy(frame.user_data.data + sizeof(t_msg_header), &setvolume_req.volume, min(MAX_DATAUNIT_LEN - sizeof(t_msg_header), sizeof(setvolume_req.volume)));
	pbuf = (t_buf *)mymsg.mtext;
	ret = pack_3761frame(&frame, pbuf->buf, MAXLINE);
    if(ret < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "demux_setvolume_req: exec pack_3761frame failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "demux_setvolume_req: pack_3761frame failed!");
        return -1;
    }
    pbuf->len = ret;
    
    // 通过集中器ID在hash表中查找socket
    netnode = find_net(setvolume_req.collector_id);
    if (NULL == netnode)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "demux_setvolume_req: exec find_net failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "demux_setvolume_req: find_net failed!");
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
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "setvolume :write pipe failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "demux_setvolume_req: write pipe failed!");
        return -1;
	}

    return 0;
}


int serv_getvolume_music(int serial, const char *in, unsigned char *out, int len)
{
    int ret             =   0;
    t_getvolume_req getvolume_req = { 0 };
    char szLogInfo[1024] = { 0 };
    t_3761frame frame   = { 0 };
    t_net_node *netnode = NULL;
    t_mymsg    mymsg    = { 0 };
    int nRecv           =   0;
    t_buf *pbuf          = NULL;
	long msgFlag = 0;
    t_msg_header header = {0};
    // json解析
    ret = demux_getvolume_req(in, &getvolume_req);
    if(MUXDEMUX_ERR == ret)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "demux_getvolume_req failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "demux_getvolume_req failed!");
        return -1;
    }
    
    // frame封装
    memset(&frame, 0, sizeof(t_3761frame));
    memcpy(frame.addr, getvolume_req.collector_id, min(sizeof(frame.addr), sizeof(getvolume_req.collector_id)));
    frame.user_data.afn = AFN_CTRL_CMD;
    frame.user_data.fn[0] = 1 << ((FN_GETVOLUME_MUSIC - 1) % 8);
    frame.user_data.fn[1] = (FN_GETVOLUME_MUSIC - 1) / 8;
    frame.user_data.len = sizeof(t_msg_header);
	sprintf(header.serial, "%012d", serial);
	memcpy(frame.user_data.data, &header, sizeof(t_msg_header));
	pbuf = (t_buf *)mymsg.mtext;
	ret = pack_3761frame(&frame, pbuf->buf, MAXLINE);
    if(ret < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_getvolume_music: exec pack_3761frame failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_getvolume_music: pack_3761frame failed!");
        return -1;
    }
    pbuf->len = ret;
    
    // 通过集中器ID在hash表中查找socket
    netnode = find_net(getvolume_req.collector_id);
    if (NULL == netnode)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_getvolume_music: exec find_net failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_getvolume_music: find_net failed!");
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
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_getvolume_music :write pipe failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		strcpy(out, "serv_getvolume_music: write pipe failed!");
        return -1;
	}

    return 0;
}



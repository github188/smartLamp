#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <fastcgi.h>
#include <fcgiapp.h>

#include "log.h"
#include "frame.h"
#include "netinfo.h"
#include "netinf.h"
#include "global.h"
#include "upserv.h"
#include "servdata.h"
#include "jsondata.h"
#include "gdmysql.h"
#include "dealhash.h"

#if 0
t_service g_upserv_func[] =
{
    {AFN_LINK_CHK, FN_LOGIN, terminal_login_proc},
    {AFN_LINK_CHK, FN_HEARTBEAT, terminal_heartbeat_proc},
    
    {AFN_CONFIRM_DENY, FN_ONOFF_SWITCH, lamp_onoff_proc},
    {AFN_CONFIRM_DENY, FN_IMME_DIM, lamp_imme_dim_proc},
    {AFN_CONFIRM_DENY, FN_ONOFF_LOCK, parklock_onoff_proc},
    {AFN_CONFIRM_DENY, FN_DI, ent_playstart_proc},
    
    {AFN_TYPE1_DATA_REQ, FN_DIM_VAL, lamp_dim_val_proc},
    {AFN_TYPE1_DATA_REQ, FN_SWITCH_IMME_INFO, lamp_switch_imme_info_proc},

    {AFN_TYPE1_DATA_REQ, FN_ENV_T, env_temperature_proc},
    {AFN_TYPE1_DATA_REQ, FN_ENV_H, env_humidity_proc},
    {AFN_TYPE1_DATA_REQ, FN_ENV_WP, env_windpower_proc},
    {AFN_TYPE1_DATA_REQ, FN_ENV_WP, env_winddirection_proc},
    {AFN_TYPE1_DATA_REQ, FN_ENV_SR, env_sunshineradiation_proc},
    {AFN_TYPE1_DATA_REQ, FN_ENV_ALL, env_sunshineall_proc},    
    {AFN_TYPE1_DATA_REQ, FN_LOCK_STATUS, parklock_status_proc},
    {AFN_TYPE1_DATA_REQ, FN_PLAY_LIST, ent_playlist_proc},
};
#else
//modify by tianyu: 完善消息对应的响应消息
t_service g_upserv_func[] =
{
    {AFN_LINK_CHK, FN_LOGIN, terminal_login_proc},
    {AFN_LINK_CHK, FN_HEARTBEAT, terminal_heartbeat_proc},
    
    {AFN_CONFIRM_DENY, FN_ONOFF_SWITCH, lamp_onoff_proc},
    {AFN_CONFIRM_DENY, FN_IMME_DIM, lamp_imme_dim_proc},
    {AFN_CTRL_CMD, FN_ONOFF_LOCK, parklock_onoff_proc},
    {AFN_CTRL_CMD, FN_ONOFF_LOCK_NEW, parklock_onoff_proc_new},
    {AFN_CTRL_CMD, FN_PLAYSTART_MUSIC, ent_playstart_proc},
    {AFN_CTRL_CMD, FN_PLAYCTRL_MUSIC, ent_playctrl_proc},
    {AFN_CTRL_CMD, FN_SETVOLUME_MUSIC, ent_setvolume_proc},
    {AFN_CTRL_CMD, FN_GETVOLUME_MUSIC, ent_getvolume_proc},
    {AFN_TYPE1_DATA_REQ, FN_DIM_VAL, lamp_dim_val_proc},
    {AFN_TYPE1_DATA_REQ, FN_SWITCH_IMME_INFO, lamp_switch_imme_info_proc},

    {AFN_TYPE1_DATA_REQ, FN_ENV_T, env_temperature_proc},
    {AFN_TYPE1_DATA_REQ, FN_ENV_H, env_humidity_proc},
    {AFN_TYPE1_DATA_REQ, FN_ENV_WP, env_windpower_proc},
    {AFN_TYPE1_DATA_REQ, FN_ENV_WP, env_winddirection_proc},
    {AFN_TYPE1_DATA_REQ, FN_ENV_SR, env_sunshineradiation_proc},
    {AFN_TYPE1_DATA_REQ, FN_ENV_ALL, env_sunshineall_proc},    
    {AFN_TYPE1_DATA_REQ, FN_LOCK_STATUS, parklock_status_proc},
    {AFN_TYPE1_DATA_REQ, FN_LOCK_STATUS_NEW, parklock_status_proc_new},
    {AFN_TYPE1_DATA_REQ, FN_PLAY_LIST, ent_playlist_proc_db},
};
#endif

void *up_service_entry(void *arg)
{
    int nRecv    = 0;
    int ret      = 0;
    t_mymsg    mymsg = { 0 };
    t_buf     *buf   = NULL;
    char       szLogInfo[1024]  = { 0 };
    
    while (1)
    {
        nRecv = msgrcv(g_recv_msqid, &mymsg, MAXLINE, 0, 0); // 阻塞接收
        if(nRecv < 0)
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "msgrcv failed!");
            WRITELOG(LOG_ERROR, szLogInfo);
            continue;
        }

        buf = (t_buf *)mymsg.mtext;
        
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "The below information is at uperv.c[58 line]");
        WRITELOG(LOG_INFO, szLogInfo);
	    //dumpInfo(mymsg.mtext, buf->len+sizeof(int)+sizeof(int)); //the first sizeof(int) is fd value; the second sizeof(int) is data length value.
	    
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "In the up_service_entry buf->fd= %d,, buf->len = %d",buf->fd, buf->len);
        WRITELOG(LOG_INFO, szLogInfo);
        // 处理业务数据
        ret = up_service_proc(buf->fd, buf->buf, buf->len);
        if(ret != 0)
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "service proc failed!");
            WRITELOG(LOG_ERROR, szLogInfo);
            continue;
        }
    }
    
    return;
}

int up_service_proc(int fd, char *data, int len)
{
    int ret = 0;
    int i   = 0;
    t_3761frame frame = { 0 };
    char        szLogInfo[1024]  = { 0 };

	if (data == NULL)
    { 
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "up_service_proc:data == NULL!");  
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }

    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "The below information is at uperv.c[86 line]");
    WRITELOG(LOG_INFO, szLogInfo);
	//dumpInfo(data, len); 
    
    ret = parse_3761frame(data, len, &frame);
    if (ret <= 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "up_service_proc:parse frame failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }

    for(i = 0; i < sizeof(g_upserv_func) / sizeof(g_upserv_func[0]); i++)
    { 
        if((frame.user_data.afn == g_upserv_func[i].afn) 
           && (frame.user_data.fn[0] == (1 << ((g_upserv_func[i].fn - 1) % 8)))
           && (frame.user_data.fn[1] == (g_upserv_func[i].fn - 1) / 8))
        {
            ret = g_upserv_func[i].upserv_proc_func(fd, &frame);
            return ret;
        }
    }

    return 0;
}


//////////////////////////////////////////////////////////
int terminal_login_proc(int fd, t_3761frame *frame)
{
    t_net_info netinfo = { 0 };
    t_3761frame rsp_frame = {0};
	t_di_confirm_deny tconfirm_deny = {0};
    // char buf[MAXLINE] = { 0 };
    t_mymsg    mymsg = { 0 };
	t_buf*      buf   = NULL;
    int  ret         = 0;
	int  iLoginFlag  = 0;
	char di[4]       = {0x00,0x00,0x00,0x01};
	char err[3]      = {0x00,0x01,0x02};
    char szLogInfo[1024]  = { 0 };
	
	if (frame == NULL)
    { 
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "terminal_login_proc:frame == NULL!");  
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }

    memcpy(netinfo.collecor_id, frame->addr, MAX_ID_LEN);
	
	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "terminal_login_proc: collecor_id[%s]", netinfo.collecor_id);  
    WRITELOG(LOG_INFO, szLogInfo);
	
	if(NULL != find_net(netinfo.collecor_id))
    {
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "terminal_login_proc:already login!");  
	    WRITELOG(LOG_INFO, szLogInfo);
	}

    /*if(NULL != find_net(netinfo.collecor_id))
    {
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "terminal_login_proc:already login!");  
	    WRITELOG(LOG_INFO, szLogInfo);
		iLoginFlag = 1;
        // return -2;
    }
*/
    // memcpy(netinfo.collecor_id, frame->addr, MAX_ID_LEN);
    netinfo.fd = fd;
    add_net(netinfo.collecor_id, &netinfo);     // 添加到hash表中

    memcpy(rsp_frame.addr, frame->addr, MAX_ID_LEN);
    rsp_frame.user_data.afn = AFN_CONFIRM_DENY;
    rsp_frame.user_data.fn[0]  = 1 << ((FN_DI - 1) % 8);
    rsp_frame.user_data.fn[1]  = (FN_DI - 1) / 8;
	rsp_frame.user_data.len = sizeof(t_di_confirm_deny);
	memset(rsp_frame.user_data.data, 0x00, MAX_DATAUNIT_LEN);
	memcpy(tconfirm_deny.di, di, 4);
	tconfirm_deny.err = err[0];

	if (iLoginFlag == 1)    // 已经登陆了
	{
	    tconfirm_deny.err = err[1];
	}
	/*********************************************************************************
	ADD BY TIANYU: 添加查询歌曲列表功能
		如果该集中器列表为空，向集中器获取歌曲列表
	**********************************************************************************/
	ret = QueryMusicNumFromDB(netinfo.collecor_id);
	if (ret == 0)
	{
		tconfirm_deny.err = err[2];		
	}
	/*********************************************************************************
	**********************************************************************************/
	memcpy(rsp_frame.user_data.data, &tconfirm_deny, sizeof(tconfirm_deny));

	buf = (t_buf *)mymsg.mtext;
    ret = pack_3761frame(&rsp_frame, buf->buf, MAXLINE);

    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "The below information is at uperv.c[175 line]");
    WRITELOG(LOG_INFO, szLogInfo);
	dumpInfo(buf->buf, ret);    

    buf->fd  = fd;
    buf->len = ret;

    mymsg.mtype = 1;
#if 0
    ret = msgsnd(g_send_msqid, &mymsg, MAXLINE, IPC_NOWAIT);
    if(ret < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "msgsnd failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -3;
    }
#else
	//modify by tianyu: 使用pipe
	pthread_mutex_lock(&g_pipe_mutex);
	ret = write_safe(g_pipe[1], (char *)&mymsg, sizeof(t_mymsg));
	pthread_mutex_unlock(&g_pipe_mutex);
	if (ret < 0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "terminal_login_proc :write pipe failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -3;
	}
#endif    

    return 0;
}


int terminal_heartbeat_proc(int fd, t_3761frame *frame)
{
    t_net_info netinfo = { 0 };
    t_3761frame rsp_frame = {0};
	t_di_confirm_deny tconfirm_deny = {0};
    t_mymsg    mymsg = { 0 };
	t_buf*      buf   = NULL;
    int  ret         = 0;
	int  iLoginFlag  = 0;
	char di[4]       = {0x00,0x00,0x00,0x04};
	char err[2]      = {0x00,0x01};
    char szLogInfo[1024]  = { 0 };
    t_get_all_envirparam_ack envir_param_ack = { 0 };
    MYSQL_RES *res; 
    int mysql_field_num=0;
    int mysql_record_row = 0;

	if (frame == NULL)
    { 
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "terminal_heartbeat_proc:frame == NULL!");  
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
//acquire the environment paramerters 
memcpy(&envir_param_ack, frame->user_data.data, frame->user_data.len);
snprintf(szLogInfo, sizeof(szLogInfo) - 1, 
	        "serv_get_all_envir_param: retcode=%d,retmsg=%s,szFengSuValue=%s,szFengXiangValue=%s,szShiDuVlaue=%s,szWenduValue=%s,szZhaoDuVlaue=%s",
			envir_param_ack.retcode,envir_param_ack.retmsg,envir_param_ack.szFengSuValue,envir_param_ack.szFengXiangValue,
			envir_param_ack.szShiDuVlaue,envir_param_ack.szWenduValue,envir_param_ack.szZhaoDuVlaue);
	WRITELOG(LOG_INFO, szLogInfo);
// the end of acquire the environment paramerters 



if (strcmp(envir_param_ack.retmsg,"SUCCESS")==0){
// write the environment parameters into the database

//write table AllEvirParam
snprintf(szLogInfo, sizeof(szLogInfo) - 1, 
	        "insert into AllEvirParam(collector_id,Temperature,Humidity,Illumination,WindDirection,WindSpeed) values(\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\')",
			frame->addr,envir_param_ack.szWenduValue,envir_param_ack.szShiDuVlaue,envir_param_ack.szZhaoDuVlaue,envir_param_ack.szFengXiangValue,envir_param_ack.szFengSuValue);
//WRITELOG(LOG_INFO, szLogInfo);

ExecuteSql(g_db_conn,szLogInfo); 

//the end of write table AllEvirParam

//write table EvirParamUpdate

//snprintf(szLogInfo, sizeof(szLogInfo) - 1, "select * from AllEvirParam where collector_id=\'%s\'",frame->addr);
//WRITELOG(LOG_INFO, szLogInfo);
snprintf(szLogInfo, sizeof(szLogInfo) - 1, "select * from EvirParamUpdate where collector_id=\'%s\'",frame->addr);
 ret=ExecuteSql(g_db_conn,szLogInfo);// query the database
if (ret==0)
{
	res=mysql_store_result(g_db_conn); // store the results of the query
	mysql_field_num = mysql_num_fields(res);//the number of the fields
	mysql_record_row = mysql_num_rows(res);// the number of the records
	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "mysql_field_num = %d,mysql_record_row = %d",mysql_field_num,mysql_record_row);
	WRITELOG(LOG_INFO, szLogInfo);

	//add by tianyu
	DoFreeMySqlResult(res);
	
	if (mysql_record_row == 0)
	{
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, 
	                "insert into EvirParamUpdate(collector_id,Temperature,Humidity,Illumination,WindDirection,WindSpeed) values(\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\')",
	                frame->addr,envir_param_ack.szWenduValue,envir_param_ack.szShiDuVlaue,envir_param_ack.szZhaoDuVlaue,envir_param_ack.szFengXiangValue,envir_param_ack.szFengSuValue);
	    WRITELOG(LOG_INFO, szLogInfo);
	    ExecuteSql(g_db_conn,szLogInfo);// write table EvirParamUpdate
	}
	else
	{
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, 
	                "update EvirParamUpdate set Temperature =\'%s\',Humidity =\'%s\',Illumination =\'%s\',WindDirection =\'%s\',WindSpeed =\'%s\' where collector_id=\'%s\'",
	                envir_param_ack.szWenduValue,envir_param_ack.szShiDuVlaue,envir_param_ack.szZhaoDuVlaue,envir_param_ack.szFengXiangValue,envir_param_ack.szFengSuValue,frame->addr);
	    WRITELOG(LOG_INFO, szLogInfo);
	    ExecuteSql(g_db_conn,szLogInfo);// write table EvirParamUpdate
	}
}
}
//the end of write the environment parameters into the database



	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "terminal_heartbeat_proc:frame->addr=%s", frame->addr);  
    WRITELOG(LOG_INFO, szLogInfo);

    memcpy(netinfo.collecor_id, frame->addr, MAX_ID_LEN);

    if(NULL != find_net(netinfo.collecor_id))
    {
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "terminal_heartbeat_proc:find collector:%s", netinfo.collecor_id);  
	    WRITELOG(LOG_INFO, szLogInfo);
		iLoginFlag = 1;
    }
	else
	{
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "terminal_heartbeat_proc:has not login!");  
	    WRITELOG(LOG_INFO, szLogInfo);
		iLoginFlag = 0;
	}

    netinfo.fd = fd;
    add_net(netinfo.collecor_id, &netinfo);     // 添加到hash表中

    memcpy(rsp_frame.addr, frame->addr, MAX_ID_LEN);
    rsp_frame.user_data.afn = AFN_CONFIRM_DENY;    
    rsp_frame.user_data.fn[0]  = 1 << ((FN_DI - 1) % 8);
    rsp_frame.user_data.fn[1]  = (FN_DI - 1) / 8;
	rsp_frame.user_data.len = sizeof(t_di_confirm_deny);
	memset(rsp_frame.user_data.data, 0x00, MAX_DATAUNIT_LEN);
	memcpy(tconfirm_deny.di, di, 4);
	tconfirm_deny.err = err[0];

	if (iLoginFlag == 1)    // 已经登陆了
	{
	    tconfirm_deny.err = err[0];
	}
	else    // 没有登陆
	{
	    tconfirm_deny.err = err[1];
	}
	
	memcpy(rsp_frame.user_data.data, &tconfirm_deny, sizeof(tconfirm_deny));
	buf = (t_buf *)mymsg.mtext;
    ret = pack_3761frame(&rsp_frame, buf->buf, MAXLINE);

    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "The below information is at uperv.c[259 line]");
    WRITELOG(LOG_INFO, szLogInfo);
    //dumpInfo(buf->buf, ret);    ///  

    buf->fd  = fd;
    buf->len = ret;

    mymsg.mtype = 1;

#if 0
    ret = msgsnd(g_send_msqid, &mymsg, MAXLINE, IPC_NOWAIT);
    if(ret < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "terminal_heartbeat_proc:msgsnd failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -3;
    }
#else
	//modify by tianyu: 使用pipe
	pthread_mutex_lock(&g_pipe_mutex);
	ret = write_safe(g_pipe[1], (char *)&mymsg, sizeof(t_mymsg));
	pthread_mutex_unlock(&g_pipe_mutex);
	if (ret < 0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "terminal_heartbeat_proc :write pipe failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -3;
	}
#endif
    return 0;
}


int lamp_onoff_proc(int fd, t_3761frame *frame)
{
    t_mymsg    mymsg  = { 0 };
    int  ret          = 0;
    char szLogInfo[1024]  = { 0 };

	if (frame == NULL)
    { 
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "lamp_onoff_proc:frame == NULL!");  
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
	
    // TODO: 在hash表中查找对应的nginx请求，返回响应信息
#if 0
    mymsg.mtype = fd;
    memcpy(mymsg.mtext, frame->user_data.data, min(sizeof(frame->user_data.data), MAXLINE));
	
    ret = msgsnd(g_serv_msqid, &mymsg, MAXLINE, IPC_NOWAIT);
    if(ret < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "msgsnd failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
#endif    
    return 0;
}

int lamp_imme_dim_proc(int fd, t_3761frame *frame)
{
    char buf[MAXLINE] = { 0 };
    t_mymsg    mymsg  = { 0 };
    int  ret          = 0;
    char szLogInfo[1024]  = { 0 };

	if (frame == NULL)
    { 
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "lamp_imme_dim_proc:frame == NULL!");  
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
	
    // TODO: 在hash表中查找对应的nginx请求，返回响应信息
#if 0    
    mymsg.mtype = fd;
    memcpy(mymsg.mtext, frame->user_data.data, min(sizeof(frame->user_data.data), MAXLINE));
    ret = msgsnd(g_serv_msqid, &mymsg, MAXLINE, IPC_NOWAIT);
    if(ret < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "msgsnd failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -2;
    }
#endif    
    return 0;
}

void finish_reply(int serial, char *out)
{
	char rspMsg[MAXLINE] = {0};
	char szLogInfo[1024]  = { 0 };
	FCGX_Request *req = NULL;
	T_Hash_St *p_st = NULL;
	if (out == NULL)
		return;
	pthread_rwlock_wrlock(&msg_hash_lock);
	p_st = find_user(serial);
	if (p_st == NULL || p_st->req == NULL)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "finish_reply: can't find req");  
        WRITELOG(LOG_ERROR, szLogInfo);
		pthread_rwlock_unlock(&msg_hash_lock);
		return;
	}
	req = p_st->req;
	delete_user(p_st);
	pthread_rwlock_unlock(&msg_hash_lock);
	sprintf(rspMsg, "%s\r\n%s", RSP_HEADER, out);
	FCGX_FPrintF(req->out, "%s" , rspMsg);
	FCGX_Finish_r(req);
	free(req);
}

int parklock_onoff_proc(int fd, t_3761frame *frame)
{
	char buf[MAXLINE] = {0};
	char cSerial[13] = {0};
	int iSerial = 0;
	char cStatus[2] = {0};
    int     ret       = 0;
    char    szLogInfo[1024]  = { 0 };
	char out[MAXLINE] = {0};
	long msgFlag = 0;
	DealLockRSP_T     tDealLockRSP = {0};
	int iRetCode   = 0;      //0成功，1 id重复，2 id不存在，3服务器错误, 99其他
	int iRetVal = 0;
	
	if (frame == NULL)
    { 
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "parklock_onoff_proc:frame == NULL!");  
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }

	//1. 解析消息序列号	2. 在hash表中找到对应的req请求
	//3.解析消息内容     4. 响应请求
	memcpy(cSerial, frame->user_data.data, 12);
	iSerial = atoi(cSerial);
	memcpy(cStatus, frame->user_data.data + 12, 1);
	#if 0
	parse_diconfirmdeny(frame->user_data.data + 12, frame->user_data.len - 12, &di_confirm_deny_list);

	/************************************************/
	if (0 == di_confirm_deny_list.confirm_deny[0].err)
    {
        iRetCode = 0;
    }
    else
    {
        iRetCode = 99;
    }
	#endif

	
	// 开/关锁成功,更新数据库状态
	#if 0
	if (iRetCode == 0)
	{
	    iRetVal = UpdateLockInfoInDB(tControlLock.szParklockID, tControlLock.iLockCtrol, &iRetCode);
	    if (iRetVal != 0)
        {
            snprintf(szLogInfo, sizeof(szLogInfo)-1, "serv_control_parklock: exec UpdateLockInfoInDB failed!");
        	WRITELOG(LOG_ERROR, szLogInfo);
        	iRetCode = 3;   // 3-服务器错误
        }
        else
        {
            snprintf(szLogInfo, sizeof(szLogInfo)-1, "serv_control_parklock: exec UpdateLockInfoInDB successfully, RetVal=%d!", iRetVal);
        	WRITELOG(LOG_INFO, szLogInfo);
        }
	}
	#endif
    iRetCode = atoi(cStatus);   // 返回码赋值
    
	// 向请求端返回响应消息
	#if 0
	if (iRetCode == 0)
	{
		strncpy(tDealLockRSP.szRetMsg, "Operate success!", strlen("Operate success!"));
	}
	else if (iRetCode == 1)
	{
		strncpy(tDealLockRSP.szRetMsg, "Operate success, but lockid repeatted!", strlen("Operate success, but lockid repeatted!"));
	}
	else if (iRetCode == 2)
	{
		strncpy(tDealLockRSP.szRetMsg, "Operate success, but lockid has not existed!", strlen("Operate success, but lockid has not existed!"));
	}
	else if (iRetCode == 3)
	{
		strncpy(tDealLockRSP.szRetMsg, "Operate success, but server error!", strlen("Operate success, but server error!"));
	}
	else if (iRetCode == 99)
	{
		strncpy(tDealLockRSP.szRetMsg, "Operate failed!", strlen("Operate failed!"));
	}
	#endif
	if (iRetCode == 0)
	{
		tDealLockRSP.iRetCode = 0;
		strncpy(tDealLockRSP.szRetMsg, "Operate success!", strlen("Operate success!"));
	}
	else
	{
		tDealLockRSP.iRetCode = -1;
		strncpy(tDealLockRSP.szRetMsg, "Operate failed!", strlen("Operate failed!"));
	}
	// 组装响应消息
    // json封装
    iRetVal = mux_parklock_ack(&tDealLockRSP, out, sizeof(out)-1);
	if (MUXDEMUX_ERR == iRetVal)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_control_parklock: exec mux_parklock_ack failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        //return -1;
        strcpy(out, "mux_parklock_ack failed!");
    }
	
	/************************************************/
	finish_reply(iSerial,out);
		
#if 0
    msgFlag = (((fd << 16) & 0xffffffff) | (SMART_LAMP_CONTROL_PARKLOCK & 0xffff));
	mymsg.mtype = msgFlag;
	
    ret = msgsnd(g_serv_msqid, &mymsg, MAXLINE, IPC_NOWAIT);
    if(ret < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "msgsnd failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -2;
    }
#endif    
    return 0;
}

int parklock_onoff_proc_new(int fd, t_3761frame *frame)
{
	char buf[MAXLINE] = {0};
	char cSerial[13] = {0};
	int iSerial = 0;
	char cStatus[2] = {0};
    int     ret       = 0;
    char    szLogInfo[1024]  = { 0 };
	char out[MAXLINE] = {0};
	long msgFlag = 0;
	CtlLockRsp_T ctl_rsp = {0};
	int iRetCode   = 0;      
	int iRetVal = 0;
	
	if (frame == NULL)
    { 
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "parklock_onoff_proc_new: frame == NULL!");  
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }

	memcpy(cSerial, frame->user_data.data, 12);
	iSerial = atoi(cSerial);
	memcpy(cStatus, frame->user_data.data + 12, 1);

    iRetCode = atoi(cStatus);   // 返回码赋值
    
	if (iRetCode == 0)
	{
		ctl_rsp.iRetCode = 0;
		strcpy(ctl_rsp.szRetMsg, "operate success!");
	}
	else
	{
		ctl_rsp.iRetCode = -1;
		strcpy(ctl_rsp.szRetMsg, "operate failed!");
	}
	// 组装响应消息
    // json封装
    iRetVal = mux_control_lock_ack(&ctl_rsp, out, sizeof(out)-1);
	if (MUXDEMUX_ERR == iRetVal)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "parklock_onoff_proc_new: exec mux_control_lock_ack failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        strcpy(out, "mux_control_lock_ack failed!");
    }
	
	/************************************************/
	finish_reply(iSerial,out);
    return 0;
}


int ent_playstart_proc(int fd, t_3761frame *frame)
{
	char buf[MAXLINE] = {0};
	char cSerial[13] = {0};
	int iSerial = 0;
    int     ret       = 0;
    char    szLogInfo[1024]  = { 0 };
	char out[MAXLINE] = {0};
	long msgFlag = 0;
    t_playstart_ack playstart_ack = { 0 };
	int iRetCode   = 0;    
    t_di_confirm_deny_list di_confirm_deny_list = { 0 };
	
	if (frame == NULL)
    { 
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "ent_playstart_proc:frame == NULL!");  
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
  
	memcpy(cSerial, frame->user_data.data, 12);
	iSerial = atoi(cSerial);
	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "ent_playstart_proc:msg_serial[%d]", iSerial);  
    WRITELOG(LOG_INFO, szLogInfo);
	parse_diconfirmdeny(frame->user_data.data + 12, frame->user_data.len - 12, &di_confirm_deny_list);
	if(0 == di_confirm_deny_list.confirm_deny[0].err)
    {
        playstart_ack.retcode = 0;
        strncpy(playstart_ack.retmsg, "success", MAX_RETMSG_LEN);
    }
    else
    {
        playstart_ack.retcode = -1;
        strncpy(playstart_ack.retmsg, "terminal error", MAX_RETMSG_LEN);
    }

	mux_playstart_ack(&playstart_ack, out, sizeof(out)-1);
	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_playstart_music:out=%s", out);
    WRITELOG(LOG_INFO, szLogInfo);

	finish_reply(iSerial,out);
	
#if 0
	msgFlag = (((fd << 16) & 0xffffffff) | (SMART_LAMP_PLAYSTART_MUSIC & 0xffff));
	mymsg.mtype = msgFlag;
	
    ret = msgsnd(g_serv_msqid, &mymsg, min(buf->len + sizeof(int) + sizeof(int), MAXLINE), IPC_NOWAIT);
    if(ret < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "msgsnd failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -2;
    }

	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "ent_playstart_proc: mymsg.mtype=%d", mymsg.mtype);
    WRITELOG(LOG_INFO, szLogInfo);
 #endif   
    return 0;
}

//add by tianyu
int ent_playctrl_proc(int fd, t_3761frame *frame)
{
	char buf[MAXLINE] = {0};
	char cSerial[13] = {0};
	int iSerial = 0;
    int     ret       = 0;
    char    szLogInfo[1024]  = { 0 };
	char out[MAXLINE] = {0};
	long msgFlag = 0;
    t_playctrl_ack playctrl_ack = { 0 };
	int iRetCode   = 0;     
    t_di_confirm_deny_list di_confirm_deny_list = { 0 };
	
	if (frame == NULL)
	{ 
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "ent_playctrl_proc:frame == NULL!");  
		WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
	}

	memcpy(cSerial, frame->user_data.data, 12);
	iSerial = atoi(cSerial);
	parse_diconfirmdeny(frame->user_data.data + 12, frame->user_data.len - 12, &di_confirm_deny_list);
	if(0 == di_confirm_deny_list.confirm_deny[0].err)
    {
        playctrl_ack.retcode = 0;
        strncpy(playctrl_ack.retmsg, "success", MAX_RETMSG_LEN);
    }
    else
    {
        playctrl_ack.retcode = -1;
        strncpy(playctrl_ack.retmsg, "terminal error", MAX_RETMSG_LEN);
    }
	mux_playctrl_ack(&playctrl_ack, out, sizeof(out)-1);
	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_playctrl_music:out=%s", out);
    WRITELOG(LOG_INFO, szLogInfo);

	finish_reply(iSerial, out);
#if 0	
	msgFlag = (((fd << 16) & 0xffffffff) | (SMART_LAMP_PLAYCTRL_MUSIC & 0xffff));
	mymsg.mtype = msgFlag;
		
	ret = msgsnd(g_serv_msqid, &mymsg, min(buf->len + sizeof(int) + sizeof(int), MAXLINE), IPC_NOWAIT);
	if(ret < 0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "msgsnd failed!");
		WRITELOG(LOG_ERROR, szLogInfo);
		return -2;
	}
	
	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "ent_playctrl_proc: mymsg.mtype=%d", mymsg.mtype);
	WRITELOG(LOG_INFO, szLogInfo);
#endif		
	return 0;

}

int ent_setvolume_proc(int fd, t_3761frame *frame)
{
	char buf[MAXLINE] = {0};
	char cSerial[13] = {0};
	int iSerial = 0;
    int     ret       = 0;
    char    szLogInfo[1024]  = { 0 };
	char out[MAXLINE] = {0};
	long msgFlag = 0;
    t_setvolume_ack setvolume_ack = { 0 };
	int iRetCode   = 0;     
    t_di_confirm_deny_list di_confirm_deny_list = { 0 };
		
	if (frame == NULL)
	{ 
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "ent_setvolume_proc:frame == NULL!");  
		WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
	}

	memcpy(cSerial, frame->user_data.data, 12);
	iSerial = atoi(cSerial);
	parse_diconfirmdeny(frame->user_data.data + 12, frame->user_data.len - 12, &di_confirm_deny_list);
	if(0 == di_confirm_deny_list.confirm_deny[0].err)
    {
        setvolume_ack.retcode = 0;
        strncpy(setvolume_ack.retmsg, "success", MAX_RETMSG_LEN);
    }
    else
    {
        setvolume_ack.retcode = -1;
        strncpy(setvolume_ack.retmsg, "terminal error", MAX_RETMSG_LEN);
    }
	mux_setvolume_ack(&setvolume_ack, out, sizeof(out)-1);

    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "demux_setvolume_req:out=%s", out);
    WRITELOG(LOG_INFO, szLogInfo);

	finish_reply(iSerial, out);
#if 0	
	msgFlag = (((fd << 16) & 0xffffffff) | (SMART_LAMP_SETVOLUME_MUSIC & 0xffff));
	mymsg.mtype = msgFlag;
			
	ret = msgsnd(g_serv_msqid, &mymsg, min(buf->len + sizeof(int) + sizeof(int), MAXLINE), IPC_NOWAIT);
	if(ret < 0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "msgsnd failed!");
		WRITELOG(LOG_ERROR, szLogInfo);
		return -2;
	}
		
	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "ent_setvolume: mymsg.mtype=%d", mymsg.mtype);
	WRITELOG(LOG_INFO, szLogInfo);
#endif			
	return 0;

}

int ent_getvolume_proc(int fd, t_3761frame *frame)
{
	char buf[MAXLINE] = {0};
	char cSerial[13] = {0};
	char cVol[5] = {0};
	int vol = 0;
	int iSerial = 0;
    int     ret       = 0;
    char    szLogInfo[1024]  = { 0 };
	char out[MAXLINE] = {0};
	long msgFlag = 0;
    t_getvolume_ack getvolume_ack = { 0 };
	int iRetCode   = 0;     
		
	if (frame == NULL)
	{ 
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "ent_setvolume_proc:frame == NULL!");  
		WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
	}

	memcpy(cSerial, frame->user_data.data, 12);
	iSerial = atoi(cSerial);
	memcpy(cVol, frame->user_data.data + 12, frame->user_data.len - 12);
	vol = atoi(cVol);
	if (vol < 0)
	{
		getvolume_ack.retcode = -1;
		strcpy(getvolume_ack.retmsg, "failed!");
	}
	else
	{
		getvolume_ack.retcode= 0;
		strcpy(getvolume_ack.retmsg, "Success!");
		getvolume_ack.volume = vol;
	}
	mux_getvolume_ack(&getvolume_ack, out, sizeof(out)-1);
    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "demux_setvolume_req:out=%s", out);
    WRITELOG(LOG_INFO, szLogInfo);

	finish_reply(iSerial, out);			
	return 0;
}


int lamp_dim_val_proc(int fd, t_3761frame *frame)
{
    char buf[MAXLINE] = { 0 };
    t_mymsg    mymsg  = { 0 };
    int  ret          = 0;
    char szLogInfo[1024]  = { 0 };

	if (frame == NULL)
    { 
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "lamp_dim_val_proc:frame == NULL!");  
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
	// TODO: 在hash表中查找对应的nginx请求，返回响应
#if 0    
    mymsg.mtype = fd;
    memcpy(mymsg.mtext, frame->user_data.data, min(sizeof(frame->user_data.data), MAXLINE));
    ret = msgsnd(g_serv_msqid, &mymsg, MAXLINE, IPC_NOWAIT);
    if(ret < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "msgsnd failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -2;
    }
#endif    
    return 0;
}


int lamp_switch_imme_info_proc(int fd, t_3761frame *frame)
{
    char buf[MAXLINE] = { 0 };
    t_mymsg    mymsg  = { 0 };
    int  ret          = 0;
    char szLogInfo[1024]  = { 0 };

	if (frame == NULL)
    { 
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "lamp_switch_imme_info_proc:frame == NULL!");  
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
	// TODO: 在hash表中查找对应的nginx请求，返回响应
#if 0    
    mymsg.mtype = fd;
    memcpy(mymsg.mtext, frame->user_data.data, min(sizeof(frame->user_data.data), MAXLINE));
    ret = msgsnd(g_serv_msqid, &mymsg, MAXLINE, IPC_NOWAIT);
    if(ret < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "msgsnd failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -2;
    }
#endif    
    return 0;
}


int env_temperature_proc(int fd, t_3761frame *frame)
{
    char buf[MAXLINE] = { 0 };
    t_mymsg    mymsg  = { 0 };
    int  ret          = 0;
    char szLogInfo[1024] = { 0 };
    t_getenvirparam_ack envir_param_ack = { 0 };

	if (frame == NULL)
    { 
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "env_temperature_proc:frame == NULL!");  
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
    /*  zhang wei codes
    mymsg.mtype = fd;
    memcpy(mymsg.mtext, frame->user_data.data, min(sizeof(frame->user_data.data), MAXLINE));
    //memcpy(mymsg.mtext, frame->user_data.data, min(sizeof(frame->user_data.len), MAXLINE));

    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "The below information is at uperv.c[460 line]");
    WRITELOG(LOG_INFO, szLogInfo);
    dumpInfo(mymsg.mtext, min(sizeof(frame->user_data.data), MAXLINE)); 
    */

	// TODO: 从hash表中查询对应的nginx请求，返回响应
#if 0
    // gui chen codes
    mymsg.mtype = fd;
    memcpy(mymsg.mtext, frame->user_data.data, sizeof(envir_param_ack));
    
    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "The below information is at uperv.c[478 line]");
    WRITELOG(LOG_INFO, szLogInfo);
    dumpInfo(mymsg.mtext, sizeof(envir_param_ack)); 

    
    ret = msgsnd(g_serv_msqid, &mymsg, MAXLINE, IPC_NOWAIT);
    if(ret < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "msgsnd failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -2;
    }
    
    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "ent_temperature_proc: mymsg.mtype=%d", mymsg.mtype);
    WRITELOG(LOG_INFO, szLogInfo);
#endif
    return 0;
}

int env_humidity_proc(int fd, t_3761frame *frame)
{
    char buf[MAXLINE] = { 0 };
    t_mymsg    mymsg  = { 0 };
    int  ret          = 0;
    char szLogInfo[1024]  = { 0 };

	if (frame == NULL)
    { 
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "env_humidity_proc:frame == NULL!");  
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
	// TODO: 从hash表中查询对应的nginx请求，返回响应
#if 0    
    mymsg.mtype = fd;
    memcpy(mymsg.mtext, frame->user_data.data, min(sizeof(frame->user_data.data), MAXLINE));
    ret = msgsnd(g_serv_msqid, &mymsg, MAXLINE, IPC_NOWAIT);
    if(ret < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "msgsnd failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -2;
    }
#endif    
    return 0;
}

int env_windpower_proc(int fd, t_3761frame *frame)
{
    char buf[MAXLINE] = { 0 };
    t_mymsg    mymsg  = { 0 };
    int  ret          = 0;
    char szLogInfo[1024]  = { 0 };

	if (frame == NULL)
    { 
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "env_windpower_proc:frame == NULL!");  
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
    // TODO: 从hash表中查询对应的nginx请求，返回响应
#if 0    
    mymsg.mtype = fd;
    memcpy(mymsg.mtext, frame->user_data.data, min(sizeof(frame->user_data.data), MAXLINE));
    ret = msgsnd(g_serv_msqid, &mymsg, MAXLINE, IPC_NOWAIT);
    if(ret < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "msgsnd failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -2;
    }
 #endif   
    return 0;
}

int env_winddirection_proc(int fd, t_3761frame *frame)
{
    char buf[MAXLINE] = { 0 };
    t_mymsg    mymsg  = { 0 };
    int   ret         = 0;
    char  szLogInfo[1024]  = { 0 };
	
	if (frame == NULL)
    { 
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "env_winddirection_proc:frame == NULL!");  
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
    // TODO: 从hash表中查询对应的nginx请求，返回响应
#if 0 
    mymsg.mtype = fd;
    memcpy(mymsg.mtext, frame->user_data.data, min(sizeof(frame->user_data.data), MAXLINE));
    ret = msgsnd(g_serv_msqid, &mymsg, MAXLINE, IPC_NOWAIT);
    if(ret < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "msgsnd failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -2;
    }
 #endif   
    return 0;
}

int env_sunshineall_proc(int fd, t_3761frame *frame)
{
    char buf[MAXLINE] = { 0 };
    t_mymsg    mymsg  = { 0 };
    int  ret          = 0;
    char szLogInfo[1024]  = { 0 };

	if (frame == NULL)
    { 
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "env_sunshineall_proc:frame == NULL!");  
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
    // TODO: 从hash表中查询对应的nginx请求，返回响应
#if 0		
    mymsg.mtype = fd;
    memcpy(mymsg.mtext, frame->user_data.data, min(sizeof(frame->user_data.data), MAXLINE));
    ret = msgsnd(g_serv_msqid, &mymsg, MAXLINE, IPC_NOWAIT);
    if(ret < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "env_sunshineall_proc msgsnd failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -2;
    }
#endif   
    return 0;
}


int env_sunshineradiation_proc(int fd, t_3761frame *frame)
{
    char buf[MAXLINE] = { 0 };
    t_mymsg    mymsg  = { 0 };
    int  ret          = 0;
    char szLogInfo[1024]  = { 0 };

	if (frame == NULL)
    { 
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "env_sunshineradiation_proc:frame == NULL!");  
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
    // TODO: 从hash表中查询对应的nginx请求，返回响应
#if 0
    mymsg.mtype = fd;
    memcpy(mymsg.mtext, frame->user_data.data, min(sizeof(frame->user_data.data), MAXLINE));
    ret = msgsnd(g_serv_msqid, &mymsg, MAXLINE, IPC_NOWAIT);
    if(ret < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "msgsnd failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -2;
    }
#endif   
    return 0;
}

int parklock_status_proc(int fd, t_3761frame *frame)
{
    char buf[MAXLINE] = {0};
	char cSerial[13] = {0};
	char cStatus[2] = {0};
	int iSerial = 0;
    int     ret       = 0;
    char    szLogInfo[1024]  = { 0 };
	char out[MAXLINE] = {0};
	long msgFlag = 0;
	RealTimeQueryLockRSP_T  tRealTimeQueryLockRSP = {0};
	int iRetCode   = 0;      //0成功，1 id重复，2 id不存在，3服务器错误, 99其他
	int iRetVal = 0;
	
	if (frame == NULL)
    { 
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "parklock_status_proc:frame == NULL!");  
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }

	memcpy(cSerial, frame->user_data.data, 12);
	iSerial = atoi(cSerial);
	memcpy(cStatus, frame->user_data.data+12, 1);

	#if 0 
	if (strcmp(frame->user_data.data+12, "0") == 0)   // pbuf->buf为0表示车位锁打开
    {
        tRealTimeQueryLockRSP.iLockStatus = 0;
    }
	else if (strcmp(frame->user_data.data+12, "1") == 0)   // pbuf->buf为1表示车位锁关闭
    {
        tRealTimeQueryLockRSP.iLockStatus = 1;
    }
	else
	{
	    tRealTimeQueryLockRSP.iLockStatus = 99;   // pbuf->buf为其他值表示查询车位锁状态失败
	    iRetCode = 99;
	}

	// 查询状态成功,更新数据库状态
	if (tRealTimeQueryLockRSP.iLockStatus == 0 || tRealTimeQueryLockRSP.iLockStatus == 1)
	{
	    iRetVal = UpdateLockInfoInDB(tRealTimeQueryLock.szParklockID, tRealTimeQueryLockRSP.iLockStatus, &iRetCode);
	    if (iRetVal != 0)
        {
            snprintf(szLogInfo, sizeof(szLogInfo)-1, "serv_realtimequery_parklock: exec UpdateLockInfoInDB failed!");
        	WRITELOG(LOG_ERROR, szLogInfo);
        	iRetCode = 3;   // 3-服务器错误
        }
        else
        {
            snprintf(szLogInfo, sizeof(szLogInfo)-1, "serv_realtimequery_parklock: exec UpdateLockInfoInDB successfully, RetVal=%d!", iRetVal);
        	WRITELOG(LOG_INFO, szLogInfo);
        }
	}
	#endif
	
    iRetCode = atoi(cStatus);   // 返回码赋值
    
	// 向请求端返回响应消息
	#if 0
	if (iRetCode == 0)
	{
		strncpy(tRealTimeQueryLockRSP.szRetMsg, "Operate success!", strlen("Operate success!"));
	}
	else if (iRetCode == 1)
	{
		strncpy(tRealTimeQueryLockRSP.szRetMsg, "Operate success, but lockid repeatted!", strlen("Operate success, but lockid repeatted!"));
	}
	else if (iRetCode == 2)
	{
		strncpy(tRealTimeQueryLockRSP.szRetMsg, "Operate success, but lockid has not existed!", strlen("Operate success, but lockid has not existed!"));
	}
	else if (iRetCode == 3)
	{
		strncpy(tRealTimeQueryLockRSP.szRetMsg, "Operate success, but server error!", strlen("Operate success, but server error!"));
	}
	else if (iRetCode == 99)
	{
		strncpy(tRealTimeQueryLockRSP.szRetMsg, "Operate failed!", strlen("Operate failed!"));
	}
	#endif
	if (iRetCode == 0)
	{
		tRealTimeQueryLockRSP.iRetCode = 0;
		strncpy(tRealTimeQueryLockRSP.szRetMsg, "Operate success! status: opened!", strlen("Operate success! status: opened!"));
		tRealTimeQueryLockRSP.iLockStatus = 0;
	}
	else if (iRetCode == 1)
	{
		tRealTimeQueryLockRSP.iRetCode = 0;
		strncpy(tRealTimeQueryLockRSP.szRetMsg, "Operate success! status: locked!", strlen("Operate success! status: locked!"));
		tRealTimeQueryLockRSP.iLockStatus = 1;
	}
	else
	{
		tRealTimeQueryLockRSP.iRetCode = -1;
		strncpy(tRealTimeQueryLockRSP.szRetMsg, "Operate failed!", strlen("Operate failed!"));
		tRealTimeQueryLockRSP.iLockStatus = -1;
	}
	// 组装响应消息
    // json封装
    iRetVal = mux_realtimequeryparklock_ack(&tRealTimeQueryLockRSP, out, sizeof(out)-1);
	if (MUXDEMUX_ERR == iRetVal)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_realtimequery_parklock: exec mux_realtimequeryparklock_ack failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        //return -1;
        strcpy(out, "mux_realtimequeryparklock_ack failed!");
    }

	finish_reply(iSerial, out);
#if 0
	buf = (t_buf *)mymsg.mtext;
	buf->fd = fd;
    buf->len = frame->user_data.len;
    memcpy(buf->buf, frame->user_data.data, min(frame->user_data.len, MAXLINE));

    msgFlag = (((fd << 16) & 0xffffffff) | (SMART_LAMP_REALTIMEQUERY_PARKLOCK & 0xffff));
	mymsg.mtype = msgFlag;
		
    ret = msgsnd(g_serv_msqid, &mymsg, MAXLINE, IPC_NOWAIT);
    if(ret < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "msgsnd failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -2;
    }
#endif    
    return 0;
}


int parklock_status_proc_new(int fd, t_3761frame *frame)
{
    char buf[MAXLINE] = {0};
	char cSerial[13] = {0};
	char cStatus[2] = {0};
	int iSerial = 0;
    int     ret       = 0;
    char    szLogInfo[1024]  = { 0 };
	char out[MAXLINE] = {0};
	long msgFlag = 0;
	QueryLockStatusRsp_T query_rsp = {0};
	int iRetCode   = 0;    
	int iRetVal = 0;
	
	if (frame == NULL)
    { 
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "parklock_status_proc_new:frame == NULL!");  
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }

	memcpy(cSerial, frame->user_data.data, 12);
	iSerial = atoi(cSerial);
	memcpy(cStatus, frame->user_data.data+12, 1);

	
    iRetCode = atoi(cStatus);   // 返回码赋值
    
	// 向请求端返回响应消息
	
	if (iRetCode == 0)
	{
		query_rsp.iRetCode = 0;
		strcpy(query_rsp.szRetMsg, "operate success! status: open");
		query_rsp.lockStatus = 0;
	}
	else if (iRetCode == 1)
	{
		query_rsp.iRetCode = 0;
		strcpy(query_rsp.szRetMsg, "operate success! status: close");
		query_rsp.lockStatus = 1;
	}
	else
	{
		query_rsp.iRetCode = -1;
		strcpy(query_rsp.szRetMsg, "operate failed! status: unknown");
		query_rsp.lockStatus = -1;
	}
	// 组装响应消息
    // json封装
    iRetVal = mux_query_lock_status_ack(&query_rsp, out, sizeof(out)-1);
	if (MUXDEMUX_ERR == iRetVal)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "parklock_status_proc_new: exec mux_realtimequeryparklock_ack failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        strcpy(out, "mux_realtimequeryparklock_ack failed!");
    }
	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "reply[%s]", out);
    WRITELOG(LOG_ERROR, szLogInfo);
	finish_reply(iSerial, out);    
    return 0;
}


int ent_playlist_proc(int fd, t_3761frame *frame)
{
    char buf[MAXLINE] = {0};
	char cSerial[13] = {0};
	int iSerial = 0;
    int     ret       = 0;
    char    szLogInfo[2048]  = { 0 };
	char out[MAXLINE] = {0};
	long msgFlag = 0;
    t_playlist_ack playlist_ack = { 0 };
	t_play_list  play_list = { 0 };
	int iRetCode   = 0;    
    t_di_confirm_deny_list di_confirm_deny_list = { 0 };

	if (frame == NULL)
    { 
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "ent_playlist_proc:frame == NULL!");  
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
	memcpy(cSerial, frame->user_data.data, 12);
	iSerial = atoi(cSerial);
	parse_playlist(frame->user_data.data+12, frame->user_data.len-12, &play_list);

	//snprintf(szLogInfo, sizeof(szLogInfo) - 1, "parse_playlist: err[%d] len=%d list[%s]", play_list.err, atoi(play_list.len), play_list.filelist);
    //WRITELOG(LOG_INFO, szLogInfo);

	if(0 == play_list.err)
    {
        playlist_ack.retcode = 0;
        strncpy(playlist_ack.retmsg, "success", MAX_RETMSG_LEN);
        memcpy(playlist_ack.filelist, play_list.filelist, MAX_FILELIST_LEN);
    }
    else
    {
        playlist_ack.retcode = -1;
        strncpy(playlist_ack.retmsg, "terminal error", MAX_RETMSG_LEN);
    }

	//snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_playlist_music: file_list[%s]", playlist_ack.filelist);
    //WRITELOG(LOG_INFO, szLogInfo);

	mux_playlist_ack(&playlist_ack, out, sizeof(out)-1);
    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "serv_playlist_music:out=%s", out);
    WRITELOG(LOG_INFO, szLogInfo);

	finish_reply(iSerial, out);
#if 0    
	buf = (t_buf *)mymsg.mtext;
	msgFlag = (((fd << 16) & 0xffffffff) | (SMART_LAMP_GETPLAYLIST_MUSIC & 0xffff));
	mymsg.mtype = msgFlag;

	buf->fd = fd;
	buf->len = frame->user_data.len;
	memcpy(buf->buf, frame->user_data.data, min(frame->user_data.len, MAXLINE));
	
    ret = msgsnd(g_serv_msqid, &mymsg, MAXLINE, IPC_NOWAIT);
    if(ret < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "msgsnd failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -2;
    }

    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "ent_playlist_proc: mymsg.mtype=%d", mymsg.mtype);
    WRITELOG(LOG_INFO, szLogInfo);
#endif    
    return 0;
}

int ent_playlist_proc_db(int fd, t_3761frame *frame)
{
	int     ret       = 0;
	int		list_len = 0;
    char    szLogInfo[2048]  = { 0 };
	char 	buf[MAXLINE] = {0};
	char 	out[MAXLINE] = {0};
	long 	msgFlag = 0;
	char 	collector_id[MAX_ID_LEN + 1] = {0};
	t_play_list  play_list = { 0 };
	int iRetCode   = 0;    
    t_di_confirm_deny_list di_confirm_deny_list = { 0 };

	if (frame == NULL)
    { 
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "ent_playlist_proc_db:frame == NULL!");  
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
	parse_playlist(frame->user_data.data, frame->user_data.len, &play_list);

	//snprintf(szLogInfo, sizeof(szLogInfo) - 1, "parse_playlist: err[%d] len=%d list[%s]", play_list.err, atoi(play_list.len), play_list.filelist);
    //WRITELOG(LOG_INFO, szLogInfo);

	if(0 != play_list.err)
    {
      	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "ent_playlist_proc_db: terminal error!");  
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
   	list_len = atoi(play_list.len);
	memcpy(collector_id, frame->addr, MAX_ID_LEN);
	parseFileListToDB(collector_id, play_list.filelist, list_len);
    return 0;
}



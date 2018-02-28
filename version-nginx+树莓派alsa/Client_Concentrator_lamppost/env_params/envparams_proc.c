#include <stdio.h>
#include <stdlib.h>
#include "lockandsensors.h"
#include "frame.h"
#include "global.h"
#include "serv.h"
#include "envparams_proc.h"
#include "log.h"

int env_temperature_req(t_3761frame *frame, t_buf *buf)
{
    // 调用具体的业务数据处理函数
    char   szLogInfo[512] = { 0 };
    //double retemvlue = 0;

    memset(buf,0,sizeof(t_buf));
	
    Get_Temperature(&_t_getenvirparam_ack);

    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "iN env_temperature_req _t_getenvirparam_ack.szValue = %s", _t_getenvirparam_ack.szValue);
    WRITELOG(LOG_INFO, szLogInfo);

    //retemvlue = atof(_t_getenvirparam_ack.szValue);
    //snprintf(szLogInfo, sizeof(szLogInfo) - 1, "retemvlue = %f", retemvlue);
    //WRITELOG(LOG_INFO, szLogInfo);

    
    //printf("GC: Environment temperature = %s\n",_t_getenvirparam_ack.szValue);
	//buf->len = sizeof(_t_getenvirparam_ack.szValue);
	buf->len = sizeof(_t_getenvirparam_ack);
	
	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "iN env_temperature sizeof(_t_getenvirparam_ack.szValue) = %d, strlen(_t_getenvirparam_ack.szValue)= %d", buf->len,strlen(_t_getenvirparam_ack.szValue));
    WRITELOG(LOG_INFO, szLogInfo);
    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "iN env_temperature _t_getenvirparam_ack.retcode= %d, retmsg=%s,nReqType = %d,szValue = %s", _t_getenvirparam_ack.retcode,_t_getenvirparam_ack.retmsg,_t_getenvirparam_ack.nReqType,_t_getenvirparam_ack.szValue);
    WRITELOG(LOG_INFO, szLogInfo);
    
	//memcpy(buf->buf, _t_getenvirparam_ack.szValue,buf->len);
	memcpy(buf->buf, &_t_getenvirparam_ack,buf->len);

    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "The below information is in envparams_proc.c [38 line]");
    WRITELOG(LOG_INFO, szLogInfo);
    //dumpInfo(buf->buf, buf->len);
    
	
    return 0;
}

int env_temperature_ack(t_buf *buf, t_3761frame *frame)
{
    char   szLogInfo[512] = { 0 };

    memcpy(frame->addr, collector_id, MAX_ADDR_LEN);
	frame->user_data.afn = AFN_TYPE1_DATA_REQ;
    frame->user_data.fn[0] = 1 << ((FN_ENV_T - 1) % 8);
    frame->user_data.fn[1] = (FN_ENV_T - 1) / 8;
	frame->user_data.len = buf->len;
    memcpy(frame->user_data.data, buf->buf, buf->len);

    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "iN env_temperature_ack buf->len = %d ", buf->len);
    WRITELOG(LOG_INFO, szLogInfo);


    //printf("frame->user_data.afn = 0x%x\n",frame->user_data.afn);
    //printf("frame->user_data.fn[0]= %d; frame->user_data.fn[1]=%d\n",frame->user_data.fn[0],frame->user_data.fn[1]);
    return 0;
}

int env_humidity_req(t_3761frame *frame, t_buf *buf)
{
    // 调用具体的业务数据处理函数
	memset(buf,0,sizeof(t_buf));

	Get_Humidity(&_t_getenvirparam_ack);
	
	buf->len = sizeof(_t_getenvirparam_ack.szValue);
	
	memcpy(buf->buf, _t_getenvirparam_ack.szValue,buf->len);
    return 0;
}


int env_humidity_ack(t_buf *buf, t_3761frame *frame)
{

	frame->user_data.afn = AFN_TYPE1_DATA_REQ;
 	frame->user_data.fn[0] = 1 << ((FN_ENV_H - 1) % 8);
   	frame->user_data.fn[1] = (FN_ENV_H - 1) / 8;
	frame->user_data.len = buf->len;
    	memcpy(frame->user_data.data, buf->buf, buf->len);
    return 0;
}

int env_windpower_req(t_3761frame *frame, t_buf *buf)
{
    // 调用具体的业务数据处理函数
	memset(buf,0,sizeof(t_buf));

	Get_WindSpeed(&_t_getenvirparam_ack);
	
	buf->len = sizeof(_t_getenvirparam_ack.szValue);
	
	memcpy(buf->buf, _t_getenvirparam_ack.szValue,buf->len);
    return 0;
}

int env_windpower_ack(t_buf *buf, t_3761frame *frame)
{

	frame->user_data.afn = AFN_TYPE1_DATA_REQ;
    	frame->user_data.fn[0] = 1 << ((FN_ENV_WP- 1) % 8);
    	frame->user_data.fn[1] = (FN_ENV_WP - 1) / 8;
	frame->user_data.len = buf->len;
    	memcpy(frame->user_data.data, buf->buf, buf->len);
    return 0;
}

int env_winddirection_req(t_3761frame *frame, t_buf *buf)
{
    // 调用具体的业务数据处理函数
	memset(buf,0,sizeof(t_buf));

	Get_WindDireciton(&_t_getenvirparam_ack);
	
	buf->len = strlen(_t_getenvirparam_ack.szValue);
	
	memcpy(buf->buf, _t_getenvirparam_ack.szValue,buf->len);
    return 0;
}

int env_winddirection_ack(t_buf *buf, t_3761frame *frame)
{

	frame->user_data.afn = AFN_TYPE1_DATA_REQ;
    	frame->user_data.fn[0] = 1 << ((FN_ENV_WD - 1) % 8);
    	frame->user_data.fn[1] = (FN_ENV_WD - 1) / 8;
	frame->user_data.len = buf->len;
    	memcpy(frame->user_data.data, buf->buf, buf->len);
    return 0;
}

int env_sunshineradiation_req(t_3761frame *frame, t_buf *buf)
{
    // 调用具体的业务数据处理函数
	memset(buf,0,sizeof(t_buf));

	Get_Illumination(&_t_getenvirparam_ack);
	
	buf->len = sizeof(_t_getenvirparam_ack.szValue);
	
	memcpy(buf->buf, _t_getenvirparam_ack.szValue,buf->len);
    return 0;
}

int env_sunshineradiation_ack(t_buf *buf, t_3761frame *frame)
{

	frame->user_data.afn = AFN_TYPE1_DATA_REQ;
    	frame->user_data.fn[0] = 1 << ((FN_ENV_SR  - 1) % 8);
    	frame->user_data.fn[1] = (FN_ENV_SR  - 1) / 8;
	frame->user_data.len = buf->len;
    	memcpy(frame->user_data.data, buf->buf, buf->len);
    return 0;
}

int env_all_req(t_3761frame *frame, t_buf *buf)
{
    // 调用具体的业务数据处理函数
    char   szLogInfo[512] = { 0 };
	memset(buf,0,sizeof(t_buf));

	Get_All(&_t_get_all_envirparam_ack);
	
	buf->len = sizeof(_t_get_all_envirparam_ack);
	
	memcpy(buf->buf, &_t_get_all_envirparam_ack,buf->len);
    return 0;
}

int env_all_ack(t_buf *buf, t_3761frame *frame)
{
    char   szLogInfo[512] = { 0 };
    
     memcpy(frame->addr, collector_id, MAX_ADDR_LEN);

	frame->user_data.afn = AFN_TYPE1_DATA_REQ;
    frame->user_data.fn[0] = 1 << ((FN_ENV_ALL - 1) % 8);
    frame->user_data.fn[1] = (FN_ENV_ALL - 1) / 8;
	frame->user_data.len = buf->len;
    memcpy(frame->user_data.data, buf->buf, buf->len);
    return 0;
}

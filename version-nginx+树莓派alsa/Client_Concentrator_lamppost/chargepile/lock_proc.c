#include <stdio.h>
#include <stdlib.h>
#include "lockandsensors.h"
#include "frame.h"
#include "global.h"
#include "serv.h"
#include "lock_proc.h"
#include "log.h"

int parklock_onoff_req(t_3761frame *frame, t_buf *buf)
{
    // 调用具体的业务数据处理函数
    char   szLogInfo[512] = { 0 };
    int iRet = 0;
    memset(buf,0,sizeof(t_buf));
    //12位的消息序列号
	if(strcmp("0",frame->user_data.data + 12)==0)
    {
    	OpenBuzzer();
		iRet = OpenLock();
		
		buf->len = 1;
		if (iRet == 0)
		{
			//操作成功
			memcpy(buf->buf,"0" ,buf->len);
		}
		else
		{
			//操作失败
			memcpy(buf->buf,"1" ,buf->len);
		}
    }
	else if(strcmp("1",frame->user_data.data + 12)==0)
	{
        OpenBuzzer();
	    iRet = CloseLock();
	    
	    buf->len = 1;
		if (iRet == 0)
			memcpy(buf->buf,"0" ,buf->len);
		else
			memcpy(buf->buf,"1" ,buf->len);
	}
	else
	{
		printf("Park lock on/off failed!\n");
		buf->len = 1;
		memcpy(buf->buf,"1" ,buf->len);
	}
	
    return 0;
}

int parklock_onoff_ack(t_buf *buf, t_3761frame *frame)
{

    memcpy(frame->addr, collector_id, MAX_ADDR_LEN);
	frame->user_data.afn = AFN_CTRL_CMD;
    frame->user_data.fn[0] = 1 << ((FN_ONOFF_LOCK - 1) % 8);
    frame->user_data.fn[1] = (FN_ONOFF_LOCK - 1) / 8;
	frame->user_data.len = buf->len+12;
	//保留原消息序列号
	memset(frame->user_data.data+12, 0, sizeof(frame->user_data.data)-12);
    memcpy(frame->user_data.data+12, buf->buf, buf->len);
		
	return 0;
}

int parklock_onoff_ack_new(t_buf *buf, t_3761frame *frame)
{
    memcpy(frame->addr, collector_id, MAX_ADDR_LEN);
	frame->user_data.afn = AFN_CTRL_CMD;
    frame->user_data.fn[0] = 1 << ((FN_ONOFF_LOCK_NEW - 1) % 8);
    frame->user_data.fn[1] = (FN_ONOFF_LOCK_NEW - 1) / 8;
	frame->user_data.len = buf->len+12;
	//保留原消息序列号
	memset(frame->user_data.data+12, 0, sizeof(frame->user_data.data)-12);
    memcpy(frame->user_data.data+12, buf->buf, buf->len);
		
	return 0;
}


int parklock_status_req(t_3761frame *frame, t_buf *buf)
{
    // 调用具体的业务数据处理函数
    memset(buf,0,sizeof(t_buf));
	char parklock_id[32] = {0};  //车位锁ID， 目前暂时没用
	int msg_len = frame->user_data.len;
	int status = 0;
	char cStatus[9] = {0}; 
	memcpy(parklock_id, frame->user_data.data + 12, msg_len - 12);

	GetLockStatus(parklock_id, &status); 
	if (0 == status)
	{
		//开锁
	}
	else if (1 == status)
	{
		//闭锁
	}
	else
	{
		//其他
		status = 3;
	}
	sprintf(cStatus, "%d", status);	
	buf->len = 1;
	memcpy(buf->buf, cStatus, 1);

    return 0;
}

int parklock_status_ack(t_buf *buf, t_3761frame *frame)
{
	frame->user_data.afn = AFN_TYPE1_DATA_REQ;
    frame->user_data.fn[0] = 1 << ((FN_LOCK_STATUS - 1) % 8);
    frame->user_data.fn[1] = (FN_LOCK_STATUS - 1) / 8;
	frame->user_data.len = buf->len + 12;
    memcpy(frame->user_data.data + 12, buf->buf, buf->len);
    return 0;
}

int parklock_status_ack_new(t_buf *buf, t_3761frame *frame)
{
	frame->user_data.afn = AFN_TYPE1_DATA_REQ;
    frame->user_data.fn[0] = 1 << ((FN_LOCK_STATUS_NEW - 1) % 8);
    frame->user_data.fn[1] = (FN_LOCK_STATUS_NEW - 1) / 8;
	frame->user_data.len = buf->len + 12;
    memcpy(frame->user_data.data + 12, buf->buf, buf->len);
    return 0;
}



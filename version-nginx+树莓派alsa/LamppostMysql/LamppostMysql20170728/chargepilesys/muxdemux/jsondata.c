#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "jsondata.h"
#include "log.h"

/**************获取播放列表请求json格式*******************
{
    collector_id:"0123456789",              // 集中器ID
}
*****************************************************/
// 获取播放列表请求json解析
int demux_playlist_req(const char *json_content, t_playlist_req *playlist_req)
{
    cJSON *root         = NULL;
    cJSON *json_level1  = NULL;
	char szLogInfo[256] = {0};
	
	// 判断函数参数是否合法
	if ((NULL == json_content) || (NULL == playlist_req))
	{
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "json_content or playlist_req is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
	    return MUXDEMUX_ERR;
	}

    root = cJSON_Parse(json_content);
    if(NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "call cJSON_Parse failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }

    // 解析collector_id
    json_level1 = cJSON_GetObjectItem(root, "collector_id");
    if(NULL == json_level1)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "not find collector_id!");
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }
    if(cJSON_String == json_level1->type)
    {
        strncpy(playlist_req->collector_id, json_level1->valuestring, MAX_ID_LEN);
        playlist_req->collector_id[MAX_ID_LEN] = '\0';
    }
    else
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "collector_id type is %d, not expected type!", json_level1->type);
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }

    cJSON_Delete(root);
    return MUXDEMUX_OK;
}

/**************获取播放列表响应json格式*******************
{
    retcode:0,              // 结果码
	retmsg:"success"        // 结果信息
	filelist:"1.wav,2.wav,3.wav,4.wav,5.wav"  // 文件名集合，用','分隔
}
*****************************************************/
// 获取播放列表响应json封装
int mux_playlist_ack(t_playlist_ack *playlist_ack, char *json_content, int json_len)
{
    cJSON *root         = NULL;
    char  *out          = NULL;
	char szLogInfo[256] = {0};
	
	// 判断函数参数是否合法
	if ((NULL == playlist_ack) || (NULL == json_content))
	{
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "playlist_ack or json_content is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
	    return MUXDEMUX_ERR;
	}
    
    root = cJSON_CreateObject();
    if(NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "exec cJSON_CreateObject to get root failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return MUXDEMUX_ERR;
    }
    cJSON_AddNumberToObject(root, "retCode", playlist_ack->retcode);
    cJSON_AddStringToObject(root, "retMsg", playlist_ack->retmsg);
	cJSON_AddStringToObject(root, "fileList", playlist_ack->filelist);
	
	out=cJSON_Print(root);
	strncpy(json_content, out, json_len - 1);
	json_content[json_len - 1] = '\0';

	cJSON_Delete(root);
	free(out);
	
    return MUXDEMUX_OK;
}

//从数据库获取播放列表响应json封装
int mux_querymusiclist_ack_db(t_playlist_ack_db *playlist_ack, char *json_content, int json_len)
{
	cJSON *root         = NULL;
	cJSON * json_level1 = NULL;
	cJSON *json_level2  = NULL;
    char  *out          = NULL;
	int index = 0;
	char szLogInfo[256] = {0};
	// 判断函数参数是否合法
	if ((NULL == playlist_ack) || (NULL == json_content))
	{
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "playlist_ack or json_content is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
	    return MUXDEMUX_ERR;
	}
	root = cJSON_CreateObject();
    if (NULL == root)
    {
    	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "exec cJSON_CreateObject to get root failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return MUXDEMUX_ERR;
    }
	cJSON_AddNumberToObject(root, "retCode", playlist_ack->retcode);
	cJSON_AddStringToObject(root, "retMsg", playlist_ack->retmsg);
	cJSON_AddNumberToObject(root, "fileNum", playlist_ack->filenum);
	json_level1 = cJSON_CreateArray();
	cJSON_AddItemToObject(root, "fileList", json_level1);
	for (index = 0; index < playlist_ack->filenum; index++ )
	{
		json_level2 = cJSON_CreateObject();
		cJSON_AddItemToArray(json_level1, json_level2);
		cJSON_AddNumberToObject(json_level2, "fileId", playlist_ack->filelist[index].music_id);
        cJSON_AddStringToObject(json_level2, "fileName", playlist_ack->filelist[index].music_name);
        cJSON_AddStringToObject(json_level2, "fileLength", playlist_ack->filelist[index].music_length);
	}
	out = cJSON_Print(root);
	strncpy(json_content, out, json_len - 1);
	json_content[json_len - 1] = '\0';
	cJSON_Delete(root); 
	free(out);
	return MUXDEMUX_OK;
}


/**************开始播放请求json格式*******************
{
    collector_id:"0123456789",              // 集中器ID
	filename:"renshenghechubuxiangfeng.wav" // 播放文件名
}
*****************************************************/
// 开始播放请求json解析
int demux_playstart_req(const char *json_content, t_playstart_req *playstart_req)
{
	cJSON *root         = NULL;
    cJSON *json_level1  = NULL;
	char szLogInfo[256] = {0};
	
	// 判断函数参数是否合法
	if ((NULL == json_content) || (NULL == playstart_req))
	{
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "json_content or playstart_req is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
	    return MUXDEMUX_ERR;
	}

	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "demux_playstart_req:json_content=%s",json_content);
    WRITELOG(LOG_INFO, szLogInfo);

    root = cJSON_Parse(json_content);
    if(NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "call cJSON_Parse failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }

    // 解析collector_id
    json_level1 = cJSON_GetObjectItem(root, "collector_id");
    if(NULL == json_level1)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "not find collector_id!");
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }
    if(cJSON_String == json_level1->type)
    {
        strncpy(playstart_req->collector_id, json_level1->valuestring, MAX_ID_LEN);
        playstart_req->collector_id[MAX_ID_LEN] = '\0';
    }
    else
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "collector_id type is %d, not expected type!", json_level1->type);
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }

    // 解析filename
    json_level1 = cJSON_GetObjectItem(root, "filename");
    if(NULL == json_level1)
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "not find filename!");
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }
    if (cJSON_String == json_level1->type)
    {
        strncpy(playstart_req->filename, json_level1->valuestring, MAX_FILENAME_LEN);
        playstart_req->filename[MAX_FILENAME_LEN] = '\0';
    }
    else
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "filename type is %d, not expected type!", json_level1->type);
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }

    cJSON_Delete(root);
    return MUXDEMUX_OK;
}



/**************开始播放响应json格式*******************
{
    retcode:0,              // 结果码
	retmsg:"success"        // 结果信息
}
*****************************************************/
// 开始播放响应json封装
int mux_playstart_ack(t_playstart_ack *playstart_ack, char *json_content, int json_len)
{
    cJSON *root         = NULL;
    char  *out          = NULL;
	char szLogInfo[256] = {0};
	
	// 判断函数参数是否合法
	if ((NULL == playstart_ack) || (NULL == json_content))
	{
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "playstart_ack or json_content is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
	    return MUXDEMUX_ERR;
	}
    
    root = cJSON_CreateObject();
    if(NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "exec cJSON_CreateObject to get root failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return MUXDEMUX_ERR;
    }
    cJSON_AddNumberToObject(root, "retCode", playstart_ack->retcode);
    cJSON_AddStringToObject(root, "retMsg", playstart_ack->retmsg);
	
	out=cJSON_Print(root);
	strncpy(json_content, out, json_len - 1);
	json_content[json_len - 1] = '\0';

	cJSON_Delete(root);
	free(out);

	return MUXDEMUX_OK;
}

/**************播放控制请求json格式*******************
{
    collector_id:"0123456789",              // 集中器ID
	command:1                               // 播放控制命令
}
*****************************************************/
// 播放控制请求json解析
int demux_playctrl_req(const char *json_content, t_playctrl_req *playctrl_req)
{
    cJSON *root         = NULL;
    cJSON *json_level1  = NULL;
	char szLogInfo[256] = {0};
	
	// 判断函数参数是否合法
	if ((NULL == json_content) || (NULL == playctrl_req))
	{
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "json_content or playctrl_req is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
	    return MUXDEMUX_ERR;
	}

    root = cJSON_Parse(json_content);
    if(NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "call cJSON_Parse failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }

    // 解析collector_id
    json_level1 = cJSON_GetObjectItem(root, "collector_id");
    if(NULL == json_level1)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "not find collector_id!");
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }
    if(cJSON_String == json_level1->type)
    {
        strncpy(playctrl_req->collector_id, json_level1->valuestring, MAX_ID_LEN);
        playctrl_req->collector_id[MAX_ID_LEN] = '\0';
    }
    else
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "collector_id type is %d, not expected type!", json_level1->type);
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }

    // 解析command
    json_level1 = cJSON_GetObjectItem(root, "command");
    if(NULL == json_level1)
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "not find command!");
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }
    if (cJSON_Number == json_level1->type)
    {
        playctrl_req->command = json_level1->valueint;
    }
    else
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "command type is %d, not expected type!", json_level1->type);
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }

    cJSON_Delete(root);
    return MUXDEMUX_OK;
}

/**************播放控制响应json格式*******************
{
    retcode:0,              // 结果码
	retmsg:"success"        // 结果信息
}
*****************************************************/
// 播放控制响应json封装
int mux_playctrl_ack(t_playctrl_ack *playctrl_ack, char *json_content, int json_len)
{
    cJSON *root         = NULL;
    char  *out          = NULL;
	char szLogInfo[256] = {0};
	
	// 判断函数参数是否合法
	if ((NULL == playctrl_ack) || (NULL == json_content))
	{
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "playctrl_ack or json_content is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
	    return MUXDEMUX_ERR;
	}
    
    root = cJSON_CreateObject();
    if(NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "exec cJSON_CreateObject to get root failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return MUXDEMUX_ERR;
    }
    cJSON_AddNumberToObject(root, "retCode", playctrl_ack->retcode);
    cJSON_AddStringToObject(root, "retMsg", playctrl_ack->retmsg);
	
	out=cJSON_Print(root);
	strncpy(json_content, out, json_len - 1);
	json_content[json_len - 1] = '\0';

	cJSON_Delete(root);
	free(out);

	return MUXDEMUX_OK;
}

/**************音量设置请求json格式*******************
{
    collector_id:"0123456789",              // 集中器ID
	volume:80                               // 音量值，为百分比放大100倍，有效范围0-100
}
*****************************************************/
// 音量设置请求json解析
int demux_setvolume_req(const char *json_content, t_setvolume_req *setvolume_req)
{
    cJSON *root         = NULL;
    cJSON *json_level1  = NULL;
	char szLogInfo[256] = {0};
	
	// 判断函数参数是否合法
	if ((NULL == json_content) || (NULL == setvolume_req))
	{
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "json_content or setvolume_req is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
	    return MUXDEMUX_ERR;
	}

    root = cJSON_Parse(json_content);
    if(NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "call cJSON_Parse failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }

    // 解析collector_id
    json_level1 = cJSON_GetObjectItem(root, "collector_id");
    if(NULL == json_level1)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "not find collector_id!");
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }
    if(cJSON_String == json_level1->type)
    {
        strncpy(setvolume_req->collector_id, json_level1->valuestring, MAX_ID_LEN);
        setvolume_req->collector_id[MAX_ID_LEN] = '\0';
    }
    else
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "collector_id type is %d, not expected type!", json_level1->type);
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }

    // 解析volume
    json_level1 = cJSON_GetObjectItem(root, "volume");
    if(NULL == json_level1)
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "not find volume!");
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }
    if (cJSON_Number == json_level1->type)
    {
        setvolume_req->volume = json_level1->valueint;
    }
    else
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "volume type is %d, not expected type!", json_level1->type);
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }

    cJSON_Delete(root);
    return MUXDEMUX_OK;
}

/**************音量设置响应json格式*******************
{
    retcode:0,              // 结果码
	retmsg:"success"        // 结果信息
}
*****************************************************/
// 音量设置响应json封装
int mux_setvolume_ack(t_setvolume_ack *setvolume_ack, char *json_content, int json_len)
{
    cJSON *root         = NULL;
    char  *out          = NULL;
	char szLogInfo[256] = {0};
	
	// 判断函数参数是否合法
	if ((NULL == setvolume_ack) || (NULL == json_content))
	{
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "setvolume_ack or json_content is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
	    return MUXDEMUX_ERR;
	}
    
    root = cJSON_CreateObject();
    if(NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "exec cJSON_CreateObject to get root failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return MUXDEMUX_ERR;
    }
    cJSON_AddNumberToObject(root, "retCode", setvolume_ack->retcode);
    cJSON_AddStringToObject(root, "retMsg", setvolume_ack->retmsg);
	
	out=cJSON_Print(root);
	strncpy(json_content, out, json_len - 1);
	json_content[json_len - 1] = '\0';

	cJSON_Delete(root);
	free(out);

	return MUXDEMUX_OK;
}

// 音量获取请求json解析
int demux_getvolume_req(const char *json_content, t_getvolume_req *getvolume_req)
{
    cJSON *root         = NULL;
    cJSON *json_level1  = NULL;
	char szLogInfo[256] = {0};
	
	// 判断函数参数是否合法
	if ((NULL == json_content) || (NULL == getvolume_req))
	{
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "json_content or getvolume_req is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
	    return MUXDEMUX_ERR;
	}

    root = cJSON_Parse(json_content);
    if(NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "call cJSON_Parse failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }

    // 解析collector_id
    json_level1 = cJSON_GetObjectItem(root, "collector_id");
    if(NULL == json_level1)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "not find collector_id!");
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }
    if(cJSON_String == json_level1->type)
    {
        strncpy(getvolume_req->collector_id, json_level1->valuestring, MAX_ID_LEN);
        getvolume_req->collector_id[MAX_ID_LEN] = '\0';
    }
    else
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "collector_id type is %d, not expected type!", json_level1->type);
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }

    cJSON_Delete(root);
    return MUXDEMUX_OK;
}

// 音量获取响应json解析
int mux_getvolume_ack(t_getvolume_ack *getvolume_ack, char *json_content, int json_len)
{
    cJSON *root         = NULL;
    char  *out          = NULL;

	char szLogInfo[256] = {0};

	// 判断函数参数是否合法
	if (getvolume_ack == NULL || json_content == NULL)
	{
	    snprintf(szLogInfo, sizeof(szLogInfo)-1, "mux_getvolume_ack: getvolume_ack or json_content is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
	
	    return MUXDEMUX_ERR;
	}
    
    root = cJSON_CreateObject();
    if (NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "mux_getvolume_ack: exec cJSON_CreateObject to get root failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		
        return MUXDEMUX_ERR;
    }
	
    cJSON_AddNumberToObject(root, "retCode", getvolume_ack->retcode); // retCode
    
    cJSON_AddStringToObject(root, "retMsg", getvolume_ack->retmsg); // retMsg
    
    cJSON_AddNumberToObject(root, "vol", getvolume_ack->volume); // lock_status
	
	out=cJSON_Print(root);
	strncpy(json_content, out, json_len - 1);
	json_content[json_len - 1] = '\0';

	cJSON_Delete(root);
	free(out);

	return MUXDEMUX_OK;
}



/**************获取环境参数请求json格式*******************
{
      collector_id:"0123456789",              // 集中器ID
	nReqType:1 // 请求类型，1表示温度，2表示湿度，3表示照度，4表示风向，5表示风速
}
*****************************************************/
// 获取环境参数请求json解析
int demux_getenvparam_req(const char *json_content, t_getenvirparam_req *envparam_req)
{    
	cJSON *root 		  = NULL;
	cJSON *json_level1	  = NULL;
	char   szLogInfo[500] = {0};
		
	// 判断函数参数是否合法
	if ((NULL == json_content) || (NULL == envparam_req))
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "json_content or envparam_req is NULL!");
		WRITELOG(LOG_ERROR, szLogInfo);
		return MUXDEMUX_ERR;
	}
//打印json 的内容
    snprintf(szLogInfo, sizeof(szLogInfo) - 1, json_content);
    WRITELOG(LOG_INFO, szLogInfo);

	
	root = cJSON_Parse(json_content);
	if(NULL == root)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "call cJSON_Parse failed!");
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	
	// 解析collector_id
	json_level1 = cJSON_GetObjectItem(root, "collector_id");
	if(NULL == json_level1)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "not find collector_id!");
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
    
	if(cJSON_String == json_level1->type)
	{
		strncpy(envparam_req->collector_id, json_level1->valuestring, MAX_ID_LEN);
		envparam_req->collector_id[MAX_ID_LEN] = '\0';
	}
	else
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "collector_id type is %d, not expected type!", json_level1->type);
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
//打印json_level1->valuestring
    //snprintf(szLogInfo, sizeof(szLogInfo) - 1, "json_level1->valuestring = %s",json_level1->valuestring);
    //WRITELOG(LOG_INFO, szLogInfo);

	
	// 解析nReqType
	json_level1 = cJSON_GetObjectItem(root, "nReqType");
	if(NULL == json_level1)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "not find nReqType!");
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}

	
    //打印envparam_req 结构体 的内容
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, " in demux_getenvparam_req envparam_req adress = 0x%x",envparam_req);
        WRITELOG(LOG_INFO, szLogInfo);

	if (cJSON_Number == json_level1->type)
	{
		envparam_req->nReqType = json_level1->valueint;
		//打印envir_param_req.nReqType 地址 结构体 的内容
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, " in demux_getenvparam_req envir_param_req.nReqType adress = 0x%x",&envparam_req->nReqType);
        WRITELOG(LOG_INFO, szLogInfo);
		//打印envparam_req 结构体 的内容
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, " in demux_getenvparam_req envparam_req->nReqType = %d",envparam_req->nReqType);
        WRITELOG(LOG_INFO, szLogInfo);
	}
	else
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "nReqType type is %d, not expected type!", json_level1->type);
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	cJSON_Delete(root);
	return MUXDEMUX_OK;
}

/**************获取环境参数响应json格式*******************
{
       retcode:0,              // 结果码
	retmsg:"success"     // 结果信息
	nReqType:1            // 请求类型，1表示温度，2表示湿度，3表示照度，4表示风向，5表示风速
	szValue:"234.9"      // 结果值，表示温度，湿度，照度，风向，风速等的具体值
}
*****************************************************/
int mux_getenvparam_ack(t_getenvirparam_ack *envparam_ack, char *json_content, int json_len)
{
    cJSON *root         = NULL;
    char  *out          = NULL;
	char szLogInfo[500] = {0};
	
	// 判断函数参数是否合法
	if ((NULL == envparam_ack) || (NULL == json_content))
	{
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "envparam_ack or json_content is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
	    return MUXDEMUX_ERR;
	}
    
    root = cJSON_CreateObject();
    if(NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "exec cJSON_CreateObject to get root failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return MUXDEMUX_ERR;
    }
    cJSON_AddNumberToObject(root, "retCode", envparam_ack->retcode);
    cJSON_AddStringToObject(root, "retMsg", envparam_ack->retmsg);	
    cJSON_AddNumberToObject(root, "nReqType", envparam_ack->nReqType);
    cJSON_AddStringToObject(root, "szValue", envparam_ack->szValue);
	
	out=cJSON_Print(root);
	strncpy(json_content, out, json_len - 1);
	json_content[json_len - 1] = '\0';

	cJSON_Delete(root);
	free(out);

	return MUXDEMUX_OK;
}


/**************获取所有的环境参数请求json格式*******************
{
       collector_id:"0123456789",              // 集中器ID
       nReqType:6
}
*****************************************************/
// 获取所有的全部环境参数请求json解析
int demux_get_all_envparam_req(const char *json_content, t_get_all_envirparam_req *all_envparam_req)
{
    cJSON *root 		= NULL;
	cJSON *json_level1	= NULL;
	char  szLogInfo[500] = {0};
		
	// 判断函数参数是否合法
	if ((NULL == json_content) || (NULL == all_envparam_req))
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "json_content or all_envparam_req is NULL!");
		WRITELOG(LOG_ERROR, szLogInfo);
		return MUXDEMUX_ERR;
	}
	
	root = cJSON_Parse(json_content);
	if(NULL == root)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "call cJSON_Parse failed!");
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	
	// 解析collector_id
	json_level1 = cJSON_GetObjectItem(root, "collector_id");
	if(NULL == json_level1)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "not find collector_id!");
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	if(cJSON_String == json_level1->type)
	{
		strncpy(all_envparam_req->collector_id, json_level1->valuestring, MAX_ID_LEN);
		all_envparam_req->collector_id[MAX_ID_LEN] = '\0';
	}
	else
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "collector_id type is %d, not expected type!", json_level1->type);
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	// 解析nReqType
	json_level1 = cJSON_GetObjectItem(root, "nReqType");
	if(NULL == json_level1)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "not find nReqType!");
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}

	if (cJSON_Number == json_level1->type)
	{
		all_envparam_req->nReqType = json_level1->valueint;
	}
	else
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "nReqType type is %d, not expected type!", json_level1->type);
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}	
	cJSON_Delete(root);
	return MUXDEMUX_OK;
}


/**************获取全部的环境参数响应json格式*******************
{
       retcode:0,              // 结果码
	retmsg:"success"     // 结果信息
	szWenduValue:"234.9"      // 结果值，表示温度
	szShiDuVlaue:"234.9"      // 结果值，表示湿度
	szZhaoDuVlaue:"234.9"      // 结果值，表示照度
	szFengXiangValue:"234.9"      // 结果值，表示风向
	szFengSuValue:"234.9"      // 结果值，表示风速		
}
*****************************************************/
int mux_get_all_envparam_ack(t_get_all_envirparam_ack *all_envparam_ack, char *json_content, int json_len)
{
    cJSON *root         = NULL;
    char  *out          = NULL;
	char szLogInfo[500] = {0};
	
	// 判断函数参数是否合法
	if ((NULL == all_envparam_ack) || (NULL == json_content))
	{
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "all_envparam_ack or json_content is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
	    return MUXDEMUX_ERR;
	}
    
    root = cJSON_CreateObject();
    if(NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "exec cJSON_CreateObject to get root failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return MUXDEMUX_ERR;
    }
    cJSON_AddNumberToObject(root, "retCode", all_envparam_ack->retcode);
    cJSON_AddStringToObject(root, "retMsg", all_envparam_ack->retmsg);	
    cJSON_AddStringToObject(root, "szWenduValue", all_envparam_ack->szWenduValue);
    cJSON_AddStringToObject(root, "szShiDuVlaue", all_envparam_ack->szShiDuVlaue);
	cJSON_AddStringToObject(root, "szZhaoDuVlaue", all_envparam_ack->szZhaoDuVlaue);
    cJSON_AddStringToObject(root, "szFengXiangValue", all_envparam_ack->szFengXiangValue);
	cJSON_AddStringToObject(root, "szFengSuValue", all_envparam_ack->szFengSuValue);
	
	out=cJSON_Print(root);
	strncpy(json_content, out, json_len - 1);
	json_content[json_len - 1] = '\0';

	cJSON_Delete(root);
	free(out);

	return MUXDEMUX_OK;
}



/**************增加车位锁json格式*******************
{
    collector_id:” XXXXXXXXX_XXXX”        //集中器id，9位行政区划码ID（或者其他ID）+4位集中器ID
    chargepile_id:” XXXXXXXXX_XXXX_XXX”   //充电桩id，9位行政区划码ID（或者其他ID）+4位集中器ID+3位充电桩ID
    parklock_id:” XXXXXXXXX_XXXX_XXX_XXX” //车位锁id，9位行政区划码ID（或者其他ID）+4位集中器ID+3位充电桩ID+3位车位锁ID
    name:”name”                           //车位锁名称
    longitude:”xxx.xxxxxx”,               //车位锁经度，浮点数类型，小数点后6位
    latitude:”yyy.yyyyyy”,                //车位锁纬度，浮点数类型，小数点后保留6位
    detail_address:”address”              //车位锁详细地址
}
*****************************************************/
// 增加车位锁json解析
int demux_addparklock_req(const char *json_content, AddLockReq_T *ptAddLockReq)
{
    cJSON *root            = NULL;
    cJSON *json_level1     = NULL;
    char   szLogInfo[1024] = {0};

    if (json_content == NULL || ptAddLockReq == NULL)
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_addparklock_req: input parameter(s) is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);

		return MUXDEMUX_ERR;
    }

	root = cJSON_Parse(json_content);
    if (NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_addparklock_req: call cJSON_Parse failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }

	// 解析collector_id
	json_level1 = cJSON_GetObjectItem(root, "collector_id");
	if (NULL == json_level1)
	{
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_addparklock_req: not find collector_id!");
		WRITELOG(LOG_ERROR, szLogInfo);
			
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	if (cJSON_String == json_level1->type)
    {
        strncpy(ptAddLockReq->szCollectorID, json_level1->valuestring, sizeof(ptAddLockReq->szCollectorID) - 1);
		ptAddLockReq->szCollectorID[sizeof(ptAddLockReq->szCollectorID) - 1] = '\0';
	}
	else
	{
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_addparklock_req: collector_id type is %d, not expected type!", json_level1->type);
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}

	// 解析chargepile_id
	json_level1 = cJSON_GetObjectItem(root, "chargepile_id");
	if (NULL == json_level1)
	{
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_addparklock_req: not find chargepile_id!");
		WRITELOG(LOG_ERROR, szLogInfo);
			
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	if (cJSON_String == json_level1->type)
    {
        strncpy(ptAddLockReq->szChargepileID, json_level1->valuestring, sizeof(ptAddLockReq->szChargepileID) - 1);
		ptAddLockReq->szChargepileID[sizeof(ptAddLockReq->szChargepileID) - 1] = '\0';
	}
	else
	{
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_addparklock_req: chargepile_id type is %d, not expected type!", json_level1->type);
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}

	// 解析parklock_id
	json_level1 = cJSON_GetObjectItem(root, "parklock_id");
	if (NULL == json_level1)
	{
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_addparklock_req: not find parklock_id!");
		WRITELOG(LOG_ERROR, szLogInfo);
			
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	if (cJSON_String == json_level1->type)
    {
        strncpy(ptAddLockReq->szParklockID, json_level1->valuestring, sizeof(ptAddLockReq->szParklockID) - 1);
		ptAddLockReq->szParklockID[sizeof(ptAddLockReq->szParklockID) - 1] = '\0';
	}
	else
	{
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_addparklock_req: parklock_id type is %d, not expected type!", json_level1->type);
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}

	// 解析name
	json_level1 = cJSON_GetObjectItem(root, "name");
	if (NULL == json_level1)
	{
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_addparklock_req: not find name!");
		WRITELOG(LOG_ERROR, szLogInfo);
			
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	if (cJSON_String == json_level1->type)
    {
        strncpy(ptAddLockReq->szName, json_level1->valuestring, sizeof(ptAddLockReq->szName) - 1);
		ptAddLockReq->szName[sizeof(ptAddLockReq->szName) - 1] = '\0';
	}
	else
	{
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_addparklock_req: name type is %d, not expected type!", json_level1->type);
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}

	// 解析longitude
	json_level1 = cJSON_GetObjectItem(root, "longitude");
	if (NULL == json_level1)
	{
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_addparklock_req: not find longitude!");
		WRITELOG(LOG_ERROR, szLogInfo);
			
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	if (cJSON_String == json_level1->type)
    {
        strncpy(ptAddLockReq->szLongitude, json_level1->valuestring, sizeof(ptAddLockReq->szLongitude) - 1);
		ptAddLockReq->szLongitude[sizeof(ptAddLockReq->szLongitude) - 1] = '\0';
	}
	else
	{
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_addparklock_req: longitude type is %d, not expected type!", json_level1->type);
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}

	// 解析latitude
	json_level1 = cJSON_GetObjectItem(root, "latitude");
	if (NULL == json_level1)
	{
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_addparklock_req: not find latitude!");
		WRITELOG(LOG_ERROR, szLogInfo);
			
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	if (cJSON_String == json_level1->type)
    {
        strncpy(ptAddLockReq->szLatitude, json_level1->valuestring, sizeof(ptAddLockReq->szLatitude) - 1);
		ptAddLockReq->szLatitude[sizeof(ptAddLockReq->szLatitude) - 1] = '\0';
	}
	else
	{
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_addparklock_req: latitude type is %d, not expected type!", json_level1->type);
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}

	// 解析detail_address
	json_level1 = cJSON_GetObjectItem(root, "detail_address");
	if (NULL == json_level1)
	{
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_addparklock_req: not find detail_address!");
		WRITELOG(LOG_ERROR, szLogInfo);
			
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	if (cJSON_String == json_level1->type)
    {
        strncpy(ptAddLockReq->szDetailAddress, json_level1->valuestring, sizeof(ptAddLockReq->szDetailAddress) - 1);
		ptAddLockReq->szDetailAddress[sizeof(ptAddLockReq->szDetailAddress) - 1] = '\0';
	}
	else
	{
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_addparklock_req: detail_address type is %d, not expected type!", json_level1->type);
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return -1;
	}

	// 打印获取到的数据值
	cJSON_Delete(root);
	
    return MUXDEMUX_OK;
}


/**************增/删/改车位锁响应json格式*******************
{
    retCode: 0  //0成功，1 id重复，2 id不存在，3服务器错误, -1其他
    retMsg:"成功"
}
*****************************************************/
// 增/删/改车位锁响应json解析
int mux_parklock_ack(DealLockRSP_T *ptDealLockRSP, char *json_content, int json_len)
{
    cJSON *root         = NULL;
    char  *out          = NULL;

	char szLogInfo[256] = {0};

	// 判断函数参数是否合法
	if (ptDealLockRSP == NULL || json_content == NULL)
	{
	    snprintf(szLogInfo, sizeof(szLogInfo)-1, "mux_parklock_ack: ptDealLockRSP or json_content is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
	
	    return MUXDEMUX_ERR;
	}
    
    root = cJSON_CreateObject();
    if (NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "mux_parklock_ack: exec cJSON_CreateObject to get root failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		
        return MUXDEMUX_ERR;
    }
	
    cJSON_AddNumberToObject(root, "retCode", ptDealLockRSP->iRetCode); // retCode
    
    cJSON_AddStringToObject(root, "retMsg", ptDealLockRSP->szRetMsg); // retMsg
	
	out=cJSON_Print(root);
	strncpy(json_content, out, json_len - 1);
	json_content[json_len - 1] = '\0';

	cJSON_Delete(root);
	free(out);

	return MUXDEMUX_OK;
}


/**************删除车位锁json格式*******************
{
    parklock_id:" XXXXXXXXX_XXXX_XXX_XXX" //车位锁id，9位行政区划码ID（或者其他ID）+4位集中器ID+3位充电桩ID+3位车位锁ID
}
*****************************************************/
// 删除车位锁json解析
int demux_delparklock_req(const char *json_content, DelLockReq_T *ptDelLockReq)
{
    cJSON *root 		   = NULL;
	cJSON *json_level1	   = NULL;
	char   szLogInfo[1024] = {0};
	
	if (json_content == NULL || ptDelLockReq == NULL)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_delparklock_req: input parameter(s) is NULL!");
		WRITELOG(LOG_ERROR, szLogInfo);
	
		return MUXDEMUX_ERR;
	}
	
	root = cJSON_Parse(json_content);
	if (NULL == root)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_delparklock_req: call cJSON_Parse failed!");
		WRITELOG(LOG_ERROR, szLogInfo);
		
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	
	// 解析parklock_id
	json_level1 = cJSON_GetObjectItem(root, "parklock_id");
	if (NULL == json_level1)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_delparklock_req: not find parklock_id!");
		WRITELOG(LOG_ERROR, szLogInfo);
				
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	if (cJSON_String == json_level1->type)
	{
		strncpy(ptDelLockReq->szParklockID, json_level1->valuestring, sizeof(ptDelLockReq->szParklockID) - 1);
		ptDelLockReq->szParklockID[sizeof(ptDelLockReq->szParklockID) - 1] = '\0';
	}
	else
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_delparklock_req: parklock_id type is %d, not expected type!", json_level1->type);
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	
	// 打印获取到的数据值
	cJSON_Delete(root);
	
    return MUXDEMUX_OK;
}


/**************修改车位锁json格式*******************
{
    collector_id:" XXXXXXXXX_XXXX" //集中器id，9位行政区划码ID（或者其他ID）+4位集中器ID
    chargepile_id:" XXXXXXXXX_XXXX_XXX" //充电桩id，9位行政区划码ID（或者其他ID）+4位集中器ID+3位充电桩ID
    parklock_id:" XXXXXXXXX_XXXX_XXX_XXX" //车位锁id，9位行政区划码ID（或者其他ID）+4位集中器ID+3位充电桩ID+3位车位锁ID
    name:"name"   //车位锁名称
    longitude:"xxx.xxxxxx", //车位锁经度，字符串类型，小数点后6位
    latitude:"yyy.yyyyyy",  //车位锁纬度，字符串类型，小数点后保留6位
    detail_address:"address" //车位锁详细地址
}
*****************************************************/
// 修改车位锁json解析
int demux_modparklock_req(const char *json_content, ModifyLockReq_T *ptModifyLockReq)
{
    cJSON *root 		   = NULL;
	cJSON *json_level1	   = NULL;
	char   szLogInfo[1024] = {0};
	
	if (json_content == NULL || ptModifyLockReq == NULL)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_modparklock_req: input parameter(s) is NULL!");
		WRITELOG(LOG_ERROR, szLogInfo);
	
		return MUXDEMUX_ERR;
	}
	
	root = cJSON_Parse(json_content);
	if (NULL == root)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_modparklock_req: call cJSON_Parse failed!");
		WRITELOG(LOG_ERROR, szLogInfo);
		
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	
	// 解析collector_id
	json_level1 = cJSON_GetObjectItem(root, "collector_id");
	if (NULL == json_level1)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_modparklock_req: not find collector_id!");
		WRITELOG(LOG_ERROR, szLogInfo);
					
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	if (cJSON_String == json_level1->type)
	{
		strncpy(ptModifyLockReq->szCollectorID, json_level1->valuestring, sizeof(ptModifyLockReq->szCollectorID) - 1);
		ptModifyLockReq->szCollectorID[sizeof(ptModifyLockReq->szCollectorID) - 1] = '\0';
	}
	else
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_modparklock_req: collector_id type is %d, not expected type!", json_level1->type);
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	
	// 解析chargepile_id
	json_level1 = cJSON_GetObjectItem(root, "chargepile_id");
	if (NULL == json_level1)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_modparklock_req: not find chargepile_id!");
		WRITELOG(LOG_ERROR, szLogInfo);
				
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	if (cJSON_String == json_level1->type)
	{
		strncpy(ptModifyLockReq->szChargepileID, json_level1->valuestring, sizeof(ptModifyLockReq->szChargepileID) - 1);
		ptModifyLockReq->szChargepileID[sizeof(ptModifyLockReq->szChargepileID) - 1] = '\0';
	}
	else
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_modparklock_req: chargepile_id type is %d, not expected type!", json_level1->type);
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	
	// 解析parklock_id
	json_level1 = cJSON_GetObjectItem(root, "parklock_id");
	if (NULL == json_level1)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_modparklock_req: not find parklock_id!");
		WRITELOG(LOG_ERROR, szLogInfo);
					
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	if (cJSON_String == json_level1->type)
	{
		strncpy(ptModifyLockReq->szParklockID, json_level1->valuestring, sizeof(ptModifyLockReq->szParklockID) - 1);
		ptModifyLockReq->szParklockID[sizeof(ptModifyLockReq->szParklockID) - 1] = '\0';
	}
	else
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_modparklock_req: parklock_id type is %d, not expected type!", json_level1->type);
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	
	// 解析name
	json_level1 = cJSON_GetObjectItem(root, "name");
	if (NULL == json_level1)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_modparklock_req: not find name!");
		WRITELOG(LOG_ERROR, szLogInfo);
					
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	if (cJSON_String == json_level1->type)
	{
		strncpy(ptModifyLockReq->szName, json_level1->valuestring, sizeof(ptModifyLockReq->szName) - 1);
		ptModifyLockReq->szName[sizeof(ptModifyLockReq->szName) - 1] = '\0';
	}
	else
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_modparklock_req: name type is %d, not expected type!", json_level1->type);
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	
	// 解析longitude
	json_level1 = cJSON_GetObjectItem(root, "longitude");
	if (NULL == json_level1)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_modparklock_req: not find longitude!");
		WRITELOG(LOG_ERROR, szLogInfo);
					
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	if (cJSON_String == json_level1->type)
	{
		strncpy(ptModifyLockReq->szLongitude, json_level1->valuestring, sizeof(ptModifyLockReq->szLongitude) - 1);
		ptModifyLockReq->szLongitude[sizeof(ptModifyLockReq->szLongitude) - 1] = '\0';
	}
	else
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_modparklock_req: longitude type is %d, not expected type!", json_level1->type);
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	
	// 解析latitude
	json_level1 = cJSON_GetObjectItem(root, "latitude");
	if (NULL == json_level1)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_modparklock_req: not find latitude!");
		WRITELOG(LOG_ERROR, szLogInfo);
					
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	if (cJSON_String == json_level1->type)
	{
		strncpy(ptModifyLockReq->szLatitude, json_level1->valuestring, sizeof(ptModifyLockReq->szLatitude) - 1);
		ptModifyLockReq->szLatitude[sizeof(ptModifyLockReq->szLatitude) - 1] = '\0';
	}
	else
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_modparklock_req: latitude type is %d, not expected type!", json_level1->type);
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	
	// 解析detail_address
	json_level1 = cJSON_GetObjectItem(root, "detail_address");
	if (NULL == json_level1)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_modparklock_req: not find detail_address!");
		WRITELOG(LOG_ERROR, szLogInfo);
					
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	if (cJSON_String == json_level1->type)
	{
		strncpy(ptModifyLockReq->szDetailAddress, json_level1->valuestring, sizeof(ptModifyLockReq->szDetailAddress) - 1);
		ptModifyLockReq->szDetailAddress[sizeof(ptModifyLockReq->szDetailAddress) - 1] = '\0';
	}
	else
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_modparklock_req: detail_address type is %d, not expected type!", json_level1->type);
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	
	// 打印获取到的数据值
	cJSON_Delete(root);
	
    return MUXDEMUX_OK;
}


/**************通过设备ID查询车位锁json格式*******************
{
    parklock_id:" XXXXXXXXX_XXXX_XXX_XXX" //车位锁id，9位行政区划码ID（或者其他ID）+4位集中器ID+3位充电桩ID+3位车位锁ID
}
*****************************************************/
// 通过设备ID查询车位锁json解析
int demux_queryidparklock_req(const char *json_content, QueryLockReq_T *ptQueryLockReq)
{
    cJSON *root 		   = NULL;
	cJSON *json_level1	   = NULL;
	char   szLogInfo[1024] = {0};
	
	if (json_content == NULL || ptQueryLockReq == NULL)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_queryidparklock_req: input parameter(s) is NULL!");
		WRITELOG(LOG_ERROR, szLogInfo);
	
		return MUXDEMUX_ERR;
	}
	
	root = cJSON_Parse(json_content);
	if (NULL == root)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_queryidparklock_req: call cJSON_Parse failed!");
		WRITELOG(LOG_ERROR, szLogInfo);
		
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	
	// 解析parklock_id
	json_level1 = cJSON_GetObjectItem(root, "parklock_id");
	if (NULL == json_level1)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_queryidparklock_req: not find parklock_id!");
		WRITELOG(LOG_ERROR, szLogInfo);
				
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	if (cJSON_String == json_level1->type)
	{
		strncpy(ptQueryLockReq->szParklockID, json_level1->valuestring, sizeof(ptQueryLockReq->szParklockID) - 1);
		ptQueryLockReq->szParklockID[sizeof(ptQueryLockReq->szParklockID) - 1] = '\0';
	}
	else
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_queryidparklock_req: parklock_id type is %d, not expected type!", json_level1->type);
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	
	// 打印获取到的数据值
	cJSON_Delete(root);
	
    return MUXDEMUX_OK;
}


/**************通过设备ID查询车位锁响应json格式*******************
{
    retCode: 0  //0成功，1 id重复，2 id不存在，3服务器错误, -1其他
    retMsg:"成功"
    chargepile:
    {
        collector_id:" XXXXXXXXX_XXXX" //集中器id，9位行政区划码ID（或者其他ID）+4位集中器ID
        chargepile_id:" XXXXXXXXX_XXXX_XXX" //充电桩id，9位行政区划码ID（或者其他ID）+4位集中器ID+3位充电桩ID
        parklock_id:" XXXXXXXXX_XXXX_XXX_XXX" //车位锁id，9位行政区划码ID（或者其他ID）+4位集中器ID+3位充电桩ID+3位车位锁ID
        name:"name"   //车位锁名称
        longitude:"xxx.xxxxxx", //车位锁经度，字符串类型，小数点后6位
        latitude:"yyy.yyyyyy",  //车位锁纬度，字符串类型，小数点后保留6位
        detail_address:"address" //车位锁详细地址
        lock_status:1 // 0-车位锁开；1-车位锁锁
    }
}
*****************************************************/
// 通过设备ID查询车位锁响应json解析
int mux_queryidparklock_ack(QueryLockRSP_T *ptQueryLockRSP, char *json_content, int json_len)
{
    cJSON *root         = NULL;
    cJSON *json_level1  = NULL;
    cJSON *json_level2  = NULL;
    char  *out          = NULL;

	char szLogInfo[256] = {0};

	// 判断函数参数是否合法
	if (ptQueryLockRSP == NULL || json_content == NULL)
	{
	    snprintf(szLogInfo, sizeof(szLogInfo)-1, "mux_queryidparklock_ack: ptQueryLockRSP or json_content is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
	
	    return MUXDEMUX_ERR;
	}
    
    root = cJSON_CreateObject();
    if (NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "mux_queryidparklock_ack: exec cJSON_CreateObject to get root failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		
        return MUXDEMUX_ERR;
    }
	
    cJSON_AddNumberToObject(root, "retCode", ptQueryLockRSP->iRetCode); // retCode
    
    cJSON_AddStringToObject(root, "retMsg", ptQueryLockRSP->szRetMsg); // retMsg

	json_level1 = cJSON_CreateObject();
    if (NULL == json_level1)
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "mux_queryidparklock_ack: exec cJSON_CreateObject to get json_level1 failed!");
        WRITELOG(LOG_ERROR, szLogInfo);

		cJSON_Delete(root);
		
        return MUXDEMUX_ERR;
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

	return MUXDEMUX_OK;
}


/**************通过gis查询车位锁json格式*******************
{
    longtitude_range:{
        start_value:"xxx.xxxxxx", //字符串型，小数点后保留6位
        end_value:"yyy.yyyyyy" //字符串型，小数点后保留6位
    }
    latitude_range:{
        start_value: "xxx.xxxxxx", //字符串型，小数点后保留6位
        end_value: "yyy.yyyyyy" //字符串型，小数点后保留6位
    }
}
*****************************************************/
// 通过gis查询车位锁json解析
int demux_querygisparklock_req(const char *json_content, QueryLockGISReq_T *ptQueryLockGISReq)
{
    cJSON *root            = NULL;
    cJSON *json_level1     = NULL;
	cJSON *json_level2     = NULL;
    char   szLogInfo[1024] = {0};

	if (json_content == NULL || ptQueryLockGISReq == NULL)
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_querygisparklock_req: input parameter(s) is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);

		return MUXDEMUX_ERR;
    }

	root = cJSON_Parse(json_content);
    if (NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_querygisparklock_req: call cJSON_Parse failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }

	// 解析longtitude_range
	json_level1 = cJSON_GetObjectItem(root, "longtitude_range");
	if (NULL == json_level1)
	{
	    snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_querygisparklock_req: not find longtitude_range!");
		WRITELOG(LOG_ERROR, szLogInfo);
				
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}

	// 解析start_value (longtitude)
	json_level2 = cJSON_GetObjectItem(json_level1, "start_value");
	if (NULL == json_level2)
	{
	    snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_querygisparklock_req: not find start_value(longtitude)!");
		WRITELOG(LOG_ERROR, szLogInfo);
				
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	if (cJSON_String == json_level2->type)
	{
	    strncpy(ptQueryLockGISReq->szStartLongitude, json_level2->valuestring, sizeof(ptQueryLockGISReq->szStartLongitude) - 1);
		ptQueryLockGISReq->szStartLongitude[sizeof(ptQueryLockGISReq->szStartLongitude) - 1] = '\0';
	}
	else
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_querygisparklock_req: start_value(longtitude) type is %d, not expected type!", json_level2->type);
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	
	// 解析end_value (longtitude)
	json_level2 = cJSON_GetObjectItem(json_level1, "end_value");
	if (NULL == json_level2)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_querygisparklock_req: not find end_value(longtitude)!");
		WRITELOG(LOG_ERROR, szLogInfo);
				
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	if (cJSON_String == json_level2->type)
	{
		strncpy(ptQueryLockGISReq->szEndLongitude, json_level2->valuestring, sizeof(ptQueryLockGISReq->szEndLongitude) - 1);
		ptQueryLockGISReq->szEndLongitude[sizeof(ptQueryLockGISReq->szEndLongitude) - 1] = '\0';
	}
	else
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_querygisparklock_req: end_value(longtitude) type is %d, not expected type!", json_level2->type);
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}

	// 解析latitude_range
	json_level1 = cJSON_GetObjectItem(root, "latitude_range");
	if (NULL == json_level1)
	{
	    snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_querygisparklock_req: not find latitude_range!");
		WRITELOG(LOG_ERROR, szLogInfo);
				
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}

	// 解析start_value (latitude)
	json_level2 = cJSON_GetObjectItem(json_level1, "start_value");
	if (NULL == json_level2)
	{
	    snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_querygisparklock_req: not find start_value(latitude)!");
		WRITELOG(LOG_ERROR, szLogInfo);
				
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	if (cJSON_String == json_level2->type)
	{
	    strncpy(ptQueryLockGISReq->szStartLatitude, json_level2->valuestring, sizeof(ptQueryLockGISReq->szStartLatitude) - 1);
		ptQueryLockGISReq->szStartLatitude[sizeof(ptQueryLockGISReq->szStartLatitude) - 1] = '\0';
	}
	else
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_querygisparklock_req: start_value(latitude) type is %d, not expected type!", json_level2->type);
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}

	// 解析end_value (latitude)
	json_level2 = cJSON_GetObjectItem(json_level1, "end_value");
	if (NULL == json_level2)
	{
	    snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_querygisparklock_req: not find end_value(latitude)!");
		WRITELOG(LOG_ERROR, szLogInfo);
				
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	if (cJSON_String == json_level2->type)
	{
	    strncpy(ptQueryLockGISReq->szEndLatitude, json_level2->valuestring, sizeof(ptQueryLockGISReq->szEndLatitude) - 1);
		ptQueryLockGISReq->szEndLatitude[sizeof(ptQueryLockGISReq->szEndLatitude) - 1] = '\0';
	}
	else
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_querygisparklock_req: end_value(latitude) type is %d, not expected type!", json_level2->type);
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}

	// 打印获取到的数据值
	cJSON_Delete(root);
	
    return MUXDEMUX_OK;
}


/**************车位锁控制json格式*******************
{
    collector_id:" XXXXXXXXX_XXXX" //集中器id，9位行政区划码ID（或者其他ID）+4位集中器ID
    chargepile_id:" XXXXXXXXX_XXXX_XXX" //充电桩id，9位行政区划码ID（或者其他ID）+4位集中器ID+3位充电桩ID
    parklock_id:" XXXXXXXXX_XXXX_XXX_XXX" //车位锁id，9位行政区划码ID（或者其他ID）+4位集中器ID+3位充电桩ID+3位车位锁ID
    lock_ctrol:1 // 0-开车位锁；1-锁车位锁
}
*****************************************************/
// 车位锁控制json解析
int demux_controlparklock_req(const char *json_content, ControlLockReq_T *ptControlLockReq)
{
    cJSON *root            = NULL;
    cJSON *json_level1     = NULL;
    char   szLogInfo[1024] = {0};

    if (json_content == NULL || ptControlLockReq == NULL)
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_controlparklock_req: input parameter(s) is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);

		return MUXDEMUX_ERR;
    }

	root = cJSON_Parse(json_content);
    if (NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_controlparklock_req: call cJSON_Parse failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }

	// 解析collector_id
	json_level1 = cJSON_GetObjectItem(root, "collector_id");
	if (NULL == json_level1)
	{
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_controlparklock_req: not find collector_id!");
		WRITELOG(LOG_ERROR, szLogInfo);
			
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	    //snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_controlparklock_req: in collector_id type json_level1 is 0x%x", json_level1);
		//WRITELOG(LOG_INFO, szLogInfo); //GC add it
	if (cJSON_String == json_level1->type)
    {
        strncpy(ptControlLockReq->szCollectorID, json_level1->valuestring, sizeof(ptControlLockReq->szCollectorID) - 1);
		ptControlLockReq->szCollectorID[sizeof(ptControlLockReq->szCollectorID) - 1] = '\0';
	}
	else
	{
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_controlparklock_req: collector_id type is %d, not expected type!", json_level1->type);
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}

	// 解析chargepile_id
	json_level1 = cJSON_GetObjectItem(root, "chargepile_id");
	if (NULL == json_level1)
	{
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_controlparklock_req: not find chargepile_id!");
		WRITELOG(LOG_ERROR, szLogInfo);
			
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	//snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_controlparklock_req: in chargepile_id type json_level1 is 0x%x", json_level1);
		//WRITELOG(LOG_INFO, szLogInfo); //GC add it
		//json_level1->type = cJSON_String;//GC add it
	if (cJSON_String == json_level1->type)
    {
        strncpy(ptControlLockReq->szChargepileID, json_level1->valuestring, sizeof(ptControlLockReq->szChargepileID) - 1);
		ptControlLockReq->szChargepileID[sizeof(ptControlLockReq->szChargepileID) - 1] = '\0';
	}
	else
	{
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_controlparklock_req: chargepile_id type is %d, not expected type!", json_level1->type);
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}

	// 解析parklock_id
	json_level1 = cJSON_GetObjectItem(root, "parklock_id");
	if (NULL == json_level1)
	{
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_controlparklock_req: not find parklock_id!");
		WRITELOG(LOG_ERROR, szLogInfo);
			
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	if (cJSON_String == json_level1->type)
    {
        strncpy(ptControlLockReq->szParklockID, json_level1->valuestring, sizeof(ptControlLockReq->szParklockID) - 1);
		ptControlLockReq->szParklockID[sizeof(ptControlLockReq->szParklockID) - 1] = '\0';
	}
	else
	{
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_controlparklock_req: parklock_id type is %d, not expected type!", json_level1->type);
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	
	// 解析lock_ctrol
	json_level1 = cJSON_GetObjectItem(root, "lock_ctrol");
    if (NULL == json_level1)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_controlparklock_req: not find lock_ctrol!");
		WRITELOG(LOG_ERROR, szLogInfo);
			
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	if (cJSON_Number == json_level1->type)
	{
		ptControlLockReq->iLockCtrol = json_level1->valueint;
	}
	else
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_controlparklock_req: lock_ctrol type is %d, not expected type!", json_level1->type);
		WRITELOG(LOG_ERROR, szLogInfo);
			
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}

	// 打印获取到的数据值
    snprintf(szLogInfo, sizeof(szLogInfo)-1, "ptControlLockReq->szCollectorID= %s,ptControlLockReq->szChargepileID= %s,ptControlLockReq->szParklockID = %s,ptControlLockReq->iLockCtrol= %d", ptControlLockReq->szCollectorID,ptControlLockReq->szChargepileID,ptControlLockReq->szParklockID,ptControlLockReq->iLockCtrol);
    WRITELOG(LOG_INFO, szLogInfo);

	
	cJSON_Delete(root);
	
    return MUXDEMUX_OK;
}


/**************通过设备ID实时查询车位锁状态json格式*******************
{
    collector_id:" XXXXXXXXX_XXXX" //集中器id，9位行政区划码ID（或者其他ID）+4位集中器ID
    parklock_id:" XXXXXXXXX_XXXX_XXX_XXX" //车位锁id，9位行政区划码ID（或者其他ID）+4位集中器ID+3位充电桩ID+3位车位锁ID
}
*****************************************************/
// 通过设备ID查询车位锁json解析
int demux_realtimequeryparklock_req(const char *json_content, RealTimeQueryLockReq_T *ptRealTimeQueryLockReq)
{
    cJSON *root 		   = NULL;
	cJSON *json_level1	   = NULL;
	char   szLogInfo[1024] = {0};
	
	if (json_content == NULL || ptRealTimeQueryLockReq == NULL)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_realtimequeryparklock_req: input parameter(s) is NULL!");
		WRITELOG(LOG_ERROR, szLogInfo);
	
		return MUXDEMUX_ERR;
	}
	
	root = cJSON_Parse(json_content);
	if (NULL == root)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_realtimequeryparklock_req: call cJSON_Parse failed!");
		WRITELOG(LOG_ERROR, szLogInfo);
		
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	
	// 解析collector_id
	json_level1 = cJSON_GetObjectItem(root, "collector_id");
	if (NULL == json_level1)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_realtimequeryparklock_req: not find collector_id!");
		WRITELOG(LOG_ERROR, szLogInfo);
				
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	if (cJSON_String == json_level1->type)
	{
		strncpy(ptRealTimeQueryLockReq->szCollectorID, json_level1->valuestring, sizeof(ptRealTimeQueryLockReq->szCollectorID) - 1);
		ptRealTimeQueryLockReq->szCollectorID[sizeof(ptRealTimeQueryLockReq->szCollectorID) - 1] = '\0';
	}
	else
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_realtimequeryparklock_req: collector_id type is %d, not expected type!", json_level1->type);
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}

	// 解析parklock_id
	json_level1 = cJSON_GetObjectItem(root, "parklock_id");
	if (NULL == json_level1)
	{
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_realtimequeryparklock_req: not find parklock_id!");
		WRITELOG(LOG_ERROR, szLogInfo);
			
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	if (cJSON_String == json_level1->type)
    {
        strncpy(ptRealTimeQueryLockReq->szParklockID, json_level1->valuestring, sizeof(ptRealTimeQueryLockReq->szParklockID) - 1);
		ptRealTimeQueryLockReq->szParklockID[sizeof(ptRealTimeQueryLockReq->szParklockID) - 1] = '\0';
	}
	else
	{
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "demux_realtimequeryparklock_req: parklock_id type is %d, not expected type!", json_level1->type);
		WRITELOG(LOG_ERROR, szLogInfo);
		cJSON_Delete(root);
		return MUXDEMUX_ERR;
	}
	
	// 打印获取到的数据值
	cJSON_Delete(root);
	
    return MUXDEMUX_OK;
}


/**************实时查询车位锁状态响应json格式*******************
{
    retCode: 0     // 0成功，1 id重复，2 id不存在，3服务器错误, -1其他
    retMsg:"成功"
    lock_status:1   // 0-车位锁开；1-车位锁锁
}
*****************************************************/
// 实时查询车位锁状态响应json解析
int mux_realtimequeryparklock_ack(RealTimeQueryLockRSP_T *ptRealTimeQueryLockRSP, char *json_content, int json_len)
{
    cJSON *root         = NULL;
    char  *out          = NULL;

	char szLogInfo[256] = {0};

	// 判断函数参数是否合法
	if (ptRealTimeQueryLockRSP == NULL || json_content == NULL)
	{
	    snprintf(szLogInfo, sizeof(szLogInfo)-1, "mux_realtimequeryparklock_ack: ptRealTimeQueryLockRSP or json_content is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
	
	    return MUXDEMUX_ERR;
	}
    
    root = cJSON_CreateObject();
    if (NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "mux_realtimequeryparklock_ack: exec cJSON_CreateObject to get root failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		
        return MUXDEMUX_ERR;
    }
	
    cJSON_AddNumberToObject(root, "retCode", ptRealTimeQueryLockRSP->iRetCode); // retCode
    
    cJSON_AddStringToObject(root, "retMsg", ptRealTimeQueryLockRSP->szRetMsg); // retMsg
    
    cJSON_AddNumberToObject(root, "lockStatus", ptRealTimeQueryLockRSP->iLockStatus); // lock_status
	
	out=cJSON_Print(root);
	strncpy(json_content, out, json_len - 1);
	json_content[json_len - 1] = '\0';

	cJSON_Delete(root);
	free(out);

	return MUXDEMUX_OK;
}


// 用户登录请求json解析
int demux_user_log_in_req(const char *json_content, UserLogInReq_T *long_in_req)
{
    cJSON *root         = NULL;
    cJSON *json_level1  = NULL;
	char szLogInfo[256] = {0};
	
	// 判断函数参数是否合法
	if ((NULL == json_content) || (NULL == long_in_req))
	{
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "json_content or long_in_req is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
	    return MUXDEMUX_ERR;
	}

    root = cJSON_Parse(json_content);
    if(NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "call cJSON_Parse failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }

    // 解析userName
    json_level1 = cJSON_GetObjectItem(root, "userName");
    if(NULL == json_level1)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "not find userName!");
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }
    if(cJSON_String == json_level1->type)
    {
        strncpy(long_in_req->user_name, json_level1->valuestring, sizeof(long_in_req->user_name)-1);
    }
    else
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "userName type is %d, not expected type!", json_level1->type);
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }

	// 解析password, 密文
    json_level1 = cJSON_GetObjectItem(root, "password");
    if(NULL == json_level1)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "not find password!");
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }
    if(cJSON_String == json_level1->type)
    {
        strncpy(long_in_req->password, json_level1->valuestring, sizeof(long_in_req->password)-1);
    }
    else
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "password type is %d, not expected type!", json_level1->type);
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }
	
    cJSON_Delete(root);
    return MUXDEMUX_OK;
}

// 用户登录响应json封装
int mux_user_log_in_ack(UserLogInRsp_T *log_in_ack, char *json_content, int json_len)
{
    cJSON *root         = NULL;
	cJSON *json_level1  = NULL;
    char  *out          = NULL;
	char szLogInfo[256] = {0};
	int i;
	char colloct_str[32] = {0};
	
	// 判断函数参数是否合法
	if ((NULL == log_in_ack) || (NULL == json_content))
	{
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "log_in_ack or json_content is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
	    return MUXDEMUX_ERR;
	}
    
    root = cJSON_CreateObject();
    if(NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "exec cJSON_CreateObject to get root failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return MUXDEMUX_ERR;
    }
    cJSON_AddNumberToObject(root, "retCode", log_in_ack->iRetCode);
    cJSON_AddStringToObject(root, "retMsg", log_in_ack->szRetMsg);
	json_level1 = cJSON_CreateObject();
	if(NULL == json_level1)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "exec cJSON_CreateObject to get json_level1 failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return MUXDEMUX_ERR;
    }
	cJSON_AddItemToObject(root, "data", json_level1);
	cJSON_AddStringToObject(json_level1, "acceeToken", log_in_ack->accessToken);
	
	for (i=0;i<log_in_ack->collector_nums;i++)
	{
		memset(colloct_str, 0, sizeof(colloct_str));
		sprintf(colloct_str, "collector_id%d", i+1);
		cJSON_AddStringToObject(json_level1, colloct_str, log_in_ack->collector[i]);
	}
	
	out=cJSON_Print(root);
	strncpy(json_content, out, json_len - 1);
	json_content[json_len - 1] = '\0';

	cJSON_Delete(root);
	free(out);
	
    return MUXDEMUX_OK;
}

// 用户退出登录请求json解析
int demux_user_log_out_req(const char *json_content, UserLogOutReq_T *long_out_req)
{
    cJSON *root         = NULL;
    cJSON *json_level1  = NULL;
	char szLogInfo[256] = {0};
	
	// 判断函数参数是否合法
	if ((NULL == json_content) || (NULL == long_out_req))
	{
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "json_content or long_out_req is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
	    return MUXDEMUX_ERR;
	}

    root = cJSON_Parse(json_content);
    if(NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "call cJSON_Parse failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }

    // 解析accessToken
    json_level1 = cJSON_GetObjectItem(root, "accessToken");
    if(NULL == json_level1)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "not find accessToken!");
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }
    if(cJSON_String == json_level1->type)
    {
        strncpy(long_out_req->accessToken, json_level1->valuestring, sizeof(long_out_req->accessToken)-1);
    }
    else
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "accessToken type is %d, not expected type!", json_level1->type);
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }
	
    cJSON_Delete(root);
    return MUXDEMUX_OK;
}

// 用户退出登录响应json封装
int mux_user_log_out_ack(UserLogOutRsp_T *log_out_ack, char *json_content, int json_len)
{
    cJSON *root         = NULL;
    char  *out          = NULL;
	char szLogInfo[256] = {0};
	int i;
	char colloct_str[32] = {0};
	
	// 判断函数参数是否合法
	if ((NULL == log_out_ack) || (NULL == json_content))
	{
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "log_out_ack or json_content is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
	    return MUXDEMUX_ERR;
	}
    
    root = cJSON_CreateObject();
    if(NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "exec cJSON_CreateObject to get root failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return MUXDEMUX_ERR;
    }
    cJSON_AddNumberToObject(root, "retCode", log_out_ack->iRetCode);
    cJSON_AddStringToObject(root, "retMsg", log_out_ack->szRetMsg);
	
	out=cJSON_Print(root);
	strncpy(json_content, out, json_len - 1);
	json_content[json_len - 1] = '\0';

	cJSON_Delete(root);
	free(out);
	
    return MUXDEMUX_OK;
}

// 获取环境参数请求json解析
int demux_get_env_req(const char *json_content,  GetEnvReq_T *get_env_req)
{
    cJSON *root         = NULL;
    cJSON *json_level1  = NULL;
	char szLogInfo[256] = {0};
	
	// 判断函数参数是否合法
	if ((NULL == json_content) || (NULL == get_env_req))
	{
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "json_content or get_env_req is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
	    return MUXDEMUX_ERR;
	}

    root = cJSON_Parse(json_content);
    if(NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "call cJSON_Parse failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }

    // 解析accessToken
    json_level1 = cJSON_GetObjectItem(root, "accessToken");
    if(NULL == json_level1)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "not find accessToken!");
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }
    if(cJSON_String == json_level1->type)
    {
        strncpy(get_env_req->accessToken, json_level1->valuestring, sizeof(get_env_req->accessToken)-1);
    }
    else
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "accessToken type is %d, not expected type!", json_level1->type);
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }

	// 解析collector_id
    json_level1 = cJSON_GetObjectItem(root, "collector_id");
    if(NULL == json_level1)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "not find collector_id!");
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }
    if(cJSON_String == json_level1->type)
    {
        strncpy(get_env_req->collector_id, json_level1->valuestring, sizeof(get_env_req->collector_id)-1);
    }
    else
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "collector_id type is %d, not expected type!", json_level1->type);
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }
	
    cJSON_Delete(root);
    return MUXDEMUX_OK;
}

// 获取环境参数请求json解析
int demux_get_env_req_no_verify(const char *json_content,  GetEnvReqNoVerify_T *get_env_req)
{
    cJSON *root         = NULL;
    cJSON *json_level1  = NULL;
	char szLogInfo[256] = {0};
	
	// 判断函数参数是否合法
	if ((NULL == json_content) || (NULL == get_env_req))
	{
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "json_content or get_env_req is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
	    return MUXDEMUX_ERR;
	}

    root = cJSON_Parse(json_content);
    if(NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "call cJSON_Parse failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }

	// 解析collector_id
    json_level1 = cJSON_GetObjectItem(root, "collector_id");
    if(NULL == json_level1)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "not find collector_id!");
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }
    if(cJSON_String == json_level1->type)
    {
        strncpy(get_env_req->collector_id, json_level1->valuestring, sizeof(get_env_req->collector_id)-1);
    }
    else
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "collector_id type is %d, not expected type!", json_level1->type);
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }
	
    cJSON_Delete(root);
    return MUXDEMUX_OK;
}


// 获取环境参数响应json封装
int mux_get_env_ack(GetEnvRsp_T *get_env_ack, char *json_content, int json_len)
{
    cJSON *root         = NULL;
	cJSON *json_level1  = NULL;
    char  *out          = NULL;
	char szLogInfo[256] = {0};
	int i;
	char colloct_str[32] = {0};
	
	// 判断函数参数是否合法
	if ((NULL == get_env_ack) || (NULL == json_content))
	{
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "get_env_ack or json_content is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
	    return MUXDEMUX_ERR;
	}
    
    root = cJSON_CreateObject();
    if(NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "exec cJSON_CreateObject to get root failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return MUXDEMUX_ERR;
    }
    cJSON_AddNumberToObject(root, "retCode", get_env_ack->iRetCode);
    cJSON_AddStringToObject(root, "retMsg", get_env_ack->szRetMsg);
	json_level1 = cJSON_CreateObject();
    if(NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "exec cJSON_CreateObject to get json_level1 failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return MUXDEMUX_ERR;
    }
	cJSON_AddItemToObject(root, "data", json_level1);
	cJSON_AddStringToObject(json_level1, "Temperature", get_env_ack->Temperature);
	cJSON_AddStringToObject(json_level1, "Humidity", get_env_ack->Humidity);
	cJSON_AddStringToObject(json_level1, "Illumination", get_env_ack->Illumination);
	cJSON_AddStringToObject(json_level1, "WindDirection", get_env_ack->WindDirection);
	cJSON_AddStringToObject(json_level1, "WindSpeed", get_env_ack->WindSpeed);
	
	out=cJSON_Print(root);
	strncpy(json_content, out, json_len - 1);
	json_content[json_len - 1] = '\0';

	cJSON_Delete(root);
	free(out);
	
    return MUXDEMUX_OK;
}

// 获取车位锁状态请求json解析
int demux_query_lock_status_req(const char *json_content, QueryLockStatusReq_T *query_lock_req)
{
    cJSON *root         = NULL;
    cJSON *json_level1  = NULL;
	char szLogInfo[256] = {0};
	
	// 判断函数参数是否合法
	if ((NULL == json_content) || (NULL == query_lock_req))
	{
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "json_content or query_lock_req is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
	    return MUXDEMUX_ERR;
	}

    root = cJSON_Parse(json_content);
    if(NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "call cJSON_Parse failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }

    // 解析accessToken
    json_level1 = cJSON_GetObjectItem(root, "accessToken");
    if(NULL == json_level1)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "not find accessToken!");
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }
    if(cJSON_String == json_level1->type)
    {
        strncpy(query_lock_req->accessToken, json_level1->valuestring, sizeof(query_lock_req->accessToken)-1);
    }
    else
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "accessToken type is %d, not expected type!", json_level1->type);
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }
	// 解析collector_id
    json_level1 = cJSON_GetObjectItem(root, "collector_id");
    if(NULL == json_level1)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "not find collector_id!");
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }
    if(cJSON_String == json_level1->type)
    {
        strncpy(query_lock_req->collector_id, json_level1->valuestring, sizeof(query_lock_req->collector_id)-1);
    }
    else
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "collector_id type is %d, not expected type!", json_level1->type);
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }
	
    cJSON_Delete(root);
    return MUXDEMUX_OK;
}

// 获取车位锁状态响应json封装
int mux_query_lock_status_ack(QueryLockStatusRsp_T *query_lock_ack, char *json_content, int json_len)
{
    cJSON *root         = NULL;
	cJSON *json_level1  = NULL;
    char  *out          = NULL;
	char szLogInfo[256] = {0};
	int i;
	char colloct_str[32] = {0};
	
	// 判断函数参数是否合法
	if ((NULL == query_lock_ack) || (NULL == json_content))
	{
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "query_lock_ack or json_content is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
	    return MUXDEMUX_ERR;
	}
    
    root = cJSON_CreateObject();
    if(NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "exec cJSON_CreateObject to get root failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return MUXDEMUX_ERR;
    }
	cJSON_AddNumberToObject(root, "retCode", query_lock_ack->iRetCode);
    cJSON_AddStringToObject(root, "retMsg", query_lock_ack->szRetMsg);
	json_level1 = cJSON_CreateObject();
    if(NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "exec cJSON_CreateObject to get json_level1 failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return MUXDEMUX_ERR;
    }
	cJSON_AddItemToObject(root, "data", json_level1);
    cJSON_AddNumberToObject(json_level1, "lockStatus", query_lock_ack->lockStatus);
	
	out=cJSON_Print(root);
	strncpy(json_content, out, json_len - 1);
	json_content[json_len - 1] = '\0';

	cJSON_Delete(root);
	free(out);
	
    return MUXDEMUX_OK;
}

// 控制车位锁开关请求json解析
int demux_control_lock_req(const char *json_content, CtlLockReq_T *control_lock_req)
{
    cJSON *root         = NULL;
    cJSON *json_level1  = NULL;
	char szLogInfo[256] = {0};
	
	// 判断函数参数是否合法
	if ((NULL == json_content) || (NULL == control_lock_req))
	{
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "json_content or control_lock_req is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
	    return MUXDEMUX_ERR;
	}

    root = cJSON_Parse(json_content);
    if(NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "call cJSON_Parse failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }

    // 解析accessToken
    json_level1 = cJSON_GetObjectItem(root, "accessToken");
    if(NULL == json_level1)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "not find accessToken!");
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }
    if(cJSON_String == json_level1->type)
    {
        strncpy(control_lock_req->accessToken, json_level1->valuestring, sizeof(control_lock_req->accessToken)-1);
    }
    else
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "accessToken type is %d, not expected type!", json_level1->type);
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }
	// 解析collector_id
    json_level1 = cJSON_GetObjectItem(root, "collector_id");
    if(NULL == json_level1)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "not find collector_id!");
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }
    if(cJSON_String == json_level1->type)
    {
        strncpy(control_lock_req->collector_id, json_level1->valuestring, sizeof(control_lock_req->collector_id)-1);
    }
    else
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "collector_id type is %d, not expected type!", json_level1->type);
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }
	// 解析lock_control
    json_level1 = cJSON_GetObjectItem(root, "lock_control");
    if(NULL == json_level1)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "not find lock_control!");
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }
    if(cJSON_Number == json_level1->type)
    {
		control_lock_req->lock_control = json_level1->valueint;
    }
    else
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "lock_control type is %d, not expected type!", json_level1->type);
        WRITELOG(LOG_ERROR, szLogInfo);
        cJSON_Delete(root);
        return MUXDEMUX_ERR;
    }
    cJSON_Delete(root);
    return MUXDEMUX_OK;
}

// 控制车位锁开关响应json封装
int mux_control_lock_ack(CtlLockRsp_T *control_lock_ack, char *json_content, int json_len)
{
    cJSON *root         = NULL;
    char  *out          = NULL;
	char szLogInfo[256] = {0};
	int i;
	char colloct_str[32] = {0};
	
	// 判断函数参数是否合法
	if ((NULL == control_lock_ack) || (NULL == json_content))
	{
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "control_lock_ack or json_content is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
	    return MUXDEMUX_ERR;
	}
    
    root = cJSON_CreateObject();
    if(NULL == root)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "exec cJSON_CreateObject to get root failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return MUXDEMUX_ERR;
    }
    cJSON_AddNumberToObject(root, "retCode", control_lock_ack->iRetCode);
    cJSON_AddStringToObject(root, "retMsg", control_lock_ack->szRetMsg);
	
	out=cJSON_Print(root);
	strncpy(json_content, out, json_len - 1);
	json_content[json_len - 1] = '\0';

	cJSON_Delete(root);
	free(out);
	
    return MUXDEMUX_OK;
}



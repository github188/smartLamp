/*****************************************************************************
Copyright: 2017-2026, Chongqing Optical&Electronical Information Institute.
File name: collector_proc.c
Description: maitain,manage the information on collectors
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

#include "cJSON.h"
#include "collector_proc.h"

/*************************************************
Function: serv_add_collector
Description: 增加集中器处理
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
int serv_add_collector(int index, const char *in, unsigned char *out, int len)
{
    return 0;
}

/*************************************************
Function: serv_del_collector
Description: 删除集中器处理
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
int serv_del_collector(int index, const char *in, unsigned char *out, int len)
{
    return 0;
}

/*************************************************
Function: serv_modify_collector
Description: 修改集中器处理
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
int serv_modify_collector(int index, const char *in, unsigned char *out, int len)
{
    return 0;
}

/*************************************************
Function: serv_queryid_collector
Description: 通过设备ID查询集中器处理
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
int serv_queryid_collector(int index, const char *in, unsigned char *out, int len)
{
    return 0;
}

/*************************************************
Function: serv_querygis_collector
Description: 通过地理位置信息查询集中器处理
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
int serv_querygis_collector(int index, const char *in, unsigned char *out, int len)
{
    return 0;
}



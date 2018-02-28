/*****************************************************************************
Copyright: 2017-2026, Chongqing Optical&Electronical Information Institute.
File name: chargepile_proc.c
Description: maitain,manage the information on charging-piles
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
#include "chargepile_proc.h"

/*************************************************
Function: serv_add_chargepile
Description: 增加充电桩处理
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
int serv_add_chargepile(int index, const char *in, unsigned char *out, int len)
{
    return 0;
}

/*************************************************
Function: serv_del_chargepile
Description: 删除充电桩处理
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
int serv_del_chargepile(int index, const char *in, unsigned char *out, int len)
{
    return 0;
}

/*************************************************
Function: serv_modify_chargepile
Description: 修改充电桩处理
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
int serv_modify_chargepile(int index, const char *in, unsigned char *out, int len)
{
    return 0;
}

/*************************************************
Function: serv_queryid_chargepile
Description: 通过设备ID查询充电桩处理
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
int serv_queryid_chargepile(int index, const char *in, unsigned char *out, int len)
{
    return 0;
}

/*************************************************
Function: serv_querygis_chargepile
Description: 通过地理位置信息查询充电桩处理
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
int serv_querygis_chargepile(int index, const char *in, unsigned char *out, int len)
{
    return 0;
}



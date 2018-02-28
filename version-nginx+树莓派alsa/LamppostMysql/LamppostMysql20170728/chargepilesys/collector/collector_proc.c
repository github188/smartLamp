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
Description: ���Ӽ���������
Calls: 
Called By: down_service_entry
Table Accessed: 
Table Updated:  
Input:  index--�߳�������
        in --ǰ�˷�����json��ʽ���ַ�����������
        len--out���泤��
Output: out--���͸�ǰ�˵�json��ʽ���ַ�����Ӧ����
Return: 0--success; ����--failure
Others: ��
*************************************************/
int serv_add_collector(int index, const char *in, unsigned char *out, int len)
{
    return 0;
}

/*************************************************
Function: serv_del_collector
Description: ɾ������������
Calls: 
Called By: down_service_entry
Table Accessed: 
Table Updated:  
Input:  index--�߳�������
        in --ǰ�˷�����json��ʽ���ַ�����������
        len--out���泤��
Output: out--���͸�ǰ�˵�json��ʽ���ַ�����Ӧ����
Return: 0--success; ����--failure
Others: ��
*************************************************/
int serv_del_collector(int index, const char *in, unsigned char *out, int len)
{
    return 0;
}

/*************************************************
Function: serv_modify_collector
Description: �޸ļ���������
Calls: 
Called By: down_service_entry
Table Accessed: 
Table Updated:  
Input:  index--�߳�������
        in --ǰ�˷�����json��ʽ���ַ�����������
        len--out���泤��
Output: out--���͸�ǰ�˵�json��ʽ���ַ�����Ӧ����
Return: 0--success; ����--failure
Others: ��
*************************************************/
int serv_modify_collector(int index, const char *in, unsigned char *out, int len)
{
    return 0;
}

/*************************************************
Function: serv_queryid_collector
Description: ͨ���豸ID��ѯ����������
Calls: 
Called By: down_service_entry
Table Accessed: 
Table Updated:  
Input:  index--�߳�������
        in --ǰ�˷�����json��ʽ���ַ�����������
        len--out���泤��
Output: out--���͸�ǰ�˵�json��ʽ���ַ�����Ӧ����
Return: 0--success; ����--failure
Others: ��
*************************************************/
int serv_queryid_collector(int index, const char *in, unsigned char *out, int len)
{
    return 0;
}

/*************************************************
Function: serv_querygis_collector
Description: ͨ������λ����Ϣ��ѯ����������
Calls: 
Called By: down_service_entry
Table Accessed: 
Table Updated:  
Input:  index--�߳�������
        in --ǰ�˷�����json��ʽ���ַ�����������
        len--out���泤��
Output: out--���͸�ǰ�˵�json��ʽ���ַ�����Ӧ����
Return: 0--success; ����--failure
Others: ��
*************************************************/
int serv_querygis_collector(int index, const char *in, unsigned char *out, int len)
{
    return 0;
}



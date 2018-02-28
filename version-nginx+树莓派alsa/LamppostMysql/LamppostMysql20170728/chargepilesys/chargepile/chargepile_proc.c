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
Description: ���ӳ��׮����
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
int serv_add_chargepile(int index, const char *in, unsigned char *out, int len)
{
    return 0;
}

/*************************************************
Function: serv_del_chargepile
Description: ɾ�����׮����
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
int serv_del_chargepile(int index, const char *in, unsigned char *out, int len)
{
    return 0;
}

/*************************************************
Function: serv_modify_chargepile
Description: �޸ĳ��׮����
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
int serv_modify_chargepile(int index, const char *in, unsigned char *out, int len)
{
    return 0;
}

/*************************************************
Function: serv_queryid_chargepile
Description: ͨ���豸ID��ѯ���׮����
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
int serv_queryid_chargepile(int index, const char *in, unsigned char *out, int len)
{
    return 0;
}

/*************************************************
Function: serv_querygis_chargepile
Description: ͨ������λ����Ϣ��ѯ���׮����
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
int serv_querygis_chargepile(int index, const char *in, unsigned char *out, int len)
{
    return 0;
}



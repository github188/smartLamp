#ifndef _SERVDATA_H_
#define _SERVDATA_H_

#ifdef _WIN32
  #pragma pack(push,1)
#else
  #pragma pack(1)
#endif

#include "jsondata.h"

#define MAX_DI_NUM     32

typedef struct
{
    char di[4];       // ���ݵ�Ԫ��ʶ
    char dev_no[2];   // װ�����
    char err;         // ������
}t_di_confirm_deny;   // ȷ��/����F3���ݵ�Ԫ��ʽ

typedef struct
{
    int num;
    t_di_confirm_deny confirm_deny[MAX_DI_NUM];
}t_di_confirm_deny_list;

typedef struct
{
    char err;
    //unsigned short len;
    char len[4];
    char filelist[MAX_FILELIST_LEN];
}t_play_list;


#ifdef _WIN32
  #pragma pack(pop)
#else
  #pragma pack()
#endif

#endif /*end _SERVDATA_H_*/



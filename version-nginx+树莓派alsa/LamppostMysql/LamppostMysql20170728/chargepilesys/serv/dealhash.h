#ifndef _DEALHASH_H_
#define _DEALHASH_H_

#ifdef _WIN32
  #pragma pack(push,1)
#else
  #pragma pack(1)
#endif

#include "uthash.h"
#include <fastcgi.h>
#include <fcgiapp.h>

extern pthread_rwlock_t msg_hash_lock;

#define DELAY_TIME	30   //超时时间
#define LIMIT_MSG	100	 //清理操作阈值

typedef struct
{
	int id; //消息序列号
	time_t time; //消息时间戳
	FCGX_Request *req; //nginx请求
	UT_hash_handle hh; //UTHash 句柄
}T_Hash_St;

void add_user(int user_id, FCGX_Request *req);
T_Hash_St * find_user(int user_id);
void delete_user(T_Hash_St *user);
void delete_all();
time_t time_sort(T_Hash_St *a, T_Hash_St *b);
void sort_by_time();
void clean_timeout_msg();

#ifdef _WIN32
  #pragma pack(pop)
#else
  #pragma pack()
#endif

#endif



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "dealhash.h"
#include "log.h"

static T_Hash_St * g_users = NULL;
pthread_rwlock_t msg_hash_lock;

void add_user(int user_id, FCGX_Request *req)
{
	T_Hash_St * s = NULL;
	char          szLogInfo[1024]  = { 0 };
	int msgCnt = 0;

	pthread_rwlock_wrlock(&msg_hash_lock);
	msgCnt = HASH_COUNT(g_users);
	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "hash items counts[%d]", msgCnt);
    WRITELOG(LOG_INFO, szLogInfo);
	// TODO: 优化清理消息数阈值及超时时间阈值
	//hash 表中请求数量过大不会影响查找效率。但是会占用内存过多
	//如果太频繁去做排序清理操作，会影响效率
	if (msgCnt >= LIMIT_MSG)
	{
		/*按时间排序、清理超时消息*/
		//pthread_rwlock_wrlock(&msg_clean_lock);
		sort_by_time();
		clean_timeout_msg();
		//pthread_rwlock_unlock(&msg_clean_lock);
	}
	
	HASH_FIND_INT(g_users, &user_id, s);
	if (s == NULL)
	{
		s = (T_Hash_St *)malloc(sizeof(T_Hash_St));
		memset(s, 0, sizeof(T_Hash_St));
		s->id = user_id;
		s->time = time(NULL);
		s->req = req;
		HASH_ADD_INT(g_users, id, s);
	}
	pthread_rwlock_unlock(&msg_hash_lock);
}

T_Hash_St * find_user(int user_id)
{
	T_Hash_St * s = NULL;
	//pthread_rwlock_rdlock(&msg_hash_lock);
	HASH_FIND_INT( g_users, &user_id, s );
	//pthread_rwlock_unlock(&msg_hash_lock);
	return s;
}

void delete_user(T_Hash_St *user) 
{
	//pthread_rwlock_wrlock(&msg_hash_lock);
    HASH_DEL(g_users, user); 
	//pthread_rwlock_unlock(&msg_hash_lock);
    free(user);
}

void delete_all() 
{
 	T_Hash_St *current_user, *tmp;
	pthread_rwlock_wrlock(&msg_hash_lock);
  	HASH_ITER(hh, g_users, current_user, tmp) 
  	{
    	HASH_DEL(g_users, current_user);  
    	free(current_user);           
  	}
	pthread_rwlock_unlock(&msg_hash_lock);
}

time_t time_sort(T_Hash_St *a, T_Hash_St *b) 
{
    return (a->time - b->time);
}

/*消息按先--->后顺序排序*/
void sort_by_time() 
{
	//pthread_rwlock_wrlock(&msg_hash_lock);
    HASH_SORT(g_users, time_sort);
	//pthread_rwlock_unlock(&msg_hash_lock);
}

void clean_timeout_msg()
{
	T_Hash_St *tmp = NULL;
	T_Hash_St *cur = NULL;
	time_t now = time(NULL);
	cur = g_users;
	char          szLogInfo[1024]  = { 0 };
	char rspMsg[1024] = {0};
	while(cur!=NULL)
	{
		if ((difftime(now,cur->time)) < (DELAY_TIME * 1.0))
			break;
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "clean_timeout_msg: serial[%d] time[%lu] now[%lu]", cur->id, cur->time, now);
    	WRITELOG(LOG_INFO, szLogInfo);
		tmp=cur;
		cur=(T_Hash_St *)(cur->hh.next);
		if (tmp->req != NULL)
		{
			memset(rspMsg, 0, sizeof(rspMsg));
			sprintf(rspMsg, "%s\r\n%s", RSP_HEADER, "request timeout!");
			FCGX_FPrintF(tmp->req->out, "%s" , rspMsg);
			FCGX_Finish_r(tmp->req);
			free(tmp->req);
		}
		delete_user(tmp);
	}
}




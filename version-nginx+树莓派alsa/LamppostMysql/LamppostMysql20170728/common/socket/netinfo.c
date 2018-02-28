#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "netinfo.h"
#include "../log/log.h"

t_net_node *net_head = NULL;

pthread_rwlock_t net_hash_lock;

void add_net(char *collector_id, t_net_info *netinfo)
{
    t_net_node *s = NULL;
	char   szLogInfo[1024] = { 0 };

	if (collector_id == NULL || netinfo == NULL)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "add_net: collector_id is NULL or netinfo is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return;
    }
	pthread_rwlock_wrlock(&net_hash_lock);
    HASH_FIND_STR(net_head, collector_id, s);  /* id already in the hash? */
    if (NULL == s)
    {
        s = (t_net_node *)malloc(sizeof(t_net_node));
        memset(s, 0, sizeof(t_net_node));
        strncpy(s->id, collector_id, MAX_ID_LEN);
		HASH_ADD_STR( net_head, id, s );   /* id: name of key field */
        /*
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "add_net: id=%s,collecor_id=%s", s->id, s->netinfo.collecor_id);
        WRITELOG(LOG_INFO, szLogInfo);
		*/
    }
    pthread_rwlock_unlock(&net_hash_lock);
    memcpy(&s->netinfo, netinfo, sizeof(t_net_info));

	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "add_net: fd=%d,collecor_id=%s", s->netinfo.fd, s->netinfo.collecor_id);
    WRITELOG(LOG_INFO, szLogInfo);
	
    return;
}

t_net_node *find_net(char *collector_id)
{
    t_net_node *s = NULL;
	char   szLogInfo[1024] = { 0 };
	
	if (collector_id == NULL)
	{
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "find_net: collector_id is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return NULL;
	}
	pthread_rwlock_rdlock(&net_hash_lock);
    HASH_FIND_STR( net_head, collector_id, s );  /* s: output pointer */
	pthread_rwlock_unlock(&net_hash_lock);
	if (s == NULL)
	{
	    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "find_net: exec HASH_FIND_STR to get s is NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return NULL;
	}
	
    return s;
}

void delete_net(t_net_node *net_node)
{
    char   szLogInfo[1024] = { 0 };
	
	if (net_node == NULL)
	{
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "delete_net: net_node is NULL!");
		WRITELOG(LOG_ERROR, szLogInfo);
		return;
	}
	pthread_rwlock_wrlock(&net_hash_lock);
    HASH_DEL(net_head, net_node);      /* user: pointer to deletee */
	pthread_rwlock_unlock(&net_hash_lock);
    free(net_node);
    return;
}

void delete_net_all()
{
    t_net_node *cur_net_node = NULL;
    t_net_node *tmp_net_node = NULL;
	pthread_rwlock_wrlock(&net_hash_lock);
    HASH_ITER(hh, net_head, cur_net_node, tmp_net_node)
    { 	
        HASH_DEL(net_head, cur_net_node);  /* delete it (users advances to next) */	
        free(cur_net_node);                /* free it */
    }
	pthread_rwlock_unlock(&net_hash_lock);
    return;
}

void set_net_info(char *collector_id, t_net_info *netinfo)
{
    t_net_node    *netnode = NULL;
	char   szLogInfo[1024] = { 0 };

    if (collector_id == NULL || netinfo == NULL)
	{
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "set_net_info: collector_id is NULL or netinfo is NULL!");
		WRITELOG(LOG_ERROR, szLogInfo);
		return;
	}
	
    netnode = find_net(collector_id);
    if (NULL != netnode)
    {
        netnode->netinfo = *netinfo;
    }
    return;
}

// °´ÕÕfdÉ¾³ý
void delete_net_fd(int fd)
{
    t_net_node *cur_net_node = NULL;
    t_net_node *tmp_net_node = NULL;
	pthread_rwlock_wrlock(&net_hash_lock);
    HASH_ITER(hh, net_head, cur_net_node, tmp_net_node)
    {
        if (fd == cur_net_node->netinfo.fd)
        {
            HASH_DEL(net_head, cur_net_node);  /* delete it */
            free(cur_net_node);                /* free it */
        }
    }
	pthread_rwlock_unlock(&net_hash_lock);
    return;
}



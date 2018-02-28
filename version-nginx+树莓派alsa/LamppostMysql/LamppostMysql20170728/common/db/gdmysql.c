#include "gdmysql.h"
#include "../cfg/config.h"

#ifdef _WIN32
/*  ����3������Ϊ��UNIX��WIN32������� */
static void pthread_mutex_init(pthread_mutex_t *param1, void *param2)
{
    *param1 = CreateSemaphore(NULL, 1, 1, NULL);
}

static void pthread_mutex_lock(pthread_mutex_t *param1)
{
    WaitForSingleObject(*param1, 5000); /* �����źŵƻ��� */
}

static void pthread_mutex_unlock(pthread_mutex_t *param1)
{
    ReleaseSemaphore(*param1, 1, NULL);
}
#endif

static pthread_mutex_t  s_dbmutex;
static pthread_mutex_t  s_reConnMutex;
MYSQL * g_db_conn = NULL;
extern Config_T g_Conf;

//��ʼ��MYSQL,���س�ʼ�����Ӿ��
void initmysql(void)
{    
	char  szLogInfo[500] = {0};
	pthread_mutex_init(&s_dbmutex, NULL);
	pthread_mutex_init(&s_reConnMutex, NULL); 
	snprintf(szLogInfo, sizeof(szLogInfo)-1, "initmysql ok.");
    WRITELOG(LOG_INFO, szLogInfo);	
	return;
}


//����MYSQL,�ɹ�����0
MYSQL *ConnectMySql(const char *hostip, const char *user, const char *passwd, const char *db, unsigned int port)
{
	MYSQL *connect = NULL;
	char  szLogInfo[1000] = {0};
	if (NULL == hostip || NULL == user || NULL == passwd || NULL == db)
	{		
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "NULL == conn_ptr || NULL == hostip || NULL == user || NULL == passwd || NULL == db");
        WRITELOG(LOG_ERROR, szLogInfo);
		return NULL;
	}
	//�����ӽ׶α����������Ϊ�����ӽ׶�mysql_init��mysql_real_connect�����̰߳�ȫ�ġ�
    pthread_mutex_lock(&s_dbmutex);
	connect = mysql_init(NULL);  	
    if(connect == NULL)
	{           
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "mysql_init error.");
	    pthread_mutex_unlock(&s_dbmutex);
        WRITELOG(LOG_ERROR, szLogInfo);
        return NULL;  
    }  
   
    if (mysql_real_connect(connect,hostip,user,passwd,db,port,NULL,0) == NULL)
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "mysql_real_connect hostip=%s,port=%d,db=%s failed,errno= %d: %s.", hostip,port,db,mysql_errno(connect), mysql_error(connect));
		pthread_mutex_unlock(&s_dbmutex);
        WRITELOG(LOG_ERROR, szLogInfo);		
		return NULL;
    }
	pthread_mutex_unlock(&s_dbmutex);
	
	snprintf(szLogInfo, sizeof(szLogInfo)-1, "mysql_real_connect hostip=%s,port=%d,db=%s success.",hostip,port,db);
    WRITELOG(LOG_INFO, szLogInfo);
	return connect;
}


//�ر����ݿ����ӣ��ɹ�����0
int CloseMySql(MYSQL *conn_ptr)
{
	char  szLogInfo[500] = {0};
	if (NULL == conn_ptr)
	{		
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "NULL == conn_ptr");
        WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
	}
	mysql_close(conn_ptr);
	return 0;
}


//ִ��SQL��䣬�������ӣ�ɾ������ѯ���޸ĵȣ��ɹ�����0
//ִ�а��������Ƶ�����
//sql��sql��䣬nLen��ʾSQL�ĳ���
int ExecuteBinSql(MYSQL *conn_ptr,const char*sql, int nLen)
{
	int nRet = 0;
	char  szLogInfo[1500] = {0};
	if (NULL == conn_ptr || NULL == sql)
	{		
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "NULL == conn_ptr || NULL == sql");
        WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
	}
	nRet = mysql_real_query(conn_ptr, sql, nLen);
	if (nRet != 0)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "mysql_real_query sql=%s failed,errno= %d: %s.",sql, mysql_errno(conn_ptr), mysql_error(conn_ptr));
        WRITELOG(LOG_ERROR, szLogInfo);
		return -2;
	}
    return 0;
}

//ִ��SQL��䣬�������ӣ�ɾ������ѯ���޸ĵȣ��ɹ�����0
//ִ�в����������Ƶ�����
//sql��sql��䣬nLen��ʾSQL�ĳ���
int ExecuteSql(MYSQL *conn_ptr,const char*sql)
{
	int nRet = 0;
	int nRet1 = 0;
	char  szLogInfo[1500] = {0};
	MYSQL *pConn = NULL;
	if (NULL == sql)
	{		
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "NULL == sql");
        WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
	}
	if (NULL == conn_ptr)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "NULL == conn_ptr");
        WRITELOG(LOG_ERROR, szLogInfo);
		g_db_conn = ConnectMySql("localhost", g_Conf.tDBInfo.szUserName, g_Conf.tDBInfo.szPassword, "gclamppost", 0);
		if (NULL == g_db_conn)
		{
			snprintf(szLogInfo, sizeof(szLogInfo)-1, "cant't connect to mysql");
        	WRITELOG(LOG_ERROR, szLogInfo);
			return -2;
		}
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "reconnect success!");
        WRITELOG(LOG_INFO, szLogInfo);
		conn_ptr = g_db_conn;
	}
	nRet = mysql_query(conn_ptr, sql);
	if (nRet != 0)
	{		
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "mysql_query sql=%s failed,errno= %d: %s.",sql, mysql_errno(conn_ptr), mysql_error(conn_ptr));
        WRITELOG(LOG_ERROR, szLogInfo);
		if ( 2006 == mysql_errno(conn_ptr) || 2013 == mysql_errno(conn_ptr) )
		{
			pthread_mutex_lock(&s_reConnMutex);
			nRet1 = mysql_query(g_db_conn, sql);
			if (0 == nRet1)
			{
				pthread_mutex_unlock(&s_reConnMutex);
				return 0;
			}
			g_db_conn = ConnectMySql("localhost", g_Conf.tDBInfo.szUserName, g_Conf.tDBInfo.szPassword, "gclamppost", 0);
			pConn = g_db_conn;
			pthread_mutex_unlock(&s_reConnMutex);
			if ( NULL != pConn )
			{
				snprintf(szLogInfo, sizeof(szLogInfo)-1, "reconnect success!");
        		WRITELOG(LOG_ERROR, szLogInfo);
				mysql_query(pConn, sql);
				return 0;
			}
			else
			{
				snprintf(szLogInfo, sizeof(szLogInfo)-1, "cant't connect to mysql");
        		WRITELOG(LOG_ERROR, szLogInfo);
				return -3;
			}
		}
		return -4;
	}
    return 0;
}

//ִ��SQL����ѯ������ȡ�����
//conn_ptr:���Ӿ��
//sql:sql���
//nLen:sql���ĳ���
//nIsBinary:�Ƿ�֧��2���ƣ�1֧�֣�0��֧��
//result:��������������
//pFiledCount:��������ֶθ������������
int ExecuteSqlQuery(MYSQL *conn_ptr,const char*sql,int nLen,int nIsBinary,MYSQL_RES *result,int *pFiledCount,int *pRowCount)
{
	int       nRet = 0;
	int       num_fields = 0;//�ֶθ���
    int       num_row = 0;//�����������
	MYSQL_RES *tmpresult = NULL;
	char  szLogInfo[1500] = {0};

	if (NULL == conn_ptr || NULL == sql || pRowCount == NULL || pFiledCount == NULL || result == NULL)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "NULL == conn_ptr || NULL == sql || pRowCount == NULL || pFiledCount == NULL || result == NULL");
        WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
	}
	if (nIsBinary == 1)//����2�������ݲ�ѯ
	{
		nRet = mysql_real_query(conn_ptr, sql, nLen);
		if (nRet != 0)
		{	    
			snprintf(szLogInfo, sizeof(szLogInfo)-1, "mysql_real_query sql=%s failed,errno= %d: %s.",sql, mysql_errno(conn_ptr), mysql_error(conn_ptr));
            WRITELOG(LOG_ERROR, szLogInfo);
	    	return -2;
		}
	}
	else
	{
		nRet =  mysql_query(conn_ptr, sql);
		if (nRet != 0)
		{
			snprintf(szLogInfo, sizeof(szLogInfo)-1, "mysql_query sql=%s failed,errno= %d: %s.", sql, mysql_errno(conn_ptr), mysql_error(conn_ptr));
            WRITELOG(LOG_ERROR, szLogInfo);
	    	return -2;
		}
	}
    tmpresult = mysql_store_result(conn_ptr);
	if (NULL == tmpresult)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "mysql_store_result failed,errno= %d: %s.", mysql_errno(conn_ptr), mysql_error(conn_ptr));
        WRITELOG(LOG_ERROR, szLogInfo);
	    return -3;
	}
	num_fields = mysql_num_fields(tmpresult);//���ؽ����������
	if (num_fields == 0)//����0���ֶΣ��϶����������
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "mysql_num_fields failed,errno= %d: %s.", mysql_errno(conn_ptr), mysql_error(conn_ptr));
        WRITELOG(LOG_ERROR, szLogInfo);
		mysql_free_result(tmpresult);
	    return -4;
	}
	num_row = (int)mysql_num_rows(tmpresult);//���ؽ����������
	*pFiledCount = num_fields;
	*pRowCount = num_row;
	snprintf(szLogInfo, sizeof(szLogInfo)-1, "query sql=%s success,num_row=%d,num_fields=%d.", sql,num_row,num_fields);
    WRITELOG(LOG_INFO, szLogInfo);
	memcpy(result, tmpresult, sizeof(MYSQL_RES));
	mysql_fetch_row(tmpresult);
	return 0;
}

//ִ�д洢����
//conn_ptr:���Ӿ��
//sql:sql���
//nLen:sql���ĳ���
//nIsBinary:�Ƿ�֧��2���ƣ�1֧�֣�0��֧��
int ExecuteProcdure(MYSQL *conn_ptr,const char*sql,int nLen,int nIsBinary)
{
	int       nRet = 0;
	int       num_fields = 0;//�ֶθ���
    int       num_row = 0;//�����������
	MYSQL_RES *tmpresult = NULL;
	char  szLogInfo[1500] = {0};

	if (NULL == conn_ptr || NULL == sql)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "NULL == conn_ptr || NULL == sql");
        WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
	}
	if (nIsBinary == 1)//����2�������ݲ�ѯ
	{
		nRet = mysql_real_query(conn_ptr, sql, nLen);
		if (nRet != 0)
		{
			snprintf(szLogInfo, sizeof(szLogInfo)-1, "mysql_real_query sql=%s failed,errno= %d: %s.", sql, mysql_errno(conn_ptr), mysql_error(conn_ptr));
            WRITELOG(LOG_ERROR, szLogInfo);
	    	return -2;
		}
	}
	else
	{
		nRet =  mysql_query(conn_ptr, sql);
		if (nRet != 0)
		{
			snprintf(szLogInfo, sizeof(szLogInfo)-1, "mysql_query sql=%s failed,errno= %d: %s.",sql, mysql_errno(conn_ptr), mysql_error(conn_ptr));
            WRITELOG(LOG_ERROR, szLogInfo);
	    	return -2;
		}
	}
    
	return 0;
}

//FETCH��ѯ�������Ҫʹ��whileѭ������FETCH���м�¼
MYSQL_ROW  DoFetchResult(MYSQL_RES *result)
{
	MYSQL_ROW  row;
	char  szLogInfo[500] = {0};
	if (NULL == result)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "NULL == result");
        WRITELOG(LOG_ERROR, szLogInfo);
		return NULL;
	}

	row = mysql_fetch_row(result);
	
	return row;
}

//�ͷŽ������ÿ�β�ѯ�����Ҫ���ͷţ��������ڴ�й¶
int  DoFreeMySqlResult(MYSQL_RES *result)
{
	char  szLogInfo[500] = {0};
	if (NULL == result)
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "NULL == result");
        WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
	}
    mysql_free_result(result);
	return 0;
}
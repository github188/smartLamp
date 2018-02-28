#ifndef _GDMYSQL_H_
#define _GDMYSQL_H_

#ifdef _WIN32
  #pragma pack(push,1)
#else
  #pragma pack(1)
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef _WIN32
#include <pthread.h>  
#include <errno.h> 
#endif

#include <mysql.h>
#include <my_global.h>
// #include "mysql.h"
#include "../log/log.h"

#ifdef _WIN32
#define  snprintf              _snprintf
#define  pthread_mutex_t       HANDLE
#endif

extern MYSQL * g_db_conn;


void       initmysql(void);
MYSQL      *ConnectMySql(const char *hostip, const char *user, const char *passwd, const char *db, unsigned int port);
int        CloseMySql(MYSQL *conn_ptr);
int        ExecuteBinSql(MYSQL *conn_ptr,const char*sql, int nLen);
int        ExecuteSql(MYSQL *conn_ptr,const char*sql);
int        ExecuteSqlQuery(MYSQL *conn_ptr,const char*sql,int nLen,int nIsBinary,MYSQL_RES *result,int *pFiledCount,int *pRowCount);
int        ExecuteProcdure(MYSQL *conn_ptr,const char*sql,int nLen,int nIsBinary);
MYSQL_ROW  DoFetchResult(MYSQL_RES *result);
int        DoFreeMySqlResult(MYSQL_RES *result);

#ifdef _WIN32
  #pragma pack(pop)
#else
  #pragma pack()
#endif

#endif

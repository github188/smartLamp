#ifndef _PARKLOCK_PROC_H_
#define _PARKLOCK_PROC_H_

#include "gdmysql.h"
#include "LockCtrl.h"
#include "frame.h"
#include "netinfo.h"
#include "netinf.h"
#include "cJSON.h"
#include "log.h"
#include "jsondata.h"
#include "servdata.h"
#include "upserv.h"
#include "global.h"



#ifdef _WIN32
  #pragma pack(push,1)
#else
  #pragma pack(1)
#endif

#define Max_ParkLock_Num  500


// 服务端响应消息(查询GIS时使用)
typedef struct
{
	char    szCollectorID[30];
    char    szChargepileID[30];
	char    szParklockID[30];
	char    szName[64];
	char    szLongitude[20];
	char    szLatitude[20];
	char    szDetailAddress[256];
	int     iLockStatus;         // 0-车位锁开；1-车位锁锁; 99-初始状态
} QueryLockGISRSP_T;


int serv_add_parklock(int index, const char *in, unsigned char *out, int len);
int serv_del_parklock(int index, const char *in, unsigned char *out, int len);
int serv_modify_parklock(int index, const char *in, unsigned char *out, int len);
int serv_queryid_parklock(int index, const char *in, unsigned char *out, int len);
int serv_querygis_parklock(int index, const char *in, unsigned char *out, int len);
int serv_control_parklock(int index, const char *in, unsigned char *out, int len);
int serv_realtimequery_parklock(int index, const char *in, unsigned char *out, int len);
int InsertLockInfoIntoDB(AddLockReq_T *ptAddLock, int *piRetVal);
int DeleteLockInfoInDB(DelLockReq_T *ptDelLock, int *piRetVal);
int ModifyLockInfoInDB(ModifyLockReq_T *ptModLock, int *piRetVal);
int UpdateLockInfoInDB(char *pszParklockID, int iLockStatus, int *piRetVal);
int QueryLockInfoFromDB(QueryLockReq_T *ptQueryLock, QueryLockRSP_T *ptQueryLockRSP);
int QueryLockInfoGISFromDB(QueryLockGISReq_T *ptQueryLockGIS, char *json_content, int json_len);
//int DealLockRspMsg(DealLockRSP_T *ptDealLockRSP, char *json_content, int json_len);
int DealQueryLockInfoRspMsg(QueryLockRSP_T *ptQueryLockRSP, char *json_content, int json_len);
int serv_query_parklock_status(int serial, const char *in, unsigned char *out, int len);
int serv_control_parklock_dev(int serial, const char *in, unsigned char *out, int len);


#ifdef _WIN32
  #pragma pack(pop)
#else
  #pragma pack()
#endif

#endif /*end _PARKLOCK_PROC_H_*/


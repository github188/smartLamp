#ifndef _SERV_H_
#define _SERV_H_

#ifdef _WIN32
  #pragma pack(push,1)
#else
  #pragma pack(1)
#endif

/******************宏定义****************/
/*线程池线程数*/
#define MAX_THREAD_NUM 				  10
#define MAX_THREAD_COUNT              64
#define MAX_URL_LEN                   128

#define COLLECTOR_ADD_URL             "/collector/devlist/add"
#define COLLECTOR_DELETE_URL          "/collector/devlist/delete"
#define COLLECTOR_MODIFY_URL          "/collector/devlist/modify"
#define COLLECTOR_QUERY_BY_ID_URL     "/collector/devlist/query/id"
#define COLLECTOR_QUERY_BY_GIS_URL    "/collector/devlist/query/gis"

#define CHARGEPILE_ADD_URL            "/charge-pile/devlist/add"
#define CHARGEPILE_DELETE_URL         "/charge-pile/devlist/delete"
#define CHARGEPILE_MODIFY_URL         "/charge-pile/devlist/modify"
#define CHARGEPILE_QUERY_BY_ID_URL    "/charge-pile/devlist/query/id"
#define CHARGEPILE_QUERY_BY_GIS_URL   "/charge-pile/devlist/query/gis"


#define PARKLOCK_ADD_URL              "/park-lock/devlist/add"
#define PARKLOCK_DELETE_URL           "/park-lock/devlist/delete"
#define PARKLOCK_MODIFY_URL           "/park-lock/devlist/modify"
#define PARKLOCK_QUERY_BY_ID_URL      "/park-lock/devlist/query/id"
#define PARKLOCK_QUERY_BY_GIS_URL     "/park-lock/devlist/query/gis"
#define PARKLOCK_CONTROL_URL          "/park-lock/devlist/control"
#define PARKLOCK_REALTIMEQUERY_URL    "/park-lock/devlist/realtimequery"
#define MUSIC_GET_PLAYLIST_URL        "/music/playlist/get"
//#define MUSIC_GET_PLAYLIST_URL        "req=musicplaylistget"
#define MUSIC_PLAYSTART_MUSIC_URL     "/music/play/start"

#define ENVIR_GET_PARAM_URL           "/envir/collect"
#define ENVIR_GET_ALL_PARAM_URL       "/envir/all/collect"
#define MUSIC_PLAYCTRL_MUSIC_URL      "/music/play/control"
#define MUSIC_SETVOLUME_MUSIC_URL     "/music/set/volume"
#define MUSIC_GETVOLUME_MUSIC_URL	  "/music/get/volume"

#define	USER_LOGIN					  "/user-manage/login"
#define USER_LOGOUT					  "/user-manage/logout"
#define ENVIR_GET_VALUES			  "/envir/collect-values"
#define ENVIR_GET_VALUES_NO_VERIFY    "/envir/collect-env"
#define PARKLOCK_QUERY_STATUS		  "/park-lock/devlist/query-status"
#define PARKLOCK_CONTROL			  "/park-lock/devlist/control-dev"	

/******************结构体定义****************/
typedef int (*ServProcFunc)(int serial, const char *in, unsigned char *out, int len);
typedef struct
{
    char             url[MAX_URL_LEN];    
    ServProcFunc     pfServProc;           
}T_Serv_Proc;


/******************函数声明****************/
static void *down_service_entry(void *arg);
void ReadConfigureInfo();
int get_msg_serial();


#ifdef _WIN32
  #pragma pack(pop)
#else
  #pragma pack()
#endif

#endif /*end _SERV_H_*/



#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#ifdef _WIN32
  #pragma pack(push,1)
#else
  #pragma pack(1)
#endif

#include "frame.h"
#include "pthread.h"
//#include "music_proc.h"

#define SOCK_RECV_MSGKEY       1024
#define SOCK_SEND_MSGKEY       1025

#define MAX_THREAD_COUNT       64
#define MAX_BUF_LEN            4*1024

#define MAX_PATH_LEN           255

#define MAX_FILENAME_LEN    125

#define min(a, b)    ((a) < (b) ? (a) : (b))

typedef struct
{
    long   mtype;              /* Message type. */
    char   mtext[MAX_BUF_LEN]; /* Message text. */
}t_mymsg;

typedef struct
{
    int len;
    char buf[MAX_BUF_LEN-sizeof(int)];
}t_buf;

//add by tianyu
typedef enum{
	//登录
	SMART_LAMP_LOGIN = 3,
	SMART_LAMP_HEARTBEAT,
	
	//集中器处理
	SMART_LAMP_ADD_COLLECTOR = 11,
	SMART_LAMP_DEL_COLLECTOR,
	SMART_LAMP_MODIFY_COLLECTOR,
	SMART_LAMP_QUERYID_COLLECTOR,
	SMART_LAMP_QUERYGIS_COLLECTOR,
	//充电桩处理
	SMART_LAMP_ADD_CHARGEPILE = 21,
	SMART_LAMP_DEL_CHARGEPILE,
	SMART_LAMP_MODIFY_CHARGEPILE,
	SMART_LAMP_QUERYID_CHARGEPILE,
	SMART_LAMP_QUERYGIS_CHARGEPILE,
	//车位锁处理
	SMART_LAMP_ADD_PARKLOCK = 31,
	SMART_LAMP_DEL_PARKLOCK,
	SMART_LAMP_MODIFY_PARKLOCK,
	SMART_LAMP_QUERYID_PARKLOCK,
	SMART_LAMP_QUERYGIS_PARKLOCK,
	SMART_LAMP_CONTROL_PARKLOCK,
	SMART_LAMP_REALTIMEQUERY_PARKLOCK,
	//音乐播放处理
	SMART_LAMP_GETPLAYLIST_MUSIC = 41,
	SMART_LAMP_PLAYSTART_MUSIC,
	SMART_LAMP_PLAYCTRL_MUSIC,
	SMART_LAMP_SETVOLUME_MUSIC,
	//环境参数采集处理
	SMART_LAMP_GET_ENVIR_PARAM = 51,
	SMART_LAMP_ALL_ENVIR_PARAM,
	//路灯控制处理
}SmartLampClientMsg;


extern int g_iRetVal;//for car lock

extern int g_sockfd;
//add by tianyu
extern unsigned int g_uiLoginNoAckCnt;
extern pthread_t g_io_thread;
extern int g_pipe[2];
extern pthread_mutex_t g_socket_mutex;
extern pthread_mutex_t g_music_mutex;
extern pthread_mutex_t g_pipe_mutex;

extern int g_serial_fd; 
extern int g_recv_msqid;
extern int g_send_msqid;
extern pthread_t g_recvtid;
extern pthread_t g_sendtid;
extern pthread_t g_eviromentid;

extern pthread_t g_aDownservTid[MAX_THREAD_COUNT];
extern unsigned int g_uiServThreadNum;

extern const char g_strAudioPath[MAX_PATH_LEN + 1];

extern char g_acServerIp[32];
extern unsigned short g_usServerPort;

extern char collector_id[MAX_ADDR_LEN+1];
extern pthread_t g_aServTid[MAX_THREAD_COUNT];

extern unsigned int g_uiHeartbeatCycle;

extern unsigned int g_uiLoginCycle;
extern unsigned int g_uiLoginSuccess;

extern unsigned int g_uiHeartbeatNoAckCnt;

extern char g_strCurrentSongName[MAX_FILENAME_LEN + 1];

extern unsigned int g_EviromentSensorsOrNot;

int Sleep(int count);


#ifdef _WIN32
  #pragma pack(pop)
#else
  #pragma pack()
#endif

#endif /*end _GLOBAL_H_*/



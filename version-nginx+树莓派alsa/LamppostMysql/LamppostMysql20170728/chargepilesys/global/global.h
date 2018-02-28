#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#ifdef _WIN32
  #pragma pack(push,1)
#else
  #pragma pack(1)
#endif

//#define SOCK_RECV_MSGKEY       1024
//#define SOCK_SEND_MSGKEY       1025
#define SERV_MSGKEY            1030

//#define MAX_ID_LEN             12

//extern int g_recv_msqid;
//extern int g_send_msqid;
extern int g_serv_msqid;
//extern unsigned int g_uiEnvironmentParamCycle;

//add by tianyu
#define RSP_HEADER "Content-type: text/html\r\n"
//#define RSP_HEADER "Content-type: application/json\r\n"
typedef enum{
	//集中器处理
	SMART_LAMP_ADD_COLLECTOR = 1,
	SMART_LAMP_DEL_COLLECTOR,
	SMART_LAMP_MODIFY_COLLECTOR,
	SMART_LAMP_QUERYID_COLLECTOR,
	SMART_LAMP_QUERYGIS_COLLECTOR,
	//充电桩处理
	SMART_LAMP_ADD_CHARGEPILE = 11,
	SMART_LAMP_DEL_CHARGEPILE,
	SMART_LAMP_MODIFY_CHARGEPILE,
	SMART_LAMP_QUERYID_CHARGEPILE,
	SMART_LAMP_QUERYGIS_CHARGEPILE,
	//车位锁处理
	SMART_LAMP_ADD_PARKLOCK = 21,
	SMART_LAMP_DEL_PARKLOCK,
	SMART_LAMP_MODIFY_PARKLOCK,
	SMART_LAMP_QUERYID_PARKLOCK,
	SMART_LAMP_QUERYGIS_PARKLOCK,
	SMART_LAMP_CONTROL_PARKLOCK,
	SMART_LAMP_REALTIMEQUERY_PARKLOCK,
	//音乐播放处理
	SMART_LAMP_GETPLAYLIST_MUSIC = 31,
	SMART_LAMP_PLAYSTART_MUSIC,
	SMART_LAMP_PLAYCTRL_MUSIC,
	SMART_LAMP_SETVOLUME_MUSIC,
	//环境参数采集处理
	SMART_LAMP_GET_ENVIR_PARAM = 41,
	SMART_LAMP_ALL_ENVIR_PARAM,
	//路灯控制处理
}SmartLampServMsg;


void dumpInfo(unsigned char *info, int length);

int Sleep(int count);


#ifdef _WIN32
  #pragma pack(pop)
#else
  #pragma pack()
#endif

#endif /*end _GLOBAL_H_*/



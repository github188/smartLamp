#ifndef _JSONDATA_H_
#define _JSONDATA_H_

#ifdef _WIN32
	#pragma pack(push, 1)
#else
	#pragma pack(1)
#endif

//#include "netinfo.h" 

#define MUXDEMUX_OK    0
#define MUXDEMUX_ERR   1

#define MAX_RETMSG_LEN      255
#define MAX_FILENAME_LEN    125        
#define MAX_FILELIST_LEN    4*1024
#define MAX_FILE_NUM        255
#define MAX_MUSIC_LENGTH    16
#define MAX_ID_LEN 			23
#define	MAX_COLLECTOR_NUM 	256

// 获取播放列表请求
typedef struct
{
    char collector_id[MAX_ID_LEN + 1];    // 集中器ID
}t_playlist_req;

// 获取播放列表响应
typedef struct
{
    int     retcode;                          // 结果码
    char    retmsg[MAX_RETMSG_LEN + 1];       // 结果信息
    char    filelist[MAX_FILELIST_LEN + 1];   // 文件列表
}t_playlist_ack;

typedef struct
{
	//char collector_id[MAX_ID_LEN + 1];
	int music_id;
	char music_name[MAX_FILENAME_LEN + 1];
	char music_length[MAX_MUSIC_LENGTH + 1];
}t_music_file;

typedef struct
{
	int     retcode;                          // 结果码
    char    retmsg[MAX_RETMSG_LEN + 1];       // 结果信息
    int 	filenum;	//文件数
    t_music_file	filelist[MAX_FILE_NUM];	//文件列表
}t_playlist_ack_db;

// 开始播放请求
typedef struct
{
    char collector_id[MAX_ID_LEN + 1];    // 集中器ID
    char filename[MAX_FILENAME_LEN + 1];      // 播放文件名
}t_playstart_req;

// 开始播放响应
typedef struct
{
    int     retcode;                      // 结果码
    char    retmsg[MAX_RETMSG_LEN + 1];   // 结果信息
}t_playstart_ack;



// 播放控制请求
typedef struct
{
    char collector_id[MAX_ID_LEN + 1];    // 集中器ID
    int  command;                         // 播放控制指令，1--暂停播放;2--恢复播放;3--停止播放
}t_playctrl_req;

// 播放控制响应
typedef struct
{
    int     retcode;                      // 结果码
    char    retmsg[MAX_RETMSG_LEN + 1];   // 结果信息
}t_playctrl_ack;



// 音量设置请求
typedef struct
{
    char collector_id[MAX_ID_LEN + 1];    // 集中器ID
    int  volume;                          // 音量值，为百分比放大100倍，有效范围0-100
}t_setvolume_req;

// 音量设置响应
typedef struct
{
    int     retcode;                      // 结果码
    char    retmsg[MAX_RETMSG_LEN + 1];   // 结果信息
}t_setvolume_ack;

//音量获取请求
typedef struct
{
	char collector_id[MAX_ID_LEN + 1];
}t_getvolume_req;

typedef struct
{
	int retcode;
	char retmsg[MAX_RETMSG_LEN + 1];
	int volume;
}t_getvolume_ack;

//获取环境参数请求,只能获取指定项的参数
typedef struct
{ 
    char collector_id[MAX_ID_LEN + 1];    // 集中器ID
    int nReqType;     //  1表示温度，2表示湿度，3表示照度，4表示风向，5表示风速
    }t_getenvirparam_req;

//获取环境参数应答，只能获取指定项的参数
typedef struct
{
    int     retcode;                                           // 结果码，成功为0，其他为错误
    char    retmsg[24];                                       // 结果信息，成功为SUCCESS，其他为错误信息
    int nReqType;     //  1表示温度，2表示湿度，3表示照度，4表示风向，5表示风速
    char    szValue[8];//结果值
}t_getenvirparam_ack;


//获取所有环境参数请求，包括温度，湿度，照度，风速，风向
typedef struct
{
    char collector_id[MAX_ID_LEN + 1];    // 集中器ID    
    int  nReqType;//为固定值6
}t_get_all_envirparam_req;

//获取所有环境参数应答，包括温度，湿度，照度，风速，风向
typedef struct
{
    int     retcode;                                           // 结果码,0为正常,其他为错误
    char    retmsg[24];                                       // 结果信息，成功为SUCCESS，其他为错误信息
    char    szWenduValue[8];                                  //温度值
    char    szShiDuVlaue[8];                                  //温度值
    char    szZhaoDuVlaue[8];                                 //照度值
    char    szFengXiangValue[8];                              //风向值
    char    szFengSuValue[8];                                 //风速值
}t_get_all_envirparam_ack;

// 增加车位锁消息结构体
// 前端请求消息
typedef struct
{
    char    szCollectorID[32];
    char    szChargepileID[32];
	char    szParklockID[32];
	char    szName[64];
	char    szLongitude[20];
	char    szLatitude[20];
	char    szDetailAddress[256];
} AddLockReq_T;

// 删除车位锁消息结构体
// 前端请求消息
typedef struct
{
	char    szParklockID[32];
} DelLockReq_T;

// 修改车位锁消息结构体
// 前端请求消息
typedef struct
{
    char    szCollectorID[32];
    char    szChargepileID[32];
	char    szParklockID[32];
	char    szName[64];
	char    szLongitude[20];
	char    szLatitude[20];
	char    szDetailAddress[256];
} ModifyLockReq_T;

// 查询车位锁(id)消息结构体
// 前端请求消息
typedef struct
{
	char    szParklockID[32];
} QueryLockReq_T;

// 实时查询车位锁status消息结构体
// 前端请求消息
typedef struct
{
	char    szCollectorID[32];
	char    szParklockID[32];
} RealTimeQueryLockReq_T;


// 查询车位锁(GIS)消息结构体
// 前端请求消息
typedef struct
{
	char    szStartLongitude[20];
    char    szEndLongitude[20];
	char    szStartLatitude[20];
	char    szEndLatitude[20];
} QueryLockGISReq_T;


// 服务端响应消息(增删改,开关锁通用)
typedef struct
{
    int     iRetCode;      //0成功，1 id重复，2 id不存在，3服务器错误, 99其他
	char    szRetMsg[256];
} DealLockRSP_T;

// 服务端响应消息(id查询时使用)
typedef struct
{
    int     iRetCode;      //0成功，1 id重复，2 id不存在，3服务器错误, 99其他
	char    szRetMsg[256];
	char    szCollectorID[32];
    char    szChargepileID[32];
	char    szParklockID[32];
	char    szName[64];
	char    szLongitude[20];
	char    szLatitude[20];
	char    szDetailAddress[256];
	int     iLockStatus;         // 0-车位锁开；1-车位锁锁; 99-初始状态
} QueryLockRSP_T;


// 开关车位锁消息结构体
// 前端请求消息
typedef struct
{
	char    szCollectorID[32];
    char    szChargepileID[32];
	char    szParklockID[32];
	int     iLockCtrol;        // 0-开车位锁；1-锁车位锁
} ControlLockReq_T;

// 服务端响应消息(实时查询车位锁状态时使用)
typedef struct
{
    int     iRetCode;      //0成功，1 id重复，2 id不存在，3服务器错误, 99其他
	char    szRetMsg[256];
	int     iLockStatus;   // 0-车位锁开；1-车位锁锁; 99-初始状态
} RealTimeQueryLockRSP_T;

typedef struct
{
	char	user_name[32];		//用户名
	char	password[64];		//密码
}UserLogInReq_T;

typedef struct
{
	int     iRetCode;							 //0--登录成功, -1--登录失败
	char    szRetMsg[256];
	char 	accessToken[64];				     //登录token值
	int     collector_nums;						 //用户关联的集中器数
	char    collector[MAX_COLLECTOR_NUM][32];	 //集中器ID
}UserLogInRsp_T;

typedef struct
{
	char 	accessToken[64];		//登录token值
}UserLogOutReq_T;

typedef struct
{
	int 	iRetCode;			 //0--退出登录成功, -1--退出登录失败
	char 	szRetMsg[256];
}UserLogOutRsp_T;

typedef struct
{
	char 	accessToken[64];
	char 	collector_id[32];
}GetEnvReq_T;

typedef struct
{
	char 	collector_id[32];
}GetEnvReqNoVerify_T;

typedef struct
{
	int 	iRetCode;			//0--操作成功;   -1 -- 操作失败
	char 	szRetMsg[256];
	char	Temperature[8];		//空气温度
	char	Humidity[8];			//空气湿度
	char	Illumination[8];		//光照强度
	char	WindDirection[8];		//风向
	char	WindSpeed[8];			//风速
}GetEnvRsp_T;

typedef struct
{
	char 	accessToken[64];
	char 	collector_id[32];
}QueryLockStatusReq_T;

typedef struct
{
	int 	iRetCode;			//0--操作成功;   -1 -- 操作失败
	char 	szRetMsg[256];
	int		lockStatus;			//0--开启状态，1--关闭状态
}QueryLockStatusRsp_T;

typedef struct
{
	char 	accessToken[64];
	char 	collector_id[32];
	int		lock_control;			//0 -- 开锁		1--闭锁
}CtlLockReq_T;

typedef struct
{
	int 	iRetCode;			//0--操作成功;   -1 -- 操作失败
	char 	szRetMsg[256];
}CtlLockRsp_T;

int demux_playstart_req(const char *json_content, t_playstart_req *playstart_req);
int mux_playstart_ack(t_playstart_ack *playstart_ack, char *json_content, int json_len);
int mux_querymusiclist_ack_db(t_playlist_ack_db *playlist_ack, char *json_content, int json_len);

//获取环境参数
int demux_getenvparam_req(const char *json_content, t_getenvirparam_req *envparam_req);
int mux_getenvparam_ack(t_getenvirparam_ack *envparam_ack, char *json_content, int json_len);
int demux_get_all_envparam_req(const char *json_content, t_get_all_envirparam_req *all_envparam_req);
int mux_get_all_envparam_ack(t_get_all_envirparam_ack *all_envparam_ack, char *json_content, int json_len);

// 车位锁控制
int demux_addparklock_req(const char *json_content, AddLockReq_T *ptAddLockReq);
int mux_parklock_ack(DealLockRSP_T *ptDealLockRSP, char *json_content, int json_len);
int demux_delparklock_req(const char *json_content, DelLockReq_T *ptDelLockReq);
int demux_modparklock_req(const char *json_content, ModifyLockReq_T *ptModifyLockReq);
int demux_queryidparklock_req(const char *json_content, QueryLockReq_T *ptQueryLockReq);
int mux_queryidparklock_ack(QueryLockRSP_T *ptQueryLockRSP, char *json_content, int json_len);
int demux_querygisparklock_req(const char *json_content, QueryLockGISReq_T *ptQueryLockGISReq);
int demux_controlparklock_req(const char *json_content, ControlLockReq_T *ptControlLockReq);
int demux_realtimequeryparklock_req(const char *json_content, RealTimeQueryLockReq_T *ptRealTimeQueryLockReq);
int mux_realtimequeryparklock_ack(RealTimeQueryLockRSP_T *ptRealTimeQueryLockRSP, char *json_content, int json_len);
//新增
int demux_user_log_in_req(const char *json_content, UserLogInReq_T *long_in_req);
int mux_user_log_in_ack(UserLogInRsp_T *log_in_ack, char *json_content, int json_len);
int demux_user_log_out_req(const char *json_content, UserLogOutReq_T *long_out_req);
int mux_user_log_out_ack(UserLogOutRsp_T *log_out_ack, char *json_content, int json_len);
int demux_get_env_req(const char *json_content,  GetEnvReq_T *get_env_req);
int demux_get_env_req_no_verify(const char *json_content,  GetEnvReqNoVerify_T *get_env_req);
int mux_get_env_ack(GetEnvRsp_T *get_env_ack, char *json_content, int json_len);
int demux_query_lock_status_req(const char *json_content, QueryLockStatusReq_T *query_lock_req);
int mux_query_lock_status_ack(QueryLockStatusRsp_T *query_lock_ack, char *json_content, int json_len);
int demux_control_lock_req(const char *json_content, CtlLockReq_T *control_lock_req);
int mux_control_lock_ack(CtlLockRsp_T *control_lock_ack, char *json_content, int json_len);


#ifdef _WIN32
	#pragma pack(pop)
#else
	#pragma pack()
#endif

#endif /*end _JSONDATA_H_*/



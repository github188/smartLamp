#ifndef _SERV_H_
#define _SERV_H_

#ifdef _WIN32
  #pragma pack(push,1)
#else
  #pragma pack(1)
#endif

////////////////////////确认/否认AFN begin/////////////////////
#define AFN_CONFIRM_DENY       0x00

#define FN_DI                  3     // 按数据单元标识进行逐个确认/否认F3

////////////////////////确认/否认AFN end///////////////////////


////////////////////////链路接口检测AFN begin/////////////////////
#define AFN_LINK_CHK           0x02

#define FN_LOGIN               1     // 集中器登录F1
#define FN_HEARTBEAT           3     // 集中器心跳F3

////////////////////////链路接口检测AFN end///////////////////////


////////////////////////控制命令AFN begin/////////////////////
#define AFN_CTRL_CMD           0xA5

/* 智慧照明子系统 */
#define FN_ONOFF_SWITCH        20     // 控制主控开关F20
#define FN_IMME_DIM            25     // 控制立即调光F25

/* 充电桩子系统 */
#define FN_ONOFF_LOCK          140    // 控制车位锁开关F140

/* 公众娱乐子系统 */
#define FN_PLAYSTART_MUSIC     160    // 开始播放F160
#define FN_PLAYCTRL_MUSIC      161    // 播放控制F161
#define FN_SETVOLUME_MUSIC     162    // 音量调节F162
#define FN_GETVOLUME_MUSIC	   163    //音量获取 F163

////////////////////////控制命令AFN end///////////////////////

////////////////////////一类数据请求AFN begin/////////////////////
#define AFN_TYPE1_DATA_REQ     0xAC

/* 智慧照明子系统 */
#define  FN_DIM_VAL            7      // 请求调光值F7
#define  FN_SWITCH_IMME_INFO   52     // 请求主控开关即时信息F52

/* 环境参数子系统 */
#define FN_ENV_T               120    // 请求环境温度F120
#define FN_ENV_H               121    // 请求环境湿度F121
#define FN_ENV_WP              122    // 请求环境风力F122
#define FN_ENV_WD              123    // 请求环境风向F123
#define FN_ENV_SR              124    // 请求环境日照辐射F124
#define FN_ENV_ALL             125    // 请求所有环境参数F125

/* 充电桩子系统 */
#define FN_LOCK_STATUS         141    // 请求车位锁开关状态F140

/* 公众娱乐子系统 */
#define FN_PLAY_LIST           164    // 请求播放列表F160

////////////////////////一类数据请求AFN end///////////////////////

/*	新增*/
#define FN_LOCK_STATUS_NEW	   142
#define	FN_ONOFF_LOCK_NEW	   143

typedef struct
{
    unsigned char  afn;
    unsigned short fn;
    int (*serv_proc_req)(t_3761frame *frame, t_buf *buf);
    int (*serv_proc_ack)(t_buf *buf, t_3761frame *frame);
}t_service;


#ifdef _WIN32
  #pragma pack(pop)
#else
  #pragma pack()
#endif

#endif /*end _SERV_H_*/



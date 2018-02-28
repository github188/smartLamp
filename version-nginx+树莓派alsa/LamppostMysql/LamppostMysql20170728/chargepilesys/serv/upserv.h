#ifndef _UPSERV_H_
#define _UPSERV_H_

#ifdef _WIN32
  #pragma pack(push,1)
#else
  #pragma pack(1)
#endif

#include "frame.h"

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
#define FN_GETVOLUME_MUSIC	   163    // 音量获取F163

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
#define FN_ENV_ALL             125    //请求所有环境参数，温度，湿度，照度，风向，风速等

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
    int (*upserv_proc_func)(int fd, t_3761frame *frame);
}t_service;

void *up_service_entry(void *arg);
int up_service_proc(int fd, char *data, int len);
int terminal_login_proc(int fd, t_3761frame *frame);
int terminal_heartbeat_proc(int fd, t_3761frame *frame);
int lamp_onoff_proc(int fd, t_3761frame *frame);
int lamp_imme_dim_proc(int fd, t_3761frame *frame);
int parklock_onoff_proc(int fd, t_3761frame *frame);
int parklock_onoff_proc_new(int fd, t_3761frame *frame);
int ent_playstart_proc(int fd, t_3761frame *frame);
//add by tianyu
int ent_playctrl_proc(int fd, t_3761frame *frame);
int ent_setvolume_proc(int fd, t_3761frame *frame);
int ent_getvolume_proc(int fd, t_3761frame *frame);
int lamp_dim_val_proc(int fd, t_3761frame *frame);
int lamp_switch_imme_info_proc(int fd, t_3761frame *frame);
int env_temperature_proc(int fd, t_3761frame *frame);
int env_humidity_proc(int fd, t_3761frame *frame);
int env_windpower_proc(int fd, t_3761frame *frame);
int env_winddirection_proc(int fd, t_3761frame *frame);
int env_sunshineradiation_proc(int fd, t_3761frame *frame);
int env_sunshineall_proc(int fd, t_3761frame *frame);
int parklock_status_proc(int fd, t_3761frame *frame);
int parklock_status_proc_new(int fd, t_3761frame *frame);
int ent_playlist_proc(int fd, t_3761frame *frame);
int ent_playlist_proc_db(int fd, t_3761frame *frame);


#ifdef _WIN32
  #pragma pack(pop)
#else
  #pragma pack()
#endif

#endif /*end _UPSERV_H_*/



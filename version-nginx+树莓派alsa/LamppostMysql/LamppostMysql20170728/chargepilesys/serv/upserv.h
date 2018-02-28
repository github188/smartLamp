#ifndef _UPSERV_H_
#define _UPSERV_H_

#ifdef _WIN32
  #pragma pack(push,1)
#else
  #pragma pack(1)
#endif

#include "frame.h"

////////////////////////ȷ��/����AFN begin/////////////////////
#define AFN_CONFIRM_DENY       0x00

#define FN_DI                  3     // �����ݵ�Ԫ��ʶ�������ȷ��/����F3

////////////////////////ȷ��/����AFN end///////////////////////


////////////////////////��·�ӿڼ��AFN begin/////////////////////
#define AFN_LINK_CHK           0x02

#define FN_LOGIN               1     // ��������¼F1
#define FN_HEARTBEAT           3     // ����������F3

////////////////////////��·�ӿڼ��AFN end///////////////////////


////////////////////////��������AFN begin/////////////////////
#define AFN_CTRL_CMD           0xA5

/* �ǻ�������ϵͳ */
#define FN_ONOFF_SWITCH        20     // �������ؿ���F20
#define FN_IMME_DIM            25     // ������������F25

/* ���׮��ϵͳ */
#define FN_ONOFF_LOCK          140    // ���Ƴ�λ������F140

/* ����������ϵͳ */
#define FN_PLAYSTART_MUSIC     160    // ��ʼ����F160
#define FN_PLAYCTRL_MUSIC      161    // ���ſ���F161
#define FN_SETVOLUME_MUSIC     162    // ��������F162
#define FN_GETVOLUME_MUSIC	   163    // ������ȡF163

////////////////////////��������AFN end///////////////////////

////////////////////////һ����������AFN begin/////////////////////
#define AFN_TYPE1_DATA_REQ     0xAC

/* �ǻ�������ϵͳ */
#define  FN_DIM_VAL            7      // �������ֵF7
#define  FN_SWITCH_IMME_INFO   52     // �������ؿ��ؼ�ʱ��ϢF52

/* ����������ϵͳ */
#define FN_ENV_T               120    // ���󻷾��¶�F120
#define FN_ENV_H               121    // ���󻷾�ʪ��F121
#define FN_ENV_WP              122    // ���󻷾�����F122
#define FN_ENV_WD              123    // ���󻷾�����F123
#define FN_ENV_SR              124    // ���󻷾����շ���F124
#define FN_ENV_ALL             125    //�������л����������¶ȣ�ʪ�ȣ��նȣ����򣬷��ٵ�

/* ���׮��ϵͳ */
#define FN_LOCK_STATUS         141    // ����λ������״̬F140

/* ����������ϵͳ */
#define FN_PLAY_LIST           164    // ���󲥷��б�F160

////////////////////////һ����������AFN end///////////////////////

/*	����*/
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



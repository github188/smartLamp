#ifndef _SERV_H_
#define _SERV_H_

#ifdef _WIN32
  #pragma pack(push,1)
#else
  #pragma pack(1)
#endif

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
#define FN_GETVOLUME_MUSIC	   163    //������ȡ F163

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
#define FN_ENV_ALL             125    // �������л�������F125

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
    int (*serv_proc_req)(t_3761frame *frame, t_buf *buf);
    int (*serv_proc_ack)(t_buf *buf, t_3761frame *frame);
}t_service;


#ifdef _WIN32
  #pragma pack(pop)
#else
  #pragma pack()
#endif

#endif /*end _SERV_H_*/



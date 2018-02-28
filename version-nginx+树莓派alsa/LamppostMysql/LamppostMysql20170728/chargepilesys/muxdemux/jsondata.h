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

// ��ȡ�����б�����
typedef struct
{
    char collector_id[MAX_ID_LEN + 1];    // ������ID
}t_playlist_req;

// ��ȡ�����б���Ӧ
typedef struct
{
    int     retcode;                          // �����
    char    retmsg[MAX_RETMSG_LEN + 1];       // �����Ϣ
    char    filelist[MAX_FILELIST_LEN + 1];   // �ļ��б�
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
	int     retcode;                          // �����
    char    retmsg[MAX_RETMSG_LEN + 1];       // �����Ϣ
    int 	filenum;	//�ļ���
    t_music_file	filelist[MAX_FILE_NUM];	//�ļ��б�
}t_playlist_ack_db;

// ��ʼ��������
typedef struct
{
    char collector_id[MAX_ID_LEN + 1];    // ������ID
    char filename[MAX_FILENAME_LEN + 1];      // �����ļ���
}t_playstart_req;

// ��ʼ������Ӧ
typedef struct
{
    int     retcode;                      // �����
    char    retmsg[MAX_RETMSG_LEN + 1];   // �����Ϣ
}t_playstart_ack;



// ���ſ�������
typedef struct
{
    char collector_id[MAX_ID_LEN + 1];    // ������ID
    int  command;                         // ���ſ���ָ�1--��ͣ����;2--�ָ�����;3--ֹͣ����
}t_playctrl_req;

// ���ſ�����Ӧ
typedef struct
{
    int     retcode;                      // �����
    char    retmsg[MAX_RETMSG_LEN + 1];   // �����Ϣ
}t_playctrl_ack;



// ������������
typedef struct
{
    char collector_id[MAX_ID_LEN + 1];    // ������ID
    int  volume;                          // ����ֵ��Ϊ�ٷֱȷŴ�100������Ч��Χ0-100
}t_setvolume_req;

// ����������Ӧ
typedef struct
{
    int     retcode;                      // �����
    char    retmsg[MAX_RETMSG_LEN + 1];   // �����Ϣ
}t_setvolume_ack;

//������ȡ����
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

//��ȡ������������,ֻ�ܻ�ȡָ����Ĳ���
typedef struct
{ 
    char collector_id[MAX_ID_LEN + 1];    // ������ID
    int nReqType;     //  1��ʾ�¶ȣ�2��ʾʪ�ȣ�3��ʾ�նȣ�4��ʾ����5��ʾ����
    }t_getenvirparam_req;

//��ȡ��������Ӧ��ֻ�ܻ�ȡָ����Ĳ���
typedef struct
{
    int     retcode;                                           // ����룬�ɹ�Ϊ0������Ϊ����
    char    retmsg[24];                                       // �����Ϣ���ɹ�ΪSUCCESS������Ϊ������Ϣ
    int nReqType;     //  1��ʾ�¶ȣ�2��ʾʪ�ȣ�3��ʾ�նȣ�4��ʾ����5��ʾ����
    char    szValue[8];//���ֵ
}t_getenvirparam_ack;


//��ȡ���л����������󣬰����¶ȣ�ʪ�ȣ��նȣ����٣�����
typedef struct
{
    char collector_id[MAX_ID_LEN + 1];    // ������ID    
    int  nReqType;//Ϊ�̶�ֵ6
}t_get_all_envirparam_req;

//��ȡ���л�������Ӧ�𣬰����¶ȣ�ʪ�ȣ��նȣ����٣�����
typedef struct
{
    int     retcode;                                           // �����,0Ϊ����,����Ϊ����
    char    retmsg[24];                                       // �����Ϣ���ɹ�ΪSUCCESS������Ϊ������Ϣ
    char    szWenduValue[8];                                  //�¶�ֵ
    char    szShiDuVlaue[8];                                  //�¶�ֵ
    char    szZhaoDuVlaue[8];                                 //�ն�ֵ
    char    szFengXiangValue[8];                              //����ֵ
    char    szFengSuValue[8];                                 //����ֵ
}t_get_all_envirparam_ack;

// ���ӳ�λ����Ϣ�ṹ��
// ǰ��������Ϣ
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

// ɾ����λ����Ϣ�ṹ��
// ǰ��������Ϣ
typedef struct
{
	char    szParklockID[32];
} DelLockReq_T;

// �޸ĳ�λ����Ϣ�ṹ��
// ǰ��������Ϣ
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

// ��ѯ��λ��(id)��Ϣ�ṹ��
// ǰ��������Ϣ
typedef struct
{
	char    szParklockID[32];
} QueryLockReq_T;

// ʵʱ��ѯ��λ��status��Ϣ�ṹ��
// ǰ��������Ϣ
typedef struct
{
	char    szCollectorID[32];
	char    szParklockID[32];
} RealTimeQueryLockReq_T;


// ��ѯ��λ��(GIS)��Ϣ�ṹ��
// ǰ��������Ϣ
typedef struct
{
	char    szStartLongitude[20];
    char    szEndLongitude[20];
	char    szStartLatitude[20];
	char    szEndLatitude[20];
} QueryLockGISReq_T;


// �������Ӧ��Ϣ(��ɾ��,������ͨ��)
typedef struct
{
    int     iRetCode;      //0�ɹ���1 id�ظ���2 id�����ڣ�3����������, 99����
	char    szRetMsg[256];
} DealLockRSP_T;

// �������Ӧ��Ϣ(id��ѯʱʹ��)
typedef struct
{
    int     iRetCode;      //0�ɹ���1 id�ظ���2 id�����ڣ�3����������, 99����
	char    szRetMsg[256];
	char    szCollectorID[32];
    char    szChargepileID[32];
	char    szParklockID[32];
	char    szName[64];
	char    szLongitude[20];
	char    szLatitude[20];
	char    szDetailAddress[256];
	int     iLockStatus;         // 0-��λ������1-��λ����; 99-��ʼ״̬
} QueryLockRSP_T;


// ���س�λ����Ϣ�ṹ��
// ǰ��������Ϣ
typedef struct
{
	char    szCollectorID[32];
    char    szChargepileID[32];
	char    szParklockID[32];
	int     iLockCtrol;        // 0-����λ����1-����λ��
} ControlLockReq_T;

// �������Ӧ��Ϣ(ʵʱ��ѯ��λ��״̬ʱʹ��)
typedef struct
{
    int     iRetCode;      //0�ɹ���1 id�ظ���2 id�����ڣ�3����������, 99����
	char    szRetMsg[256];
	int     iLockStatus;   // 0-��λ������1-��λ����; 99-��ʼ״̬
} RealTimeQueryLockRSP_T;

typedef struct
{
	char	user_name[32];		//�û���
	char	password[64];		//����
}UserLogInReq_T;

typedef struct
{
	int     iRetCode;							 //0--��¼�ɹ�, -1--��¼ʧ��
	char    szRetMsg[256];
	char 	accessToken[64];				     //��¼tokenֵ
	int     collector_nums;						 //�û������ļ�������
	char    collector[MAX_COLLECTOR_NUM][32];	 //������ID
}UserLogInRsp_T;

typedef struct
{
	char 	accessToken[64];		//��¼tokenֵ
}UserLogOutReq_T;

typedef struct
{
	int 	iRetCode;			 //0--�˳���¼�ɹ�, -1--�˳���¼ʧ��
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
	int 	iRetCode;			//0--�����ɹ�;   -1 -- ����ʧ��
	char 	szRetMsg[256];
	char	Temperature[8];		//�����¶�
	char	Humidity[8];			//����ʪ��
	char	Illumination[8];		//����ǿ��
	char	WindDirection[8];		//����
	char	WindSpeed[8];			//����
}GetEnvRsp_T;

typedef struct
{
	char 	accessToken[64];
	char 	collector_id[32];
}QueryLockStatusReq_T;

typedef struct
{
	int 	iRetCode;			//0--�����ɹ�;   -1 -- ����ʧ��
	char 	szRetMsg[256];
	int		lockStatus;			//0--����״̬��1--�ر�״̬
}QueryLockStatusRsp_T;

typedef struct
{
	char 	accessToken[64];
	char 	collector_id[32];
	int		lock_control;			//0 -- ����		1--����
}CtlLockReq_T;

typedef struct
{
	int 	iRetCode;			//0--�����ɹ�;   -1 -- ����ʧ��
	char 	szRetMsg[256];
}CtlLockRsp_T;

int demux_playstart_req(const char *json_content, t_playstart_req *playstart_req);
int mux_playstart_ack(t_playstart_ack *playstart_ack, char *json_content, int json_len);
int mux_querymusiclist_ack_db(t_playlist_ack_db *playlist_ack, char *json_content, int json_len);

//��ȡ��������
int demux_getenvparam_req(const char *json_content, t_getenvirparam_req *envparam_req);
int mux_getenvparam_ack(t_getenvirparam_ack *envparam_ack, char *json_content, int json_len);
int demux_get_all_envparam_req(const char *json_content, t_get_all_envirparam_req *all_envparam_req);
int mux_get_all_envparam_ack(t_get_all_envirparam_ack *all_envparam_ack, char *json_content, int json_len);

// ��λ������
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
//����
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



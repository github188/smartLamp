#ifndef _LINK_CHK_H_
#define _LINK_CHK_H_

#ifdef _WIN32
  #pragma pack(push,1)
#else
  #pragma pack(1)
#endif

#define MAX_DI_NUM     32

typedef struct
{
    char di[4];       // ���ݵ�Ԫ��ʶ
    char dev_no[2];   // װ�����
    char err;         // ������
}t_di_confirm_deny;   // ȷ��/����F3���ݵ�Ԫ��ʽ

typedef struct
{
    int num;
    t_di_confirm_deny confirm_deny[MAX_DI_NUM];
}t_di_confirm_deny_list;

int parse_diconfirmdeny(char *data, int len, t_di_confirm_deny_list *di_confirm_deny_list);
int terminal_login_req(void);
int terminal_heartbeat_req(void);
int terminal_linkchk_ack(t_3761frame *frame, t_buf *buf);
int terminal_login_ack(t_di_confirm_deny_list *pdi_confirm_deny_list);
int terminal_heartbeat_ack(t_di_confirm_deny_list *pdi_confirm_deny_list);

#ifdef _WIN32
  #pragma pack(pop)
#else
  #pragma pack()
#endif

#endif /*end _LINK_CHK_H_*/


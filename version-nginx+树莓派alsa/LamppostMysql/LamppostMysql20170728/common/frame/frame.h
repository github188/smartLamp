#ifndef _FRAME_H_
#define _FRAME_H_

#ifdef _WIN32
  #pragma pack(push,1)
#else
  #pragma pack(1)
#endif

#define MAX_DATAUNIT_LEN    4*1024
#define MAX_ADDR_LEN        20

typedef struct
{
    char func_code:4;
    char fcv:1;
    char fcb_acd:1;
    char prm:1;
    char dir:1;
}t_ctrl_field;

typedef struct
{
    char p_r_seq:4;
    char con:1;
    char fin:1;
    char fir:1;
    char tpv:1;
}t_seq;

typedef struct
{
   unsigned char  afn;             // AFN
    t_seq seq;             // SEQ
    char  pn[2];           // DA
   unsigned char  fn[2];           // DT
    unsigned short len;    // 数据单元长度
    char  data[MAX_DATAUNIT_LEN];       // 数据单元
}t_user_data;

typedef struct
{
    t_ctrl_field ctrl_field; 
    //char addr[MAX_ADDR_LEN + 1];
    char addr[MAX_ADDR_LEN];
    t_user_data user_data;
}t_3761frame;

typedef struct
{
	char serial[12]; //消息序列号
}t_msg_header;

#ifdef _WIN32
  #pragma pack(pop)
#else
  #pragma pack()
#endif

#endif  /*end _FRAME_H_*/


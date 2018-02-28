#ifndef _LAMP_PROC_H_
#define _LAMP_PROC_H_

#ifdef _WIN32
  #pragma pack(push,1)
#else
  #pragma pack(1)
#endif


#ifdef _WIN32
  #pragma pack(pop)
#else
  #pragma pack()
#endif

int lamp_onoff_req(t_3761frame *frame, t_buf *buf);
int lamp_onoff_ack(t_buf *buf, t_3761frame *frame);
int lamp_imme_dim_req(t_3761frame *frame, t_buf *buf);
int lamp_imme_dim_ack(t_buf *buf, t_3761frame *frame);
int lamp_dim_val_req(t_3761frame *frame, t_buf *buf);
int lamp_dim_val_ack(t_buf *buf, t_3761frame *frame);
int lamp_switch_imme_info_req(t_3761frame *frame, t_buf *buf);
int lamp_switch_imme_info_ack(t_buf *buf, t_3761frame *frame);

#endif /*end _LAMP_PROC_H_*/


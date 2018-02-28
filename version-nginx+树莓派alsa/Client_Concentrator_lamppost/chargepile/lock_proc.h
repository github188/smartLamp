#ifndef _LOCK_PROC_H_
#define _LOCK_PROC_H_

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

int parklock_onoff_req(t_3761frame *frame, t_buf *buf);
int parklock_onoff_ack(t_buf *buf, t_3761frame *frame);
int parklock_onoff_ack_new(t_buf *buf, t_3761frame *frame);
int parklock_status_req(t_3761frame *frame, t_buf *buf);
int parklock_status_ack(t_buf *buf, t_3761frame *frame);
int parklock_status_ack_new(t_buf *buf, t_3761frame *frame);

#endif /*end _LOCK_PROC_H_*/


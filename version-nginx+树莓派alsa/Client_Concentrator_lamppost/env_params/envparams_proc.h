#ifndef _ENVPARAMS_PROC_H_
#define _ENVPARAMS_PROC_H_

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

int env_temperature_req(t_3761frame *frame, t_buf *buf);
int env_temperature_ack(t_buf *buf, t_3761frame *frame);
int env_humidity_req(t_3761frame *frame, t_buf *buf);
int env_humidity_ack(t_buf *buf, t_3761frame *frame);
int env_windpower_req(t_3761frame *frame, t_buf *buf);
int env_windpower_ack(t_buf *buf, t_3761frame *frame);
int env_winddirection_req(t_3761frame *frame, t_buf *buf);
int env_winddirection_ack(t_buf *buf, t_3761frame *frame);
int env_sunshineradiation_req(t_3761frame *frame, t_buf *buf);
int env_sunshineradiation_ack(t_buf *buf, t_3761frame *frame);
int env_all_req(t_3761frame *frame, t_buf *buf);
int env_all_ack(t_buf *buf, t_3761frame *frame);

#endif /*end _ENVPARAMS_PROC_H_*/


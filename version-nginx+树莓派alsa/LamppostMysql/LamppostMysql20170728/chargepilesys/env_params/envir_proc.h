#ifndef _ENVIR_PROC_H_
#define _ENVIR_PROC_H_

#ifdef _WIN32
  #pragma pack(push,1)
#else
  #pragma pack(1)
#endif

/*
int serv_get_envir_wendu(int index, const char *in, unsigned char *out, int len);
int serv_get_envir_shidu(int index, const char *in, unsigned char *out, int len);
int serv_get_envir_zhaodu(int index, const char *in, unsigned char *out, int len);
int serv_get_envir_fengxiang(int index, const char *in, unsigned char *out, int len);
int serv_get_envir_fengshu(int index, const char *in, unsigned char *out, int len);
int serv_get_envir_all(int index, const char *in, unsigned char *out, int len);
*/

int serv_get_envir_param(int index, const char *in, unsigned char *out, int len);
int serv_get_all_envir_param(int index, const char *in, unsigned char *out, int len);
//int serv_getANDsend_all_envirparam_to_mysql(const char *in);
int serv_user_login(int serial, const char *in, unsigned char *out, int len);
int serv_user_logout(int serial, const char *in, unsigned char *out, int len);
int serv_get_env_values(int serial, const char *in, unsigned char *out, int len);
int serv_get_env_values_no_verify(int serial, const char *in, unsigned char *out, int len);

#ifdef _WIN32
  #pragma pack(pop)
#else
  #pragma pack()
#endif

#endif /*end _ENVIR_PROC_H_*/


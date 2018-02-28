#ifndef _CHARGEPILE_PROC_H_
#define _CHARGEPILE_PROC_H_

#ifdef _WIN32
  #pragma pack(push,1)
#else
  #pragma pack(1)
#endif

int serv_add_chargepile(int index, const char *in, unsigned char *out, int len);
int serv_del_chargepile(int index, const char *in, unsigned char *out, int len);
int serv_modify_chargepile(int index, const char *in, unsigned char *out, int len);
int serv_queryid_chargepile(int index, const char *in, unsigned char *out, int len);
int serv_querygis_chargepile(int index, const char *in, unsigned char *out, int len);


#ifdef _WIN32
  #pragma pack(pop)
#else
  #pragma pack()
#endif

#endif /*end _CHARGEPILE_PROC_H_*/


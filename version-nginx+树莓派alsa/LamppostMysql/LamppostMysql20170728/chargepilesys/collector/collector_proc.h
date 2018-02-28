#ifndef _COLLECTOR_PROC_H_
#define _COLLECTOR_PROC_H_

#ifdef _WIN32
  #pragma pack(push,1)
#else
  #pragma pack(1)
#endif

int serv_add_collector(int index, const char *in, unsigned char *out, int len);
int serv_del_collector(int index, const char *in, unsigned char *out, int len);
int serv_modify_collector(int index, const char *in, unsigned char *out, int len);
int serv_queryid_collector(int index, const char *in, unsigned char *out, int len);
int serv_querygis_collector(int index, const char *in, unsigned char *out, int len);


#ifdef _WIN32
  #pragma pack(pop)
#else
  #pragma pack()
#endif

#endif /*end _COLLECTOR_PROC_H_*/



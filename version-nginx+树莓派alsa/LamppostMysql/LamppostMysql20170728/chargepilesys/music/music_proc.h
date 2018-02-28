#ifndef _MUSIC_PROC_H_
#define _MUSIC_PROC_H_

#ifdef _WIN32
  #pragma pack(push,1)
#else
  #pragma pack(1)
#endif

int serv_getplaylist_music(int index, const char *in, unsigned char *out, int len);
int serv_getplaylist_music_DB(int serial, const char *in, unsigned char *out, int len);
int serv_playstart_music(int index, const char *in, unsigned char *out, int len);
int serv_playctrl_music(int index, const char *in, unsigned char *out, int len);
int serv_setvolume_music(int index, const char *in, unsigned char *out, int len);
int serv_getvolume_music(int serial, const char *in, unsigned char *out, int len);
int QueryMusicNumFromDB(char *collector_id);

#ifdef _WIN32
  #pragma pack(pop)
#else
  #pragma pack()
#endif

#endif /*end _MUSIC_PROC_H_*/


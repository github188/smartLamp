#ifndef _MUSIC_PROC_H_
#define _MUSIC_PROC_H_

#ifdef _WIN32
  #pragma pack(push,1)
#else
  #pragma pack(1)
#endif

//#define MAX_FILENAME_LEN    32
#define MAX_FULLNAME_LEN    255
#define MAX_FILE_NUM        255

typedef struct
{
    unsigned short    usNum;
    char filelist[MAX_FILE_NUM][MAX_FILENAME_LEN + 1];
}T_FileList;

int GetAudioList(char *basePath, T_FileList *ptFileList);
int ent_playlist_req(t_3761frame *frame, t_buf *buf);
int ent_playlist_ack(t_buf *buf, t_3761frame *frame);
int ent_playstart_req(t_3761frame *frame, t_buf *buf);
int ent_playstart_ack(t_buf *buf, t_3761frame *frame);
int ent_playctrl_req(t_3761frame *frame, t_buf *buf);
int ent_playctrl_ack(t_buf *buf, t_3761frame *frame);
int ent_volumeset_req(t_3761frame *frame, t_buf *buf);
int ent_volumeset_ack(t_buf *buf, t_3761frame *frame);
int ent_volumeget_req(t_3761frame *frame, t_buf *buf);
int ent_volumeget_ack(t_buf *buf, t_3761frame *frame);

#ifdef _WIN32
  #pragma pack(pop)
#else
  #pragma pack()
#endif

#endif /*end _MUSIC_PROC_H_*/



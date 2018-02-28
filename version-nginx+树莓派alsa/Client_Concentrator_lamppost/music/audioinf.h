/*********************************************************
 * filename: audioinf.h
 * discribe: audio interface
 * author:   zhangwei
 * date  :   2017-03-06
 * modify details:
 * 1. 2017-03-6    created
 **********************************************************/
#ifndef _AUDIOINF_H_
#define _AUDIOINF_H_

#ifdef _WIN32
  #pragma pack(push,1)
#else
  #pragma pack(1)
#endif

#define ALSA_PCM_NEW_HW_PARAMS_API
#include    <alsa/asoundlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define    ADI_ERR    0
#define    ADI_OK     1

#define    PLAY_STATE       0
#define    PAUSE_STATE      1
#define    RESUME_STATE     2
#define    STOP_STATE       3

typedef struct _t_audio_params
{
    snd_pcm_access_t     access;       // 采样数据存储格式
    snd_pcm_format_t     format;       // 采样位数
    unsigned int         channels;     // 采样通道数
    unsigned int         rate;         // 采样率
    snd_pcm_uframes_t    period_size;  // 采样数据大小，以帧为单位
    unsigned int         period_time;  // 采样间隔时间，以us为单位
} t_audio_params;

//wav头的结构如下所示:
typedef struct    _header
{
    char              fccID[4];          // 资源交换文件标志("RIFF")
    unsigned long     dwSize;            // 从下个地址开始到文件结尾的总字节数,即从WAVE标志开始
    char              fccType[4];        // WAVE文件标志("WAVE")
}HEADER;

typedef struct    _fmt
{
    char              fccID[4];          // 波形格式标志("fmt ")
    unsigned long     dwSize;            // 过滤字节(一般为00000010H，即16表示FMT无附加信息，18则表示有2字节的附加信息)
    unsigned short    wFormatTag;        // 格式种类，为1时表示数据为线性PCM编码
    unsigned short    wChannels;         // 通道数，单通道为1，双通道为2
    unsigned long     dwSamplesPerSec;   // 采样率
    unsigned long     dwAvgBytesPerSec;  // 波形数据传输速率(bytes/s)
    unsigned short    wBlockAlign;       // 数据调整数(按字节计算，其值为通道数*样本数据位数/8)
    unsigned short    wBitsPerSample;    // 样本数据位数
}FMT;

typedef struct    _data
{
    char              fccID[4];          // 数据标识符("data")
    unsigned long     dwSize;            // 采样数据总数
}DATA;

int device_open(snd_pcm_stream_t stream);  // stream--播放时取值为SND_PCM_STREAM_PLAYBACK, 录音时取值SND_PCM_STREAM_CAPTURE
int device_setparams(t_audio_params *audio_params);
int device_capture(int duration, char *filename);    // duration--录音时长，秒为单位; filename--录音文件名
int device_play(char *filename);    // filename--播放文件名
int device_close();
int device_pause();
int device_resume();
int audio_addwavheader(char *src_file, int channels, int rate, int bits, char *dst_file);

int audio_record(t_audio_params *audio_params, int duration, char *pcm_file, char *wav_file);
int audio_play(t_audio_params *audio_params, char *pcm_file);
int audio_play_ex(t_audio_params *audio_params, FILE *pcm_fp);
int audio_getwavheader(char *filename, t_audio_params *audio_params);
int audio_getwavdata(char *src_file, char *dst_file);
int audio_play_wav(char *wav_file);
int audio_pause_wav();
int audio_resume_wav();
int audio_stop_wav();
int audio_set_volume(int vol);

extern unsigned char        guc_devopening  ;
//add by tianyu
extern int guc_ref;

static snd_pcm_hw_params_t *g_hw_params     = NULL;
static snd_pcm_t           *g_pcm_handle    = NULL;

static unsigned long        g_ulPlayCmd     = 0;

#ifdef __cplusplus
}
#endif

#ifdef _WIN32
  #pragma pack(pop)
#else
  #pragma pack()
#endif

#endif


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
    snd_pcm_access_t     access;       // �������ݴ洢��ʽ
    snd_pcm_format_t     format;       // ����λ��
    unsigned int         channels;     // ����ͨ����
    unsigned int         rate;         // ������
    snd_pcm_uframes_t    period_size;  // �������ݴ�С����֡Ϊ��λ
    unsigned int         period_time;  // �������ʱ�䣬��usΪ��λ
} t_audio_params;

//wavͷ�Ľṹ������ʾ:
typedef struct    _header
{
    char              fccID[4];          // ��Դ�����ļ���־("RIFF")
    unsigned long     dwSize;            // ���¸���ַ��ʼ���ļ���β�����ֽ���,����WAVE��־��ʼ
    char              fccType[4];        // WAVE�ļ���־("WAVE")
}HEADER;

typedef struct    _fmt
{
    char              fccID[4];          // ���θ�ʽ��־("fmt ")
    unsigned long     dwSize;            // �����ֽ�(һ��Ϊ00000010H����16��ʾFMT�޸�����Ϣ��18���ʾ��2�ֽڵĸ�����Ϣ)
    unsigned short    wFormatTag;        // ��ʽ���࣬Ϊ1ʱ��ʾ����Ϊ����PCM����
    unsigned short    wChannels;         // ͨ��������ͨ��Ϊ1��˫ͨ��Ϊ2
    unsigned long     dwSamplesPerSec;   // ������
    unsigned long     dwAvgBytesPerSec;  // �������ݴ�������(bytes/s)
    unsigned short    wBlockAlign;       // ���ݵ�����(���ֽڼ��㣬��ֵΪͨ����*��������λ��/8)
    unsigned short    wBitsPerSample;    // ��������λ��
}FMT;

typedef struct    _data
{
    char              fccID[4];          // ���ݱ�ʶ��("data")
    unsigned long     dwSize;            // ������������
}DATA;

int device_open(snd_pcm_stream_t stream);  // stream--����ʱȡֵΪSND_PCM_STREAM_PLAYBACK, ¼��ʱȡֵSND_PCM_STREAM_CAPTURE
int device_setparams(t_audio_params *audio_params);
int device_capture(int duration, char *filename);    // duration--¼��ʱ������Ϊ��λ; filename--¼���ļ���
int device_play(char *filename);    // filename--�����ļ���
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


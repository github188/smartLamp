#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    "audioinf.h"
#include    "log.h"
unsigned char        guc_devopening=0;
//add by tianyu
int       guc_ref=0;

#define MIN_VOL -10239
#define MAX_VOL 400

int device_open(snd_pcm_stream_t stream)
{
    int    err            =   0;
    char   szLogInfo[500] = { 0 };

    if(1 == guc_devopening)
    {
        return ADI_ERR;
    }
    err = snd_pcm_open(&g_pcm_handle, "default", stream, 0);
    if (err < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "device_open: open device failed: %s", snd_strerror(err));
        WRITELOG(LOG_ERROR, szLogInfo);
        return ADI_ERR;
    }
    
    err = snd_pcm_hw_params_malloc (&g_hw_params); //为参数变量分配空间
    if(err != 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "device_open: allocate memory failed: %s", snd_strerror(err));
        WRITELOG(LOG_ERROR, szLogInfo);
        snd_pcm_close(g_pcm_handle);
        g_pcm_handle = NULL;
        return ADI_ERR;
    }

    guc_devopening = 1;

    return ADI_OK;
}

int device_setparams(t_audio_params *audio_params)
{
    int    err            =   0;
    char   szLogInfo[500] = { 0 };
    
    if(0 == guc_devopening)
    {
        return ADI_ERR;
    }

    if(NULL == audio_params)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "set device_setparams: NULL == audio_params");
        WRITELOG(LOG_ERROR, szLogInfo);
        return ADI_ERR;
    }
        
    err = snd_pcm_hw_params_any (g_pcm_handle, g_hw_params); //参数初始化
    if(err < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "device_setparams: any failed: %s", snd_strerror(err));
        WRITELOG(LOG_ERROR, szLogInfo);
        return ADI_ERR;
    }
    
    err = snd_pcm_hw_params_set_access(g_pcm_handle, g_hw_params, audio_params->access);
    if(err < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "device_setparams: access failed: %s", snd_strerror(err));
        WRITELOG(LOG_ERROR, szLogInfo);
        return ADI_ERR;
    }
    
    err = snd_pcm_hw_params_set_format(g_pcm_handle, g_hw_params, audio_params->format);
    if(err < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "device_setparams: format failed: %s", snd_strerror(err));
        WRITELOG(LOG_ERROR, szLogInfo);
        return ADI_ERR;
    }
    
    err = snd_pcm_hw_params_set_rate_near(g_pcm_handle, g_hw_params, &audio_params->rate, NULL);
    if(err < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "device_setparams: rate failed: %s", snd_strerror(err));
        WRITELOG(LOG_ERROR, szLogInfo);
        return ADI_ERR;
    }
    
    err = snd_pcm_hw_params_set_channels(g_pcm_handle, g_hw_params, audio_params->channels);
    if(err < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "device_setparams: channels failed: %s", snd_strerror(err));
        WRITELOG(LOG_ERROR, szLogInfo);
        return ADI_ERR;
    }

    err = snd_pcm_hw_params_set_period_size_near(g_pcm_handle, g_hw_params, &audio_params->period_size, NULL);
    if(err < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "device_setparams: period_size failed: %s", snd_strerror(err));
        WRITELOG(LOG_ERROR, szLogInfo);
        return ADI_ERR;
    }
    
    err = snd_pcm_hw_params(g_pcm_handle, g_hw_params);
    if(err < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "device_setparams: failed: %s", snd_strerror(err));
        WRITELOG(LOG_ERROR, szLogInfo);
        return ADI_ERR;
    }
    
    return ADI_OK;
}

int device_getparams(t_audio_params *audio_params)
{
    int    err            =   0;
    char   szLogInfo[500] = { 0 };
    
    if(0 == guc_devopening)
    {
        return ADI_ERR;
    }
    
    if(NULL == audio_params)
    {
        return ADI_ERR;
    }

    err = snd_pcm_hw_params_get_access(g_hw_params, &audio_params->access);
    if(err < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "get params:access failed: %s", snd_strerror(err));
        WRITELOG(LOG_ERROR, szLogInfo);
        return ADI_ERR;
    }
    
    err = snd_pcm_hw_params_get_format(g_hw_params, &audio_params->format);
    if(err < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "get params:format failed: %s", snd_strerror(err));
        WRITELOG(LOG_ERROR, szLogInfo);
        return ADI_ERR;
    }
    
    err = snd_pcm_hw_params_get_channels(g_hw_params, &audio_params->channels);
    if(err < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "get params:channels failed: %s", snd_strerror(err));
        WRITELOG(LOG_ERROR, szLogInfo);
        return ADI_ERR;
    }
    
    err = snd_pcm_hw_params_get_rate(g_hw_params, &audio_params->rate, NULL);
    if(err < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "get params:rate failed: %s", snd_strerror(err));
        WRITELOG(LOG_ERROR, szLogInfo);
        return ADI_ERR;
    }
    
    err = snd_pcm_hw_params_get_period_size(g_hw_params, &audio_params->period_size, NULL);
    if(err < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "get params:period_size failed: %s", snd_strerror(err));
        WRITELOG(LOG_ERROR, szLogInfo);
        return ADI_ERR;
    }
    
    err = snd_pcm_hw_params_get_period_time(g_hw_params, &audio_params->period_time, NULL);
    if(err < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "get params:period_time failed: %s", snd_strerror(err));
        WRITELOG(LOG_ERROR, szLogInfo);
        return ADI_ERR;
    }
    
    return ADI_OK;
}

int device_capture(int duration, char *filename)
{
    int                  err            =   0;
    char                 szLogInfo[500] = { 0 };
    FILE *               fp             = NULL;
    int                  size           = 0;
    int                  cnt            = 0;
    t_audio_params       audio_params   = { 0 };
    char                *buffer         = NULL;
    snd_pcm_sframes_t    rc             = 0;

    if(0 == guc_devopening)
    {
        return ADI_ERR;
    }

    if(NULL == filename)
    {
        return ADI_ERR;
    }

    fp = fopen(filename, "wb");
    if(NULL == fp)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "open file failed! filename[%s]", filename);
        WRITELOG(LOG_ERROR, szLogInfo);
        return ADI_ERR;
    }
    
    err = device_getparams(&audio_params);
    if(err != ADI_OK)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "get params failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        fclose(fp);
        return err;
    }

    if((SND_PCM_FORMAT_S8 == audio_params.format)
       || (SND_PCM_FORMAT_U8 == audio_params.format))
    {
        size = audio_params.period_size * audio_params.channels;
    }
    else if((SND_PCM_FORMAT_S16_LE == audio_params.format)
       || (SND_PCM_FORMAT_S16_BE == audio_params.format)
       || (SND_PCM_FORMAT_U16_LE == audio_params.format)
       || (SND_PCM_FORMAT_U16_BE == audio_params.format))
    {
        size = audio_params.period_size * audio_params.channels * 2;
    }
    else
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "unsuport param format[%d]!", audio_params.format);
        WRITELOG(LOG_ERROR, szLogInfo);
        fclose(fp);
        return ADI_ERR;
    }
    
    buffer = (char *) malloc(size);
    if(NULL == buffer)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "allocate buffer failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        fclose(fp);
        return ADI_ERR;
    }
    
    cnt = (duration * 1000000) / audio_params.period_time;
    while (cnt > 0)
    {
        memset(buffer, 0, size);
        rc = snd_pcm_readi(g_pcm_handle, buffer, audio_params.period_size);        
        if (rc == -EPIPE)
        {
            snd_pcm_prepare(g_pcm_handle);
            continue;
        }
        else if (rc < 0)
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "error read from audio card: %s", snd_strerror(rc));
            WRITELOG(LOG_ERROR, szLogInfo);
            free(buffer);
            fclose(fp);
            return ADI_ERR;
        }
        
        rc = fwrite(buffer, 1, size, fp);
        if (rc != size)
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "error write to file[%s]: wrote %d bytes", filename, rc);
            WRITELOG(LOG_ERROR, szLogInfo);
            free(buffer);
            fclose(fp);
            return ADI_ERR;
        }
        
        cnt--;
    }

    free(buffer);
    fclose(fp);
    
    return ADI_OK;
}

int device_play(char *filename)
{
    int                  err            =   0;
    char                 szLogInfo[500] = { 0 };
    FILE *               fp             = NULL;
 	int fd = 0;   
    if(NULL == filename)
    {
         snprintf(szLogInfo, sizeof(szLogInfo) - 1, "file name is null!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return ADI_ERR;
    }

    //fp = fopen(filename, "rb");
    fd = open(filename, O_CREAT|O_RDWR, 0666);
	if ( -1 == fd)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "open file failed! filename[%s]", filename);
        WRITELOG(LOG_ERROR, szLogInfo);
        return ADI_ERR;
	}
	fp = fdopen(fd, "r");
    if(NULL == fp)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "open file failed! filename[%s]", filename);
        WRITELOG(LOG_ERROR, szLogInfo);
        return ADI_ERR;
    }

	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "music open  fd[%d] fp[%p]", fd, fp);
    WRITELOG(LOG_ERROR, szLogInfo);
	
    err = device_play_ex(fp);
    if(ADI_OK != err)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "device_play call failed! filename[%s]", filename);
        WRITELOG(LOG_ERROR, szLogInfo);
		fclose(fp);
        return ADI_ERR;
    }
    
    fclose(fp);
    
    return ADI_OK;
}

int device_play_ex(FILE *fp)
{
    int                  err            =   0;
    char                 szLogInfo[500] = { 0 };
    int                  size           = 0;
    int                  cnt            = 0;
    t_audio_params       audio_params   = { 0 };
    char                *buffer         = NULL;
    snd_pcm_sframes_t    rc             = 0;

    if(0 == guc_devopening)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "audio device is busying!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return ADI_ERR;
    }

    if(NULL == fp)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "file handle is null!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return ADI_ERR;
    }
    
    err = device_getparams(&audio_params);
    if(err != ADI_OK)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "get params failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return err;
    }

    if((SND_PCM_FORMAT_S8 == audio_params.format)
       || (SND_PCM_FORMAT_U8 == audio_params.format))
    {
        size = audio_params.period_size * audio_params.channels;
    }
    else if((SND_PCM_FORMAT_S16_LE == audio_params.format)
       || (SND_PCM_FORMAT_S16_BE == audio_params.format)
       || (SND_PCM_FORMAT_U16_LE == audio_params.format)
       || (SND_PCM_FORMAT_U16_BE == audio_params.format))
    {
        size = audio_params.period_size * audio_params.channels * 2;
    }
    else
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "unsuport param format[%d]!", audio_params.format);
        WRITELOG(LOG_ERROR, szLogInfo);
        return ADI_ERR;
    }
    
    buffer = (char *) malloc(size);
    if(NULL == buffer)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "allocate buffer failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return ADI_ERR;
    }

	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "music file: buffer[%p] size[%d] fp[%p] fd[%d]", buffer, size, fp, fileno(fp));
    WRITELOG(LOG_INFO, szLogInfo);

    do
    {
        if(PAUSE_STATE == g_ulPlayCmd)
        {
            err = device_pause();
            if(ADI_ERR == err )
            {
                snprintf(szLogInfo, sizeof(szLogInfo) - 1, "audio_pause call failed: %s", snd_strerror(err));
                WRITELOG(LOG_ERROR, szLogInfo);
				free(buffer);
                return ADI_ERR;
            }
            while(PAUSE_STATE == g_ulPlayCmd)
            {
                sleep(1);
            }
        }

        if(RESUME_STATE == g_ulPlayCmd)
        {
            device_resume();
            g_ulPlayCmd = PLAY_STATE;
        }
        
        if(STOP_STATE == g_ulPlayCmd)
        {
            g_ulPlayCmd = PLAY_STATE;
            break;
        }
        memset(buffer, 0, size);
        rc = fread(buffer, 1, size, fp);
        if ((0 == rc) && (0 == feof(fp)))
        {  
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "error read file. rc[%d] errno[%d] errmsg[%s] buffer[%p]size[%d] fp[%p] fd[%d]", 
																rc, errno, strerror(errno), buffer, size, fp, fileno(fp));
            WRITELOG(LOG_ERROR, szLogInfo);
            free(buffer);
            return ADI_ERR;
        }
        
        rc = snd_pcm_writei(g_pcm_handle, buffer, audio_params.period_size);
        if (-EPIPE == rc)
        {  
            /* EPIPE means underrun */
            snd_pcm_prepare(g_pcm_handle);
        }
        else if (rc < 0)
        {  
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "error writei: %s", snd_strerror(rc));
            WRITELOG(LOG_ERROR, szLogInfo);
            free(buffer);
            return ADI_ERR;
        }
        else if (rc < audio_params.period_size)
        {  
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "short writei, write %d frames", rc);
            WRITELOG(LOG_WARN, szLogInfo);
        }
    } while(0 == feof(fp));

    free(buffer);
    
    return ADI_OK;
}

int device_close()
{
    if(0 == guc_devopening)
    {
        return ADI_ERR;
    }
    snd_pcm_hw_params_free(g_hw_params); //释放参数变量空间
    g_hw_params = NULL;

    snd_pcm_drain(g_pcm_handle);
    snd_pcm_close(g_pcm_handle);
    g_pcm_handle = NULL;

    guc_devopening = 0;
    
    return ADI_OK;
}

int device_pause()
{
    int err = 0;
    char   szLogInfo[500] = { 0 };
    int hw_can_pause = 0;
    if(0 == guc_devopening)
    {
        return ADI_ERR;
    }
    hw_can_pause = snd_pcm_hw_params_can_pause(g_hw_params);
    if(hw_can_pause)
    {
        printf("alsa-pause: pause supported by hardware\n");
        err = snd_pcm_pause(g_pcm_handle, 1);
        if(err < 0)
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "snd_pcm_pause call failed: %s", snd_strerror(err));
            WRITELOG(LOG_ERROR, szLogInfo);
            return ADI_ERR;
        }
    }
    else
    {
        printf("alsa-pause: pause NOT supported by hardware\n");
        err = snd_pcm_drop(g_pcm_handle);
        if(err < 0)
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "snd_pcm_drop call failed: %s", snd_strerror(err));
            WRITELOG(LOG_ERROR, szLogInfo);
            return ADI_ERR;
        }
    }
    
    return ADI_OK;
}

int device_resume()
{
    int err = 0;
    char   szLogInfo[500] = { 0 };
    snd_pcm_state_t pcm_state = 0;
    int hw_can_pause = 0;
    printf("GC: Enter to device_resume()......!!!!,guc_devopening = %d\n ",guc_devopening);
    guc_devopening = 1;
    if(0 == guc_devopening)
    {
        return ADI_ERR;
    }
    printf("GC: Enter to device_resume()......!!!!\n ");
    pcm_state = snd_pcm_state(g_pcm_handle);
    if(SND_PCM_STATE_SUSPENDED == pcm_state)
    {
        err = snd_pcm_resume(g_pcm_handle);
        while(-EAGAIN == err)
        {
            sleep(1);
            err = snd_pcm_resume(g_pcm_handle);
        }
    }

    hw_can_pause = snd_pcm_hw_params_can_pause(g_hw_params);
    if(hw_can_pause)
    {
        err = snd_pcm_pause(g_pcm_handle, 0);
        if(err < 0)
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "snd_pcm_pause call failed: %s", snd_strerror(err));
            WRITELOG(LOG_ERROR, szLogInfo);
            return ADI_ERR;
        }
    }
    else
    {
        err = snd_pcm_prepare(g_pcm_handle);
        if(err < 0)
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "snd_pcm_prepare call failed: %s", snd_strerror(err));
            WRITELOG(LOG_ERROR, szLogInfo);
            return ADI_ERR;
        }
    }
    
    return ADI_OK;
}


int audio_addwavheader(char *src_file, int channels, int rate, int bits, char *dst_file)
{
    char      szLogInfo[500] = { 0 };
    HEADER    pcmHEADER      = { 0 };
    FMT       pcmFMT         = { 0 };
    DATA      pcmDATA        = { 0 };
    FILE     *fp             = NULL;
    FILE     *fpCpy          = NULL;
    unsigned short    m_pcmData = 0;
    
    fp = fopen(src_file, "rb");
    if(NULL == fp) //读取文件
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "open pcm file %s error", src_file);
        WRITELOG(LOG_ERROR, szLogInfo);
        return ADI_ERR;
    }

    fpCpy=fopen(dst_file, "wb+"); //为转换建立一个新文件
    if(NULL == fpCpy)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "create wav file %s error, errno=%d, strerror=%s", dst_file, errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
        fclose(fp);
        return ADI_ERR;
    }
    
    //以下是创建wav头的HEADER;但.dwsize未定，因为不知道Data的长度
    strncpy(pcmHEADER.fccID, "RIFF", sizeof(pcmHEADER.fccID));
    strncpy(pcmHEADER.fccType, "WAVE", sizeof(pcmHEADER.fccType));
    fseek(fpCpy, sizeof(HEADER), 1); //跳过HEADER的长度，以便下面继续写入wav文件的数据    
    if(ferror(fpCpy))    
    {      
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "fseek error");
        WRITELOG(LOG_ERROR, szLogInfo);
        fclose(fp);
        fclose(fpCpy);
        return ADI_ERR;
    }
    
    //以下是创建wav头的FMT
    strncpy(pcmFMT.fccID,"fmt ", sizeof(pcmFMT.fccID));
    pcmFMT.dwSize = 16;
    pcmFMT.wFormatTag = 1;
    pcmFMT.wChannels = channels;
    pcmFMT.dwSamplesPerSec = rate;
    pcmFMT.dwAvgBytesPerSec = rate * channels * (bits / 8);
    pcmFMT.wBlockAlign = channels * (bits / 8);
    pcmFMT.wBitsPerSample = bits;
    
    fwrite(&pcmFMT, sizeof(FMT), 1, fpCpy); //将FMT写入.wav文件
    if(ferror(fpCpy))    
    {      
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "fwrite error");
        WRITELOG(LOG_ERROR, szLogInfo);
        fclose(fp);
        fclose(fpCpy);
        return ADI_ERR;
    }
    
    //以下是创建wav头的DATA;   但由于DATA.dwsize未知所以不能写入.wav文件
    strncpy(pcmDATA.fccID, "data", sizeof(pcmDATA.fccID));  
    pcmDATA.dwSize = 0; //给pcmDATA.dwsize   0以便于下面给它赋值
    
    fseek(fpCpy, sizeof(DATA), 1); //跳过DATA的长度，以便以后再写入wav头的DATA
    if(ferror(fpCpy))    
    {      
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "fseek error");
        WRITELOG(LOG_ERROR, szLogInfo);
        fclose(fp);
        fclose(fpCpy);
        return ADI_ERR;
    }
    fread(&m_pcmData, sizeof(unsigned   short), 1, fp); //从.pcm中读入数据
    if(ferror(fp))    
    {      
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "fread error");
        WRITELOG(LOG_ERROR, szLogInfo);
        fclose(fp);
        fclose(fpCpy);
        return ADI_ERR;
    }
    
    while(!feof(fp)) //在.pcm文件结束前将他的数据转化并赋给.wav
    {
        pcmDATA.dwSize += 2; //计算数据的长度；每读入一个数据，长度就加2
   
        fwrite(&m_pcmData,sizeof(unsigned   short),1, fpCpy); //将数据写入.wav文件
        if(ferror(fpCpy))    
        {      
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "fwrite error");
            WRITELOG(LOG_ERROR, szLogInfo);
            fclose(fp);
            fclose(fpCpy);
            return ADI_ERR;
        }
        fread(&m_pcmData, sizeof(unsigned   short), 1, fp); //从.pcm中读入数据
        if(ferror(fp))    
        {      
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "fread error");
            WRITELOG(LOG_ERROR, szLogInfo);
            fclose(fp);
            fclose(fpCpy);
            return ADI_ERR;
        }
     }
    
    fclose(fp); //关闭文件    
    pcmHEADER.dwSize = 4 + sizeof(FMT) + sizeof(DATA) + pcmDATA.dwSize;   //根据pcmDATA.dwsize得出pcmHEADER.dwsize的值
  
     rewind(fpCpy); //将fpCpy变为.wav的头，以便于写入HEADER和DATA
     fwrite(&pcmHEADER,sizeof(HEADER),1,fpCpy); //写入HEADER
     if(ferror(fpCpy))    
     {      
         snprintf(szLogInfo, sizeof(szLogInfo) - 1, "fwrite error");
         WRITELOG(LOG_ERROR, szLogInfo);
         fclose(fpCpy);
         return ADI_ERR;
     }
     fseek(fpCpy,sizeof(FMT),1); //跳过FMT,因为FMT已经写入
     if(ferror(fpCpy))    
     {      
         snprintf(szLogInfo, sizeof(szLogInfo) - 1, "fseek error");
         WRITELOG(LOG_ERROR, szLogInfo);
         fclose(fpCpy);
         return ADI_ERR;
     }
     fwrite(&pcmDATA,sizeof(DATA),1,fpCpy);   //写入DATA
     if(ferror(fpCpy))    
     {      
         snprintf(szLogInfo, sizeof(szLogInfo) - 1, "fwrite error");
         WRITELOG(LOG_ERROR, szLogInfo);
         fclose(fpCpy);
         return ADI_ERR;
     }
     
     fclose(fpCpy);   //关闭文件

     return ADI_OK;
}

int audio_record(t_audio_params *audio_params, int duration, char *pcm_file, char *wav_file)
{
    int    err            =   0;
    int    bits           =   0;
    char   szLogInfo[500] = { 0 };

	// added by zzx 20170320 begin
    if (audio_params == NULL || pcm_file == NULL || wav_file == NULL)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "audio_record: audio_params == NULL || pcm_file == NULL || wav_file == NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
		return 0;
    }
	// added by zzx 20170320 end

    err = device_open(SND_PCM_STREAM_CAPTURE);
    if(ADI_ERR == err)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "audio_record: open device failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return err;
    }

    err = device_setparams(audio_params);
    if(ADI_ERR == err)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "audio_record: set device params failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        device_close();
        return err;
    }

    
    err = device_capture(duration, pcm_file);
    if(ADI_ERR == err)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "audio_record: capture deivce failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        device_close();
        return err;
    }

    device_close();

    ////////////加wav文件头/////////////
    if((SND_PCM_FORMAT_S8 == audio_params->format)
       || (SND_PCM_FORMAT_U8 == audio_params->format))
    {
        bits = 8;
    }
    if((SND_PCM_FORMAT_S16_LE == audio_params->format)
       || (SND_PCM_FORMAT_S16_BE == audio_params->format)
       || (SND_PCM_FORMAT_U16_LE == audio_params->format)
       || (SND_PCM_FORMAT_U16_BE == audio_params->format))
    {
        bits = 16;
    }
    else
    {
        bits = 8;
    }
    err = audio_addwavheader(pcm_file, audio_params->channels, audio_params->rate, bits, wav_file);
    if(ADI_ERR == err)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "audio_record: add wav header failed! pcm_file[%s], wav_file[%s]", pcm_file, wav_file);
        WRITELOG(LOG_ERROR, szLogInfo);
        return err;
    }

    return ADI_OK;
}

int audio_play(t_audio_params *audio_params, char *pcm_file)
{
    int    err            =   0;
    char   szLogInfo[500] = { 0 };
    
    err = device_open(SND_PCM_STREAM_PLAYBACK);
    if(ADI_ERR == err)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "open device failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return err;
    }

    err = device_setparams(audio_params);
    if(ADI_ERR == err)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "set params failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        device_close();
        return err;
    }

    err = device_play(pcm_file);
    if(ADI_ERR == err)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "play file failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        device_close();
        return err;
    }

    device_close();
    
}

int audio_play_ex(t_audio_params *audio_params, FILE *pcm_fp)
{
    int    err            =   0;
    char   szLogInfo[500] = { 0 };
    
    err = device_open(SND_PCM_STREAM_PLAYBACK);
    if(ADI_ERR == err)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "open device failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return err;
    }

    err = device_setparams(audio_params);
    if(ADI_ERR == err)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "set params failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        device_close();
        return err;
    }

    err = device_play_ex(pcm_fp);
    if(ADI_ERR == err)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "play file failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        device_close();
        return err;
    }

    device_close();
    
}

int audio_getwavheader(char *filename, t_audio_params *audio_params)
{
    FILE  *fp             = NULL;
    char   szLogInfo[500] = { 0 };
    HEADER header         = { 0 };
    FMT    fmt            = { 0 };
    DATA   data           = { 0 };
    int    header_len     =   0;
    
    fp = fopen(filename, "rb"); //打开wav文件
    if(NULL == fp)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "open wav file %s error,errno=%d,strerror=%s", filename, errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }

    fread(&header, sizeof(HEADER), 1, fp); //从wav文件中读出HEADER头
    if(ferror(fp))    
    {      
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "read wav file %s error,errno=%d,strerror=%s", filename, errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
        fclose(fp);
        return -1;
    }
    if((0 != strncmp(header.fccID, "RIFF", sizeof(header.fccID)))
       || (0 != strncmp(header.fccType, "WAVE", sizeof(header.fccType))))
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "wav file %s format error", filename);
        WRITELOG(LOG_ERROR, szLogInfo);
        fclose(fp);
        return -1;
    }

    fread(&fmt, sizeof(FMT), 1, fp); //从wav文件中读出FMT头
    if(ferror(fp))    
    {      
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "read wav file %s error,errno=%d,strerror=%s", filename, errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
        fclose(fp);
        return -1;
    }
    if(18 == fmt.dwSize)
    {
        fseek(fp, 2, SEEK_CUR); //跳过FMT,因为FMT已经写入
        if(ferror(fp))    
        {      
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "seek wav file %s error,errno=%d,strerror=%s", filename, errno, strerror(errno));
            WRITELOG(LOG_ERROR, szLogInfo);
            fclose(fp);
            return -1;
        }
    }

    fread(&data, sizeof(DATA), 1, fp); //从wav文件中读出DATA头
    if(ferror(fp))
    {      
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "read wav file %s error,errno=%d,strerror=%s", filename, errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
        fclose(fp);
        return -1;
    }

    header_len = ftell(fp);
    if(ferror(fp))
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "tell wav file %s error,errno=%d,strerror=%s", filename, errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
        fclose(fp);
        return -1;
    }
    
    fclose(fp);
    
    //memset(audio_params, 0, sizeof(t_audio_params));
    audio_params->access = SND_PCM_ACCESS_RW_INTERLEAVED;
    audio_params->channels = fmt.wChannels;    
    if(32 == fmt.wBitsPerSample)
    {
        audio_params->format = SND_PCM_FORMAT_S32_LE;
    }
    else if(24 == fmt.wBitsPerSample)
    {
        audio_params->format = SND_PCM_FORMAT_S24_LE;
    }
    else if(16 == fmt.wBitsPerSample)
    {
        audio_params->format = SND_PCM_FORMAT_S16_LE;
    }
    else
    {
        audio_params->format = SND_PCM_FORMAT_S8;
    }    
    audio_params->period_size = 32;
    audio_params->rate = fmt.dwSamplesPerSec;
    
    return header_len;
}

int audio_getwavdata(char *src_file, char *dst_file)
{
    FILE  *s_fp             =   NULL;
    FILE  *d_fp             =   NULL;
    char   szLogInfo[500] = { 0 };
    unsigned char aucWavData[1024] = { 0 };
    int           len       = 0;
    
    s_fp = fopen(src_file, "rb"); //打开wav文件
    if(NULL == s_fp)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "open wav file %s error,errno=%d,strerror=%s", src_file, errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
        return ADI_ERR;
    }

    d_fp = fopen(dst_file, "wb+"); //为转换建立一个新文件
    if(NULL == d_fp)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "create pcm file %s error, errno=%d, strerror=%s", dst_file, errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
        fclose(s_fp);
        return ADI_ERR;
    }
    
    while(!feof(s_fp)) //在wav文件结束前将他的数据转化并赋给pcm文件
    {
        len = fread(aucWavData, sizeof(unsigned char), sizeof(aucWavData), s_fp); //从wav中读入数据
        if(ferror(s_fp))    
        {      
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "read wav file %s error, errno=%d, strerror=%s", src_file, errno, strerror(errno));
            WRITELOG(LOG_ERROR, szLogInfo);
            fclose(s_fp);
            fclose(d_fp);
            return ADI_ERR;
        }
   
        fwrite(aucWavData, sizeof(unsigned char), len, d_fp); //将数据写入pcm文件
        if(ferror(d_fp))    
        {      
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "write pcm file %s error, errno=%d, strerror=%s", dst_file, errno, strerror(errno));
            WRITELOG(LOG_ERROR, szLogInfo);
            fclose(s_fp);
            fclose(d_fp);
            return ADI_ERR;
        }
     }

     fclose(s_fp);
     fclose(d_fp);
     
     return ADI_OK;
}
int audio_play_wav(char *wav_file)
{
    int    err            =   0;
    char   szLogInfo[500] = { 0 };
    
    t_audio_params audio_params = { 0 };
    FILE  *fp  = NULL;
    int    header_len     = 0;
	int fd = 0;
	
    header_len = audio_getwavheader(wav_file, &audio_params);
    if(-1 == header_len)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "audio_getwavheader failed, wav file=%s", wav_file);
        WRITELOG(LOG_ERROR, szLogInfo);
        return ADI_ERR;
    }

	fd = open(wav_file, O_CREAT|O_RDWR, 0666);
	if ( -1 == fd)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "open file failed! filename[%s]", wav_file);
        WRITELOG(LOG_ERROR, szLogInfo);
        return ADI_ERR;
	}
	fp = fdopen(fd, "r");
    if(NULL == fp)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "open file failed! filename[%s]", wav_file);
        WRITELOG(LOG_ERROR, szLogInfo);
        return ADI_ERR;
    }

	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "music open  fd[%d] fp[%p]", fd, fp);
    WRITELOG(LOG_ERROR, szLogInfo);

/*
    fp = fopen(wav_file, "rb"); //打开wav文件
    if(NULL == fp)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "open wav file %s error,errno=%d,strerror=%s", wav_file, errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
        return ADI_ERR;
    }
*/
    fseek(fp, header_len, SEEK_SET);
    if(ferror(fp))    
    {      
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "seek wav file %s error,errno=%d,strerror=%s", wav_file, errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
        fclose(fp);
        return ADI_ERR;
    }
    
    err = audio_play_ex(&audio_params, fp);
    if(ADI_ERR == err)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "audio_play failed, wav file=%s", wav_file);
        WRITELOG(LOG_ERROR, szLogInfo);
        fclose(fp);
        return err;
    }

    fclose(fp);
    return ADI_OK;
}

int audio_pause_wav()
{
    g_ulPlayCmd = PAUSE_STATE;
    return ADI_OK;
}

int audio_resume_wav()
{
  
    g_ulPlayCmd = RESUME_STATE;
    return ADI_OK;
}

int audio_stop_wav()
{
    g_ulPlayCmd = STOP_STATE;
    return ADI_OK;
}

void SetAlsaMasterVolume(long volume)
{
    long min, max;
    long ll, lr;
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    const char *card = "default";
    const char *selem_name = "PCM";
    snd_mixer_elem_t *elem;

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);
/*
    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    snd_mixer_selem_set_playback_volume_all(elem, volume * max / 100);
*/
    for(elem=snd_mixer_first_elem(handle); elem; elem=snd_mixer_elem_next(elem))
    {
        if (snd_mixer_elem_get_type(elem) == SND_MIXER_ELEM_SIMPLE && snd_mixer_selem_is_active(elem))
        {
                snd_mixer_selem_set_playback_volume_range(elem, 0, 100);
                printf("elem name[%s] min[%d] max[%d]\n", snd_mixer_selem_get_name(elem), min, max);
                //snd_mixer_selem_set_playback_volume_all(elem, volume);
                snd_mixer_selem_set_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, volume);
                snd_mixer_selem_set_playback_volume(elem, SND_MIXER_SCHN_FRONT_RIGHT, volume);
        }

    }
    snd_mixer_close(handle);
}

long GetAlsaMasterVolume()
{
    long min, max;
    long ll, lr;
	long midVolume;
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    const char *card = "default";
    const char *selem_name = "PCM";
    snd_mixer_elem_t *elem;

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);
	printf("***********func: GetAlsaMasterVolume*********\n");
/*
    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    snd_mixer_selem_set_playback_volume_all(elem, volume * max / 100);
*/
    for(elem=snd_mixer_first_elem(handle); elem; elem=snd_mixer_elem_next(elem))
    {
        if (snd_mixer_elem_get_type(elem) == SND_MIXER_ELEM_SIMPLE && snd_mixer_selem_is_active(elem))
        {
        		snd_mixer_selem_set_playback_volume_range(elem, 0, 100);
                snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, &ll);
                snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_RIGHT, &lr);
				printf(">>>>>>>>>>>vol: ll[%ld] lr[%ld]<<<<<<<<<\n", ll, lr);
				midVolume = (ll + lr) >> 1;
        }

    }
    snd_mixer_close(handle);
	return midVolume;
}


// vol: 范围为0-100，代表的是百分比放大100倍后的数
int audio_set_volume(int vol)
{
    char   szLogInfo[500] = { 0 };
#if 0
    char cmd[64] = { 0 };
    vol = (int)(vol/2+50);
    snprintf(cmd, sizeof(cmd) - 1, "amixer sset PCM %d%%", vol, vol);
    system(cmd);
    return ADI_OK;
#else
	vol = (int)(vol/2 + 50);
	SetAlsaMasterVolume(vol);
	return 0;
#endif
}

int audio_get_volume()
{
	int vol = 0;
	vol = GetAlsaMasterVolume();
	if (vol <= 50)
		vol = 0;
	else
		vol = 2 * (vol - 50);
	return vol;
}


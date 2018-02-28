#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <libavformat/avformat.h>
#include "log.h"
#include "frame.h"
#include "global.h"
#include "link_chk.h"
#include "serv.h"
#include "audioinf.h"
#include "music_proc.h"


int GetAudioList(char *basePath, T_FileList *ptFileList)
{
    DIR *dir = NULL;
    struct dirent *ptr = NULL;
    char szLogInfo[512] = { 0 };
    
    dir = opendir(basePath);
    if (NULL == dir)
    {
        sprintf(szLogInfo, "opendir [%s] failed! errno=%d, strerror=%s\n", basePath, errno, strerror(errno));
        return -1;
    }
    memset(ptFileList, 0, sizeof(T_FileList));
    ptr = readdir(dir);
    while (ptr != NULL)
    {
        if((8 == ptr->d_type) && (ptFileList->usNum < MAX_FILE_NUM))
        {
            snprintf(ptFileList->filelist[ptFileList->usNum++], MAX_FILENAME_LEN, "%s", ptr->d_name); // d_name为文件名
        }
        ptr = readdir(dir);
    }
    closedir(dir);
    
    return 0;
}

#define AV_TIME_BASE	1000000

int getAudioDuration(const char *filename, char *cDuration, int len)
{
	double duration = 0.0;
	int hours, mins, secs, us;
	long long dur = 0;
	
    av_register_all();

    AVFormatContext *pFormatCtx = NULL;

    // Open video file
    if(avformat_open_input(&pFormatCtx, filename, NULL, NULL) != 0)
    {
        return -1; // Couldn't open file
    }

    avformat_find_stream_info(pFormatCtx, NULL);

    duration = (double)pFormatCtx->duration;

    //printf("file duration[%lf]\n", duration);
    avformat_close_input(&pFormatCtx);
    avformat_free_context(pFormatCtx);

	dur = duration + 5000;
	secs = dur / AV_TIME_BASE;
	us = dur % AV_TIME_BASE;
	mins = secs / 60;
	secs %= 60;
	hours = mins / 60;
	mins %= 60;
	//printf("file_name[%s] %02d:%02d:%02d.%02d\n", filename,  hours, mins, secs, (100*us)/ AV_TIME_BASE);
	snprintf(cDuration, len, "%02d:%02d:%02d", hours, mins, secs);
    return 0;
}



int GetAudioListAndLength(char *basePath, T_FileList *ptFileList)
{
	DIR *dir = NULL;
    struct dirent *ptr = NULL;
    char szLogInfo[512] = { 0 };
    char tmpDir[MAX_FILENAME_LEN] = {0};
	char tmpDur[MAX_FILENAME_LEN] = {0};
    dir = opendir(basePath);
    if (NULL == dir)
    {
        sprintf(szLogInfo, "opendir [%s] failed! errno=%d, strerror=%s\n", basePath, errno, strerror(errno));
        return -1;
    }
    memset(ptFileList, 0, sizeof(T_FileList));
    ptr = readdir(dir);
    while (ptr != NULL)
    {
        if((8 == ptr->d_type) && (ptFileList->usNum < MAX_FILE_NUM))
        {
            //snprintf(ptFileList->filelist[ptFileList->usNum++], MAX_FILENAME_LEN, "%s", ptr->d_name); // d_name为文件名
            memset(tmpDir, 0, sizeof(tmpDir));
			sprintf(tmpDir, "%s%s", basePath, ptr->d_name);
			getAudioDuration(tmpDir, tmpDur, sizeof(tmpDur));
			snprintf(ptFileList->filelist[ptFileList->usNum++], MAX_FILENAME_LEN, "%s %s", ptr->d_name, tmpDur);
        }
        ptr = readdir(dir);
    }
    closedir(dir);
    
	return 0;
}


#define MAX_FILELIST_LEN    4*1024

#pragma pack(1)

typedef struct
{
    char err;
    //unsigned short len;
    char len[4];
    char filelist[MAX_FILELIST_LEN];
}t_play_list;
#pragma pack()


int ent_playlist_req(t_3761frame *frame, t_buf *buf)//req: from the server; buf that is user_data of the frame is music file list.
{                                                       // in here frame is not used.
    int index = 0;
    char filename[MAX_FILENAME_LEN + 1] = {0};
    //char szLogInfo[512] = { 0 };
    int  iRet = 0;
    T_FileList filelist = { 0 };
    int len = 0;
    t_play_list m_list= {0};
		
    printf("Enter to ent_playlist_req...\n");
    // 调用具体的业务数据处理函数
    //printf("buf before= 0x%x\n",buf);
    memset(buf, 0, sizeof(t_buf));
    //printf("buf later= 0x%x\n",buf);
	char tmp[MAX_FILELIST_LEN] = {0};
	
    //GetAudioList(g_strAudioPath, &filelist);
    GetAudioListAndLength(g_strAudioPath, &filelist);
    for(index = 0; index < filelist.usNum; index++)
    {
        printf("index=%d, filelist.usNum=%d\n", index, filelist.usNum);
        if(0 == index)
        {
            if((len + strlen(filelist.filelist[index])) > MAX_BUF_LEN)
            {
                break;
            }
            strncpy(tmp, filelist.filelist[index], strlen(filelist.filelist[index]));
            len += strlen(filelist.filelist[index]);
        }
        else
        {
            if((len + 1 + strlen(filelist.filelist[index])) > MAX_BUF_LEN)
            {
                break;
            }
            strncat(tmp, ",", 1);
            len += 1;
            
            strncat(tmp, filelist.filelist[index], strlen(filelist.filelist[index]));
            len += strlen(filelist.filelist[index]);
        }
    }

	m_list.err = 0;
	sprintf(m_list.len, "%04d", len);
	
	memcpy(m_list.filelist, tmp, len);
	printf(" file_list [%s] len[%d]\n", m_list.filelist, len);
	
    buf->len = sizeof(char) + sizeof(m_list.len) + len;
	//strncpy(buf->buf, (char *)&m_list, buf->len);
	memcpy(buf->buf, &m_list, buf->len);
	//dumpInfo(tmp, len);
	//dumpInfo(&m_list, sizeof(m_list));
	//dumpInfo(buf->buf, buf->len);	
    printf("GC:buf=0x%x, len = %d; \n", buf, len);
    
    return 0;
}

int ent_playlist_ack(t_buf *buf, t_3761frame *frame)//frame is the ack frame. 
{
    printf("Enter to ent_playlist_ack...\n");
    printf("GC: ent_playlist_ack1 ptBuf = 0x%x\n",buf);
    memcpy(frame->addr, collector_id, MAX_ADDR_LEN);
    frame->user_data.afn = AFN_TYPE1_DATA_REQ;
    frame->user_data.fn[0] = 1 << ((FN_PLAY_LIST - 1) % 8);
    frame->user_data.fn[1] = (FN_PLAY_LIST - 1) / 8;
    frame->user_data.len = buf->len + 12;
    memcpy(frame->user_data.data + 12, buf->buf, min(buf->len,MAX_DATAUNIT_LEN - 12));
    printf("GC: ent_playlist_ack2 ptBuf = 0x%x\n",buf);
    printf("GC: ent_playlist_ack2 frame = 0x%x\n",frame);
    return 0;
}

int ent_playlist_ack_db()
{
    int iRet = 0;
	int len = 0;
	int index = 0;
    t_3761frame tframe= {0};
    char szLogInfo[512] = { 0 };
    t_mymsg mymsg = { 0 };
    t_buf  *ptBuf = NULL;
    T_FileList filelist = { 0 };
	t_play_list *m_list = NULL;
	char tmp[MAX_FILELIST_LEN] = {0};
	GetAudioListAndLength(g_strAudioPath, &filelist);
	for(index = 0; index < filelist.usNum; index++)
    {
        if(0 == index)
        {
            if((len + strlen(filelist.filelist[index])) > MAX_BUF_LEN)
            {
                break;
            }
            strncpy(tmp, filelist.filelist[index], strlen(filelist.filelist[index]));
            len += strlen(filelist.filelist[index]);
        }
        else
        {
            if((len + 1 + strlen(filelist.filelist[index])) > MAX_BUF_LEN)
            {
                break;
            }
            strncat(tmp, ",", 1);
            len += 1;
            
            strncat(tmp, filelist.filelist[index], strlen(filelist.filelist[index]));
            len += strlen(filelist.filelist[index]);
        }
    }
	m_list = (t_play_list *)tframe.user_data.data;
	m_list->err = 0;
	sprintf(m_list->len, "%04d", len);
	memcpy(m_list->filelist, tmp, len);
	
    mymsg.mtype = 1;
	ptBuf = (t_buf *)(mymsg.mtext);
    memcpy(tframe.addr, collector_id, MAX_ADDR_LEN);
	
	tframe.user_data.afn = AFN_TYPE1_DATA_REQ;
    tframe.user_data.fn[0] = 1 << ((FN_PLAY_LIST - 1) % 8);
    tframe.user_data.fn[1] = (FN_PLAY_LIST - 1) / 8;
	
    tframe.user_data.len = sizeof(char) + sizeof(m_list->len) + len;
    iRet = pack_3761frame(&tframe, ptBuf->buf, MAX_BUF_LEN);
    if(iRet <= 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "pack_3761frame failed! iRet=%d", iRet);
        WRITELOG(LOG_ERROR, szLogInfo);
        return -2;
    }
	
	ptBuf->len= iRet;

	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "The below information is in ent_playstart_ack function[177 line]");
    WRITELOG(LOG_INFO, szLogInfo);
	//dumpInfo(mymsg.mtext, ptBuf->len + sizeof(int));

	pthread_mutex_lock(&g_pipe_mutex);
	iRet = write_safe(g_pipe[1],&mymsg,sizeof(t_mymsg));
	pthread_mutex_unlock(&g_pipe_mutex);
	if (iRet == -1)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "write pipe failed! errno=%d, errmsg[%s]", errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
		return -3;
	}
	
    return 0;
}

int ent_playstart_req(t_3761frame *frame, t_buf *buf)
{
    char filename[MAX_FILENAME_LEN + 1] = {0};
    char fullname[MAX_FULLNAME_LEN + 1] = {0};
    char szLogInfo[512] = { 0 };
    int  iRet = 0;
	char cSerial[13] = {0};
    printf("Enter to ent_playstart_req...\n");
    
    // 发确认消息给服务器
    iRet = ent_playstart_ack(NULL, frame);
    if(iRet < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "ent_playstart_ack failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
    
    // 调用具体的业务数据处理函数
    memcpy(cSerial, frame->user_data.data, 12);
	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "********ent_playstart_req: msg_serial[%s]******", cSerial);
    WRITELOG(LOG_INFO, szLogInfo);
	
    memcpy(filename, frame->user_data.data+12, min(MAX_FILENAME_LEN, frame->user_data.len-12));


    // 判断是否为当前正在播放的歌曲
    snprintf(fullname, sizeof(fullname) - 1, "%s%s", g_strAudioPath, filename);

	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "********ent_playstart_req: filename[%s] thread_id[%lu]******", filename, pthread_self());
    WRITELOG(LOG_ERROR, szLogInfo);
#if 0
	if (guc_devopening==1)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "zhu:audio_stop_wav enter! thread_id[%lu]", pthread_self());
        WRITELOG(LOG_ERROR, szLogInfo);
     	audio_stop_wav();
    
		//sleep(500);
	}
	while(guc_devopening==1);

	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "********break while, ready audio_play_wav: thread_id[%lu]******", pthread_self());
    WRITELOG(LOG_ERROR, szLogInfo);
	
    iRet = audio_play_wav(fullname);
	
#else
	//add by tianyu, 同一时间只能允许一个线程去播放
	// TODO: 缩小锁的范围 或者丢弃中间态的消息

	pthread_mutex_lock(&g_music_mutex);
	if (guc_ref >= 1 && guc_devopening == 0 )  //前一线程正在做初始化动作，还未播放音乐
	{
		while(guc_devopening == 0); //等待播放
	}
    
	if (guc_ref >= 1 && guc_devopening==1)
	{
		
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "zhu:audio_stop_wav enter! thread_id[%lu]", pthread_self());
        WRITELOG(LOG_ERROR, szLogInfo);
     	audio_stop_wav();
    	while(guc_devopening==1); //等待播放结束
		//sleep(500);
	}
	guc_ref++;
	pthread_mutex_unlock(&g_music_mutex);

	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "********break while, ready audio_play_wav: thread_id[%lu]******", pthread_self());
    WRITELOG(LOG_ERROR, szLogInfo);
    iRet = audio_play_wav(fullname);
	guc_ref--;
#endif

	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "********finish audio_play_wav: iRet[%d] thread_id[%lu]******", iRet, pthread_self());
    WRITELOG(LOG_ERROR, szLogInfo);

    /*
    if(g_strCurrentSongName[0] == '\0')
    {
        strcpy(g_strCurrentSongName,filename);
        snprintf(fullname, sizeof(fullname) - 1, "%s%s", g_strAudioPath, filename);
        iRet = audio_play_wav(fullname);
    }
    else
    {
        if(0!= strcmp(g_strCurrentSongName,filename))
        {
           device_close();
            snprintf(fullname, sizeof(fullname) - 1, "%s%s", g_strAudioPath, filename);
            iRet = audio_play_wav(fullname);
        }
        else
            return 0;
        
    }
    */
    
    if(ADI_OK != iRet)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "audio_play_wav failed! filename=%s", fullname);
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
    return 0;
}

int ent_playstart_ack(t_buf *buf, t_3761frame *frame)
{
    int iRet = 0;
    t_3761frame tframe= {0};
    char szLogInfo[512] = { 0 };
    t_mymsg mymsg = { 0 };
    t_buf  *ptBuf = NULL;
    t_di_confirm_deny_list di_confirm_deny_list = { 0 };
    const char playstart_di[4] = {0x00, 0x00, 0x13, 0x80};
	char cSerial[13] = {0};
    printf("Enter to ent_playstart_ack...\n");
    
    mymsg.mtype = 1;
	ptBuf = (t_buf *)(mymsg.mtext);
	
    memcpy(tframe.addr, collector_id, MAX_ADDR_LEN);
	
#if 0
    tframe.user_data.afn = AFN_CONFIRM_DENY;
    tframe.user_data.fn[0] = 1 << ((FN_DI - 1) % 8);
    tframe.user_data.fn[1] = (FN_DI - 1) / 8;
#else
	//modify by tianyu
	tframe.user_data.afn = AFN_CTRL_CMD;
    tframe.user_data.fn[0] = 1 << ((FN_PLAYSTART_MUSIC - 1) % 8);
    tframe.user_data.fn[1] = (FN_PLAYSTART_MUSIC - 1) / 8;
#endif
    memset(&di_confirm_deny_list, 0x00, sizeof(t_di_confirm_deny_list));
    di_confirm_deny_list.num = 1;
    memcpy(di_confirm_deny_list.confirm_deny[0].di, playstart_di, 4);
	memcpy(tframe.user_data.data, frame->user_data.data, 12);
	memcpy(cSerial, frame->user_data.data, 12);
	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "********ent_playstart_ack: msg_serial[%s]******", cSerial);
    WRITELOG(LOG_INFO, szLogInfo);
    iRet = pack_diconfirmdeny(&di_confirm_deny_list, tframe.user_data.data+12, MAX_DATAUNIT_LEN);
    if(iRet < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "pack_diconfirmdeny failed! iRet=%d", iRet);
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
    tframe.user_data.len = iRet + 12;

    iRet = pack_3761frame(&tframe, ptBuf->buf, MAX_BUF_LEN);
    if(iRet <= 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "pack_3761frame failed! iRet=%d", iRet);
        WRITELOG(LOG_ERROR, szLogInfo);
        return -2;
    }
	
	ptBuf->len= iRet;

	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "The below information is in ent_playstart_ack function[177 line]");
    WRITELOG(LOG_INFO, szLogInfo);
	//dumpInfo(mymsg.mtext, ptBuf->len + sizeof(int));
#if 0
	iRet = msgsnd(g_send_msqid, &mymsg, ptBuf->len + sizeof(int), IPC_NOWAIT);
    if(iRet < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "msgsnd failed! msqid=%d, len=%d,errno=%d, errmsg[%s]", g_send_msqid, ptBuf->len + sizeof(int), errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
        return -3;
    }
#else
	pthread_mutex_lock(&g_pipe_mutex);
	iRet = write_safe(g_pipe[1],&mymsg,sizeof(t_mymsg));
	pthread_mutex_unlock(&g_pipe_mutex);
	if (iRet == -1)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "write pipe failed! errno=%d, errmsg[%s]", errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
		return -3;
	}
#endif
	
    return 0;
}

int ent_playctrl_req(t_3761frame *frame, t_buf *buf)
{
    int  command = 0;
    char szLogInfo[512] = { 0 };
    int  err = 0;
    int  iRet = 0;
    t_di_confirm_deny_list di_confirm_deny_list = { 0 };
    const char playctrl_di[4] = {0x00, 0x00, 0x14, 0x01};

    //printf("Enter to ent_playctrl_req..., frame->user_data.len = %d\n",frame->user_data.len);
    //printf("Enter to ent_playctrl_req..., frame->user_data.data = %s\n",frame->user_data.data);

    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "The below information is in ent_playctrl_req function[204 line]");
    WRITELOG(LOG_INFO, szLogInfo);
    //dumpInfo(frame->user_data.data, frame->user_data.len);
    // 调用具体的业务数据处理函数
    if((sizeof(int)+ 12) != frame->user_data.len)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "frame user data len error, len=%d", frame->user_data.len);
        WRITELOG(LOG_ERROR, szLogInfo);
        err = 1;
    }
    else
    {
        memcpy(&command, frame->user_data.data + 12, frame->user_data.len-12);
        //command = (int)frame->user_data.data[3];
        printf("command = %d \n",command);
        //memmove(&command, frame->user_data.data, frame->user_data.len);
        // 暂停播放
        if(1 == command)
        {
            audio_pause_wav();
        }
        // 恢复播放
        else if(2 == command)
        {
            audio_resume_wav();
        }
        // 停止播放
        else if(3 == command)
        {
            audio_stop_wav();
        }
        else
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "invalid play control command: %d", command);
            WRITELOG(LOG_ERROR, szLogInfo);
            err = 2;
        }
    }

    memset(&di_confirm_deny_list, 0x00, sizeof(t_di_confirm_deny_list));
    di_confirm_deny_list.num = 1;
    memcpy(di_confirm_deny_list.confirm_deny[0].di, playctrl_di, 4);
    di_confirm_deny_list.confirm_deny[0].err = err;
	memset(frame->user_data.data+12, 0, sizeof(frame->user_data.data)-12);
    iRet = pack_diconfirmdeny(&di_confirm_deny_list, frame->user_data.data+12, MAX_DATAUNIT_LEN-12);
    if(iRet < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "pack_diconfirmdeny failed! iRet=%d", iRet);
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
    frame->user_data.len = iRet+12;
    
    return 0;
}

int ent_playctrl_ack(t_buf *buf, t_3761frame *frame)
{
    printf("Enter to ent_playctrl_ack...\n");
    
    memcpy(frame->addr, collector_id, MAX_ADDR_LEN);
#if 0	
    frame->user_data.afn = AFN_CONFIRM_DENY;
    frame->user_data.fn[0] = 1 << ((FN_DI - 1) % 8);
    frame->user_data.fn[1] = (FN_DI - 1) / 8;
    frame->user_data.len = buf->len;
#else
	//modify by tianyu
	frame->user_data.afn = AFN_CTRL_CMD;
    frame->user_data.fn[0] = 1 << ((FN_PLAYCTRL_MUSIC - 1) % 8);
    frame->user_data.fn[1] = (FN_PLAYCTRL_MUSIC - 1) / 8;
#endif
	//frame->user_data.len = buf->len;
    //memcpy(frame->user_data.data, buf->buf, buf->len);
    
    return 0;
}

int ent_volumeset_req(t_3761frame *frame, t_buf *buf)
{
    int  volume = 0;
    char szLogInfo[512] = { 0 };
    int  err = 0;
    int  iRet = 0;
    t_di_confirm_deny_list di_confirm_deny_list = { 0 };
    const char playctrl_di[4] = {0x00, 0x00, 0x14, 0x02};//di:data identifier
    printf("Enter to ent_volumeset_req...\n");
    
    // 调用具体的业务数据处理函数
    if((sizeof(int)+12) != frame->user_data.len)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "frame user data len error, len=%d", frame->user_data.len);
        WRITELOG(LOG_ERROR, szLogInfo);
        err = 1;
    }
    else
    {
        memcpy(&volume, frame->user_data.data+12, frame->user_data.len-12);
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "volume =%d", volume);
        WRITELOG(LOG_INFO, szLogInfo);
        // 设置音量
        audio_set_volume(volume);
    }
    
    memset(&di_confirm_deny_list, 0x00, sizeof(t_di_confirm_deny_list));
    di_confirm_deny_list.num = 1;
    memcpy(di_confirm_deny_list.confirm_deny[0].di, playctrl_di, 4);
    di_confirm_deny_list.confirm_deny[0].err = err;
	memset(frame->user_data.data+12, 0, sizeof(frame->user_data.data)-12);
    iRet = pack_diconfirmdeny(&di_confirm_deny_list, frame->user_data.data + 12, MAX_DATAUNIT_LEN - 12);
    if(iRet < 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "pack_diconfirmdeny failed! iRet=%d", iRet);
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
    frame->user_data.len = iRet + 12;
    
    return 0;
}

int ent_volumeset_ack(t_buf *buf, t_3761frame *frame)
{
    printf("Enter to ent_volumeset_ack...\n");
    
    memcpy(frame->addr, collector_id, MAX_ADDR_LEN);
#if 0
    frame->user_data.afn = AFN_CONFIRM_DENY;
    frame->user_data.fn[0] = 1 << ((FN_DI - 1) % 8);
    frame->user_data.fn[1] = (FN_DI - 1) / 8;
#else
	frame->user_data.afn = AFN_CTRL_CMD;
    frame->user_data.fn[0] = 1 << ((FN_SETVOLUME_MUSIC - 1) % 8);
    frame->user_data.fn[1] = (FN_SETVOLUME_MUSIC - 1) / 8;
#endif
	//frame->user_data.len = buf->len;
    //memcpy(frame->user_data.data, buf->buf, buf->len);
    
    return 0;
}

int ent_volumeget_req(t_3761frame *frame, t_buf *buf)
{
    int  volume = 0;
	char cVol[5] = {0};
    char szLogInfo[512] = { 0 };
    int  iRet = 0;
    
    // 调用具体的业务数据处理函数
    memset(buf, 0 , sizeof(t_buf));
    volume = audio_get_volume();
	printf("ent_volumeget_req: vol[%d]\n", volume);
	sprintf(cVol, "%d", volume);
	buf->len = strlen(cVol);
	memcpy(buf->buf, cVol, strlen(cVol));
    return 0;
}

int ent_volumeget_ack(t_buf *buf, t_3761frame *frame)
{
    printf("Enter to ent_volumeget_ack...\n");
    
    memcpy(frame->addr, collector_id, MAX_ADDR_LEN);

	frame->user_data.afn = AFN_CTRL_CMD;
    frame->user_data.fn[0] = 1 << ((FN_GETVOLUME_MUSIC - 1) % 8);
    frame->user_data.fn[1] = (FN_GETVOLUME_MUSIC - 1) / 8;
    frame->user_data.len = buf->len + 12;
    memcpy(frame->user_data.data + 12, buf->buf, min(buf->len,MAX_DATAUNIT_LEN - 12));
    return 0;
}



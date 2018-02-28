#include <pthread.h>
#include <time.h>
#include "../cfg/config.h"   // �������ļ��ж�ȡ��־������
#include "log.h"



/*********************************************************************/
/*                         ��̬�ֲ���������                          */
/*********************************************************************/
static int    CREATE_MUTEX_OBJ(void);
static void   CLOSE_MUTEX_OBJ(void);
static char * LOG_LEVEL(int nLevel);
static int    AccessCfgFile(char *sCfgName, char *sCfgFullName);
static int    AccessLogFile(char *sLogName, char *sLogFullName);
static int    InitLogEnv(char *sCfgFullName, char *sLogFullName);
static void   GetBackupName(int nIndex, char *sOutput);
static int    GetNextLogfileIndex(void);
static void   GetTime(char *sTimeStr);
static void   _WriteLogFile(const char *sInfo);

/*********************************************************************/
/*                         ��̬����                                  */
/*********************************************************************/
static T_LogInfo t_log;     //�洢��־ģ�����
static char sField[2048];   //�洢���ص��ַ�����


/**********************************************************************
* Linux/Uinx��û��findfirst, findnext, findclose����
* ���´����Ƕ���ģ��, �ڲ��ұ����ļ�ʱʹ��, ��win��������
* ע��: ֻʵ������Ա���־ģ����ض�����, ��������
**********************************************************************/
#ifndef _WIN32  

struct finddata_t
{
    char name[PATH_MAX_LEN];
    long time_write;
};

static struct dirent * direntp = NULL;
static DIR  * dirp = NULL;
static char   scan_filename[PATH_MAX_LEN];
static char   dir_path[PATH_MAX_LEN];

long findfirst(char *filepath, struct finddata_t *finddata)
{
    struct stat statbuf;
    char   tmp_path[PATH_MAX_LEN];
    int    len;
    int    cmplen;
    
    memset(finddata, 0x00, sizeof(struct finddata_t));
    memset(scan_filename, 0x00, PATH_MAX_LEN);

    strncpy(dir_path, filepath, PATH_MAX_LEN);
    strncpy(scan_filename, GetField(filepath, "*", 1), PATH_MAX_LEN); //���ڱȽ�,ȥ��ͨ���*

    cmplen = strlen(scan_filename);

    //���Ŀ¼·��
    len = strlen(dir_path);
    for (; dir_path[len] != '/'; len--) ;
    dir_path[len]='\0';

    if (dirp != NULL)
    {
        closedir(dirp);
        dirp = NULL;
    }
    else
    {
        if ((dirp = opendir(dir_path)) == NULL) //��Ŀ¼
        {
            return -1L;
        }
    }

    //��ȡĿ¼���ݽ��бȽ�
    while ((direntp=readdir(dirp)) != NULL)
    {
        snprintf(tmp_path, PATH_MAX_LEN, "%s/%s", dir_path, direntp->d_name);
        if (strncmp(tmp_path, scan_filename, cmplen) == 0)
        {
            strncpy(finddata->name, tmp_path, PATH_MAX_LEN);
            stat(tmp_path, &statbuf);
            finddata->time_write = statbuf.st_mtime;
            return 0L;
        }
    }
    if (NULL != dirp)
    {
        closedir(dirp);
        dirp = NULL;
    }
    return -1L;
}

long findnext(long j, struct finddata_t *finddata)
{
    int    cmplen;
    char   tmp_path[PATH_MAX_LEN];
    struct stat statbuf;

    if (NULL == dirp)
    {
        return -1L;
    }

    memset(finddata, 0x00, sizeof(struct finddata_t));

    cmplen = strlen(scan_filename);

    while ((direntp = readdir(dirp)) != NULL)
    {
        snprintf(tmp_path, PATH_MAX_LEN, "%s/%s", dir_path, direntp->d_name);
        if (strncmp(tmp_path, scan_filename, cmplen) == 0)
        {
            strncpy(finddata->name, tmp_path, PATH_MAX_LEN);
            stat(tmp_path, &statbuf);
            finddata->time_write = statbuf.st_mtime;
            return 0L;
        }
    }

    if (NULL != dirp)
    {
        closedir(dirp);
        dirp = NULL;
    }
    return -1L;
}

long findclose(long j)
{
    if (NULL != dirp)
    {
        closedir(dirp);
        dirp = NULL;
    }
    return 0L;
}

#endif


/**********************************************************************
* �������ƣ� int CREATE_MUTEX_OBJ
* ���������� ����������(�����ڱ�ģ��)
* ��������� ��
* ��������� ��
* �� �� ֵ�� 0 -- �ɹ�, -1 -- ʧ��
* ����˵���� 
* �޸�����    �汾��     �޸���      �޸�����
* -----------------------------------------------
* 2010/01/01   V1.0       XXXX          XXXX
**********************************************************************/
static int CREATE_MUTEX_OBJ(void)
{
    if (MUTEX_INIT(t_log.hLogMutex))
    {
        return -1;
    }

    return 0;
}

/**********************************************************************
* �������ƣ� void CLOSE_MUTEX_OBJ
* ���������� ���ٻ�����(�����ڱ�ģ��)
* ��������� ��
* ��������� ��
* �� �� ֵ�� ��
* ����˵���� 
* �޸�����    �汾��     �޸���      �޸�����
* -----------------------------------------------
* 2010/01/01   V1.0       XXXX          XXXX
**********************************************************************/
static void CLOSE_MUTEX_OBJ(void)
{
    MUTEX_DESTROY(t_log.hLogMutex); 

    return;
}

/**********************************************************************
* �������ƣ� int GET_MUTEX_OBJ
* ���������� ����������(�����ڱ�ģ��)
* ��������� ��
* ��������� ��
* �� �� ֵ�� 0 -- �ɹ�, -1 -- ʧ��
* ����˵���� 
* �޸�����    �汾��     �޸���      �޸�����
* -----------------------------------------------
* 2010/01/01   V1.0       XXXX          XXXX
**********************************************************************/
int GET_MUTEX_OBJ(void)
{
#ifdef _WIN32
    DWORD retcode;
    retcode = WaitForSingleObject(t_log.hLogMutex, IO_FILE_TIMEOUT);

    if (retcode == WAIT_FAILED || retcode == WAIT_TIMEOUT)
    {
        return -1;
    }

#else
    struct timeval  now;
    struct timespec timeout;
    int retcode=0;

    gettimeofday(&now,NULL);                             //��ȡ��ǰ����ʱ��
    timeout.tv_sec  = now.tv_sec + IO_FILE_TIMEOUT/1000; //ָ����ʱʱ��
    timeout.tv_nsec = now.tv_usec * 1000;
#ifndef _CYGWIN
    retcode = pthread_mutex_timedlock(&t_log.hLogMutex, &timeout);
#else
    retcode = pthread_mutex_lock(&t_log.hLogMutex);
#endif
    if (retcode == ETIMEDOUT || retcode != 0)
    {
        return -1;
    }
#endif

    return 0;
}

/**********************************************************************
* �������ƣ� void RELEASE_MUTEX_OBJ
* ���������� ����������(�����ڱ�ģ��)
* ��������� ��
* ��������� ��
* �� �� ֵ�� ��
* ����˵���� 
* �޸�����    �汾��     �޸���      �޸�����
* -----------------------------------------------
* 2010/01/01   V1.0       XXXX          XXXX
**********************************************************************/
void RELEASE_MUTEX_OBJ(void)
{
    MUTEX_UNLOCK(t_log.hLogMutex);
}

/**********************************************************************
* �������ƣ� char * LOG_LEVEL
* ���������� ��ȡ��־�����Ӧ�����ַ���
* ��������� nLevel -- ��־����(0~5)
* ��������� ��
* �� �� ֵ�� ��־��������
* ����˵���� 
* �޸�����    �汾��     �޸���      �޸�����
* -----------------------------------------------
* 2010/01/01   V1.0       XXXX          XXXX
**********************************************************************/
static char * LOG_LEVEL(int nLevel)
{
    switch (nLevel)
    {
        case LOG_FATAL:   return "FATAL";
        case LOG_ERROR:   return "ERROR";
        case LOG_WARN :   return "WARN";
        case LOG_INFO :   return "INFO";
        case LOG_TRACE:   return "TRACE";
        case LOG_DEBUG:   return "DEBUG";
        default       :   return "OTHER";
    }
}

/**********************************************************************
* �������ƣ� char * strtok_div
* ���������� �ֽ�����s�����ַ���div�ָ�����, ������strtok����
* ��������� sBuf    -- �����ַ���
*            sDivStr -- �ָ��ַ���
* ��������� ��
* �� �� ֵ�� ָ���������ָ��, δ�ҵ�����NULL
* ����˵���� 
* �޸�����    �汾��     �޸���      �޸�����
* -----------------------------------------------
* 2010/01/01   V1.0       XXXX          XXXX
**********************************************************************/
char * strtok_div(char *sBuf, char *sDivStr) 
{
    static char sTemp1[1024];
    static char sTemp2[1024];
    char *p;
    int   nOffset;

    if (sBuf != NULL)
    {
        memset(sTemp1, 0, sizeof(sTemp1));
        memset(sTemp2, 0, sizeof(sTemp2));
        strcpy(sTemp1, sBuf);
    }

    if (strlen(sTemp1) == 0)
    {
        return NULL;
    }

    if ((p = strstr(sTemp1, sDivStr)) == NULL)
    {
        //���pָ��һ�����ݶ�����û�зָ���, �򷵻�pָ��������
        strcpy(sTemp2, sTemp1);
        memset(sTemp1, 0, sizeof(sTemp1));
    }
    else
    {
        nOffset = p - sTemp1;
        strcpy(sTemp2, sTemp1);
        sTemp2[nOffset] = '\0';
        strcpy(sTemp1, &sTemp1[nOffset + strlen(sDivStr)]);
    }
    p = sTemp2;

    return p;
}

/**********************************************************************
* �������ƣ� int GetFieldNum
* ���������� ��ȡ����sBuf����ָ���ַ���sDivStr�ָ�����������
* ��������� sBuf    -- �����ַ���
*            sDivStr -- �ָ��ַ���
* ��������� ��
* �� �� ֵ�� ��������(Դ�������ָ��ַ���ʱ����1)
* ����˵���� 
* �޸�����    �汾��     �޸���      �޸�����
* -----------------------------------------------
* 2010/01/01   V1.0       XXXX          XXXX
**********************************************************************/
int GetFieldNum(const char *sBuf, char *sDivStr)
{
    int  nNum = 0;
    char sTemp[1024] = {0};

    snprintf(sTemp, 1024, "%s", sBuf);    

    if ((strtok_div(sTemp, sDivStr)) != NULL)
    {
        nNum++;
        while (strtok_div(NULL, sDivStr) != NULL)
        {
            nNum++;
        }
    }

    return nNum;
}

/**********************************************************************
* �������ƣ� char * GetField
* ���������� ��ȡ����sBuf����ָ���ַ���sDivStr�ָ��ĵ�nIndex��������
* ��������� sBuf    -- �����ַ���
*            sDivStr -- �ָ��ַ���
*            nIndex  -- ���������(��1��ʼ)
* ��������� ��
* �� �� ֵ�� ָ���������ָ��, δ�ҵ����ؿ��ַ���"\0"
* ����˵���� 
* �޸�����    �汾��     �޸���      �޸�����
* -----------------------------------------------
* 2010/01/01   V1.0       XXXX          XXXX
**********************************************************************/
char * GetField(char *sBuf, char *sDivStr, int nIndex)
{
    char *p;
    int   i;

    memset(sField, 0, sizeof(sField));

    do
    {
        if (   NULL == sBuf
            || strlen(sBuf) == 0
            || nIndex <= 0
            || nIndex > GetFieldNum(sBuf, sDivStr))
        {
            break;
        }

        if ((p = strtok_div(sBuf, sDivStr)) == NULL)
        {
            break;
        }

        if (nIndex == 1)
        {
            strncpy(sField, p, sizeof(sField)-1);
            break;
        }

        for (i = 1; i < nIndex; i++)
        {
            if ((p = strtok_div(NULL, sDivStr)) == NULL)
            {
                break;
            }
        }

        if (p == NULL)
        {
            break;
        }

        strcpy(sField, p);
    }while(0);
    
    return sField;    
}

/**********************************************************************
* �������ƣ� int AccessCfgFile
* ���������� ��������ļ��Ƿ����, ��ƴװ��ȫ·��
* ��������� sCfgName     -- �����ļ���(����·��/����·��)
* ��������� sCfgFullName -- ȫ·����־�ļ���
* �� �� ֵ�� 0 -- �ɹ�,  -1 -- ʧ��
* ����˵���� 
* �޸�����    �汾��     �޸���      �޸�����
* -----------------------------------------------
* 2010/01/01   V1.0       XXXX          XXXX
**********************************************************************/
static int AccessCfgFile(char *sCfgName, char *sCfgFullName)
{
    char  sTmpFileName[PATH_MAX_LEN];
    char *pFind = NULL;

    if (NULL == sCfgName)
    {
        return -1;
    }
    
    memset(sTmpFileName, 0, sizeof(sTmpFileName));

    if (GetFieldNum(sCfgName, PATH_DIV) < 2)
    {
        //��Ϊ����·������, ��Ҫ����Ĭ��·��
#ifdef _WIN32
        snprintf(sTmpFileName, PATH_MAX_LEN, "%s\\etc\\%s", GET_HOME_DIR, sCfgName);
#else
        snprintf(sTmpFileName, PATH_MAX_LEN, "%s/etc/%s", GET_HOME_DIR, sCfgName);
#endif
    }
    else
    {
        //��Ϊ��������ļ����Ѵ�ȫ·��
        snprintf(sTmpFileName, PATH_MAX_LEN, "%s", sCfgName);
    }

    if (0 == access(sTmpFileName, W_OK))
    {
        //�˴����ݴ�ȫ·�������ļ���
        strcpy(sCfgFullName, sTmpFileName);
    }
    else
    {
        //��Ҫ�����ܺ�������ļ�.scr
        pFind = strstr(sTmpFileName, ".ini");
        if (pFind != NULL)
        {
            *pFind = '\0';
            strcat(sTmpFileName, ".scr");
        }

        if (0 == access(sTmpFileName, W_OK))
        {
            strcpy(sCfgFullName, sTmpFileName);
        }
        else
        {
            return -1;
        }
    }

    return 0;
}

/**********************************************************************
* �������ƣ� int AccessLogFile
* ���������� �����־�ļ��Ƿ����, ��ƴװ��ȫ·��
* ��������� sLogName     -- ��־�ļ���(����·��/����·��)
* ��������� sLogFullName -- ȫ·����־�ļ���
* �� �� ֵ�� 0 -- �ɹ�,  -1 -- ʧ��
* ����˵���� 
* �޸�����    �汾��     �޸���      �޸�����
* -----------------------------------------------
* 2010/01/01   V1.0       XXXX          XXXX
**********************************************************************/
static int AccessLogFile(char *sLogName, char *sLogFullName)
{
    if (NULL == sLogName)
    {
        return -1;
    }

    if (GetFieldNum(sLogName, PATH_DIV) < 2)
    {
        //��Ϊ����·������, ��Ҫ����Ĭ��·��
#ifdef _WIN32
        snprintf(sLogFullName, PATH_MAX_LEN, "%s\\log\\%s", GET_HOME_DIR, sLogName);
#else
        snprintf(sLogFullName, PATH_MAX_LEN, "%s/log/%s", GET_HOME_DIR, sLogName);
#endif
    }
    else
    {
        //��Ϊ��������ļ����Ѵ�ȫ·��
        snprintf(sLogFullName, PATH_MAX_LEN, "%s", sLogName);
    }

    return 0;
}

/**********************************************************************
* �������ƣ� int InitLogEnv
* ���������� ��ȡ�����ļ�, ��ʼ����־����
* ��������� sCfgFullName -- ȫ·�������ļ���
*            sLogFullName -- ȫ·����־�ļ���
* ��������� ��
* �� �� ֵ�� 0 -- �ɹ�,  -1 -- ʧ��
* ����˵���� 
* �޸�����    �汾��     �޸���      �޸�����
* -----------------------------------------------
* 2010/01/01   V1.0       XXXX          XXXX
**********************************************************************/
static int InitLogEnv(char *sCfgFullName, char *sLogFullName)
{
    int nLogSize = 0;

    //�������־����
    t_log.nLoglevel = GetPrivateProfileInt("LOG", "LogLevel", LOG_INFO, sCfgFullName);

    //��־�ļ���󳤶�, ��λMB
    nLogSize = GetPrivateProfileInt("LOG", "LogMaxSize", 10, sCfgFullName);
    if (nLogSize < 1 || nLogSize > 100)
    {
        nLogSize = 10;           //���ó���[1,100]��ΧĬ��5M
    }
    t_log.lMaxLogSize = nLogSize*1024*1024;

    //ÿ�������Ƿ��½�����־�ļ�
    t_log.nNewLogFile = GetPrivateProfileInt("LOG", "LogExclude", YES, sCfgFullName);

    //��󱸷���־�ļ�����
    t_log.nMaxBakCount = GetPrivateProfileInt("LOG", "BackupCount", 10, sCfgFullName);
    if (t_log.nMaxBakCount < 1 || t_log.nMaxBakCount > 999)
    {
        t_log.nMaxBakCount = 5;   //���ó���[1,999]��ΧĬ��10��
    }

    //�Ƿ������־λ����Ϣ(�ļ���/�к�)��־
    t_log.nLogPosition = GetPrivateProfileInt("LOG", "LogPosition", YES, sCfgFullName);

    //���������Ƿ��ѽ����״���־�ļ�����
    t_log.nFirstBackuped = NO;

    //����״̬Ĭ�Ϲر�, �ȴ�����������ɹ�����
    t_log.nWorking       = NO;

    //��¼��־�ļ���
    snprintf(t_log.sLogName, PATH_MAX_LEN, "%s", sLogFullName);

    //����������, WIN32��Ϊ�ź���
    if (CREATE_MUTEX_OBJ() < 0)
    {
        return -1;
    }

    t_log.nWorking = YES;
	
    return 0;
}

/**********************************************************************
* �������ƣ� int StartLog
* ���������� ������־���ܣ���������Դ
* ��������� sCfgName -- �����ļ���(����·��/����·��)
*            sLogName -- ��־�ļ���(����·��/����·��)
* ��������� ��
* �� �� ֵ�� 0 -- �ɹ�,  -1 -- ʧ��
* ����˵���� 
* �޸�����    �汾��     �޸���      �޸�����
* -----------------------------------------------
* 2010/01/01   V1.0       XXXX          XXXX
**********************************************************************/
int StartLog(char * sCfgName, char * sLogName)
{
    char sCfgFullName[PATH_MAX_LEN] = {0};
    char sLogFullName[PATH_MAX_LEN] = {0};

    memset(&t_log, 0, sizeof(T_LogInfo));

    if (NULL==sCfgName || NULL==sLogName)
    {
        return -1;
    }

   
    if (   0 > AccessCfgFile(sCfgName, sCfgFullName)    //���������ļ��Ƿ���ڲ����ȫ·����, �������
        || 0 > AccessLogFile(sLogName, sLogFullName)    //������־�ļ��Ƿ���ڲ����ȫ·����, �����ھʹ���
        || 0 > InitLogEnv(sCfgFullName, sLogFullName))  //�������ó�ʼ����־����
    /*
      if ( 0 > AccessLogFile(sLogName, sLogFullName)      //������־�ļ��Ƿ���ڲ����ȫ·����, �����ھʹ���
        || 0 > InitLogEnv(sCfgName, sLogFullName))  //�������ó�ʼ����־����
      {
             return -1;
      }
     */
    return 0;
}

/**********************************************************************
* �������ƣ� void CloseLog
* ���������� �ر���־����, �ͷ������Դ
* ��������� ��
* ��������� ��
* �� �� ֵ�� ��
* ����˵���� 
* �޸�����    �汾��     �޸���      �޸�����
* -----------------------------------------------
* 2010/01/01   V1.0       XXXX          XXXX
**********************************************************************/
void CloseLog()
{
    t_log.nWorking = NO;

    if (t_log.fpLogFile != NULL)
    {
        fclose(t_log.fpLogFile);
        t_log.fpLogFile = NULL;
    }

    CLOSE_MUTEX_OBJ();
    return;
}

/**********************************************************************
* �������ƣ� void GetBackupName
* ���������� ��õ�ǰ�����ļ���
* ��������� nIndex  -- �ļ����
* ��������� sOutput -- �����ļ���
* �� �� ֵ�� ��
* ����˵���� 
* �޸�����    �汾��     �޸���      �޸�����
* -----------------------------------------------
* 2010/01/01   V1.0       XXXX          XXXX
**********************************************************************/
static void GetBackupName(int nIndex, char *sOutput)
{
    snprintf(sOutput, PATH_MAX_LEN, "%s_%03d", t_log.sLogName, nIndex);
}

/**********************************************************************
* �������ƣ� int GetNextLogfileIndex
* ���������� �����һ����־�ļ����
* ��������� ��
* ��������� ��
* �� �� ֵ�� �ļ����
* ����˵���� 
* �޸�����    �汾��     �޸���      �޸�����
* -----------------------------------------------
* 2010/01/01   V1.0       XXXX          XXXX
**********************************************************************/
static int GetNextLogfileIndex(void)
{
    struct finddata_t  tfile;
    long               hFile;
    int                nNext    = -1;
    time_t             lLastTime =  0;
    char               sLastName[PATH_MAX_LEN] = {0};
    char               sFindPara[PATH_MAX_LEN] = {0};
    char              *pNo;
    
    //����ƥ��sFindParaָ�����ļ�
    sprintf(sFindPara, "%s_*", t_log.sLogName);
    if ((hFile = findfirst(sFindPara, &tfile)) != -1L)
    {
        strcpy(sLastName, tfile.name);
        lLastTime = tfile.time_write;

        while (findnext(hFile, &tfile) == 0)
        {
            //���������޸Ĺ����ļ�
            if (tfile.time_write > lLastTime)
            {
                snprintf(sLastName, PATH_MAX_LEN, "%s", tfile.name);
                lLastTime = tfile.time_write;
            }
        }
        findclose(hFile);
    }

    if (strlen(sLastName) > 0)
    {
        //ȡ�������޸Ĺ����ļ����
        pNo = GetField(sLastName, "_", GetFieldNum(sLastName, "_"));

        if (pNo != NULL)
        {
            nNext = atoi(pNo) + 1;
            if (nNext > t_log.nMaxBakCount)
            {
                nNext = 1;
            }
            return nNext;
        }
    }

    //δ�ҵ����ϸ�ʽ����־�ļ�ʱ, Ĭ�Ϸ���1
    return 1;
}

/**********************************************************************
* �������ƣ� void GetTime
* ���������� ��ȡ��ǰ����ʱ���ַ���
* ��������� ��
* ��������� sTimeStr -- ����ʱ���ַ���(��΢��)
* �� �� ֵ�� ��
* ����˵���� 
* �޸�����    �汾��     �޸���      �޸�����
* -----------------------------------------------
* 2010/01/01   V1.0       XXXX          XXXX
**********************************************************************/
static void GetTime(char *sTimeStr)
{
#ifdef _WIN32
    SYSTEMTIME  sys_time;

    GetLocalTime(&sys_time);
    sprintf(sTimeStr, "%04d.%02d.%02d %02d:%02d:%02d.%03d ", 
        sys_time.wYear, sys_time.wMonth, sys_time.wDay,
        sys_time.wHour, sys_time.wMinute, sys_time.wSecond,
        sys_time.wMilliseconds);
#else
    struct tm sys_time;
    struct timeval tp;
    time_t current_time;
    char   usec[20];

    current_time = time(NULL);                //ע:��ʱ�亯��ֻ֧�ֵ�2017��
    localtime_r(&current_time, &sys_time);    //�����̰߳�ȫ

    gettimeofday(&tp, NULL);
    sprintf(usec, "%06d", tp.tv_usec);

    sprintf(sTimeStr, "%04d.%02d.%02d %02d:%02d:%02d.%3.3s ", 
        sys_time.tm_year+1900, sys_time.tm_mon+1, sys_time.tm_mday,
        sys_time.tm_hour, sys_time.tm_min, sys_time.tm_sec, 
        &usec[3]);
#endif
    return;
}

/**********************************************************************
* �������ƣ� char * _FormatMsg
* ���������� ��ʽ����Ϣ����
* ��������� fmt      -- ��ʽ����Ϣ���ݴ�
*            ...      -- ��ʽ�������������
* ��������� ��
* �� �� ֵ�� �Ѹ�ʽ������Ϣ�ַ���
* ����˵���� ��Ԫ�ڲ���������Ҫֱ�ӵ���!
* �޸�����    �汾��     �޸���      �޸�����
* -----------------------------------------------
* 2010/01/01   V1.0       XXXX          XXXX
**********************************************************************/
char * _FormatMsg(const char * fmt,...)
{
    va_list args;
    static char buffer[LOG_BUF_LEN];

    va_start(args,fmt);
    vsnprintf(buffer, LOG_BUF_LEN, fmt, args);
    va_end(args);

    return buffer;
}

/**********************************************************************
* �������ƣ� void _WriteLogFile
* ���������� д��־����, �������д���ļ�����
* ��������� sInfo -- ��д����־��Ϣ
* ��������� ��
* �� �� ֵ�� ��
* ����˵���� 
* �޸�����    �汾��     �޸���      �޸�����
* -----------------------------------------------
* 2010/01/01   V1.0       XXXX          XXXX
**********************************************************************/
static void _WriteLogFile(const char *sInfo)
{
    char sTime[60];
    char sBakName[PATH_MAX_LEN];

    //����Ϊ����ģʽ
    if (t_log.nWorking != YES)
    {
        return;
    }

    //�����״�������Ҫ������ʷ��־�ļ�
    if (t_log.nFirstBackuped == NO)
    {
        //��������ȷ���Ƿ��������ļ�
        if (t_log.nNewLogFile == YES)
        {
            GetBackupName(GetNextLogfileIndex(), sBakName); //��ñ����ļ���
            unlink(sBakName);                               //��ñ����ļ�����, ��ɾ��
            rename(t_log.sLogName, sBakName);               //ԭ����־�ļ�����Ϊ��������
            t_log.fpLogFile = fopen(t_log.sLogName, "wt+"); //�½�����־�ļ�
        }
        else
        {
            t_log.fpLogFile = fopen(t_log.sLogName, "at+"); //��ԭ����־�ļ���׷��
        }

        t_log.nFirstBackuped = YES;
    }

    //д����־ʱ�估����
    GetTime(sTime);
    fputs(sTime, t_log.fpLogFile);
    fputs(sInfo, t_log.fpLogFile);
    fflush(t_log.fpLogFile);

    //�ļ����ڹ涨��С, ��Ҫ���ݺ��л�������־�ļ�
    if (ftell(t_log.fpLogFile) >= t_log.lMaxLogSize)
    {
        fclose(t_log.fpLogFile);
        t_log.fpLogFile = NULL;
        GetBackupName(GetNextLogfileIndex(), sBakName);     //��ñ����ļ���
        unlink(sBakName);                                   //��ñ����ļ�����, ��ɾ��
        rename(t_log.sLogName, sBakName);                   //ԭ����־�ļ�����Ϊ��������
        t_log.fpLogFile = fopen(t_log.sLogName, "wt+");     //�½�����־�ļ�
    }

    return;
}

/**********************************************************************
* �������ƣ� void _WriteLogFileLongMsg
* ���������� д��־����, ���ɱ����
* ��������� file     -- ���������ڵ��ļ�����, Ĭ��ʹ�� __FILE__
*            line     -- ���������ڵ��ļ�����, Ĭ��ʹ�� __LINE__
*            loglevel -- д��־����, ��ο�ͷ�ļ�
*            fmt      -- ��ʽ����־���ݴ�
*            ...      -- ��ʽ�������������
* ��������� ��
* �� �� ֵ�� ��
* ����˵���� 
* �޸�����    �汾��     �޸���      �޸�����
* -----------------------------------------------
* 2010/01/01   V1.0       XXXX          XXXX
**********************************************************************/
void _WriteLogFileLongMsg(char *file, long line, int loglevel, const char *fmt, ...)
{
    va_list args;
    char    sContent[LOG_BUF_LEN];
    char    sLogMsg[LOG_BUF_LEN];
    char    sFileName[PATH_MAX_LEN];

    if (fmt == NULL || file == NULL || loglevel > t_log.nLoglevel)
    {
        return;
    }

    va_start(args,fmt);
    vsnprintf(sContent, LOG_BUF_LEN, fmt, args);
    va_end(args);

    strcpy(sFileName, GetField(file, PATH_DIV, GetFieldNum(file, PATH_DIV))); //ȥ������·��, ֻ�����ļ���
    if (t_log.nLogPosition == YES)
    {
        snprintf(sLogMsg, LOG_BUF_LEN, "[%-5s] F[%-14s] L[%06d] %s\r\n", LOG_LEVEL(loglevel), sFileName, line, sContent);
    }
    else
    {
        snprintf(sLogMsg, LOG_BUF_LEN, "[%-5s] %s\r\n", LOG_LEVEL(loglevel), sContent);
    }
    _WriteLogFile(sLogMsg);

    return;
}






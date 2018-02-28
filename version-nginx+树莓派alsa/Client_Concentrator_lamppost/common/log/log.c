#include <pthread.h>
#include <time.h>
#include "../cfg/config.h"   // 从配置文件中读取日志配置项
#include "log.h"



/*********************************************************************/
/*                         静态局部函数声明                          */
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
/*                         静态变量                                  */
/*********************************************************************/
static T_LogInfo t_log;     //存储日志模块参数
static char sField[2048];   //存储返回的字符串域


/**********************************************************************
* Linux/Uinx下没有findfirst, findnext, findclose函数
* 以下代码是对其模拟, 在查找备份文件时使用, 与win操作兼容
* 注意: 只实现了针对本日志模块的特定处理, 请勿他用
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
    strncpy(scan_filename, GetField(filepath, "*", 1), PATH_MAX_LEN); //用于比较,去掉通配符*

    cmplen = strlen(scan_filename);

    //获得目录路径
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
        if ((dirp = opendir(dir_path)) == NULL) //打开目录
        {
            return -1L;
        }
    }

    //读取目录数据进行比较
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
* 函数名称： int CREATE_MUTEX_OBJ
* 功能描述： 创建互斥量(仅用于本模块)
* 输入参数： 无
* 输出参数： 无
* 返 回 值： 0 -- 成功, -1 -- 失败
* 其它说明： 
* 修改日期    版本号     修改人      修改内容
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
* 函数名称： void CLOSE_MUTEX_OBJ
* 功能描述： 销毁互斥量(仅用于本模块)
* 输入参数： 无
* 输出参数： 无
* 返 回 值： 无
* 其它说明： 
* 修改日期    版本号     修改人      修改内容
* -----------------------------------------------
* 2010/01/01   V1.0       XXXX          XXXX
**********************************************************************/
static void CLOSE_MUTEX_OBJ(void)
{
    MUTEX_DESTROY(t_log.hLogMutex); 

    return;
}

/**********************************************************************
* 函数名称： int GET_MUTEX_OBJ
* 功能描述： 互斥量加锁(仅用于本模块)
* 输入参数： 无
* 输出参数： 无
* 返 回 值： 0 -- 成功, -1 -- 失败
* 其它说明： 
* 修改日期    版本号     修改人      修改内容
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

    gettimeofday(&now,NULL);                             //获取当前绝对时间
    timeout.tv_sec  = now.tv_sec + IO_FILE_TIMEOUT/1000; //指定超时时间
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
* 函数名称： void RELEASE_MUTEX_OBJ
* 功能描述： 互斥量解锁(仅用于本模块)
* 输入参数： 无
* 输出参数： 无
* 返 回 值： 无
* 其它说明： 
* 修改日期    版本号     修改人      修改内容
* -----------------------------------------------
* 2010/01/01   V1.0       XXXX          XXXX
**********************************************************************/
void RELEASE_MUTEX_OBJ(void)
{
    MUTEX_UNLOCK(t_log.hLogMutex);
}

/**********************************************************************
* 函数名称： char * LOG_LEVEL
* 功能描述： 获取日志级别对应描述字符串
* 输入参数： nLevel -- 日志级别(0~5)
* 输出参数： 无
* 返 回 值： 日志级别描述
* 其它说明： 
* 修改日期    版本号     修改人      修改内容
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
* 函数名称： char * strtok_div
* 功能描述： 分解输入s中以字符串div分隔的域, 功能与strtok类似
* 输入参数： sBuf    -- 输入字符串
*            sDivStr -- 分隔字符串
* 输出参数： 无
* 返 回 值： 指向数据域的指针, 未找到返回NULL
* 其它说明： 
* 修改日期    版本号     修改人      修改内容
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
        //如果p指向一个数据而后面没有分隔串, 则返回p指向的这个串
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
* 函数名称： int GetFieldNum
* 功能描述： 获取输入sBuf中以指定字符串sDivStr分隔的数据域数
* 输入参数： sBuf    -- 输入字符串
*            sDivStr -- 分隔字符串
* 输出参数： 无
* 返 回 值： 数据域数(源串不含分隔字符串时返回1)
* 其它说明： 
* 修改日期    版本号     修改人      修改内容
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
* 函数名称： char * GetField
* 功能描述： 获取输入sBuf中以指定字符串sDivStr分隔的第nIndex个数据域
* 输入参数： sBuf    -- 输入字符串
*            sDivStr -- 分隔字符串
*            nIndex  -- 数据域序号(从1开始)
* 输出参数： 无
* 返 回 值： 指向数据域的指针, 未找到返回空字符串"\0"
* 其它说明： 
* 修改日期    版本号     修改人      修改内容
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
* 函数名称： int AccessCfgFile
* 功能描述： 检查配置文件是否存在, 并拼装其全路径
* 输入参数： sCfgName     -- 配置文件名(绝对路径/不带路径)
* 输出参数： sCfgFullName -- 全路径日志文件名
* 返 回 值： 0 -- 成功,  -1 -- 失败
* 其它说明： 
* 修改日期    版本号     修改人      修改内容
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
        //认为不含路径部分, 需要补足默认路径
#ifdef _WIN32
        snprintf(sTmpFileName, PATH_MAX_LEN, "%s\\etc\\%s", GET_HOME_DIR, sCfgName);
#else
        snprintf(sTmpFileName, PATH_MAX_LEN, "%s/etc/%s", GET_HOME_DIR, sCfgName);
#endif
    }
    else
    {
        //认为入参配置文件名已带全路径
        snprintf(sTmpFileName, PATH_MAX_LEN, "%s", sCfgName);
    }

    if (0 == access(sTmpFileName, W_OK))
    {
        //此处兼容带全路径配置文件名
        strcpy(sCfgFullName, sTmpFileName);
    }
    else
    {
        //需要检查加密后的配置文件.scr
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
* 函数名称： int AccessLogFile
* 功能描述： 检查日志文件是否存在, 并拼装其全路径
* 输入参数： sLogName     -- 日志文件名(绝对路径/不带路径)
* 输出参数： sLogFullName -- 全路径日志文件名
* 返 回 值： 0 -- 成功,  -1 -- 失败
* 其它说明： 
* 修改日期    版本号     修改人      修改内容
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
        //认为不含路径部分, 需要补足默认路径
#ifdef _WIN32
        snprintf(sLogFullName, PATH_MAX_LEN, "%s\\log\\%s", GET_HOME_DIR, sLogName);
#else
        snprintf(sLogFullName, PATH_MAX_LEN, "%s/log/%s", GET_HOME_DIR, sLogName);
#endif
    }
    else
    {
        //认为入参配置文件名已带全路径
        snprintf(sLogFullName, PATH_MAX_LEN, "%s", sLogName);
    }

    return 0;
}

/**********************************************************************
* 函数名称： int InitLogEnv
* 功能描述： 读取配置文件, 初始化日志环境
* 输入参数： sCfgFullName -- 全路径配置文件名
*            sLogFullName -- 全路径日志文件名
* 输出参数： 无
* 返 回 值： 0 -- 成功,  -1 -- 失败
* 其它说明： 
* 修改日期    版本号     修改人      修改内容
* -----------------------------------------------
* 2010/01/01   V1.0       XXXX          XXXX
**********************************************************************/
static int InitLogEnv(char *sCfgFullName, char *sLogFullName)
{
    int nLogSize = 0;

    //输出的日志级别
    t_log.nLoglevel = GetPrivateProfileInt("LOG", "LogLevel", LOG_INFO, sCfgFullName);

    //日志文件最大长度, 单位MB
    nLogSize = GetPrivateProfileInt("LOG", "LogMaxSize", 10, sCfgFullName);
    if (nLogSize < 1 || nLogSize > 100)
    {
        nLogSize = 10;           //配置超出[1,100]范围默认5M
    }
    t_log.lMaxLogSize = nLogSize*1024*1024;

    //每次启动是否新建空日志文件
    t_log.nNewLogFile = GetPrivateProfileInt("LOG", "LogExclude", YES, sCfgFullName);

    //最大备份日志文件数量
    t_log.nMaxBakCount = GetPrivateProfileInt("LOG", "BackupCount", 10, sCfgFullName);
    if (t_log.nMaxBakCount < 1 || t_log.nMaxBakCount > 999)
    {
        t_log.nMaxBakCount = 5;   //配置超出[1,999]范围默认10个
    }

    //是否输出日志位置信息(文件名/行号)标志
    t_log.nLogPosition = GetPrivateProfileInt("LOG", "LogPosition", YES, sCfgFullName);

    //程序启动是否已进行首次日志文件备份
    t_log.nFirstBackuped = NO;

    //工作状态默认关闭, 等待互斥量申请成功后开启
    t_log.nWorking       = NO;

    //记录日志文件名
    snprintf(t_log.sLogName, PATH_MAX_LEN, "%s", sLogFullName);

    //创建互斥量, WIN32下为信号量
    if (CREATE_MUTEX_OBJ() < 0)
    {
        return -1;
    }

    t_log.nWorking = YES;
	
    return 0;
}

/**********************************************************************
* 函数名称： int StartLog
* 功能描述： 开启日志功能，并分配资源
* 输入参数： sCfgName -- 配置文件名(绝对路径/不带路径)
*            sLogName -- 日志文件名(绝对路径/不带路径)
* 输出参数： 无
* 返 回 值： 0 -- 成功,  -1 -- 失败
* 其它说明： 
* 修改日期    版本号     修改人      修改内容
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

   
    if (   0 > AccessCfgFile(sCfgName, sCfgFullName)    //测试配置文件是否存在并输出全路径名, 必须存在
        || 0 > AccessLogFile(sLogName, sLogFullName)    //测试日志文件是否存在并输出全路径名, 不存在就创建
        || 0 > InitLogEnv(sCfgFullName, sLogFullName))  //根据配置初始化日志环境
    /*
      if ( 0 > AccessLogFile(sLogName, sLogFullName)      //测试日志文件是否存在并输出全路径名, 不存在就创建
        || 0 > InitLogEnv(sCfgName, sLogFullName))  //根据配置初始化日志环境
      {
             return -1;
      }
     */
    return 0;
}

/**********************************************************************
* 函数名称： void CloseLog
* 功能描述： 关闭日志功能, 释放相关资源
* 输入参数： 无
* 输出参数： 无
* 返 回 值： 无
* 其它说明： 
* 修改日期    版本号     修改人      修改内容
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
* 函数名称： void GetBackupName
* 功能描述： 获得当前备份文件名
* 输入参数： nIndex  -- 文件序号
* 输出参数： sOutput -- 备份文件名
* 返 回 值： 无
* 其它说明： 
* 修改日期    版本号     修改人      修改内容
* -----------------------------------------------
* 2010/01/01   V1.0       XXXX          XXXX
**********************************************************************/
static void GetBackupName(int nIndex, char *sOutput)
{
    snprintf(sOutput, PATH_MAX_LEN, "%s_%03d", t_log.sLogName, nIndex);
}

/**********************************************************************
* 函数名称： int GetNextLogfileIndex
* 功能描述： 获得下一个日志文件序号
* 输入参数： 无
* 输出参数： 无
* 返 回 值： 文件序号
* 其它说明： 
* 修改日期    版本号     修改人      修改内容
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
    
    //搜索匹配sFindPara指定的文件
    sprintf(sFindPara, "%s_*", t_log.sLogName);
    if ((hFile = findfirst(sFindPara, &tfile)) != -1L)
    {
        strcpy(sLastName, tfile.name);
        lLastTime = tfile.time_write;

        while (findnext(hFile, &tfile) == 0)
        {
            //查找最新修改过的文件
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
        //取得最新修改过的文件编号
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

    //未找到符合格式的日志文件时, 默认返回1
    return 1;
}

/**********************************************************************
* 函数名称： void GetTime
* 功能描述： 获取当前日期时间字符串
* 输入参数： 无
* 输出参数： sTimeStr -- 日期时间字符串(含微秒)
* 返 回 值： 无
* 其它说明： 
* 修改日期    版本号     修改人      修改内容
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

    current_time = time(NULL);                //注:该时间函数只支持到2017年
    localtime_r(&current_time, &sys_time);    //符合线程安全

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
* 函数名称： char * _FormatMsg
* 功能描述： 格式化消息内容
* 输入参数： fmt      -- 格式化消息内容串
*            ...      -- 格式化输入参数部分
* 输出参数： 无
* 返 回 值： 已格式化的消息字符串
* 其它说明： 单元内部函数，不要直接调用!
* 修改日期    版本号     修改人      修改内容
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
* 函数名称： void _WriteLogFile
* 功能描述： 写日志函数, 完成最后的写入文件动作
* 输入参数： sInfo -- 待写入日志信息
* 输出参数： 无
* 返 回 值： 无
* 其它说明： 
* 修改日期    版本号     修改人      修改内容
* -----------------------------------------------
* 2010/01/01   V1.0       XXXX          XXXX
**********************************************************************/
static void _WriteLogFile(const char *sInfo)
{
    char sTime[60];
    char sBakName[PATH_MAX_LEN];

    //必须为工作模式
    if (t_log.nWorking != YES)
    {
        return;
    }

    //程序首次启动需要处理历史日志文件
    if (t_log.nFirstBackuped == NO)
    {
        //根据配置确定是否生成新文件
        if (t_log.nNewLogFile == YES)
        {
            GetBackupName(GetNextLogfileIndex(), sBakName); //获得备份文件名
            unlink(sBakName);                               //如该备份文件存在, 则删除
            rename(t_log.sLogName, sBakName);               //原有日志文件改名为备份名称
            t_log.fpLogFile = fopen(t_log.sLogName, "wt+"); //新建空日志文件
        }
        else
        {
            t_log.fpLogFile = fopen(t_log.sLogName, "at+"); //在原有日志文件中追加
        }

        t_log.nFirstBackuped = YES;
    }

    //写入日志时间及内容
    GetTime(sTime);
    fputs(sTime, t_log.fpLogFile);
    fputs(sInfo, t_log.fpLogFile);
    fflush(t_log.fpLogFile);

    //文件大于规定大小, 需要备份后切换到新日志文件
    if (ftell(t_log.fpLogFile) >= t_log.lMaxLogSize)
    {
        fclose(t_log.fpLogFile);
        t_log.fpLogFile = NULL;
        GetBackupName(GetNextLogfileIndex(), sBakName);     //获得备份文件名
        unlink(sBakName);                                   //如该备份文件存在, 则删除
        rename(t_log.sLogName, sBakName);                   //原有日志文件改名为备份名称
        t_log.fpLogFile = fopen(t_log.sLogName, "wt+");     //新建空日志文件
    }

    return;
}

/**********************************************************************
* 函数名称： void _WriteLogFileLongMsg
* 功能描述： 写日志函数, 带可变参数
* 输入参数： file     -- 调用者所在的文件名称, 默认使用 __FILE__
*            line     -- 调用者所在的文件行数, 默认使用 __LINE__
*            loglevel -- 写日志级别, 请参考头文件
*            fmt      -- 格式化日志内容串
*            ...      -- 格式化输入参数部分
* 输出参数： 无
* 返 回 值： 无
* 其它说明： 
* 修改日期    版本号     修改人      修改内容
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

    strcpy(sFileName, GetField(file, PATH_DIV, GetFieldNum(file, PATH_DIV))); //去掉多余路径, 只保留文件名
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






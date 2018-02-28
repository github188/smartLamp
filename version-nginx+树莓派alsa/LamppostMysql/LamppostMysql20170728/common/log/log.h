
/*
* 基本功能说明：
*   0 Windows/Linux/Unix兼容(代码遵循POSIX标准。但UNIX还未测试(可能会有不支持pthread_mutex_timedlock情况))
*   1 日志信息分级，可通过配置决定输出的日志级别
*   2 线程安全，支持多线程同时输出日志
*   3 可格式化输出日志
*   4 可输出日志级别、源文件名及行号
*   5 可配置决定是否连接多次运行的日志信息
*   6 日志最大长度可配置，超过最大长度则自动备份
*   7 日志备份文件数量可配置，备份文件名为日志文件名后增加备份编号"_???"
*   8 备份文件编号在允许范围内循环使用[1,999]
* 
* 日志级别说明：
*   日志级别越高, 输出内容越详细
*   LOG_FATAL   0  严重错误
*   LOG_ERROR   1  一般错误
*   LOG_WARN    2  警告信息
*   LOG_INFO    3  一般信息
*   LOG_TRACE   4  跟踪信息
*   LOG_DEBUG   5  调试信息
*   LOG_ALL     6  全部
* 
* 使用方法说明：
*   1 拷贝 log.h、log.c(pp) 文件到工程目录, 并加入工程 (Linux或Uinx需要添加libpthrad.a库)
*   2 包含 log.h 头文件
*   3 将 log.ini 中的日志配置项拷贝到当前工程配置文件中
*   4 在程序启动时调用 StartLog() 开启日志功能; 在程序退出时调用 CloseLog() 关闭日志功能，释放资源
*   5 在程序中请使用以下两个宏定义语句输出日志信息，不要直接使用 _WriteLogFileLongMsg() 日志函数
*     注意, 由于VC6到VS2005不支持C99特性, 又为使WIN32与Linux代码保持一致, 故出现以下两种变化:
*           (1)写日志宏分为固定参数日志宏WRITELOG, 和多参数日志宏WRITELOGEX
*           (2)WRITELOGEX宏中间格式化串与参数部分需要使用()括起来
*     日志宏使用范例：
*         WRITELOG(LOG_INFO, "CPService V1.01.01");
*         WRITELOGEX(LOG_INFO, ("CPService%d %s", 100, "V1.01.01"));
*     输出结果为
*         2017.03.30 13:32:54.031 [INFO ] F[serv.c        ] L[00023] CPService V1.01.01
*         2017.03.30 13:32:54.054 [INFO ] F[serv.c        ] L[00024] CPService100 V1.01.01
*
* 性能说明：
*   日志写入性能与文件大小无明显关系，写入速度： 
*   PC机
*       Suse Linux9 IMP 测试机 40个线程写入量约 45000条/秒
*       Suse Linux9 IMP 测试机   单线程写入量约 56000条/秒
*       Windowns XP     开发机 40个线程写入量约 27000条/秒  数据可能不准
*       Windowns XP     开发机   单线程写入量约 28500条/秒  数据可能不准
*   AIX
*       未测试
*   HP-UX
*       未测试
*/

#ifndef _LOG_H_
#define _LOG_H_

#ifdef _WIN32
  #pragma pack(push,1)
#else
  #pragma pack(1)
#endif

/*********************************************************************/
/*                           头文件定义                              */
/*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h> 
#include <time.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
  #include <windows.h>
  #include <io.h>
#else
  #include <sys/io.h>
  #include <unistd.h>
  #include <sys/time.h>
  #include <sys/stat.h>
  #include <dirent.h>
  #include <pthread.h>
#endif

//#include "impfunc.h"  


/*********************************************************************/
/*                           函数声明                                */
/*********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

int    GET_MUTEX_OBJ(void);
void   RELEASE_MUTEX_OBJ(void);

int    StartLog(char * sCfgName, char * sLogName); //请使用不带路径文件名或绝对路径文件名, 不要使用相对路径
void   CloseLog(void);
char * _FormatMsg(const char * fmt,...);
void   _WriteLogFileLongMsg(char *file, long line, int loglevel, const char *fmt, ...);

char * strtok_div(char *sBuf, char *sDivStr);
char * GetField(char *sBuf, char *sDivStr, int nIndex);
int    GetFieldNum(const char *sBuf, char *sDivStr);

#ifdef __cplusplus
}
#endif

/*********************************************************************/
/*                           宏定义                                  */
/*********************************************************************/
//日志级别定义
#define LOG_FATAL       (int)0     //严重错误
#define LOG_ERROR       (int)1     //一般错误
#define LOG_WARN        (int)2     //警告信息
#define LOG_INFO        (int)3     //一般信息
#define LOG_TRACE       (int)4     //跟踪信息
#define LOG_DEBUG       (int)5     //调试信息
#define LOG_ALL         (int)6     //全部

#define YES             (int)1
#define NO              (int)0
#define LOG_BUF_LEN     (int)4000  //最大缓冲长度
#define PATH_MAX_LEN    (int)256   //最大路径长度
#define IO_FILE_TIMEOUT (int)5000  //获取互斥量超时时长为5000毫秒
#define F_L             __FILE__,__LINE__


//写日志宏
#define WRITELOG(level, fmt) \
do { \
    if (GET_MUTEX_OBJ() < 0) \
    { \
      break; \
    } \
    _WriteLogFileLongMsg(F_L, level, fmt); \
    RELEASE_MUTEX_OBJ(); \
} while (0)

#ifdef _WIN32

  #define WRITELOGEX(level, fmt) \
  do { \
      if (GET_MUTEX_OBJ() < 0) \
      { \
          break; \
      } \
      _WriteLogFileLongMsg(F_L, level, _FormatMsg##fmt); \
      RELEASE_MUTEX_OBJ(); \
  } while (0)

#else

  #define WRITELOGEX(level, fmt) \
  do { \
      if (GET_MUTEX_OBJ() < 0) \
      { \
          break; \
      } \
      _WriteLogFileLongMsg(F_L, level, _FormatMsg fmt); \
      RELEASE_MUTEX_OBJ(); \
  } while (0)

#endif


//跨平台兼容定义
#ifdef _WIN32
  #define snprintf         _snprintf
  #define vsnprintf        _vsnprintf
  #define findclose        _findclose
  #define findfirst        _findfirst
  #define findnext         _findnext
  #define finddata_t       _finddata_t
  /*lint -e652*/
  #define access           _access
  /*lint +e652*/
  #define W_OK             (int)06              //WIN SDK _access使用
  /*lint -e773*/
  #define MUTEX_INIT(A)    ((A = CreateSemaphore(NULL, 1, 1, NULL))==NULL)
  /*lint +e773*/
  #define MUTEX_UNLOCK(A)  ReleaseSemaphore(A, 1, NULL)
  #define MUTEX_DESTROY(A) CloseHandle(A)
  #define GET_HOME_DIR    (getenv("HOME") ? getenv("HOME") : "C:\\guangdianyuan")
  #define PATH_DIV         "\\"                 //路径分隔符号
#else
  #define HANDLELOG        pthread_mutex_t
  /*lint -e773*/
  #define MUTEX_INIT(A)    pthread_mutex_init(&A, NULL)
  /*lint +e773*/
  #define MUTEX_UNLOCK(A)  pthread_mutex_unlock(&A)
  #define MUTEX_DESTROY(A) pthread_mutex_destroy(&A)
  //#define GET_HOME_DIR    (getenv("HOME") ? getenv("HOME") : "/usr/local/arm/dianti")
  #define GET_HOME_DIR    "/usr/local/arm/smartlamppost"
  #define PATH_DIV         "/"                  //路径分隔符号
#endif


/*********************************************************************/
/*                           数据类型定义                            */
/*********************************************************************/
//日志模块全局参数结构
typedef struct
{
    int       nLoglevel;                //输出的日志级别
    long      lMaxLogSize;              //日志文件最大长度
    int       nMaxBakCount;             //最大备份日志文件数量
    int       nNewLogFile;              //启动时是否新建空日志文件, 否则追加到上次的日志文件中
    int       nLogPosition;             //是否输出日志位置信息(文件名/行号)
    FILE    * fpLogFile;                //日志文件句柄
    int       nFirstBackuped;           //接口启动首次备份标志
    int       nWorking;                 //工作标志
    char      sLogName[PATH_MAX_LEN];   //日志文件全路径名称
#ifdef _WIN32
    HANDLE    hLogMutex;                //文件操作同步量
#else
    HANDLELOG hLogMutex;                //文件操作同步量
#endif
} T_LogInfo;


#ifdef _WIN32
  #pragma pack(pop)
#else
  #pragma pack()
#endif

#endif /*~end _LOG_H_*/

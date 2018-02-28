
/*
* ��������˵����
*   0 Windows/Linux/Unix����(������ѭPOSIX��׼����UNIX��δ����(���ܻ��в�֧��pthread_mutex_timedlock���))
*   1 ��־��Ϣ�ּ�����ͨ�����þ����������־����
*   2 �̰߳�ȫ��֧�ֶ��߳�ͬʱ�����־
*   3 �ɸ�ʽ�������־
*   4 �������־����Դ�ļ������к�
*   5 �����þ����Ƿ����Ӷ�����е���־��Ϣ
*   6 ��־��󳤶ȿ����ã�������󳤶����Զ�����
*   7 ��־�����ļ����������ã������ļ���Ϊ��־�ļ��������ӱ��ݱ��"_???"
*   8 �����ļ����������Χ��ѭ��ʹ��[1,999]
* 
* ��־����˵����
*   ��־����Խ��, �������Խ��ϸ
*   LOG_FATAL   0  ���ش���
*   LOG_ERROR   1  һ�����
*   LOG_WARN    2  ������Ϣ
*   LOG_INFO    3  һ����Ϣ
*   LOG_TRACE   4  ������Ϣ
*   LOG_DEBUG   5  ������Ϣ
*   LOG_ALL     6  ȫ��
* 
* ʹ�÷���˵����
*   1 ���� log.h��log.c(pp) �ļ�������Ŀ¼, �����빤�� (Linux��Uinx��Ҫ���libpthrad.a��)
*   2 ���� log.h ͷ�ļ�
*   3 �� log.ini �е���־�����������ǰ���������ļ���
*   4 �ڳ�������ʱ���� StartLog() ������־����; �ڳ����˳�ʱ���� CloseLog() �ر���־���ܣ��ͷ���Դ
*   5 �ڳ�������ʹ�����������궨����������־��Ϣ����Ҫֱ��ʹ�� _WriteLogFileLongMsg() ��־����
*     ע��, ����VC6��VS2005��֧��C99����, ��ΪʹWIN32��Linux���뱣��һ��, �ʳ����������ֱ仯:
*           (1)д��־���Ϊ�̶�������־��WRITELOG, �Ͷ������־��WRITELOGEX
*           (2)WRITELOGEX���м��ʽ���������������Ҫʹ��()������
*     ��־��ʹ�÷�����
*         WRITELOG(LOG_INFO, "CPService V1.01.01");
*         WRITELOGEX(LOG_INFO, ("CPService%d %s", 100, "V1.01.01"));
*     ������Ϊ
*         2017.03.30 13:32:54.031 [INFO ] F[serv.c        ] L[00023] CPService V1.01.01
*         2017.03.30 13:32:54.054 [INFO ] F[serv.c        ] L[00024] CPService100 V1.01.01
*
* ����˵����
*   ��־д���������ļ���С�����Թ�ϵ��д���ٶȣ� 
*   PC��
*       Suse Linux9 IMP ���Ի� 40���߳�д����Լ 45000��/��
*       Suse Linux9 IMP ���Ի�   ���߳�д����Լ 56000��/��
*       Windowns XP     ������ 40���߳�д����Լ 27000��/��  ���ݿ��ܲ�׼
*       Windowns XP     ������   ���߳�д����Լ 28500��/��  ���ݿ��ܲ�׼
*   AIX
*       δ����
*   HP-UX
*       δ����
*/

#ifndef _LOG_H_
#define _LOG_H_

#ifdef _WIN32
  #pragma pack(push,1)
#else
  #pragma pack(1)
#endif

/*********************************************************************/
/*                           ͷ�ļ�����                              */
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
/*                           ��������                                */
/*********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

int    GET_MUTEX_OBJ(void);
void   RELEASE_MUTEX_OBJ(void);

int    StartLog(char * sCfgName, char * sLogName); //��ʹ�ò���·���ļ��������·���ļ���, ��Ҫʹ�����·��
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
/*                           �궨��                                  */
/*********************************************************************/
//��־������
#define LOG_FATAL       (int)0     //���ش���
#define LOG_ERROR       (int)1     //һ�����
#define LOG_WARN        (int)2     //������Ϣ
#define LOG_INFO        (int)3     //һ����Ϣ
#define LOG_TRACE       (int)4     //������Ϣ
#define LOG_DEBUG       (int)5     //������Ϣ
#define LOG_ALL         (int)6     //ȫ��

#define YES             (int)1
#define NO              (int)0
#define LOG_BUF_LEN     (int)4000  //��󻺳峤��
#define PATH_MAX_LEN    (int)256   //���·������
#define IO_FILE_TIMEOUT (int)5000  //��ȡ��������ʱʱ��Ϊ5000����
#define F_L             __FILE__,__LINE__


//д��־��
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


//��ƽ̨���ݶ���
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
  #define W_OK             (int)06              //WIN SDK _accessʹ��
  /*lint -e773*/
  #define MUTEX_INIT(A)    ((A = CreateSemaphore(NULL, 1, 1, NULL))==NULL)
  /*lint +e773*/
  #define MUTEX_UNLOCK(A)  ReleaseSemaphore(A, 1, NULL)
  #define MUTEX_DESTROY(A) CloseHandle(A)
  #define GET_HOME_DIR    (getenv("HOME") ? getenv("HOME") : "C:\\guangdianyuan")
  #define PATH_DIV         "\\"                 //·���ָ�����
#else
  #define HANDLELOG        pthread_mutex_t
  /*lint -e773*/
  #define MUTEX_INIT(A)    pthread_mutex_init(&A, NULL)
  /*lint +e773*/
  #define MUTEX_UNLOCK(A)  pthread_mutex_unlock(&A)
  #define MUTEX_DESTROY(A) pthread_mutex_destroy(&A)
  //#define GET_HOME_DIR    (getenv("HOME") ? getenv("HOME") : "/usr/local/arm/dianti")
  #define GET_HOME_DIR    "/usr/local/arm/smartlamppost"
  #define PATH_DIV         "/"                  //·���ָ�����
#endif


/*********************************************************************/
/*                           �������Ͷ���                            */
/*********************************************************************/
//��־ģ��ȫ�ֲ����ṹ
typedef struct
{
    int       nLoglevel;                //�������־����
    long      lMaxLogSize;              //��־�ļ���󳤶�
    int       nMaxBakCount;             //��󱸷���־�ļ�����
    int       nNewLogFile;              //����ʱ�Ƿ��½�����־�ļ�, ����׷�ӵ��ϴε���־�ļ���
    int       nLogPosition;             //�Ƿ������־λ����Ϣ(�ļ���/�к�)
    FILE    * fpLogFile;                //��־�ļ����
    int       nFirstBackuped;           //�ӿ������״α��ݱ�־
    int       nWorking;                 //������־
    char      sLogName[PATH_MAX_LEN];   //��־�ļ�ȫ·������
#ifdef _WIN32
    HANDLE    hLogMutex;                //�ļ�����ͬ����
#else
    HANDLELOG hLogMutex;                //�ļ�����ͬ����
#endif
} T_LogInfo;


#ifdef _WIN32
  #pragma pack(pop)
#else
  #pragma pack()
#endif

#endif /*~end _LOG_H_*/

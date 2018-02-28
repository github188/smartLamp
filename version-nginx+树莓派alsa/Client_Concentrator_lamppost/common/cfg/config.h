
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <dirent.h>

#ifdef _WIN32
	#pragma pack(push, 1)
#else
	#pragma pack(1)
#endif


#ifndef MAX_PATH
#define MAX_PATH                        255
#endif

#define WORK_DIR            "test"
#define WORK_DIR_DEFINE     "/usr/local/arm/smartlamppost"

	// 数据库信息
	typedef struct
	{
		char	szIPAddr[30];
		char	szUserName[30];
		char	szPassword[30];
		char	szDBName[30];
		int 	iDBPort;
	} DBInfo_T;

    // 配置信息结构体
	typedef struct
	{
		DBInfo_T tDBInfo;
	}Config_T;



    int             GetPrivateProfileInt( char *AppName
                                        , char *KeyName
                                        , int def
                                        , char *FileName );
   
    int             GetPrivateProfileString( char *AppName
                                           , char *KeyName
                                           , char *def
                                           , char *out
                                           , int len
                                           , char *FileName );
   
    int             killblk( char *str );
    int             fgetstr( FILE *fp, char *str );


    int             cutpre( char *str );
	char            *strlwr( char *mstr );
	int             cuttail( char *str );
    
#ifdef _WIN32
	#pragma pack(pop)
#else
	#pragma pack()
#endif


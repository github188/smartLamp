#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <pthread.h>
#include <fastcgi.h>
#include <fcgiapp.h>
#include <signal.h>
//add by tianyu
#include <time.h>
#include <unistd.h>
#include "log.h"
#include "netinf.h"
#include "global.h"
#include "collector_proc.h"
#include "chargepile_proc.h"
#include "parklock_proc.h"
#include "music_proc.h"
#include "envir_proc.h"
#include "upserv.h"
#include "serv.h"
#include "config.h"
#include "gdmysql.h"
//#include "thread_pool.h"
#include "thpool.h"
#include "dealhash.h"

/******************宏定义**************/
#define CONFIG_INI    "smartlamppost.ini"
#define LOG_FILE      "smartlamppost.log"


/******************全局变量定义**************/
static pthread_t g_aDownservTid[MAX_THREAD_COUNT] = { 0 };
static pthread_t g_aUpservTid[MAX_THREAD_COUNT] = { 0 };
static pthread_mutex_t g_accept_mutex = PTHREAD_MUTEX_INITIALIZER;
static unsigned int g_uiDownservThreadNum = 1;
static unsigned int g_uiUpservThreadNum   = 16;

static unsigned short g_usListenPort    = 9009;
static unsigned int   g_uiRecvBufLen    = 8;
static unsigned int   g_uiSendBufLen    = 8;

Config_T g_Conf = {0};


static unsigned long g_msg_serial = 0;
static pthread_mutex_t g_serial_mutex = PTHREAD_MUTEX_INITIALIZER;
static int MAX_INT = ((unsigned)(-1))>>1;
static threadpool thpool= NULL;

//static int MIN_INT = ~MAX_INT;

static T_Serv_Proc g_atServProc[] = {
    ////////////////////集中器处理//////////////////////
    { COLLECTOR_ADD_URL,             serv_add_collector       },    // 增加集中器处理
    { COLLECTOR_DELETE_URL,          serv_del_collector       },    // 删除集中器处理
    { COLLECTOR_MODIFY_URL,          serv_modify_collector    },    // 修改集中器处理
    { COLLECTOR_QUERY_BY_ID_URL,     serv_queryid_collector   },    // 查询集中器处理(通过设备ID)
    { COLLECTOR_QUERY_BY_GIS_URL,    serv_querygis_collector  },    // 查询集中器处理(通过地理位置信息)

    ////////////////////充电桩处理//////////////////////
    { CHARGEPILE_ADD_URL,            serv_add_chargepile      },    // 增加充电桩处理
    { CHARGEPILE_DELETE_URL,         serv_del_chargepile      },    // 删除充电桩处理
    { CHARGEPILE_MODIFY_URL,         serv_modify_chargepile   },    // 修改充电桩处理
    { CHARGEPILE_QUERY_BY_ID_URL,    serv_queryid_chargepile  },    // 查询充电桩处理(通过设备ID)
    { CHARGEPILE_QUERY_BY_GIS_URL,   serv_querygis_chargepile },    // 查询充电桩处理(通过地理位置信息)

    ////////////////////车位锁处理//////////////////////
    { PARKLOCK_ADD_URL,              serv_add_parklock        },    // 增加车位锁处理
    { PARKLOCK_DELETE_URL,           serv_del_parklock        },    // 删除车位锁处理
    { PARKLOCK_MODIFY_URL,           serv_modify_parklock     },    // 修改车位锁处理
    { PARKLOCK_QUERY_BY_ID_URL,      serv_queryid_parklock    },    // 查询车位锁处理(通过设备ID)
    { PARKLOCK_QUERY_BY_GIS_URL,     serv_querygis_parklock   },    // 查询车位锁处理(通过地理位置信息)
    { PARKLOCK_CONTROL_URL,          serv_control_parklock    },    // 车位锁开关处理
    { PARKLOCK_REALTIMEQUERY_URL,    serv_realtimequery_parklock }, // 实时查询车位锁状态

    ////////////////////音乐播放处理//////////////////////
    { MUSIC_GET_PLAYLIST_URL,        serv_getplaylist_music_DB   },    // 请求播放列表
    { MUSIC_PLAYSTART_MUSIC_URL,     serv_playstart_music     },    // 开始播放请求处理
    { MUSIC_PLAYCTRL_MUSIC_URL,      serv_playctrl_music      },    // 播放控制请求处理
    { MUSIC_SETVOLUME_MUSIC_URL,     serv_setvolume_music     },    // 设置音量请求处理
    { MUSIC_GETVOLUME_MUSIC_URL,     serv_getvolume_music 	  },	//获取音量值

    ////////////////////环境参数采集处理//////////////////
    { ENVIR_GET_PARAM_URL,           serv_get_envir_param     },
    { ENVIR_GET_ALL_PARAM_URL,       serv_get_all_envir_param },     

    ////////////////////路灯控制处理//////////////////////

	////////////////////新提供的接口/////////////////////
    { USER_LOGIN,					 serv_user_login		  },    //用户登录
    { USER_LOGOUT,					 serv_user_logout  		  },	//用户退出登录
    { ENVIR_GET_VALUES,				 serv_get_env_values	  },    //获取环境参数
    { PARKLOCK_QUERY_STATUS, 		 serv_query_parklock_status},   //获取车位锁状态
    { PARKLOCK_CONTROL, 			 serv_control_parklock_dev},	//控制车位锁
    { ENVIR_GET_VALUES_NO_VERIFY,    serv_get_env_values_no_verify}
};


int get_msg_serial()
{
	int serial;
	pthread_mutex_lock(&g_serial_mutex);
	g_msg_serial = (++g_msg_serial)% MAX_INT;
	serial = g_msg_serial;
	pthread_mutex_unlock(&g_serial_mutex);
	return serial;
}


/*自定义回调函数*/
int m_routine(void * arg)
{
	/*提取请求，调用相应处理函数*/
	char         *url              = NULL;
    char          strRecv[MAXLINE]     = { 0 };
    char          strSend[MAXLINE]    = { 0 };
	char 		  rspMsg[MAXLINE] 	   = { 0 };
    char          *content_length  = NULL;
	char          szLogInfo[1024]  = { 0 };
	int i, iRet = 0;
	int msg_serial = 0;
	pthread_t self = pthread_self();
	T_Hash_St *p_hash = NULL;
	
	FCGX_Request *req = (FCGX_Request *)arg;
	if (req == NULL)
		return -1;

	url = FCGX_GetParam("REQUEST_URI", req->envp);
    
    snprintf(szLogInfo, sizeof(szLogInfo) - 1, "^^^^^GC: url = %s...............self[%lu] req[%p]^^^^^^", url, self, req);
    WRITELOG(LOG_INFO, szLogInfo);
	if(NULL == url)
    {
         snprintf(szLogInfo, sizeof(szLogInfo) - 1, "url is NULL, self[%lu]", self);
         WRITELOG(LOG_ERROR, szLogInfo);
         sprintf(rspMsg, "%s\r\n%s", RSP_HEADER, "url is NULL");
		 iRet = -1;
		 goto EXIT;
    }

	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "url=%s", url);
    WRITELOG(LOG_INFO, szLogInfo);

    content_length = FCGX_GetParam("CONTENT_LENGTH", req->envp);
    if(NULL == content_length)
    {
         snprintf(szLogInfo, sizeof(szLogInfo) - 1, "content_length is NULL, self[%lu]", self);
         WRITELOG(LOG_ERROR, szLogInfo);
         sprintf(rspMsg, "%s\r\n%s", RSP_HEADER, "content_length is NULL");
		 iRet = -1;
		 goto EXIT;
     }

	snprintf(szLogInfo, sizeof(szLogInfo) - 1, "content_length=%s", content_length);
    WRITELOG(LOG_INFO, szLogInfo);

    FCGX_GetStr(strRecv, atoi(content_length), req->in);
    if(0 == strcmp(strRecv, ""))
    {
         snprintf(szLogInfo, sizeof(szLogInfo) - 1, "recv content is empty, self[%lu]", self);
         WRITELOG(LOG_ERROR, szLogInfo);
         sprintf(rspMsg, "%s\r\n%s", RSP_HEADER, "recv content is empty");
		 iRet = -1;	
		 goto EXIT;
     }

     for(i = 0; i < sizeof(g_atServProc)/sizeof(g_atServProc[0]); i++)
     {
         if(0 == strcasecmp(url, g_atServProc[i].url))
         {
         		msg_serial = get_msg_serial();
				//传入消息序列号，处理函数工作: 解析请求、封装数据、发送
				snprintf(szLogInfo, sizeof(szLogInfo) - 1, "strRecv =%s", strRecv);
                WRITELOG(LOG_INFO, szLogInfo);
                iRet = g_atServProc[i].pfServProc(msg_serial, strRecv, strSend, sizeof(strSend) - 1);

                if(0 != iRet)
                {
                    snprintf(szLogInfo, 
                        sizeof(szLogInfo) - 1,
                        "g_atServProc[%d].pfServProc failed, url=%s, self[%lu]",
                        i,
                        url,
                        self
                        );
                    WRITELOG(LOG_ERROR, szLogInfo);
					sprintf(rspMsg, "%s\r\n%s", RSP_HEADER, strSend);
					goto EXIT;
                }
				//需要发送到集中器的指令才需要插入到hash表中
				// TODO:  当新增发往集中器的指令时，需添加到判断条件中
				if ( 0 == strcasecmp(url, PARKLOCK_CONTROL_URL) || 0 == strcasecmp(url, PARKLOCK_REALTIMEQUERY_URL)
						|| /*0 == strcasecmp(url, MUSIC_GET_PLAYLIST_URL) ||*/ 0 == strcasecmp(url, MUSIC_PLAYSTART_MUSIC_URL)
						|| 0 == strcasecmp(url, MUSIC_PLAYCTRL_MUSIC_URL) || 0 == strcasecmp(url, MUSIC_SETVOLUME_MUSIC_URL)
						|| 0 == strcasecmp(url, ENVIR_GET_PARAM_URL) || 0 == strcasecmp(url, ENVIR_GET_ALL_PARAM_URL)
						|| 0 == strcasecmp(url, MUSIC_GETVOLUME_MUSIC_URL) || 0 == strcasecmp(url, PARKLOCK_QUERY_STATUS)
						|| 0 == strcasecmp(url, PARKLOCK_CONTROL)
						)	
				{
					snprintf(szLogInfo, sizeof(szLogInfo) - 1, "*******add hash: msg_serial[%d]******", msg_serial);
               		WRITELOG(LOG_INFO, szLogInfo);
					add_user(msg_serial, req);
					return 0;
				}
				else
				{
					sprintf(rspMsg, "%s\r\n%s", RSP_HEADER, strSend);
					FCGX_FPrintF(req->out, "%s" , rspMsg);
					FCGX_Finish_r(req);
		 			free(req);
		 			req = NULL;
					snprintf(szLogInfo, sizeof(szLogInfo) - 1, "rspMsg[%s]", strSend);
           			WRITELOG(LOG_INFO, szLogInfo);
					return 0;
				}
            }
        }
        if(i == sizeof(g_atServProc)/sizeof(g_atServProc[0]))
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "unknown url, self[%lu]", self);
            WRITELOG(LOG_ERROR, szLogInfo);
            sprintf(rspMsg, "%s\r\n%s", RSP_HEADER, "unknown url");
			iRet = -1;
			goto EXIT;
        }
EXIT:
	if (iRet != 0)
	{
		 FCGX_FPrintF(req->out, "%s" , rspMsg);
		 FCGX_Finish_r(req);
		 free(req);
		 req = NULL;
	}
		
	return iRet;
}



/******************函数定义******************/
static void *down_service_entry(void *arg)
{
    int           iRet             = 0;
    int           i                = 0;
    int           thread_index     = (long)arg;
    pid_t         pid              = getpid();
    FCGX_Request  *request         = NULL;
    char         *url              = NULL;
    char          strRecv[256]     = { 0 };
    char          strSend[1024]    = { 0 };
	char 		  rspMsg[1024] 	   = { 0 };
    char          *content_length  = NULL;
    char          szLogInfo[1024]  = { 0 };

    
    for (;;)
    {	

		request = (FCGX_Request *)malloc(sizeof(FCGX_Request));
		FCGX_InitRequest(request, 0, 0);
		
        /* Some platforms require accept() serialization, some don't.. */
        pthread_mutex_lock(&g_accept_mutex);
        iRet = FCGX_Accept_r(request);
        pthread_mutex_unlock(&g_accept_mutex);
		memset(rspMsg, 0, sizeof(rspMsg));
        if (iRet < 0)
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "FCGX_Accept_r failed! pthread_index=%d, pid=%d, iRet=%d, listen_sock=%d.", thread_index, pid, iRet, request->listen_sock);
            WRITELOG(LOG_ERROR, szLogInfo);

            Sleep(10);   // 休眠10ms
			free(request);
			request = NULL;
            continue;
        }

		/*添加到任务队列中*/
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "^^^^^^^add_work. req[%p]^^^^^^^^", request);
        WRITELOG(LOG_INFO, szLogInfo);
		if (thpool_add_work(thpool, (void*)m_routine, (void*)request) != 0)
		{
			snprintf(szLogInfo, sizeof(szLogInfo) - 1, "threadPoll add workd failed! pthread_index=%d id[%lu]", thread_index, pthread_self());
            WRITELOG(LOG_ERROR, szLogInfo);
			sprintf(rspMsg, "%s\r\n%s", RSP_HEADER, "process request failed!");
			FCGX_FPrintF(request->out, "%s" , rspMsg);
			FCGX_Finish_r(request);
			free(request);
			request = NULL;
			sleep(2);
			continue;
		}
    }

    return NULL;
}


int main(int argc, char *argv[])
{
    int    index = 0;
    int    iRet  = 0;
    char   szLogInfo[1024] = { 0 };
	
   // time_t  old_time = 0;
    //time_t  new_time = 0;
   // const char * Get_EnvirParam_CMD = "\"collector_id\":\"12345678014\",\"nReqType\":6";

    // 初始化日志系统
    StartLog(CONFIG_INI, LOG_FILE);
    // StartLog("smartlamppost.ini", "smartlamppost.log");

    // printf("Exec StartLog!\n");
    FCGX_Init();

    //读取配置文件
    ReadConfigureInfo();

	signal(SIGPIPE, SIG_IGN);

	/*创建线程池*/
	thpool = thpool_init(MAX_THREAD_NUM);
	if (thpool == NULL)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "create threadPool failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		sleep(1);
		exit(-1);
	}
	
	/*创建pipe*/
	iRet = pipe(g_pipe);
	if (iRet == -1)
	{
		snprintf(szLogInfo, sizeof(szLogInfo) - 1, "create pipe failed! ret=%d", iRet);
        WRITELOG(LOG_ERROR, szLogInfo);
		sleep(1);
		exit(iRet);
	}
	
	if ((setnonblock(g_pipe[0])== -1) || (setnonblock(g_pipe[1]) == -1))
	{
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "set pipe nonblock error! errno=%d, errmsg[%s]", errno, strerror(errno));
        WRITELOG(LOG_ERROR, szLogInfo);
        return -8;
		sleep(1);
		exit(-1);
    }
	
	pthread_mutex_init(&g_pipe_mutex, NULL);
    pthread_rwlock_init(&net_hash_lock, NULL);
	pthread_rwlock_init(&msg_hash_lock, NULL);
	
    // 创建监听服务
    iRet = OpenServer(g_usListenPort, MAXEPOLLSIZE, g_uiRecvBufLen * 1024, g_uiSendBufLen * 1024, 0, 1);
    if(iRet != 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "OpenServer failed! ret=%d", iRet);
        WRITELOG(LOG_ERROR, szLogInfo);
        printf("%s\n", szLogInfo);
		pthread_mutex_destroy(&g_pipe_mutex);
		pthread_rwlock_destroy(&net_hash_lock);
		pthread_rwlock_destroy(&msg_hash_lock);
		close(g_pipe[0]);
		close(g_pipe[1]);
        sleep(1);
        exit(iRet);
    }

    // 创建下行业务处理线程
    //暂时使用1个接收线程
    for(index = 0; index < g_uiDownservThreadNum; index++)
    {
        iRet = pthread_create(&g_aDownservTid[index], NULL, down_service_entry, (void*)(long)index);
        if(0 != iRet)
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "create down service thread failed! index=%d, iRet=%d, quit...", index, iRet);
            WRITELOG(LOG_ERROR, szLogInfo);
            printf("%s\n", szLogInfo);
			pthread_mutex_destroy(&g_pipe_mutex);
			pthread_rwlock_destroy(&net_hash_lock);
			pthread_rwlock_destroy(&msg_hash_lock);
			close(g_pipe[0]);
			close(g_pipe[1]);
            sleep(1);
            exit(iRet);
        }
    }
    
    // 上行业务处理线程
    
    for(index = 0; index < g_uiUpservThreadNum; index++)
    {
        iRet = pthread_create(&g_aUpservTid[index], NULL, up_service_entry, (void*)(long)index);
        if(0 != iRet)
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "create up service thread failed!, iRet=%d, quit...", iRet, index);
            WRITELOG(LOG_ERROR, szLogInfo);
			pthread_mutex_destroy(&g_pipe_mutex);
			pthread_rwlock_destroy(&net_hash_lock);
			pthread_rwlock_destroy(&msg_hash_lock);
			close(g_pipe[0]);
			close(g_pipe[1]);
            sleep(1);
            exit(iRet);
        }
    }
    
    // 创建数据库连接
    initmysql();    
   
    do
    {
        //g_db_conn = ConnectMySql("localhost", "root", "cqgdxxyjy@123", "gclamppost", 0);
		// = ConnectMySql("localhost", "root", "root123", "gclamppost", 0);
		g_db_conn = ConnectMySql("localhost", g_Conf.tDBInfo.szUserName, g_Conf.tDBInfo.szPassword, "gclamppost", 0);
		//ExecuteSql(g_db_conn,"insert into friends(name, age) values(\'gui chen\', 41)");
        if(NULL == g_db_conn)
         {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Connect database failed!");
            WRITELOG(LOG_ERROR, szLogInfo);
            sleep(1);
         }
         else
         {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Connect database SUCCESS!");
            WRITELOG(LOG_INFO, szLogInfo);
         }
    }while(NULL == g_db_conn);

    
    while(1)
    {   /*
        new_time = time(NULL);
        if(new_time - old_time > g_uiEnvironmentParamCycle)
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "Get_EnvirParam_CMD = %s",Get_EnvirParam_CMD);
            WRITELOG(LOG_INFO, szLogInfo);

            serv_getANDsend_all_envirparam_to_mysql(Get_EnvirParam_CMD);
             
            old_time = new_time;
        }*/
        sleep(1);
    }

	thpool_destroy(thpool);
	delete_all();
	pthread_mutex_destroy(&g_pipe_mutex);
	pthread_rwlock_destroy(&net_hash_lock);
	pthread_rwlock_destroy(&msg_hash_lock);
	close(g_pipe[0]);
	close(g_pipe[1]);
	
	return 0;
}


// 读取配置文件内容
void ReadConfigureInfo()
{
    char szLogInfo[1500] = {0};
    memset(&g_Conf, 0x00, sizeof(Config_T));

    GetPrivateProfileString("DBInfo", "IPAddr",   "", g_Conf.tDBInfo.szIPAddr,   sizeof(g_Conf.tDBInfo.szIPAddr)-1,   CONFIG_INI); 
    GetPrivateProfileString("DBInfo", "UserName", "", g_Conf.tDBInfo.szUserName, sizeof(g_Conf.tDBInfo.szUserName)-1, CONFIG_INI); 
    GetPrivateProfileString("DBInfo", "Password", "", g_Conf.tDBInfo.szPassword, sizeof(g_Conf.tDBInfo.szPassword)-1, CONFIG_INI);
    GetPrivateProfileString("DBInfo", "DBName",   "", g_Conf.tDBInfo.szDBName,   sizeof(g_Conf.tDBInfo.szDBName)-1,   CONFIG_INI);

    g_Conf.tDBInfo.iDBPort = GetPrivateProfileInt("DBInfo", "DBPort", 3306, CONFIG_INI);

    snprintf(szLogInfo, sizeof(szLogInfo)-1,
        "IPAddr=%s,UserName=%s,Password=%s,DBName=%s,DBPort=%d",
        g_Conf.tDBInfo.szIPAddr,g_Conf.tDBInfo.szUserName,g_Conf.tDBInfo.szPassword,g_Conf.tDBInfo.szDBName,g_Conf.tDBInfo.iDBPort);
    WRITELOG(LOG_INFO, szLogInfo);
}


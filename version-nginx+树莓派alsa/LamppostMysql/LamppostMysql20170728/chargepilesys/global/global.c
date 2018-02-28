#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "global.h"
#include "log.h"

//int g_recv_msqid = -1;
//int g_send_msqid = -1;
int g_serv_msqid = -1;
//unsigned int g_uiEnvironmentParamCycle =10; //10 seconds


void dumpInfo(unsigned char *info, int length)
{
	char log_str_buf[1048] = {0};
	int index = 0;
	int j = 0;
	char buffer[1024] = {0};
	memset(buffer, 0, 1024);
	for (index = 0; index < length; index++, j+=2)
	{
		sprintf(&buffer[j],"%02x",info[index]);
	}
	snprintf(log_str_buf, sizeof(log_str_buf)-1, "dump info length = %d, %s\n", length, buffer);
	WRITELOG(LOG_INFO, log_str_buf);
}


// 休眠函数(以ms为单位)
int Sleep(int count)
{
    int             secs=0, msecs=0;
    struct timeval  timeout,timeo;

    if (count < 0)
    {
        return -1;
    }
	
    if (count < 1000)
    {
        timeout.tv_sec = 0;
        timeout.tv_usec = count * 1000;
    }
    else
    {
        timeout.tv_sec = count / 1000;  
        timeout.tv_usec = (count % 1000) * 1000;
    }
    select(0, NULL, NULL, NULL, &timeout);
    //nanosleep(&timeout,&timeo);
    return 0;
}




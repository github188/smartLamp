#include <stdio.h>
#include <stdlib.h>
#include <netinf.h>

extern char errorMessage[1024];
static int  g_nCount = 0;
int main(int argc, char *argv[])
{
    int ret = 0;
    char * acSvrIP = NULL;
    unsigned short usSvrPort = 0;
    int link_num = 0;
    
    if(argc < 4)
    {
        snprintf(errorMessage, sizeof(errorMessage) - 1, "input parameter num is less than 3!");
        printf("%s\n", errorMessage);
        return -1;
    }

    acSvrIP = argv[1];
    usSvrPort = atoi(argv[2]);
    link_num = atoi(argv[3]);
    
    while(1)
    {
        ret = ConnectSCPServer(acSvrIP, usSvrPort, 0);
        if(ret < 0)
        {
            snprintf(errorMessage, sizeof(errorMessage) - 1, "connect server failed!");
            printf("%s\n", errorMessage);
            return -1;
        }

        snprintf(errorMessage, sizeof(errorMessage) - 1, "connect server success.[%d]", ++g_nCount);
        printf("%s\n", errorMessage);

        sleep(0);

        if(g_nCount > link_num)
        {
            snprintf(errorMessage, sizeof(errorMessage) - 1, "reach input link-nmu[%d], quit...", link_num);
            printf("%s\n", errorMessage);
            return 0;
        }
    }
    
    return 0;
}



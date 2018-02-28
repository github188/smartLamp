#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "netinf.h"
#include "servdata.h"

int parse_diconfirmdeny(char *data, int len, t_di_confirm_deny_list *di_confirm_deny_list)
{
    int i = 0;
    int num = 0;

    if (data == NULL || di_confirm_deny_list == NULL)
    {
        return -1;
    }
	
    num = len / sizeof(t_di_confirm_deny);
    for(i = 0; i < num; i++)
    {
        memcpy(di_confirm_deny_list->confirm_deny[i].di, data + i * sizeof(t_di_confirm_deny), 4);
        memcpy(di_confirm_deny_list->confirm_deny[i].dev_no, data + i * sizeof(t_di_confirm_deny) + 4, 2);
        memcpy(&di_confirm_deny_list->confirm_deny[i].err, data + i * sizeof(t_di_confirm_deny) + 6, 2);
    }
    di_confirm_deny_list->num = num;

    return 0;
}


int parse_playlist(char *data, int len, t_play_list *play_list)
{
    int offset = 0;
	int iLen = 0;
	if (data == NULL || play_list == NULL)
    {
        return -1;
    }

	
    memset(play_list, 0, sizeof(t_play_list));
    
    memcpy(&play_list->err, data, sizeof(char));

    offset += sizeof(char);
    //memcpy(&play_list->len, data + offset, sizeof(unsigned short));

	
    //offset += sizeof(unsigned short);
	memcpy(&play_list->len, data + offset, sizeof(play_list->len));
	offset += sizeof(play_list->len);
	iLen = atoi(play_list->len);
	
    //memcpy(play_list->filelist, data + offset, min(play_list->len, MAX_RETMSG_LEN));
    memcpy(play_list->filelist, data + offset, iLen );
	
    return 0;
}


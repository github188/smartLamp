#ifndef _NETINFO_H_
#define _NETINFO_H_

#ifdef _WIN32
  #pragma pack(push,1)
#else
  #pragma pack(1)
#endif

#include "../hash/uthash.h"

#define MAX_ID_LEN             23

typedef struct
{
    int    fd;
    char   collecor_id[MAX_ID_LEN + 1];
    int    (*recv_proc)(int fd);
    int    (*send_proc)(int fd);
}t_net_info;

typedef struct
{
    char   id[MAX_ID_LEN + 1];  /* key */
    t_net_info netinfo;
    UT_hash_handle hh;          /* makes this structure hashable */
}t_net_node;

void add_net(char *collector_id, t_net_info *netinfo);
t_net_node *find_net(char *collector_id);
void delete_net(t_net_node *net_node);
void delete_net_all();
void set_net_info(char *collector_id, t_net_info *netinfo);
void delete_net_fd(int fd);


#ifdef _WIN32
  #pragma pack(pop)
#else
  #pragma pack()
#endif

#endif /*end _NETINFO_H_*/



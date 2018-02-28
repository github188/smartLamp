#ifndef _NETINF_H_
#define _NETINF_H_

#ifdef _WIN32
  #pragma pack(push,1)
#else
  #pragma pack(1)
#endif

#define MAXEPOLLSIZE    10000
#define MAXLINE         4*1024

#define SOCK_RECV_MSGKEY       1024
#define SOCK_SEND_MSGKEY       1025

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

typedef struct
{
    long   mtype;          /* Message type. */
    char   mtext[MAXLINE]; /* Message text. */
}t_mymsg;

typedef struct
{
    int fd;
    int len;
    char buf[MAXLINE];
}t_buf;

extern int g_recv_msqid;
extern int g_send_msqid;

//add by tianyu
extern int g_pipe[2];
extern pthread_mutex_t g_pipe_mutex;
extern pthread_rwlock_t net_hash_lock;

int setnonblock(int sockfd);
int OpenServer(int port, int total, int sendbuflen, int recvbuflen, int blockORnot, int reuseORnot);
int ConnectServer(char * serverip, int serverport, int blockORnot);
int Send(int sock, char * buf, size_t size, int flag, int timeout);
int Recv(int sock, char * buf, size_t size, int flag, int timeout);
void *recv_entry(void *arg);
void *send_entry(void *arg);
int recvproc(int fd);
int sendproc();
//add by tianyu
int msg_recv_nob(int msg_id, t_mymsg *msg, int size, long type);
int read_safe(int fd, char *buf, int size);
int write_safe(int fd, char *buf, int size);
void * io_proc(void * arg);

#ifdef _WIN32
  #pragma pack(pop)
#else
  #pragma pack()
#endif

#endif



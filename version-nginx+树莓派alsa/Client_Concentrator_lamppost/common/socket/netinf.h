#ifndef _NETINF_H_
#define _NETINF_H_

#ifdef _WIN32
  #pragma pack(push,1)
#else
  #pragma pack(1)
#endif


int ConnectServer(char * serverip, int serverport, int nonblock);
int Send(int sock, char * buf, size_t size, int flag, int timeout);
int Recv(int sock, char * buf, size_t size, int flag, int timeout);


#ifdef _WIN32
  #pragma pack(pop)
#else
  #pragma pack()
#endif

#endif



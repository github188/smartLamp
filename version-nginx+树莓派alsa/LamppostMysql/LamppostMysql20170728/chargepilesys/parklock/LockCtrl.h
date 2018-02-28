#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h>      //文件控制定义 
#include <termios.h>    //终端控制定义 
#include <errno.h> 

#define DEVICE "/dev/ttySAC2" 


int init_serial(void);  // 打开串口并初始化设置
int uart_send(int fd, char *data, int datalen);   // 串口发送数据
int uart_recv(int fd, char *data, int datalen);    // 串口接收数据
int OpenLock(void);      // 开车位锁
int CloseLock(void);     // 关车位锁
int GetLockStatus(void); // 获取车位锁 状态


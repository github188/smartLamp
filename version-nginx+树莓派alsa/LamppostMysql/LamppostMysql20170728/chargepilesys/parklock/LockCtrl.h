#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h>      //�ļ����ƶ��� 
#include <termios.h>    //�ն˿��ƶ��� 
#include <errno.h> 

#define DEVICE "/dev/ttySAC2" 


int init_serial(void);  // �򿪴��ڲ���ʼ������
int uart_send(int fd, char *data, int datalen);   // ���ڷ�������
int uart_recv(int fd, char *data, int datalen);    // ���ڽ�������
int OpenLock(void);      // ����λ��
int CloseLock(void);     // �س�λ��
int GetLockStatus(void); // ��ȡ��λ�� ״̬


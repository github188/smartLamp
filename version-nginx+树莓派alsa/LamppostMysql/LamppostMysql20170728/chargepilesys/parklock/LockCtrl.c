#include "LockCtrl.h"
#include "log.h"


int serial_fd = 0; 

// �򿪴��ڲ���ʼ������
int init_serial(void) 
{ 
    serial_fd = open(DEVICE, O_RDWR | O_NOCTTY | O_NDELAY); 
    if (serial_fd < 0) 
	{ 
        perror("open"); 
        return -1; 
    } 
    //������Ҫ���ýṹ��termios <termios.h> 
    struct termios options; 
   
    /**1. tcgetattr�������ڻ�ȡ���ն���صĲ����� 
      *    ����fdΪ�ն˵��ļ������������صĽ��������termios�ṹ���� 
      */ 
    tcgetattr(serial_fd, &options); 
 	
    /*2. �޸�����õĲ���*/ 
    options.c_cflag |= (CLOCAL | CREAD);    // ���ÿ���ģʽ״̬���������ӣ�����ʹ�� 
    options.c_cflag &= ~CSIZE;              // �ַ����ȣ���������λ֮ǰһ��Ҫ�������λ 
    options.c_cflag &= ~CRTSCTS;            // ��Ӳ������ 
    options.c_cflag |= CS8;                 // 8λ���ݳ��� 
    options.c_cflag &= ~CSTOPB;             // 1λֹͣλ
    options.c_iflag |= IGNPAR;              // ����ż����λ 
    options.c_oflag = 0;                    // ���ģʽ 
    options.c_lflag = 0;                    // �������ն�ģʽ 
    cfsetospeed(&options, B9600);           // ���ò����� 
    
    /**3. ���������ԣ�TCSANOW�����иı�������Ч*/ 
    tcflush(serial_fd, TCIFLUSH);           // ������ݿ��Խ��գ������� 
    tcsetattr(serial_fd, TCSANOW, &options); 
 
    return 0; 
} 


/** 
*���ڷ������� 
*@fd:���������� 
*@data:���������� 
*@datalen:���ݳ��� 
*/ 
int uart_send(int fd, char *data, int datalen) 
{ 
    int len = 0;
    char szLogInfo[500] = {0};
	
    len = write(fd, data, datalen);    // ʵ��д��ĳ��� 
    
    snprintf(szLogInfo, sizeof(szLogInfo)-1, "uart_send:len=%d,datalen=%d", len, datalen);
    WRITELOG(LOG_INFO, szLogInfo);
    
    if (len == datalen)      // ���ͳɹ�
    { 
        return len; 
    } 
    else 
    { 
        tcflush(fd, TCOFLUSH);   // TCOFLUSHˢ��д������ݵ������� 
        return -1; 
    } 
    
    // return 0; 
} 


/** 
*���ڽ������� 
*Ҫ����������pc�˷���ascii�ļ� 
*/ 
int uart_recv(int fd, char *data, int datalen) 
{ 
    int len = 0, ret = 0; 
	char szLogInfo[500] = {0};
    fd_set fs_read; 
    struct timeval tv_timeout; 
   
    FD_ZERO(&fs_read); 
    FD_SET(fd, &fs_read); 
    tv_timeout.tv_sec = (10*20/115200+2); 
    tv_timeout.tv_usec = 0; 
   
    ret = select(fd+1, &fs_read, NULL, NULL, &tv_timeout); 
    //�������0��������������״̬�ı�ǰ�ѳ���timeoutʱ��,���󷵻�-1 
    snprintf(szLogInfo, sizeof(szLogInfo)-1, "uart_recv:ret=%d", ret);
    WRITELOG(LOG_INFO, szLogInfo);
   
    if (FD_ISSET(fd, &fs_read)) 
	{ 
        len = read(fd, data, datalen); 
        // printf("len = %d\n", len); 
        return len; 
    } 
	else 
	{ 
        perror("select"); 
        return -1; 
    } 
} 

/** 
*����λ�� 
*/ 
int OpenLock(void) 
{ 
    int  iDataLen       = 0;
	int  iRetVal        = 0;
	char szRcvBuf[20]   = {0};
	char szLogInfo[500] = {0};
	
	unsigned char szOpen[6] = {0x55,0x01,0x01,0x01,0x9A,0xAA};     // 55 01 01 01 9A AA
    
    iRetVal = init_serial(); 
	if (iRetVal != 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "OpenLock: exec init_serial failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
    }
    
    iDataLen = uart_send(serial_fd, szOpen, sizeof(szOpen));
	if (iDataLen == -1)
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "OpenLock: exec uart_send failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
    }
   
    iDataLen = uart_recv(serial_fd, szRcvBuf, sizeof(szRcvBuf));
	if (iDataLen == -1)
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "OpenLock: exec uart_recv failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
    }
   
    snprintf(szLogInfo, sizeof(szLogInfo)-1, "OpenLock: received len=%d!", iDataLen);
    WRITELOG(LOG_INFO, szLogInfo);
	
    if (szRcvBuf[0] == 0x5A)
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "OpenLock: open lock success!");
        WRITELOG(LOG_INFO, szLogInfo);
    }
    else if (szRcvBuf[0] == 0x5B)
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "OpenLock: open lock failed!");
        WRITELOG(LOG_INFO, szLogInfo);
    }
	else
	{
	    snprintf(szLogInfo, sizeof(szLogInfo)-1, "OpenLock: received data is wrong!");
        WRITELOG(LOG_ERROR, szLogInfo);
	}

	close(serial_fd); 

	return 0;
}


/** 
*�س�λ�� 
*/ 
int CloseLock(void) 
{ 
    int  iDataLen       = 0;
	int  iRetVal        = 0;
	char szRcvBuf[20]   = {0};
	char szLogInfo[500] = {0};
	
	unsigned char szClose[6] = {0x55,0x01,0x01,0x02,0x78,0xAA};    // 55 01 01 02 78 AA 
    
    iRetVal = init_serial(); 
	if (iRetVal != 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "CloseLock: exec init_serial failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
    }
    
    iDataLen = uart_send(serial_fd, szClose, sizeof(szClose));
	if (iDataLen == -1)
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "CloseLock: exec uart_send failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
    }
   
    iDataLen = uart_recv(serial_fd, szRcvBuf, sizeof(szRcvBuf));
	if (iDataLen == -1)
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "CloseLock: exec uart_recv failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
    }
   
    snprintf(szLogInfo, sizeof(szLogInfo)-1, "CloseLock: received len=%d!", iDataLen);
    WRITELOG(LOG_INFO, szLogInfo);
	
    if (szRcvBuf[0] == 0x5A)
    {
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "CloseLock: close lock success!");
        WRITELOG(LOG_INFO, szLogInfo);
    }
    else if (szRcvBuf[0] == 0x5B)
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "CloseLock: close lock failed!");
        WRITELOG(LOG_INFO, szLogInfo);
    }
	else
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "CloseLock: received data is wrong!");
        WRITELOG(LOG_ERROR, szLogInfo);
	}

	close(serial_fd); 

	return 0;
}


/** 
*��ȡ��λ�� ״̬
*/ 
int GetLockStatus(void) 
{ 
    int  iDataLen       = 0;
	int  iRetVal        = 0;
	char szRcvBuf[20]   = {0};
	char szLogInfo[500] = {0};
	
	unsigned char szStatus[6] = {0x55,0x01,0x01,0x06,0x19,0xAA};   // 55 01 01 06 19 AA 
    
    iRetVal = init_serial(); 
	if (iRetVal != 0)
    {
        snprintf(szLogInfo, sizeof(szLogInfo)-1, "GetLockStatus: exec init_serial failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
    }
    
    iDataLen = uart_send(serial_fd, szStatus, sizeof(szStatus));
	if (iDataLen == -1)
    {
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "GetLockStatus: exec uart_send failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
    }
   
    iDataLen = uart_recv(serial_fd, szRcvBuf, sizeof(szRcvBuf));
	if (iDataLen == -1)
    {
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "GetLockStatus: exec uart_recv failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
		return -1;
    }
   
	snprintf(szLogInfo, sizeof(szLogInfo)-1, "GetLockStatus: received len=%d!", iDataLen);
    WRITELOG(LOG_INFO, szLogInfo);
	
    if (szRcvBuf[0] == 0x5A)
    {
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "GetLockStatus: get lock status success!");
        WRITELOG(LOG_INFO, szLogInfo);
        
        if (szRcvBuf[4] == 0x00)
        {
            snprintf(szLogInfo, sizeof(szLogInfo)-1, "GetLockStatus: Locked!");
            WRITELOG(LOG_INFO, szLogInfo);
        }
        else if (szRcvBuf[4] == 0x01)
        {
			snprintf(szLogInfo, sizeof(szLogInfo)-1, "GetLockStatus: Opened!");
            WRITELOG(LOG_INFO, szLogInfo);
        }
		else if (szRcvBuf[4] == 0x02)
        {
			snprintf(szLogInfo, sizeof(szLogInfo)-1, "GetLockStatus: Middle status(0~90)!");
            WRITELOG(LOG_INFO, szLogInfo);
        }
		else if (szRcvBuf[4] == 0x03)
        {
			snprintf(szLogInfo, sizeof(szLogInfo)-1, "GetLockStatus: Middle status(90~180)!");
            WRITELOG(LOG_INFO, szLogInfo);
        }
		else if (szRcvBuf[4] == 0x88)
        {
			snprintf(szLogInfo, sizeof(szLogInfo)-1, "GetLockStatus: Running status!");
            WRITELOG(LOG_INFO, szLogInfo);
        }
		else if (szRcvBuf[4] == 0x10)
        {
			snprintf(szLogInfo, sizeof(szLogInfo)-1, "GetLockStatus: Opened and no car on it!");
            WRITELOG(LOG_INFO, szLogInfo);
        }
    }
    else if (szRcvBuf[0] == 0x5B)
    {
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "GetLockStatus: Get lock status failed!");
        WRITELOG(LOG_ERROR, szLogInfo);
    }
	else
	{
		snprintf(szLogInfo, sizeof(szLogInfo)-1, "GetLockStatus: received data is wrong!");
        WRITELOG(LOG_ERROR, szLogInfo);
	}

	close(serial_fd); 

	return 0;
}


// ������
/*
int main(int argc, char **argv) 
{ 
    int iRetVal = 0;
	
    // ����
    iRetVal = OpenLock();
	if (iRetVal != 0)
	{
	    printf("Open lock failed!\n");
		return -1;
	}
 
    // ����
    sleep(10);
	iRetVal = CloseLock();
	if (iRetVal != 0)
	{
	    printf("Close lock failed!\n");
		return -1;
	}

	// ����
	sleep(10);
    iRetVal = OpenLock();
	if (iRetVal != 0)
	{
	    printf("Open lock failed!\n");
		return -1;
	}

	// ��ȡ��λ�� ״̬
    sleep(10);
	iRetVal = GetLockStatus();
	if (iRetVal != 0)
	{
	    printf("Get lock status failed!\n");
		return -1;
	}
 
    return 0; 
}
*/



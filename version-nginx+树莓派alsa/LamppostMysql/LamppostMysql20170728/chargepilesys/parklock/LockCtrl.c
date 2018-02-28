#include "LockCtrl.h"
#include "log.h"


int serial_fd = 0; 

// 打开串口并初始化设置
int init_serial(void) 
{ 
    serial_fd = open(DEVICE, O_RDWR | O_NOCTTY | O_NDELAY); 
    if (serial_fd < 0) 
	{ 
        perror("open"); 
        return -1; 
    } 
    //串口主要设置结构体termios <termios.h> 
    struct termios options; 
   
    /**1. tcgetattr函数用于获取与终端相关的参数。 
      *    参数fd为终端的文件描述符，返回的结果保存在termios结构体中 
      */ 
    tcgetattr(serial_fd, &options); 
 	
    /*2. 修改所获得的参数*/ 
    options.c_cflag |= (CLOCAL | CREAD);    // 设置控制模式状态，本地连接，接收使能 
    options.c_cflag &= ~CSIZE;              // 字符长度，设置数据位之前一定要屏掉这个位 
    options.c_cflag &= ~CRTSCTS;            // 无硬件流控 
    options.c_cflag |= CS8;                 // 8位数据长度 
    options.c_cflag &= ~CSTOPB;             // 1位停止位
    options.c_iflag |= IGNPAR;              // 无奇偶检验位 
    options.c_oflag = 0;                    // 输出模式 
    options.c_lflag = 0;                    // 不激活终端模式 
    cfsetospeed(&options, B9600);           // 设置波特率 
    
    /**3. 设置新属性，TCSANOW：所有改变立即生效*/ 
    tcflush(serial_fd, TCIFLUSH);           // 溢出数据可以接收，但不读 
    tcsetattr(serial_fd, TCSANOW, &options); 
 
    return 0; 
} 


/** 
*串口发送数据 
*@fd:串口描述符 
*@data:待发送数据 
*@datalen:数据长度 
*/ 
int uart_send(int fd, char *data, int datalen) 
{ 
    int len = 0;
    char szLogInfo[500] = {0};
	
    len = write(fd, data, datalen);    // 实际写入的长度 
    
    snprintf(szLogInfo, sizeof(szLogInfo)-1, "uart_send:len=%d,datalen=%d", len, datalen);
    WRITELOG(LOG_INFO, szLogInfo);
    
    if (len == datalen)      // 发送成功
    { 
        return len; 
    } 
    else 
    { 
        tcflush(fd, TCOFLUSH);   // TCOFLUSH刷新写入的数据但不传送 
        return -1; 
    } 
    
    // return 0; 
} 


/** 
*串口接收数据 
*要求启动后，在pc端发送ascii文件 
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
    //如果返回0，代表在描述符状态改变前已超过timeout时间,错误返回-1 
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
*开车位锁 
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
*关车位锁 
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
*获取车位锁 状态
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


// 主函数
/*
int main(int argc, char **argv) 
{ 
    int iRetVal = 0;
	
    // 开锁
    iRetVal = OpenLock();
	if (iRetVal != 0)
	{
	    printf("Open lock failed!\n");
		return -1;
	}
 
    // 闭锁
    sleep(10);
	iRetVal = CloseLock();
	if (iRetVal != 0)
	{
	    printf("Close lock failed!\n");
		return -1;
	}

	// 开锁
	sleep(10);
    iRetVal = OpenLock();
	if (iRetVal != 0)
	{
	    printf("Open lock failed!\n");
		return -1;
	}

	// 获取车位锁 状态
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



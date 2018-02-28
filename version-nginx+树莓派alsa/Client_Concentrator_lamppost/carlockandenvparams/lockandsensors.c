#include <unistd.h>

#include "global.h"
#include "lockandsensors.h"

//extern int g_iRetVal;
// 打开串口并初始化设置
int init_serial(void) 
{ 
    
   if (g_serial_fd >0) 
   	{close(g_serial_fd);
     g_serial_fd=-1;
	 sleep(2);
   }
   
    g_serial_fd = open(DEVICE, O_RDWR | O_NOCTTY | O_NDELAY); 
    if (g_serial_fd < 0) 
	{ 
        perror("open"); 
        return -1; 
    } 
    //串口主要设置结构体termios <termios.h> 
    struct termios options; 
   
    /**1. tcgetattr函数用于获取与终端相关的参数。 
      *    参数fd为终端的文件描述符，返回的结果保存在termios结构体中 
      */ 
    tcgetattr(g_serial_fd, &options); 
 	
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
    tcflush(g_serial_fd, TCIOFLUSH);           // 溢出数据可以接收，但不读 
    tcsetattr(g_serial_fd, TCSANOW, &options); 
 
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
    len = write(fd, data, datalen);    // 实际写入的长度 
    
    printf("uart_send:len=%d,datalen=%d\n", len, datalen); 
    
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
    fd_set fs_read; 
    struct timeval tv_timeout; 
   
    FD_ZERO(&fs_read); 
    FD_SET(fd, &fs_read); 
    tv_timeout.tv_sec = (10*20/115200+2); 
    tv_timeout.tv_usec = 0; 
   
    ret = select(fd+1, &fs_read, NULL, NULL, &tv_timeout); 
    printf("ret = %d\n", ret);   //如果返回0，代表在描述符状态改变前已超过timeout时间,错误返回-1 
    printf("fd = %d\n", fd); 
    if (FD_ISSET(fd, &fs_read)) 
	{ 
        len = read(fd, data, datalen); 
		
        printf("len = %d\n", len); 
		//close(fd);
		//sleep(2);
        return len; 
    } 
	else 
	{ 
        perror("select"); 
		//close(fd);
		//sleep(2);
        return -1; 
    } 
   
    // return 0; 
} 

/** 
*开车位锁 
*/ 
int OpenLock(void) 
{ 
    int  iDataLen      = 0;
	//int  g_iRetVal       = 0;
	char szRcvBuf[20]  = {0};
	int iRet = 0;
	unsigned char szOpen[6] = {0x55,0x01,0x01,0x01,0x9A,0xAA};     // 55 01 01 01 9A AA
    
    g_iRetVal = init_serial(); 
	if (g_iRetVal != 0)
    {
        printf("OpenLock: exec init_serial failed!\n");
		return -1;
    }
    
    iDataLen = uart_send(g_serial_fd, szOpen, sizeof(szOpen));
	if (iDataLen == -1)
    {
        printf("OpenLock: exec uart_send failed!\n");
		close(g_serial_fd); 
		g_serial_fd = -1;
		return -1;
    }
   
    iDataLen = uart_recv(g_serial_fd, szRcvBuf, sizeof(szRcvBuf));
	if (iDataLen == -1)
    {
        printf("OpenLock: exec uart_recv failed!\n");
		close(g_serial_fd); 
		g_serial_fd = -1;
		return -1;
    }
   
    printf("OpenLock: received len=%d\n", iDataLen); 
	
    if (szRcvBuf[0] == 0x5A)
    {
        printf("OpenLock: open lock success!\n");
    }
    else if (szRcvBuf[0] == 0x5B)
    {
        printf("OpenLock: open lock failed!\n"); 
		iRet = -1;
    }
	else
	{
	    printf("OpenLock: received data is wrong!\n"); 
		iRet = -1;
	}

	close(g_serial_fd); 
	g_serial_fd = -1;
	return iRet;
}


/** 
*关车位锁 
*/ 
int CloseLock(void) 
{ 
    int  iDataLen      = 0;
//	int  iRetVal       = 0;
	char szRcvBuf[20]  = {0};
	int iRet = 0;
	unsigned char szClose[6] = {0x55,0x01,0x01,0x02,0x78,0xAA};    // 55 01 01 02 78 AA 
    
    g_iRetVal = init_serial(); 
	if (g_iRetVal != 0)
    {
        printf("CloseLock: exec init_serial failed!\n");
		return -1;
    }
    
    iDataLen = uart_send(g_serial_fd, szClose, sizeof(szClose));
	if (iDataLen == -1)
    {
        printf("CloseLock: exec uart_send failed!\n");
		close(g_serial_fd); 
		g_serial_fd = -1;
		return -1;
    }
   
    iDataLen = uart_recv(g_serial_fd, szRcvBuf, sizeof(szRcvBuf));
	if (iDataLen == -1)
    {
        printf("CloseLock: exec uart_recv failed!\n");
		close(g_serial_fd); 
		g_serial_fd = -1;
		return -1;
    }
   
    printf("CloseLock: received len=%d\n", iDataLen); 
	
    if (szRcvBuf[0] == 0x5A)
    {
        printf("CloseLock: Close lock success!\n");
    }
    else if (szRcvBuf[0] == 0x5B)
    {
        printf("CloseLock: Close lock failed!\n"); 
		iRet = -1;
    }
	else
	{
	    printf("CloseLock: received data is wrong!\n"); 
		iRet = -1;
	}

	close(g_serial_fd); 
	g_serial_fd = -1;
	return iRet;
}


/** 
*获取车位锁 状态
*/ 
int GetLockStatus(char *parklock_id, int *status) 
{ 
    int  iDataLen      = 0;
//	int  iRetVal       = 0;
	char szRcvBuf[20]  = {0};
	
	unsigned char szStatus[6] = {0x55,0x01,0x01,0x06,0x19,0xAA};   // 55 01 01 06 19 AA 
    
    g_iRetVal = init_serial(); 
	if (g_iRetVal != 0)
    {
        printf("GetLockStatus: exec init_serial failed!\n");
		*status = -1;
		return -1;
    }
    
    iDataLen = uart_send(g_serial_fd, szStatus, sizeof(szStatus));
	if (iDataLen == -1)
    {
        printf("GetLockStatus: exec uart_send failed!\n");
		close(g_serial_fd); 
		g_serial_fd = -1;
		*status = -1;
		return -1;
    }
   
    iDataLen = uart_recv(g_serial_fd, szRcvBuf, sizeof(szRcvBuf));
	if (iDataLen == -1)
    {
        printf("GetLockStatus: exec uart_recv failed!\n");
		close(g_serial_fd); 
		g_serial_fd = -1;
		*status = -1;
		return -1;
    }
   
    printf("GetLockStatus: received len=%d\n", iDataLen); 
	
    if (szRcvBuf[0] == 0x5A)
    {
        printf("GetLockStatus: get lock status success!\n");
        
        if (szRcvBuf[4] == 0x00)
        {
            printf("Locked!\n");
			*status = 1; //闭锁
        }
        else if (szRcvBuf[4] == 0x01)
        {
            printf("Opened!\n");
			*status = 0; //开锁
        }
		else if (szRcvBuf[4] == 0x02)
        {
            printf("Middle status(0~90)!\n");
			*status = 1; //闭锁
        }
		else if (szRcvBuf[4] == 0x03)
        {
            printf("Middle status(90~180)!\n");
			*status = 0; //开锁
        }
		else if (szRcvBuf[4] == 0x88)
        {
            printf("Running status!\n");
			*status = 2; //其他状态
        }
		else if (szRcvBuf[4] == 0x10)
        {
            printf("Opened and no car on it!\n");
			*status = 0; //开锁
        }
    }
    else if (szRcvBuf[0] == 0x5B)
    {
        printf("GetLockStatus: Get lock status failed!\n");
		*status = 3;
    }
	else
	{
	    printf("GetLockStatus: GetLockStatus: received data is wrong!\n"); 
		*status = 3;
	}

	close(g_serial_fd); 
	g_serial_fd = -1;
	return 0;
}

/** 
*关蜂鸣器 
*/ 
int CloseBuzzer(void) 
{ 
    int  iDataLen      = 0;
	char szRcvBuf[20]  = {0};
	
	unsigned char szClose[7] = {0x55,0x01,0x02,0x15,0x00,0x5C,0xAA};    // 55 01 02 15 00 5C AA
    
    g_iRetVal = init_serial(); 
	if (g_iRetVal != 0)
    {
        printf("CloseBuzzer: exec init_serial failed!\n");
		return -1;
    }
    
    iDataLen = uart_send(g_serial_fd, szClose, sizeof(szClose));
	if (iDataLen == -1)
    {
        printf("CloseBuzzer: exec uart_send failed!\n");
		close(g_serial_fd); 
		g_serial_fd = -1;
		return -1;
    }
   
    iDataLen = uart_recv(g_serial_fd, szRcvBuf, sizeof(szRcvBuf));
	if (iDataLen == -1)
    {
        printf("CloseBuzzer: exec uart_recv failed!\n");
		close(g_serial_fd); 
		g_serial_fd = -1;
		return -1;
    }
   
    printf("CloseBuzzer: received len=%d\n", iDataLen); 
	
    if (szRcvBuf[0] == 0x5A)
    {
        printf("CloseBuzzer: Close buzzer success!\n");
    }
    else if (szRcvBuf[0] == 0x5B)
    {
        printf("CloseBuzzer: Close buzzer failed!\n"); 
    }
	else
	{
	    printf("CloseBuzzer: received data is wrong!\n"); 
	}
	printf("******close g_serial_fd[%d]*****\n", g_serial_fd);
	close(g_serial_fd); 
	g_serial_fd = -1;
	return 0;
}

int OpenBuzzer(void)
{
        int  iDataLen      = 0;
        char szRcvBuf[20]  = {0};
        
        unsigned char szClose[7] = {0x55,0x01,0x02,0x15,0x01,0x02,0xAA};    // 55,01,02,15,01,02,AA 
        
        g_iRetVal = init_serial(); 
        if (g_iRetVal != 0)
        {
            printf("OpenBuzzer: exec init_serial failed!\n");
            return -1;
        }
        
        iDataLen = uart_send(g_serial_fd, szClose, sizeof(szClose));
        if (iDataLen == -1)
        {
            printf("OpenBuzzer: exec uart_send failed!\n");
			close(g_serial_fd); 
    		g_serial_fd = -1;
            return -1;
        }
       
        iDataLen = uart_recv(g_serial_fd, szRcvBuf, sizeof(szRcvBuf));
        if (iDataLen == -1)
        {
            printf("OpenBuzzer: exec uart_recv failed!\n");
			close(g_serial_fd); 
    		g_serial_fd = -1;
            return -1;
        }
       
        printf("OpenBuzzer: received len=%d\n", iDataLen); 
        
        if (szRcvBuf[0] == 0x5A)
        {
            printf("OpenBuzzer: Open buzzer success!\n");
        }
        else if (szRcvBuf[0] == 0x5B)
        {
            printf("OpenBuzzer: Open buzzer failed!\n"); 
        }
        else
        {
            printf("OpenBuzzer: received data is wrong!\n"); 
        }
    
        close(g_serial_fd); 
    	g_serial_fd = -1;
        return 0;


}

// Read Temperature Sensor value
int ReadAtmoTempSensors(void) 
{ 
    int  iDataLen      = 0;
//	int  iRetVal       = 0;
	char szRcvBuf[8]  = {0};

unsigned char atmoTemp[8] = {0x01,0x03,0x00,0x01,0x00,0x01,0xD5,0xCA}; //01 03 00 01 00 01 d5 ca
   
	if (g_iRetVal != 0)
    {
        printf("AtmoTemp: exec init_serial failed!\n");
		return -1;
    }
    
    iDataLen = uart_send(g_serial_fd, atmoTemp, sizeof(atmoTemp));
	if (iDataLen == -1)
    {
        printf("AtmoTemp: exec uart_send failed!\n");
		
		return -1;
    }
	
    
    iDataLen = uart_recv(g_serial_fd, szRcvBuf, sizeof(szRcvBuf));
	if (iDataLen == -1)
    {
        printf("AtmoTemp: exec uart_recv failed!\n");
		tcflush(g_serial_fd, TCIOFLUSH); 
		return -1;
		
    }
   
   	printf("AtmoTemp: received len=%d\n", iDataLen); 
	//printf("AtmoTemp: receiveddata3 =%d,receiveddata4 =%d\n", szRcvBuf[3],szRcvBuf[4]); 
	
    	if (szRcvBuf[0] == 0x01)
    {
        printf("AtmoTemp: get data success!\n");
    }
	else
	{
	    printf("AtmoTemp: received data is wrong!\n"); 
	}

	//close(g_serial_fd); 
	
	tcflush(g_serial_fd, TCIOFLUSH); 
	Sleep(5);

	return (szRcvBuf[3]*256+szRcvBuf[4]);
}

// Read Humidity Sensor value
int ReadHumiditySensors(void) 
{ 
    int  iDataLen      = 0;
//	int  iRetVal       = 0;
	char szRcvBuf[8]  = {0};

unsigned char Humidity[8] = {0x01,0x03,0x00,0x02,0x00,0x01,0x25,0xCA}; 
    //g_iRetVal = init_serial(); 
	if (g_iRetVal != 0)
    {
        printf("Humidity: exec init_serial failed!\n");
		return -1;
    }
    
    iDataLen = uart_send(g_serial_fd, Humidity, sizeof(Humidity));
	if (iDataLen == -1)
    {
        printf("Humidity: exec uart_send failed!\n");
		return -1;
    }
  
    iDataLen = uart_recv(g_serial_fd, szRcvBuf, sizeof(szRcvBuf));
	if (iDataLen == -1)
    {
        printf("Humidity: exec uart_recv failed!\n");
		tcflush(g_serial_fd, TCIOFLUSH); 
		return -1;
    }
   
    printf("Humidity: received len=%d\n", iDataLen); 
	//printf("Humidity: received data =%s\n", szRcvBuf); 
	
    	if (szRcvBuf[0] == 0x01)
    {
        printf("Humidity: get data success!\n");
    }
	else
	{
	    printf("Humidity: received data is wrong!\n"); 
	}

	//close(g_serial_fd); 
	
	tcflush(g_serial_fd, TCIOFLUSH); 
	Sleep(5);

	return (szRcvBuf[3]*256+szRcvBuf[4]);
}

// Read Illumination Sensor value
int ReadIlluminationSensors(void) 
{ 
    	int  iDataLen      = 0;
//	int  iRetVal       = 0;
	char szRcvBufH[8]  = {0};
	char szRcvBufL[8]  = {0};

unsigned char IlluminationH[8] = {0x01,0x03,0x00,0x04,0x00,0x01,0xC5,0xCB}; 

unsigned char IlluminationL[8] = {0x01,0x03,0x00,0x05,0x00,0x01,0x94,0x0B}; 

  //  g_iRetVal = init_serial(); 
	if (g_iRetVal != 0)
    {
        printf("IlluminationH: exec init_serial failed!\n");
		return -1;
    }
    
//// Get the high value of illumination sensor
    iDataLen = uart_send(g_serial_fd, IlluminationH, sizeof(IlluminationH));
	if (iDataLen == -1)
    {
        printf("IlluminationH: exec uart_send failed!\n");
		return -1;
    }
   
    iDataLen = uart_recv(g_serial_fd, szRcvBufH, sizeof(szRcvBufH));
	if (iDataLen == -1)
    {
        printf("IlluminationH: exec uart_recv failed!\n");
		tcflush(g_serial_fd, TCIOFLUSH); 
		return -1;
    }
   
    printf("IlluminationH: received len=%d\n", iDataLen); 
	//printf("Humidity: received data =%s\n", szRcvBuf); 
	
    	if (szRcvBufH[0] == 0x01)
    {
        printf("IlluminationH: get data success!\n");
    }
	else
	{
	    printf("IlluminationH: received data is wrong!\n"); 
	}
	 
	tcflush(g_serial_fd, TCIOFLUSH); 
	Sleep(5);



//// Get the low value of illumination sensor
    iDataLen = uart_send(g_serial_fd, IlluminationL, sizeof(IlluminationL));
	if (iDataLen == -1)
    {
        printf("IlluminationL: exec uart_send failed!\n");
		return -1;
    }
   
    iDataLen = uart_recv(g_serial_fd, szRcvBufL, sizeof(szRcvBufL));
	if (iDataLen == -1)
    {
        printf("IlluminationL: exec uart_recv failed!\n");
		 tcflush(g_serial_fd, TCIOFLUSH); 
		return -1;
    }
  
    printf("IlluminationL: received len=%d\n", iDataLen); 
	//printf("Humidity: received data =%s\n", szRcvBuf); 
	
    	if (szRcvBufL[0] == 0x01)
    {
        printf("IlluminationL: get data success!\n");
    }
	else
	{
	    printf("IlluminationL: received data is wrong!\n"); 
	}

	//close(g_serial_fd);
	tcflush(g_serial_fd, TCIOFLUSH); 
	Sleep(5);

	return (szRcvBufH[3]*256*256*256+szRcvBufH[4]*256*256+szRcvBufL[3]*256+szRcvBufL[4]);
}

// Read WindSpeed Sensor value
int ReadWindSpeedSensors(void) 
{ 
    int  iDataLen      = 0;
//	int  iRetVal       = 0;
	char szRcvBuf[8]  = {0};

    unsigned char WindSpeed[8] = {0x01,0x03,0x00,0x06,0x00,0x01,0x64,0x0B}; 
   // g_iRetVal = init_serial(); 
	if (g_iRetVal != 0)
    {
        printf("WindSpeed: exec init_serial failed!\n");
		return -1;
    }
    
    iDataLen = uart_send(g_serial_fd, WindSpeed, sizeof(WindSpeed));
	if (iDataLen == -1)
    {
        printf("WindSpeed: exec uart_send failed!\n");
		return -1;
    }
   
    iDataLen = uart_recv(g_serial_fd, szRcvBuf, sizeof(szRcvBuf));
	
	if (iDataLen == -1)
    {
        printf("WindSpeed: exec uart_recv failed!\n");
		tcflush(g_serial_fd, TCIOFLUSH); 
		return -1;
    }
   
    printf("WindSpeed: received len=%d\n", iDataLen); 
	//printf("Humidity: received data =%s\n", szRcvBuf); 
	
    	if (szRcvBuf[0] == 0x01)
    {
        printf("WindSpeed: get data success!\n");
    }
	else
	{
	    printf("WindSpeed: received data is wrong!\n"); 
	}

	//close(g_serial_fd); 
	 
	tcflush(g_serial_fd, TCIOFLUSH); 
	Sleep(5);

	return (szRcvBuf[3]*256+szRcvBuf[4]);
}


// Read WindDirection Sensor value
int ReadWindDirectionSensors(void) 
{ 
    int  iDataLen      = 0;
//	int  iRetVal       = 0;
	char szRcvBuf[8]  = {0};

unsigned char WindDirection[8] = {0x01,0x03,0x00,0x07,0x00,0x01,0x35,0xCB}; 
   // g_iRetVal = init_serial(); 
	if (g_iRetVal != 0)
    {
        printf("WindDirection: exec init_serial failed!\n");
		return -1;
    }
    
    iDataLen = uart_send(g_serial_fd, WindDirection, sizeof(WindDirection));
	if (iDataLen == -1)
    {
        printf("WindDirection: exec uart_send failed!\n");
		return -1;
    }
    
    iDataLen = uart_recv(g_serial_fd, szRcvBuf, sizeof(szRcvBuf));
	if (iDataLen == -1)
    {
        printf("WindDirection: exec uart_recv failed!\n");
		tcflush(g_serial_fd, TCIOFLUSH); 
		return -1;
    }
   
    printf("WindDirection: received len=%d\n", iDataLen); 
	//printf("Humidity: received data =%s\n", szRcvBuf); 
	
    	if (szRcvBuf[0] == 0x01)
    {
        printf("WindDirection: get data success!\n");
    }
	else
	{
	    printf("WindDirection: received data is wrong!\n"); 
	}

	//close(g_serial_fd); 
	
	tcflush(g_serial_fd, TCIOFLUSH); 
	Sleep(5);

	return (szRcvBuf[3]*256+szRcvBuf[4]);
}


	

/*    // 开锁
    iRetVal = OpenLock();
	if (iRetVal != 0)
	{
	    printf("Open lock failed!\n");
		return -1;
	}
 
    // 闭锁
    sleep(5);
	iRetVal = CloseLock();
	if (iRetVal != 0)
	{
	    printf("Close lock failed!\n");
		return -1;
	}

	// 开锁
	sleep(5);
    iRetVal = OpenLock();
	if (iRetVal != 0)
	{
	    printf("Open lock failed!\n");
		return -1;
	}

	// 获取车位锁 状态
    sleep(5);
	iRetVal = GetLockStatus();
	if (iRetVal != 0)
	{
	    printf("Get lock status failed!\n");
		return -1;
	}
*/


void Get_Temperature(t_getenvirparam_ack *getenvirparam_ack)
{
//Get temperature value
       float fTemperatureResult = 0.0;

	fTemperatureResult = ReadAtmoTempSensors();
	if (fTemperatureResult != -1)
	{
	printf("Temperature = %.1f\n",fTemperatureResult/10);
	//snprintf(_t_get_all_envirparam_ack.szWenduValue, sizeof(_t_get_all_envirparam_ack.szWenduValue) - 1, "%.1f", fTemperatureResult/10);
	snprintf(getenvirparam_ack->szValue, sizeof(getenvirparam_ack->szValue) - 1, "%.1f", fTemperatureResult/10);
    getenvirparam_ack->nReqType = 1; 
    getenvirparam_ack->retcode = 0;
    snprintf(getenvirparam_ack->retmsg, sizeof(getenvirparam_ack->retmsg) - 1, "SUCCESS");
	}
	else
	{
	//_t_get_all_envirparam_ack.retcode = -1;
	getenvirparam_ack->retcode =-1;
	printf("Get Temperature failed!\n");
	}
}

void Get_Humidity(t_getenvirparam_ack *getenvirparam_ack)
{
//Get Humidity value
       float fHumidityResult = 0.0;

	fHumidityResult = ReadHumiditySensors();
	if (fHumidityResult != -1)
	{
	 printf("Humidity = %.1f%%\n",fHumidityResult/10);
	 //snprintf(_t_get_all_envirparam_ack.szShiDuVlaue, sizeof(_t_get_all_envirparam_ack.szShiDuVlaue) - 1, "%.1f", fHumidityResult/10);
	 snprintf(getenvirparam_ack->szValue, sizeof(getenvirparam_ack->szValue) - 1, "%.1f", fHumidityResult/10);
        getenvirparam_ack->nReqType = 2;
	}
	else
	{
	//_t_get_all_envirparam_ack.retcode = -1;
	getenvirparam_ack->retcode =-1;
	printf("Get Humidity failed!\n");
	}
}

void Get_Illumination(t_getenvirparam_ack *getenvirparam_ack)
{
//Get Illumination value
	int iIlluminationResult = 0;

	iIlluminationResult = ReadIlluminationSensors();
	if (iIlluminationResult != -1)
	{
	 printf("Illumination = %d LUX\n",iIlluminationResult);
	 //snprintf(_t_get_all_envirparam_ack.szZhaoDuVlaue, sizeof(_t_get_all_envirparam_ack.szZhaoDuVlaue) - 1, "%d", iIlluminationResult);
	 snprintf(getenvirparam_ack->szValue, sizeof(getenvirparam_ack->szValue) - 1, "%d", iIlluminationResult);
	getenvirparam_ack->nReqType = 3;
	}
	else
	{
	//_t_get_all_envirparam_ack.retcode = -1;
	getenvirparam_ack->retcode =-1;
	printf("Get Humidity failed!\n");
	}
}



void Get_WindDireciton(t_getenvirparam_ack *getenvirparam_ack)
{
//Get WindDirection value
	float fWindDirectionResult = 0.0;

	fWindDirectionResult = ReadWindDirectionSensors();
	if (fWindDirectionResult != -1)
	{
	 printf("WindDirection = %.1f \n",fWindDirectionResult/10);
	// snprintf(_t_get_all_envirparam_ack.szFengXiangValue, sizeof(_t_get_all_envirparam_ack.szFengXiangValue) - 1, "%.1f", fWindDirectionResult/10);
	 snprintf(getenvirparam_ack->szValue, sizeof(getenvirparam_ack->szValue) - 1, "%.1f", fWindDirectionResult/10);
	getenvirparam_ack->nReqType = 4;
	}
	else
	{
	//_t_get_all_envirparam_ack.retcode = -1;
	getenvirparam_ack->retcode =-1;
	printf("Get WindDirection failed!\n");
	}
}

void Get_WindSpeed(t_getenvirparam_ack *getenvirparam_ack)
{
//Get WindSpeed value
	float fWindSpeedResult = 0.0;

	fWindSpeedResult = ReadWindSpeedSensors();
	if (fWindSpeedResult != -1)
	{
	 printf("WindSpeed = %.1f m/s\n",fWindSpeedResult/10);
	 // snprintf(_t_get_all_envirparam_ack.szFengSuValue, sizeof(_t_get_all_envirparam_ack.szFengSuValue) - 1, "%.1f", fWindSpeedResult/10);
	 snprintf(getenvirparam_ack->szValue, sizeof(getenvirparam_ack->szValue) - 1, "%.1f", fWindSpeedResult/10);
	getenvirparam_ack->nReqType = 5;
	}
	else
	{
	//_t_get_all_envirparam_ack.retcode = -1;
	getenvirparam_ack->retcode =-1;
	printf("Get WindSpeed failed!\n");
	}
}

void Get_All(t_get_all_envirparam_ack *getallenvirparam_ack)
{
//Get WindDirection value
float fTemperatureResult = 0.0;
float fHumidityResult = 0.0;
int iIlluminationResult = 0;
float fWindDirectionResult = 0.0;
float fWindSpeedResult = 0.0;

//getallenvirparam_ack->retmsg = "SUCCESS";//错误
snprintf(getallenvirparam_ack->retmsg, sizeof(getallenvirparam_ack->retmsg) - 1, "SUCCESS");
getallenvirparam_ack->retcode = 0;


//Temperature 
	fTemperatureResult = ReadAtmoTempSensors();
	sleep(1);

	if (fTemperatureResult != -1)
	{
	 printf("Temperature = %.1f\n",fTemperatureResult/10);
	 snprintf(getallenvirparam_ack->szWenduValue, sizeof(getallenvirparam_ack->szWenduValue) - 1, "%.1f", fTemperatureResult/10);
	 //snprintf(_t_getenvirparam_ack.szValue, sizeof(_t_getenvirparam_ack.szValue) - 1, "%.1f", fWindDirectionResult/10);
	//getallenvirparam_ack->nReqType = 6;
	}
	else
	{
	getallenvirparam_ack->retcode = -1;
	snprintf(getallenvirparam_ack->retmsg, sizeof(getallenvirparam_ack->retmsg) - 1, "UNSUCCESS");
	printf("Get WindDirection failed!\n");
	}
	

//Humidity
	fHumidityResult = ReadHumiditySensors();
	if (fHumidityResult != -1)
	{
	 printf("Humidity = %.1f%%\n",fHumidityResult/10);
	 snprintf(getallenvirparam_ack->szShiDuVlaue, sizeof(getallenvirparam_ack->szShiDuVlaue) - 1, "%.1f", fHumidityResult/10);
	 //snprintf(_t_getenvirparam_ack.szValue, sizeof(_t_getenvirparam_ack.szValue) - 1, "%.1f", fWindDirectionResult/10);
	//getallenvirparam_ack->nReqType = 6;
	}
	else
	{
	getallenvirparam_ack->retcode = -1;
	snprintf(getallenvirparam_ack->retmsg, sizeof(getallenvirparam_ack->retmsg) - 1, "UNSUCCESS");
	printf("Get WindDirection failed!\n");
	}

sleep(1);

//Illumination
	iIlluminationResult = ReadIlluminationSensors();
	if (iIlluminationResult != -1)
	{
	 printf("Illumination = %d LUX\n",iIlluminationResult);
	 snprintf(getallenvirparam_ack->szZhaoDuVlaue, sizeof(getallenvirparam_ack->szZhaoDuVlaue) - 1, "%d", iIlluminationResult);
	 //snprintf(getenvirparam_ack->szValue, sizeof(getenvirparam_ack->szValue) - 1, "%d", iIlluminationResult);
	//getallenvirparam_ack->nReqTypev = 6;
	}
	else
	{
	getallenvirparam_ack->retcode = -1;
	snprintf(getallenvirparam_ack->retmsg, sizeof(getallenvirparam_ack->retmsg) - 1, "UNSUCCESS");
	printf("Get Humidity failed!\n");
	}   
   sleep(1);
//WindDirection
	fWindDirectionResult = ReadWindDirectionSensors();
	if (fWindDirectionResult != -1)
	{
	 printf("WindDirection = %.1f \n",fWindDirectionResult/10);
	 snprintf(getallenvirparam_ack->szFengXiangValue, sizeof(getallenvirparam_ack->szFengXiangValue) - 1, "%.1f", fWindDirectionResult/10);
	 //snprintf(_t_getenvirparam_ack.szValue, sizeof(_t_getenvirparam_ack.szValue) - 1, "%.1f", fWindDirectionResult/10);
	//getallenvirparam_ack->nReqTypev = 6;
	}
	else
	{
	getallenvirparam_ack->retcode = -1;
	snprintf(getallenvirparam_ack->retmsg, sizeof(getallenvirparam_ack->retmsg) - 1, "UNSUCCESS");
	printf("Get WindDirection failed!\n");
	}
	
    sleep(1);

//WindSpeed
	fWindSpeedResult = ReadWindSpeedSensors();
	if (fWindSpeedResult != -1)
	{
	 printf("WindSpeed = %.1f m/s\n",fWindSpeedResult/10);
	 snprintf(getallenvirparam_ack->szFengSuValue, sizeof(getallenvirparam_ack->szFengSuValue) - 1, "%.1f", fWindSpeedResult/10);
	 //snprintf(getenvirparam_ack->szValue, sizeof(_t_getenvirparam_ack->szValue) - 1, "%.1f", fWindSpeedResult/10);
	//getallenvirparam_ack->nReqTypev = 6;
	}
	else
	{
	getallenvirparam_ack->retcode = -1;
	snprintf(getallenvirparam_ack->retmsg, sizeof(getallenvirparam_ack->retmsg) - 1, "UNSUCCESS");
	printf("Get WindSpeed failed!\n");
	}
	
}

// Get the environment parameters

int Get_envirparam(int  _nReqType)
{
 	_t_getenvirparam_ack.retcode = 0;
 	_t_get_all_envirparam_ack.retcode = 0;

	if(_nReqType==6)
		{
			//Get_Temperature();
			//Get_Humidity();
			//Get_Illumination();
			//Get_WindDireciton();
			//Get_WindSpeed();
			Get_All(&_t_get_all_envirparam_ack);
			return 0;
		}
	else if(_nReqType>=1 && _nReqType<=5)
		{     // 1表示温度，2表示湿度，3表示照度，4表示风向，5表表示风速 
			if(_nReqType == 1) 
				{
				Get_Temperature(&_t_getenvirparam_ack);
				return 0;
				}
			else if(_nReqType == 2)
				{
				Get_Humidity(&_t_getenvirparam_ack);
				return 0;
				}
			else if(_nReqType == 3)
				{
				Get_Illumination(&_t_getenvirparam_ack);
				return 0;
				}
			else if(_nReqType == 4)
				{
				Get_WindDireciton(&_t_getenvirparam_ack);
				return 0;
				}
			else if(_nReqType == 5)
				{
				Get_WindSpeed(&_t_getenvirparam_ack);
				return 0;
				}			
		}
	else
		{
			printf("the require type error!\n");
			printf(" type value =%d\n",_nReqType);
			return -1;
		}

}
// the thread of environment parameters
void Thread_envirparam(void)
{
	pthread_t id;
	int iRet =0;

        iRet = pthread_create(&id, NULL, Get_envirparam, nReqType);
        if(0 != iRet)
        {
            printf("the create of  the thread failed!\n");
        }
    
}

// 主函数
/*int main(int argc, char *argv[]) 
{ 
nReqType = atoi(argv[1]);
Thread_envirparam();
 sleep(1);
   return 0; 
}*/



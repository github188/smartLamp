#ifndef _LOCK_SENSORS_H_
#define _LOCK_SENSORS_H_

#ifdef _WIN32
  #pragma pack(push,1)
#else
  #pragma pack(1)
#endif


#ifdef _WIN32
  #pragma pack(pop)
#else
  #pragma pack()
#endif

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

//#define DEVICE "/dev/ttySAC2" 
#define DEVICE "/dev/ttyAMA0"

//获取单个环境参数应答，只能获取指定项的参数
typedef struct
{
    int     retcode;     // 结果码，成功为0，其他为错误
    char    retmsg[24];    // 结果信息，成功为SUCCESS，其他为错误信息
    int     nReqType;// 1表示温度，2表示湿度，3表示照度，4表示风向，5表表示风速 
    char    szValue[8];//结果值
}t_getenvirparam_ack;

//获取所有环境参数应答，包括温度，湿度，照度，风速，风向
typedef struct
{
    int     retcode;                                           // 结果码,0为正常,其他为错误
    char    retmsg[24];                                       // 结果信息，成功为SUCCESS，其他为错误信息
    char    szWenduValue[8];                                  //温度值
    char    szShiDuVlaue[8];                                  //温度值
    char    szZhaoDuVlaue[8];                                 //照度值
    char    szFengXiangValue[8];                              //风向值
    char    szFengSuValue[8];                                 //风速值
}t_get_all_envirparam_ack;


//获取环境参数请求,只能获取指定项的参数

int  nReqType;//  1表示温度，2表示湿度，3表示照度，4表示风向，5表表示风速6 所有环境参数



// the results of the environment parameters
//int iIlluminationResult= 0;
//float fTemperatureResult= 0;
//float fHumidityResult= 0;
//float fWindSpeedResult= 0;
//float fWindDirectionResult= 0;

t_getenvirparam_ack _t_getenvirparam_ack;
t_get_all_envirparam_ack _t_get_all_envirparam_ack;

int init_serial(void);  // 打开串口并初始化设置
int uart_send(int fd, char *data, int datalen);   // 串口发送数据
int uart_recv(int fd, char *data, int datalen);    // 串口接收数据
int OpenLock(void);      // 开车位锁
int CloseLock(void);     // 关车位锁
int GetLockStatus(char *parklock_id, int *status); // 获取车位锁 状态

int CloseBuzzer(void);//关蜂鸣器
int OpenBuzzer(void);//开蜂鸣器

int ReadAtmoTempSensors(void);
int ReadHumiditySensors(void);
int ReadIlluminationSensors(void);
int ReadWindSpeedSensors(void);
int ReadWindDirectionSensors(void);

#endif /*end _LOCK_SENSORS_H_*/

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
#include <fcntl.h>      //�ļ����ƶ��� 
#include <termios.h>    //�ն˿��ƶ��� 
#include <errno.h> 

//#define DEVICE "/dev/ttySAC2" 
#define DEVICE "/dev/ttyAMA0"

//��ȡ������������Ӧ��ֻ�ܻ�ȡָ����Ĳ���
typedef struct
{
    int     retcode;     // ����룬�ɹ�Ϊ0������Ϊ����
    char    retmsg[24];    // �����Ϣ���ɹ�ΪSUCCESS������Ϊ������Ϣ
    int     nReqType;// 1��ʾ�¶ȣ�2��ʾʪ�ȣ�3��ʾ�նȣ�4��ʾ����5���ʾ���� 
    char    szValue[8];//���ֵ
}t_getenvirparam_ack;

//��ȡ���л�������Ӧ�𣬰����¶ȣ�ʪ�ȣ��նȣ����٣�����
typedef struct
{
    int     retcode;                                           // �����,0Ϊ����,����Ϊ����
    char    retmsg[24];                                       // �����Ϣ���ɹ�ΪSUCCESS������Ϊ������Ϣ
    char    szWenduValue[8];                                  //�¶�ֵ
    char    szShiDuVlaue[8];                                  //�¶�ֵ
    char    szZhaoDuVlaue[8];                                 //�ն�ֵ
    char    szFengXiangValue[8];                              //����ֵ
    char    szFengSuValue[8];                                 //����ֵ
}t_get_all_envirparam_ack;


//��ȡ������������,ֻ�ܻ�ȡָ����Ĳ���

int  nReqType;//  1��ʾ�¶ȣ�2��ʾʪ�ȣ�3��ʾ�նȣ�4��ʾ����5���ʾ����6 ���л�������



// the results of the environment parameters
//int iIlluminationResult= 0;
//float fTemperatureResult= 0;
//float fHumidityResult= 0;
//float fWindSpeedResult= 0;
//float fWindDirectionResult= 0;

t_getenvirparam_ack _t_getenvirparam_ack;
t_get_all_envirparam_ack _t_get_all_envirparam_ack;

int init_serial(void);  // �򿪴��ڲ���ʼ������
int uart_send(int fd, char *data, int datalen);   // ���ڷ�������
int uart_recv(int fd, char *data, int datalen);    // ���ڽ�������
int OpenLock(void);      // ����λ��
int CloseLock(void);     // �س�λ��
int GetLockStatus(char *parklock_id, int *status); // ��ȡ��λ�� ״̬

int CloseBuzzer(void);//�ط�����
int OpenBuzzer(void);//��������

int ReadAtmoTempSensors(void);
int ReadHumiditySensors(void);
int ReadIlluminationSensors(void);
int ReadWindSpeedSensors(void);
int ReadWindDirectionSensors(void);

#endif /*end _LOCK_SENSORS_H_*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "frame.h"
#include "../log/log.h"


// 8421BCD码转数字字符串
int BCD2String(char *pcBCDData, int iBCDDataLen, char *strDigitBuf, int iBufLen)
{
    int  i    = 0;
    char cBCD = 0;
	char szLogInfo[1024]  = { 0 };

    if (pcBCDData==NULL || strDigitBuf==NULL)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "BCD2String: pcBCDData==NULL || strDigitBuf==NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
	
    if (2 * iBCDDataLen > iBufLen)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "BCD2String: 2 * iBCDDataLen(%d) > iBufLen(%d)!", iBCDDataLen, iBufLen);
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
	
    for(i = 0; i < iBCDDataLen; i++)
    {
        cBCD = (pcBCDData[i] >> 4);
        if (0x0f == cBCD)
        {
            break;
        }
        strDigitBuf[2 * i] = cBCD + '0';
        if(!isdigit(strDigitBuf[2 * i]))
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "BCD2String: strDigitBuf[2 * i] is not digit!", i);
            WRITELOG(LOG_ERROR, szLogInfo);
            return -2;
        }

        cBCD = (pcBCDData[i] & 0x0f);
        if(0x0f == cBCD)
        {
            break;
        }
        strDigitBuf[2 * i + 1] = cBCD + '0';
        if(!isdigit(strDigitBuf[2 * i + 1]))
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "BCD2String: strDigitBuf[2 * i +1 ] is not digit!", i);
            WRITELOG(LOG_ERROR, szLogInfo);
            return -2;
        }
    }
    
    return 0;
}


// 8421数字字符串转BCD码
int String2BCD(char *strDigitData, int strDigitDataLen, char *pcBCDBuf, int iBCDBufLen)
{
    int  i = 0;
	char szLogInfo[1024]  = { 0 };

    if (strDigitData==NULL || pcBCDBuf==NULL)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "String2BCD: strDigitData==NULL || pcBCDBuf==NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
	
    if (strDigitDataLen > 2 * iBCDBufLen)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "String2BCD: strDigitDataLen(%d) > 2 * iBCDBufLen(%d)!", strDigitDataLen, iBCDBufLen);
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
	
    for(i = 0; i < strDigitDataLen; i+=2)
    {
        if(!isdigit(strDigitData[i]))
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "String2BCD: strDigitData[i] is not digit!", i);
            WRITELOG(LOG_ERROR, szLogInfo);
            return -2;
        }
        pcBCDBuf[i / 2] = strDigitData[i] - '0';
        pcBCDBuf[i / 2] <<= 4;

        if((i + 1 >= strDigitDataLen))
        {
            break;
        }
        if(!isdigit(strDigitData[i + 1]))
        {
            snprintf(szLogInfo, sizeof(szLogInfo) - 1, "String2BCD: isdigit(strDigitData[i + 1] is not digit!", i);
            WRITELOG(LOG_ERROR, szLogInfo);
            return -2;
        }
        pcBCDBuf[i / 2] += strDigitData[i + 1] - '0';
    }
    
    return 0;
}


// 组装校验和
unsigned char check_sum(unsigned char *data, int len)
{
    int i = 0;
    unsigned char ucCS = 0;
    char szLogInfo[1024]  = { 0 };

	if (data == NULL)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "check_sum: data == NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return 0;
    }
	
    for(i = 0; i < len; i++)
    {
        ucCS += data[i];
    }

    return ucCS;
}

// 解析3761帧
int parse_3761frame(char *data, int len, t_3761frame *frame)
{
    int offset = 0;
    unsigned char ucBeginChar = 0x68;
    unsigned char ucEndChar = 0x16;
    unsigned short usL = 0;
    char acAddr[5] = { 0 };
    char A1[5] = { 0 };
    char A2[5] = { 0 };
    char A3 = 0;
	char szLogInfo[1024]  = { 0 };

	if (data == NULL || frame == NULL)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "parse_3761frame: data == NULL || frame == NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
    
    if(0 != memcmp(data, &ucBeginChar, 1))
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "parse_3761frame: BeginChar is not 0x68!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -2;
    }

    // 跳过68H
    offset += 1;
    memcpy(&usL, data + offset, 2);  // 用户数据长度由D2～D15组成，采用BIN编码，是控制域、地址域、链路用户数据（应用层）的字节总数
    usL >>= 2;
	
    // 跳过L域
    offset += 2;
    // 跳过L域
    offset += 2;
    // 跳过68H
    offset += 1;

    // 控制域
    memcpy(&frame->ctrl_field, data + offset, 1);

    // 跳过控制域
    offset += 1;
    // 地址域
    // memcpy(&acAddr, data + offset, 5);
    memcpy(acAddr, data + offset, 5);
    BCD2String(&acAddr[0], 2, A1, sizeof(A1));
    BCD2String(&acAddr[2], 2, A2, sizeof(A2));
    A3 = acAddr[4];
    snprintf(frame->addr, sizeof(frame->addr), "%s%s%03d", A1, A2, (char)(A3 >> 1));    // D1～D7组成0～127个主站地址
	
    // 跳过地址域
    offset += 5;
    memcpy(&frame->user_data.afn, data + offset, 1); 

    // 跳过应用层功能码AFN
    offset += 1;
    memcpy(&frame->user_data.seq, data + offset, 1);

    // 跳过帧序列域SEQ
    offset += 1;
    memcpy(frame->user_data.pn, data + offset, 2);
    
    // 跳过DA
    offset += 2;
    memcpy(frame->user_data.fn, data + offset, 2);

    // 跳过DT
    offset += 2;
    memcpy(frame->user_data.data, data + offset, usL - 12);    // 减去控制域、地址域、链路用户数据共12字节
    frame->user_data.len = usL - 12;

    offset += usL - 12;
    
    return offset;
}


// 封装3761帧
int pack_3761frame(t_3761frame *frame, char *data, int len)
{
    int offset = 0;
    unsigned char ucBeginChar = 0x68;
    unsigned char ucEndChar = 0x16;
    unsigned short usL = 0;
    char A1[2] = { 0 };
    char A2[2] = { 0 };
    char A3 = 0;
    unsigned char ucCS = 0;
	char szLogInfo[1024] = { 0 };

	if (frame == NULL || data == NULL)
    {
        snprintf(szLogInfo, sizeof(szLogInfo) - 1, "pack_3761frame: frame == NULL || data == NULL!");
        WRITELOG(LOG_ERROR, szLogInfo);
        return -1;
    }
    
    // 起始字符（68H）
    memcpy(data, &ucBeginChar, 1);
    offset += 1;

    // 长度L
    usL = 12 + frame->user_data.len;  // 控制域、地址域、链路用户数据（应用层）的字节总数
    usL <<= 2;
	usL |= 0x02;   // D0=0、D1=1为路灯系统规约使用
    memcpy(data + offset, &usL, 2);
    offset += 2;

    // 长度L
    memcpy(data + offset, &usL, 2);
    offset += 2;

    // 起始字符（68H）
    memcpy(data + offset, &ucBeginChar, 1);
    offset += 1;

    // 控制域C
    memcpy(data + offset, &frame->ctrl_field, 1);
    offset += 1;

    // 地址域A
    String2BCD(&frame->addr[0], 4, A1, 2);
    memcpy(data + offset, A1, 2);
    offset += 2;
    
    String2BCD(&frame->addr[4], 4, A2, 2);
    memcpy(data + offset, A2, 2);
    offset += 2;
    
    A3 = atoi(&frame->addr[8]);
    A3 <<= 1;   // D1～D7组成0～127个主站地址
    if((0 == strcmp(A1, "9999")) && (0 == strcmp(A2, "9999")))    // A1A2=99999999H且A3的D0位为"1"时表示系统广播地址
    {
        A3 += 1;
    }
    memcpy(data + offset, &A3, 1);
    offset += 1;

    //////////////////链路用户数据begin//////////////////
    // AFN
    memcpy(data + offset, &frame->user_data.afn, 1);
    offset += 1;
    
    // SEQ
    memcpy(data + offset, &frame->user_data.seq, 1);
    offset += 1;

    ////////////数据单元标识begin///////////////
    // DA
    memcpy(data + offset, frame->user_data.pn, 2);
    offset += 2;

    // DT
    memcpy(data + offset, frame->user_data.fn, 2);
    offset += 2;
    ////////////数据单元标识end///////////////
    
    // 数据单元
    memcpy(data + offset, frame->user_data.data, frame->user_data.len);
    offset += frame->user_data.len;
    
    //////////////////链路用户数据end//////////////////

    // 校验和CS
    ucCS = check_sum(data + 6, offset - 6);   // 帧校验和是用户数据区所有字节的八位位组算术和，用户数据区包括控制域、地址域、链路用户数据（应用层）三部分
    memcpy(data + offset, &ucCS, 1);
    offset += 1;

    // 结束字符（16H）
    memcpy(data + offset, &ucEndChar, 1);
    offset += 1;
    
    return offset;
}



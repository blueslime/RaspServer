//////////////////////////////////////////////////////////////////////////
// for MultiflexNG
// http://robot.up-tech.com
// zwj_uptech@126.com
//////////////////////////////////////////////////////////////////////////

#ifndef MFBACK_H
#define MFBACK_H 

#include "debuglevel.h"
#ifdef WIN32
#include "StdAfx.h"
#endif

#include <stdio.h>
//#include "protocol.h"

#ifdef __LINUX__
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#endif
#include <stdlib.h>
#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/sockios.h>
#define SOCKET int
#endif

#include "type.h"

void MFInit();

//LCD
void MF2X4LCD(const char*item,int insize);

//Ultrasonic
void MFUSonicEnable();
int MFGetUltrasonic();

//Delay
void DelayMS(int inMS);

//SerialCom
void MFComSend(const void *pBuffer, const int iLength);

//Digi. IO
void MFDigiInit(int interval);
void MFSetPortDirect(unsigned int inData);		//设置IO方向
int MFGetDigiInput(int inID);					//读取IO值，返回值：高电压为1，低电压为0
void MFDigiOutput(int inID,int inVal);			//

//AD
void MFADInit(int inID,int interval);
void MFADInit(int interval);
void MFADEnable(int inID);
int MFGetAD(int inID);

//Servo
void MFServoInit(int interval);
void MFSetServoMode(int inID,int inMode);
void MFSetServoPos(int inID,int inPos,int inSpeed);
void MFSetServoRotaSpd(int inID,int inSpeed);
int MFGetServoPos(int inID);
void MFServoAction();
void MFDisableServo(int inID);

//Video Capture
bool MFCapOpen();
void MFCapClose();
void MFCapSetH(int inHmax,int inHmin);
void MFCapSetS(int inSmax,int inSmin);
void MFCapSetI(int inImax,int inImin);
void MFCapSetMode(int inMode);
int MFCapGetCenterX();
int MFCapGetCenterY();
int MFCapGetSum();
void MFCapConnect(char* inTarIP,int inPort);
void MFCapImgCallback(PIMGREC inFunc);

//SpeechRecongn
void MFSRStart();
void MFSRStop();
bool MFSRPause();
bool MFSRContinue();
bool MFSRInsertText(int index,const char*item,int insize);
int MFSRGetResIndex();
void MFSRClearItems();
bool MFSRisStarted();

//mp3
void MFMp3Play(const char*item,int insize);
void MFMp3Stop();

//UDP
int UDPConnect(char *inTarIP, int inTarPort);	//建立连接
void UDPSend(unsigned char *inbuf, int inLen);	//发送数据
int UDPListen(int inPort);						//打开监听
void UDPRecvCallback(PBUFREC inFunc);				//设置接收解析函数

//EXT
int MFGetFromAddr(unsigned char inAddr);
void MFSetToAddr(unsigned char inAddr,unsigned char inVal);

//debug
int GetCmdVaildprc();
int GetCmdWaitMax();
int GetCmdCurCnt();
bool ComCreat(int inNo,unsigned int inBaudrate);	//inNo:  1-RS422   2-mega128

void DisplaySendFrame(bool inFlag);
void DisplaySendBytes(bool inFlag);
void DisplayRecvFrame(bool inFlag);
void DisplayRecvBytes(bool inFlag);

int GetRecvFPS(int inSec);
#endif


//////////////////////////////////////////////////////////////////////////
// for MultiflexNG
// http://robot.up-tech.com 
// Designed by lsy
// Modified by zwj @ 20090223
// zwj_uptech@126.com
//////////////////////////////////////////////////////////////////////////
#ifndef SERIAL_H_H
#define SERIAL_H_H
//#include <queue>
#ifdef __LINUX__
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <termios.h>
#endif//__LINUX__

#include <stdio.h>
#include "../type.h"
#include "CQueue.h"

struct CMDBUF 
{
	UCHAR * pCmdBuf;	//指令数组指针
	UINT nLen;			//指令长度
};

#define RINGLEN 64
#define RBUFLEN 128

class CSerialCom
{
public:
	bool BufEmpty();
	void ResetCnt();
	virtual void DataRecv(unsigned char* inBuf,int inLen){};			//接收数据
	BOOL Create(int inCom , unsigned int inBaudrate);		//打开一个串口端口
	void Close();											//关闭当前打开的端口
	void Send(const void *pBuffer, const int iLength);		//通过缓冲列表进行指令发送（标准接口）
	CSerialCom();
	virtual ~CSerialCom();
	static void *ComMonitor(void *pParam);
	static void *SendingThread(void *pParam);
	
	bool m_bCmdRsp;
	
	//debug
	unsigned int nMaxCnt;
	unsigned int nWaitMax;
	unsigned int nSendCnt;
	unsigned int nRspCnt;
	unsigned int nCurCnt;

	bool bDisplaySendFrame;
	bool bDisplayRecvFrame;
	bool bDisplaySendByte;
	bool bDisplayRecvByte;

protected:
	bool bToStop;
	//串口属性
private:
	CQueue<CMDBUF *> m_cmdlist;
	//////////////////////////////////////////////////////////////////////////
	//环形缓冲区
	unsigned char m_ringBuf[RINGLEN * RBUFLEN];
	int m_nRCmdLen[RINGLEN];
//	pthread_mutex_t wLock;
	int m_nGet;
	int m_nPut;
	//////////////////////////////////////////////////////////////////////////
	int m_iCom;
//	pthread_mutex_t mutex;
	struct termios m_oldtio;
	pthread_t pidSendThread, pidMonitorThread;
};


#endif

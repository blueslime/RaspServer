//////////////////////////////////////////////////////////////////////////
// for MultiflexNG
// http://robot.up-tech.com 
// zwj_uptech@126.com
//////////////////////////////////////////////////////////////////////////

#if !defined(AFX_UDPBASIC_H__88CF2711_7C7A_47A6_96F1_4A225C9E1C9B__INCLUDED_)
#define AFX_UDPBASIC_H__88CF2711_7C7A_47A6_96F1_4A225C9E1C9B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include "../type.h"

class CUDPBasic  
{
public:
	void Send(unsigned char *inbuf, int inLen);
	int Connect(char* inTarIP,int inTarPort);
	void Send(char *inTarIP, int inPort,unsigned char *inbuf, int inLen);
	virtual void AfterRecv(char* inbuf,int inlen);
	static void* UDPSerRecThread(void *pParam);
	int Listen(int inPort);
	CUDPBasic();
	virtual ~CUDPBasic();

	void SetFromIP(char* inIP);
	
	//server
	int m_LSkt;
	int m_nLPort;
	struct sockaddr_in serv_addr; /* host address information */
	pthread_t pidUDPSerThread;
	PBUFREC pRec;

protected:
	char m_fromip[16];

	//send
	int m_SSkt;
};

#endif // !defined(AFX_UDPBASIC_H__88CF2711_7C7A_47A6_96F1_4A225C9E1C9B__INCLUDED_)

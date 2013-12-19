//////////////////////////////////////////////////////////////////////////
// for MultiflexNG
// http://robot.up-tech.com 
// zwj_uptech@126.com
//////////////////////////////////////////////////////////////////////////

#if !defined(TCPLIN_H__50B8E916_809D_4A0A_8586_D6679CAFD737__INCLUDED_)
#define TCPLIN_H__50B8E916_809D_4A0A_8586_D6679CAFD737__INCLUDED_

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
#include "st_MF270.h"


#define SOCKET int

class CTCPLin;
typedef struct _tag_SOCKET_LISTEN_PARAM
{
	SOCKET listen_socket;	//listen socket
	SOCKET rec_socket;		//recieve socket
	CTCPLin * pListener;	//NetListener pointer
	bool toExit;
	bool m_bConnected;		//flag
}SOCKETLISTENPARAM;

class CTCPLin  
{
public:
	void Disconnect();
	virtual void AfterConn(struct sockaddr_in* inParam,SOCKET inskt){};
	virtual void AfterRecv(char* inbuf,int inlen);
	void Send(unsigned char* inBuf,int inLen);
	static void * TCPListenThread(void *pParam);
	bool Listen(int inPort);
	CTCPLin();
	virtual ~CTCPLin();
	
	PBUFREC pRec;
	SOCKETLISTENPARAM m_ListenP;		//listenparam list
protected:
	int m_nListenPort;
	//SOCKET m_socket;

	struct sockaddr_in serv_addr; /* host address information */ 

	pthread_t pidListenThread;

	char m_recbuf[1024];
	int m_nRecCnt;
};

#endif // !defined(TCPLIN_H__50B8E916_809D_4A0A_8586_D6679CAFD737__INCLUDED_)

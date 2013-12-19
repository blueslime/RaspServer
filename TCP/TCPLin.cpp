// TCPLin.cpp: implementation of the CTCPLin class.
//
//////////////////////////////////////////////////////////////////////

#include "TCPLin.h"
#include "../background.h"

#define BACK_LOG 10
#define MAX_SOCKETBUFFER 512

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTCPLin::CTCPLin()
{
	pRec = 0;
	m_ListenP.listen_socket = INVALID_SOCKET;
	m_nListenPort = 0;
	m_nRecCnt = 0;
	m_ListenP.m_bConnected = false;
}

CTCPLin::~CTCPLin()
{
	m_ListenP.pListener = NULL;
}

bool CTCPLin::Listen(int inPort)
{
	if (inPort < 0 ||(m_nListenPort == inPort && m_ListenP.listen_socket != INVALID_SOCKET))
	{
		return false;
	}

	if (m_ListenP.listen_socket != INVALID_SOCKET)
	{
		close(m_ListenP.listen_socket);
	}


	if ((m_ListenP.listen_socket = socket(AF_INET,SOCK_STREAM,0)) < 0)
	{
		fprintf( stderr, "CTCPLin::Listen : create socket error! \n");
	}

	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(inPort);

	if (bind(m_ListenP.listen_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR)
	{
		fprintf( stderr, "CTCPLin::Listen : Bind address to listen fd failed! \n");
		close(m_ListenP.listen_socket);
		m_ListenP.listen_socket = INVALID_SOCKET;
		return false;
	}

	if (listen(m_ListenP.listen_socket, BACK_LOG) < 0)
	{
		fprintf( stderr, "CTCPLin::Listen : call listen failure!\n");
		close(m_ListenP.listen_socket);
		m_ListenP.listen_socket = INVALID_SOCKET;
		return false;
	}

	//开启监听线程
	m_ListenP.pListener = this;
	m_ListenP.m_bConnected = false;
	m_ListenP.toExit = false;
	pthread_create(&pidListenThread,NULL,TCPListenThread,(void*)&m_ListenP);

	return true;
}

void * CTCPLin::TCPListenThread(void *pParam)
{
	SOCKETLISTENPARAM *psockparam=(SOCKETLISTENPARAM*)pParam;
	
	//listen	
	struct sockaddr_in cli_addr; /* connector's address information */
	socklen_t length = sizeof(cli_addr);

	SOCKET ClientSocket = accept(psockparam->listen_socket,(struct sockaddr*)&cli_addr,&length);	//阻塞，等待client连接进来
	
	if (ClientSocket < 0)
	{
		fprintf( stderr, "TCPListenThread : error comes when call accept!\n");
		return NULL;
	}

	printf("TCPListenThread : Receive a connection from %s!!(fd=%d)\n", inet_ntoa(cli_addr.sin_addr), ClientSocket);
	psockparam->rec_socket = ClientSocket;
	psockparam->m_bConnected = true;

	psockparam->pListener->AfterConn(&(cli_addr),ClientSocket);
	//连接成功，新开启一个监听线程，本线程变更为接收线程(开启线程未做)

	//接收数据缓冲
	char buffer[MAX_SOCKETBUFFER];
	memset((char *) buffer, 0, sizeof(buffer));

	int irecv;
	while (1)
	{
		irecv = recv(ClientSocket,buffer,MAX_SOCKETBUFFER,0);
		//假如连接断开则跳出循环
		if (irecv <= 0)
		{
			break;
		}

		//数据已经接收，发送到指定地点处理
		//printf("TCPListenThread : Receive !' %s '\n", buffer);
		psockparam->pListener->AfterRecv(buffer,irecv);
	}
	
	psockparam->m_bConnected = false;
	close(ClientSocket);
	psockparam->rec_socket = INVALID_SOCKET;
	fprintf( stderr, "TCPListenThread : Disconnect! No:%d\n", WSAGetLastError());

	//restart listen!
	if (psockparam->pListener != NULL)
	{
		psockparam->pListener->Listen(1000);
	}

	return NULL;
}


void CTCPLin::Send(unsigned char *inBuf, int inLen)
{
	if (m_ListenP.m_bConnected == true)
	{
		send(m_ListenP.rec_socket,inBuf,inLen,0);
	}
}

void CTCPLin::AfterRecv(char* inbuf,int inlen)
{
// 	if (pRec != 0)
// 	{
// 		(*pRec)(inbuf,inlen);
// 	}

	memcpy(&(m_recbuf[m_nRecCnt]),inbuf,inlen);
	m_nRecCnt += inlen;
	while (m_nRecCnt >= (int)sizeof(STMfth))
	{
		STMfth* p = (STMfth*)m_recbuf;
		
		//printf("CTCPLin::AfterRecv! flag=%d max=%d  min=%d len=%d\n", p->flag,p->major,p->minor,inlen);
		switch (p->flag)
		{
		case MF_HTH:
			MFCapSetH(p->major,p->minor);
			break;

		case MF_STH:
			MFCapSetS(p->major,p->minor);
			break;

		case MF_ITH:
			MFCapSetI(p->major,p->minor);
			break;

		case MF_MODE:
			MFCapSetMode(p->major);
			break;
		}

		m_nRecCnt -= sizeof(STMfth);

		//判断是否还有多余数据未处理
		if (m_nRecCnt == 0)
		{
			break;
		} 
		else
		{
			memcpy(m_recbuf,m_recbuf+sizeof(STMfth),m_nRecCnt);
		}
	}
}

void CTCPLin::Disconnect()
{
	if (m_ListenP.listen_socket != INVALID_SOCKET)
	{
		close(m_ListenP.listen_socket);
		m_ListenP.listen_socket = INVALID_SOCKET;
	}
}

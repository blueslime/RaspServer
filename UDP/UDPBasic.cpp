// UDPBasic.cpp: implementation of the CUDPBasic class.
//
//////////////////////////////////////////////////////////////////////

#include "UDPBasic.h"
#include "../background.h"

#define MAX_SOCKETBUFFER 1024

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUDPBasic::CUDPBasic()
{
	m_LSkt = m_SSkt =-1;
	m_nLPort = 0;
	pRec = 0;
}

CUDPBasic::~CUDPBasic()
{

}

int CUDPBasic::Listen(int inPort)
{
	if (m_LSkt != -1)
	{
		close(m_LSkt);
		m_LSkt = INVALID_SOCKET;
	}

	if ((m_LSkt = socket(AF_INET,SOCK_DGRAM,0)) < 0)
	{
		fprintf( stderr, "create server socket error! \n");
	}
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(inPort);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(m_LSkt, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) == -1)
	{
		fprintf( stderr, " Bind address to listen fd failed! \n");
		close(m_LSkt);
		m_LSkt = INVALID_SOCKET;
		return -1;
	}
	m_nLPort = inPort;

	pthread_create(&pidUDPSerThread,NULL,UDPSerRecThread,(void*)this);

	fprintf(stderr,"UDP Listen @ Port %d !\n",m_nLPort);
	return 1;
}

void* CUDPBasic::UDPSerRecThread(void *pParam)
{	
	CUDPBasic* pudp = (CUDPBasic*)pParam;
	int RecSkt = pudp->m_LSkt;
	int sin_size;
	int irecv;
	char fromip[16];
	struct sockaddr_in cl_addr;
	
	sin_size = sizeof(struct sockaddr_in);

	//开启接收数据缓冲
	char buffer[MAX_SOCKETBUFFER];
	memset((char *) buffer, 0, MAX_SOCKETBUFFER);

	while (1)
	{
		irecv = recvfrom(RecSkt,buffer,MAX_SOCKETBUFFER,0,(struct sockaddr*)&(cl_addr),(socklen_t*)&sin_size);
		//假如连接断开则跳出循环
		if (irecv <= 0)
		{
			fprintf(stderr,"Recv Err!\n");
			break;
		}

		//数据已经接收，开始解析
		sprintf(fromip,"%s",inet_ntoa(cl_addr.sin_addr));
		pudp->SetFromIP(fromip);
		//fprintf(stderr,"Receive %d byte : %s  from %s\n ",irecv, buffer,fromip);

		pudp->AfterRecv(buffer, irecv);
	}
	return 0;
}

void CUDPBasic::SetFromIP(char *inIP)
{
	strcpy(m_fromip,inIP);
}

void CUDPBasic::AfterRecv(char *inbuf, int inlen)
{
	//将接收到的数据传给
	if (pRec != 0)
	{
		(*pRec)(inbuf,inlen);
	}
}

void CUDPBasic::Send(char *inTarIP, int inPort,unsigned char *inbuf, int inLen)
{
	int udpskt;

	if((udpskt = socket(AF_INET,SOCK_DGRAM,0))==-1)
	{
		fprintf(stderr,"udp socket err!\n");
		return;
	}
	//fprintf(stderr,"udp socket create!skt = %d %s :%d\n",m_udpskt,inTarIP,inPort);

	struct sockaddr_in targaddr;

	bzero(&targaddr,sizeof(struct sockaddr_in));
	targaddr.sin_family=AF_INET;
	targaddr.sin_port=htons(inPort);             ///target 的监听端口
	targaddr.sin_addr.s_addr=inet_addr(inTarIP); ///target 的地址
	
	//send to targ
	sendto(udpskt,inbuf,inLen,0,(struct sockaddr*)&targaddr,sizeof(struct sockaddr));

	close(udpskt);
	
	//fprintf(stderr,"send %d bytes! skt = %d\n",res,m_SSkt);

	return;
}

int CUDPBasic::Connect(char *inTarIP, int inTarPort)
{
	if (m_SSkt != -1)
	{
		close(m_SSkt);
		m_SSkt = -1;
	}

	struct sockaddr_in servaddr;

	/* init servaddr */
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(inTarPort);
	if(inet_pton(AF_INET, inTarIP, &servaddr.sin_addr) <= 0)
	{
		fprintf(stderr,"[%s] is not a valid IPaddress\n", inTarIP);
		return -1;
	}

	m_SSkt = socket(AF_INET, SOCK_DGRAM, 0);

	/* connect to server */
	if(connect(m_SSkt, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
	{
		fprintf(stderr,"CUDPBasic::Connect: Connect Err!\n");
		close(m_SSkt);
		m_SSkt = -1;
		return -1;
	}
	fprintf(stderr,"CUDPBasic::Connect: Connect succeed!\n");

	return 1;
}

void CUDPBasic::Send(unsigned char *inbuf, int inLen)
{
	if (m_SSkt == -1)
	{
		fprintf(stderr,"CUDPBasic::Send : m_SSkt is not Ready!\n ");
		return;
	}

	write(m_SSkt, inbuf, inLen);
	//fprintf(stderr,"CUDPBasic::Send : %d byte is send!\n ",inLen);
}

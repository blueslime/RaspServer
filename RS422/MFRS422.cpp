// MFRS422.cpp: implementation of the CMFRS422 class.
//
//////////////////////////////////////////////////////////////////////

#include "MFRS422.h"
#include <string.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMFRS422::CMFRS422()
{
	//包头
	sendbuf[0] = 0x55;
	sendbuf[1] = 0xaa;

	//指令地址和内容
	for (int i=4;i<MF422BUFLEN;i++)
	{
		sendbuf[i] = 0;
	}

	//buf
	m_bFrameStart = false;
	m_nFrameLength = 0;
	m_nRcvIndex = 0;
	m_last = 0;

	//data init
	m_nUSInterval = 0;

	//us
	m_data53USonic[0] = m_data53USonic[1] = 0;
	m_b53USEnable = false;

	m_bCmdRsp = true;
}

CMFRS422::~CMFRS422()
{

}

void CMFRS422::Printf2X4LCD(const char *item, int insize)
{
	if (insize <0)
	{
		return;
	}
	if (insize > 0xff)
	{
		insize = 0xff;
	}

	sendbuf[2] = 0xC0;		//ID
	sendbuf[3] = insize;		//Len of Param
	sendbuf[4] = 0x05;		//Function modules(GPIO)
	sendbuf[5] = 0x20;		//Method(Direction)
	
	memcpy(&sendbuf[6],item,insize);		//param (inTxt)
	
	//cal sum
	m_CalBufSum(sendbuf,sendbuf[3]+7);
	
	Send((char*)sendbuf,sendbuf[3]+7);

	//////////////////////////////////////////////////////////////////////////
	//old
// 	sendbuf[2] = 0x52;		
// 	sendbuf[3] = (unsigned char)insize;		
// 	sendbuf[4] = 0x00;
// 	memcpy((&sendbuf[5]),item,insize);
// 	
// 	//cal sum
// 	m_CalBufSum(sendbuf,sendbuf[3]+6);
// 	
// 	Send(sendbuf,sendbuf[3]+6);
	//////////////////////////////////////////////////////////////////////////

//	fprintf(stderr,"CMFRS422::Printf2X4LCD\r\n");
}

void CMFRS422::DataRecv(unsigned char *inBuf, int inLen)
{
// 	fprintf(stderr,"MFRS422: recv Date :");
// 	for (int j=0;j<inLen;j++)
// 	{
// 		fprintf(stderr," %.2x",inBuf[j]);
// 	}
// 	fprintf(stderr,"\n\n");

	for (int i=0;i<inLen;i++)
	{
		m_ParseByte(inBuf[i]);
	}
}

void CMFRS422::m_ParseByte(unsigned char inchar)
{
	if (m_last == 0x55 && inchar == 0xaa && m_bFrameStart == false)
	{
		m_bFrameStart = true;
		parsbuf[0] = m_last;
		parsbuf[1] = inchar;
		m_nRcvIndex = 2;
		m_last=0x00;
		return;
	}

	if (m_bFrameStart)
	{
		//put received data into buffer
		parsbuf[m_nRcvIndex]=inchar;
		if (m_nRcvIndex==3)
		{
			m_nFrameLength=inchar+8;
		}
		m_nRcvIndex++;
		//receive one frame, invoke ParseFrame to parse
		if (m_nRcvIndex==m_nFrameLength)
		{ 
			m_ParseFrame(parsbuf, m_nRcvIndex); 
			m_ResetRcvBuf(); 
			m_nFrameLength = 6;
			m_bFrameStart = false;
		}

		//receive buffer overflow
		if (m_nRcvIndex>=MF422BUFLEN) 
		{
			m_ResetRcvBuf();
			m_nFrameLength = 5;
			m_bFrameStart = false;
		}
	}
	else
		m_last=inchar;
}

void CMFRS422::m_ParseFrame(unsigned char *inBuf, int inLen)
{
	m_bCmdRsp = true;
	//////////////////////////////////////////////////////////////////////////
// 	 	fprintf(stderr,"MFRS422::m_ParseFrame ");
// 	 	for (int i=0;i<inLen;i++)
// 	 	{
// 	 		fprintf(stderr,"%.2x ",inBuf[i]);
// 	 	}
// 	 	fprintf(stderr," \n");
	//////////////////////////////////////////////////////////////////////////
	switch(inBuf[2])	//addr
	{
	case 0x53:
		m_Parse0x53Dev();
		break;

	case 0x60:
		m_Parse0x60Dev();
		break;
	}
}

void CMFRS422::m_ResetRcvBuf()
{
	for (int i=0;i<MF422BUFLEN;i++)
	{
		parsbuf[i] = 0;
	}
}

void* CMFRS422::updateUSonicThread(void *pParam)
{
	CMFRS422* pcmd = (CMFRS422*) pParam;
		
	//fprintf(stderr,"pcmd->m_ADInterval=%d   pcmd->bToStop = %d",pcmd->m_ADInterval,pcmd->bToStop);

	while (pcmd->m_nUSInterval > 0 && pcmd->bToStop == false)
	{
		//fprintf(stderr,"for (i=0;i<MFADNO;i++)",i);
		pcmd->InqUSonic();
		
		//fprintf(stderr,"to Sleep(%d)\n",pcmd->m_ADInterval);
		Sleep(pcmd->m_nUSInterval);
	}
	return 0;
}

void CMFRS422::m_CalBufSum(unsigned char *inBuf, int inLen)
{
	inBuf[inLen-1] = 0;
	for (int i=0;i<inLen-1;i++)
	{
		inBuf[inLen-1]+= inBuf[i];
	}
}

void CMFRS422::InqUSonic()
{
	if (m_b53USEnable == false)
	{
		return;
	}

	sendbuf[2] = 0x60;		
	sendbuf[3] = 0x00;		
	sendbuf[4] = 0x05;
	sendbuf[5] = 0x00;

	//cal sum
	m_CalBufSum(sendbuf,7);

	//////////////////////////////////////////////////////////////////////////
// 	fprintf(stderr,"CMFRS422::InqUSonic: ");
// 	for (int i=0;i<7;i++)
// 	{
// 		fprintf(stderr,"%.2x ",sendbuf[i]);
// 	}
// 	fprintf(stderr," \n");
	//////////////////////////////////////////////////////////////////////////

	Send(sendbuf,7);
}

void CMFRS422::m_Parse0x53Dev()
{
	switch(parsbuf[4])
	{
	case 0x01:
		m_data53USonic[0] = parsbuf[5];
		m_data53USonic[1] = parsbuf[6];
		break;
	}
}

int CMFRS422::Get53USonic()
{
	m_wTemp = (WORD)m_data53USonic[0];
	m_wTemp <<= 8;
	m_wTemp &= 0xff00;
	m_wTemp |= (0x00ff & (WORD)m_data53USonic[1]);
	
	return (int)m_wTemp;
}

void CMFRS422::ScheUpdateUSonic(int interval)
{
	if (interval < 0)
	{
		interval = 0;
	}
	
	m_nUSInterval = interval*1000;
	
	pthread_create(&pidUpdateUSThread,NULL,updateUSonicThread,(void*)this);
}

void CMFRS422::USonicEnable()
{
	m_b53USEnable = true;
}	

void CMFRS422::m_Parse0x60Dev()
{
	switch(parsbuf[5])
	{
	case 0x00:
		m_data53USonic[0] = parsbuf[7];
		m_data53USonic[1] = parsbuf[8];
		break;
	}
}

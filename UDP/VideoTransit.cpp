// VideoTransit.cpp: implementation of the CVideoTransit class.
//
//////////////////////////////////////////////////////////////////////

#include "VideoTransit.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVideoTransit::CVideoTransit()
{
	m_bEnable = true;
	m_header[0] = 0x55;
	m_header[1] = 0xaa;
}

CVideoTransit::~CVideoTransit()
{

}

void CVideoTransit::ImgTrans(unsigned char *inBuf, int inWidth, int inHeight)
{
//	fprintf(stderr,"CVideoTransit::ImgTrans\n");
	Send(m_header,2);
	for(int i=0;i<450;i++)
	{
		//////////////////////////////////////////////////////////////////////////
		//m_Framebuf[0] = i;
		//memcpy(m_Framebuf+1,inBuf+1024*i,1024);
		//Send(m_Framebuf,1025);
		//////////////////////////////////////////////////////////////////////////
		Send(inBuf+512*i,512);
	}
}

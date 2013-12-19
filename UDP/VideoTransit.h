//////////////////////////////////////////////////////////////////////////
// for MultiflexNG
// http://robot.up-tech.com 
// zwj_uptech@126.com
//////////////////////////////////////////////////////////////////////////

#if !defined(AFX_VIDEOTRANSIT_H__6E7F3054_9F56_4F86_B088_F9C71C4D57C4__INCLUDED_)
#define AFX_VIDEOTRANSIT_H__6E7F3054_9F56_4F86_B088_F9C71C4D57C4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "UDPBasic.h"

class CVideoTransit : public CUDPBasic  
{
public:
	CVideoTransit();
	virtual ~CVideoTransit();

public:
	void ImgTrans(unsigned char* inBuf,int inWidth,int inHeight);
	bool m_bEnable;

protected:
	unsigned char m_Framebuf[1025];
	unsigned char m_header[2];
};

#endif // !defined(AFX_VIDEOTRANSIT_H__6E7F3054_9F56_4F86_B088_F9C71C4D57C4__INCLUDED_)

// MFRS422.h: interface for the CMFRS422 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MFRS422_H__7A12D71C_E066_42EA_9930_A36A355DA428__INCLUDED_)
#define AFX_MFRS422_H__7A12D71C_E066_42EA_9930_A36A355DA428__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\SERIALCOM\serial.h"

#define  MF422BUFLEN 64

class CMFRS422 : public CSerialCom  
{
public:
	void USonicEnable();
	void ScheUpdateUSonic(int interval);
	int Get53USonic();
	void InqUSonic();
	void Printf2X4LCD(const char*item,int insize);
	void DataRecv(unsigned char* inBuf,int inLen);
	CMFRS422();
	virtual ~CMFRS422();

	
protected:
	void m_Parse0x60Dev();
	void m_ResetRcvBuf();
	void m_ParseFrame(unsigned char *inBuf, int inLen);
	void m_ParseByte(unsigned char inchar);
	static void* updateUSonicThread(void *pParam);
	void m_Parse0x53Dev();
	void m_CalBufSum(unsigned char *inBuf, int inLen);
	
	//buf
	int m_nRcvIndex;
	int m_nFrameLength;

	unsigned char sendbuf[MF422BUFLEN];
	unsigned char parsbuf[MF422BUFLEN];
	
	char m_last;
	bool m_bFrameStart;

	//usonic
	bool m_b53USEnable;
	int m_nUSInterval;
	pthread_t pidUpdateUSThread;
	unsigned char m_data53USonic[2];
	WORD m_wTemp;
};

#endif // !defined(AFX_MFRS422_H__7A12D71C_E066_42EA_9930_A36A355DA428__INCLUDED_)

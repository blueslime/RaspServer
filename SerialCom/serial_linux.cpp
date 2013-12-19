#include "../debuglevel.h"
#include "../type.h"
#include "serial.h"
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pthread.h>
#include <termios.h>
#include <string.h>
#include <errno.h>

//#define BAUDRATE B19200
//#define BAUDRATE B115200

static char com_device[]="/dev/ttyS0";

BOOL CSerialCom::Create(int inCom , unsigned int inBaudrate)
{
	struct termios newtio;
	 if (inCom<0 || inCom > 9)
		 return FALSE;
	com_device[9] =  '0'+(char)inCom;
	 m_iCom = open(com_device, O_RDWR | O_NOCTTY ); // | O_NDELAY);
	 if(m_iCom < 0) {

	  fprintf(stderr,"Open com failed");

	  perror(com_device);
	  return FALSE;
	 }

	tcgetattr(m_iCom, &m_oldtio);

	bzero(&newtio,sizeof(struct termios));

	newtio.c_cflag|= (CLOCAL | CREAD);
	//newtio.c_cflag|=BAUDRATE;
	newtio.c_cflag|=inBaudrate;
	newtio.c_cflag&=~CSTOPB;
	newtio.c_cflag&=~PARENB;
	newtio.c_cflag&=~CSIZE;
	newtio.c_cflag|=CS8;
	newtio.c_cflag&=~CRTSCTS;

	newtio.c_lflag=0;

	newtio.c_oflag=0;

	newtio.c_cc[VMIN]=1;
	newtio.c_cc[VTIME]=0;

	newtio.c_iflag&=~(IXON|IXOFF|IXANY); 

	int st;
	//st = cfsetispeed(&newtio, BAUDRATE);
	st = cfsetispeed(&newtio, inBaudrate);
	if (st<0)
	{
		fprintf(stderr,"CSerialCom::Create: cfsetispeed err %d\n",st);
		return FALSE;
	}
	
	//st = cfsetospeed(&newtio, BAUDRATE);
	st = cfsetospeed(&newtio, inBaudrate);
	if (st<0)
	{
		fprintf(stderr,"CSerialCom::Create: cfsetospeed err %d\n",st);
		return FALSE;
	}

	tcsetattr(m_iCom, TCSANOW, &newtio);

	tcflush(m_iCom, TCIFLUSH);
	

	//create threads
	bToStop = FALSE;
	pthread_create(&pidSendThread,NULL,SendingThread,(void*)this);
	pthread_create(&pidMonitorThread,NULL,ComMonitor,(void*)this);

	fprintf(stderr,"CSerialCom::Create %s succeed!!! \n",com_device);

	return TRUE;
}

CSerialCom::CSerialCom()
	:bToStop(false), m_iCom(0)
{
	//pthread_mutex_init(&mutex, NULL);
	nMaxCnt =0;
	nWaitMax = 5000000;
	nSendCnt = 0;
	nRspCnt = 0;
	nCurCnt = 0;

	//pthread_mutex_init(&wLock,NULL);
	m_nGet = 0;
	m_nPut = m_nGet;

	bDisplaySendFrame = false;
	bDisplayRecvFrame = false;
	bDisplaySendByte = false;
	bDisplayRecvByte = false;
}

CSerialCom::~CSerialCom()
{
	CMDBUF *p;
	bToStop = true;
	if (m_iCom > 0)
	{
		pthread_cancel(pidMonitorThread);
		pthread_cancel(pidSendThread);
		close(m_iCom);
	}
	while (m_cmdlist.tryPop(p))
	{
		delete [] p->pCmdBuf;
		delete p;
	}
//	pthread_mutex_destroy(&mutex);
}

void CSerialCom::Close()
{
	CMDBUF *p;
	bToStop = TRUE;
	if (m_iCom > 0)
	{
		pthread_cancel(pidSendThread);
		pthread_cancel(pidMonitorThread);
		close(m_iCom);
		m_iCom = 0;
	}	
	while (m_cmdlist.tryPop(p))
	{
		delete [] p->pCmdBuf;
		delete p;
	}
}

static int sendBuffer(int fd, const unsigned char *buff, int len)
{
	/*/////////////////////////////////////////////////////////////////////////
	fprintf(stderr,"static int sendBuffer: ");
	for (int i=0;i<len;i++)
	{
		fprintf(stderr,"%.2x ",buff[i]);
	}
	fprintf(stderr,"   %dbytes",len);
	fprintf(stderr," \n");
	/////////////////////////////////////////////////////////////////////////*/
	int size;
	while (len > 0)
	{
		if ((size = write(fd, buff, len)) < 0)
		{
#ifdef __LINUX__
			if (errno == EINTR)
				continue;
			else
#endif
				return size;
		}
		else
		{
			len -= size;
			buff += size;
		}
	}
	return 0;
}

void CSerialCom::Send(const void *pBuffer, const int iLength)
{
	//////////////////////////////////////////////////////////////////////////
	/*/1、列表缓冲
	CMDBUF * m_pTempCmd = new CMDBUF;
	m_pTempCmd->pCmdBuf = new UCHAR[iLength];
	memcpy(m_pTempCmd->pCmdBuf,pBuffer,iLength);
	m_pTempCmd->nLen = iLength;

	//////////////////////////////////////////////////////////////////////////
// 	fprintf(stderr,"CSerialCom::Send: ");
// 	for (int i=0;i<iLength;i++)
// 	{
// 		fprintf(stderr,"%.2x ",m_pTempCmd->pCmdBuf[i]);
// 	}
// 	fprintf(stderr," \n");
	//////////////////////////////////////////////////////////////////////////

//	m_cmdqueue.push(m_pTempCmd);
	m_cmdlist.push(m_pTempCmd);


	/////////////////////////////////////////////////////////////////////////*/
	//2、环形缓冲
	//超过单个指令缓存大小的数据直接发送，避免破坏环形缓冲区
	if (iLength > RBUFLEN)
	{
		sendBuffer(m_iCom,(unsigned char*)pBuffer, iLength);
		fprintf(stderr,"too big to ring !!\n");
		return;
	}

	//正常处理流程
// 	if (m_nPut == m_nGet)
// 	{
// 		m_nPut ++;
// 	}
	
	memcpy(m_ringBuf+(m_nPut*RBUFLEN),pBuffer,iLength);
	m_nRCmdLen[m_nPut] = iLength;
	//////////////////////////////////////////////////////////////////////////
	//fprintf(stderr,"put data in %d now put++\n",m_nPut);

	//////////////////////////////////////////////////////////////////////////
	if (bDisplaySendFrame == true)
	{
		fprintf(stderr,"[Debug] CSerialCom::Send: ");
		for (int i=0;i<iLength;i++)
		{
			fprintf(stderr,"%.2x ",*(m_ringBuf+(m_nPut*RBUFLEN)+i));
		}
		fprintf(stderr,"   %dbytes",iLength);
		fprintf(stderr," \n");
	}
	//////////////////////////////////////////////////////////////////////////

	m_nPut ++;

	if (m_nPut >= RINGLEN)
	{
		//已经到环尾，将数据放到环首
		m_nPut = 0;
		//fprintf(stderr,"to ring end!!!!!!!!!!!!!!\n");
	}
	
	//fprintf(stderr,"m_nput = %d , m_nGet = %d\n",m_nPut,m_nGet);
	/////////////////////////////////////////////////////////////////////////*/

}

void *CSerialCom::ComMonitor(void *pParam)
{
	CSerialCom *pSCom = (CSerialCom *)pParam;
	unsigned char Buffer[512];
	int len = sizeof(Buffer);
	int ret;
	pthread_detach(pthread_self());
	while (!pSCom->bToStop)
	{
		//fprintf(stderr,"begain read\n");
		if ((ret = read(pSCom->m_iCom, Buffer, len)) < 0)
		{
#ifdef __LINUX__
			if (errno == EINTR)
				continue;
#endif
			//trace
			fprintf(stderr,"commonitor: read err!\n");
			perror("Com read error");
			continue;
		}
		else if (ret == 0)
		{
#ifdef DEBUG_LEVEL_2
			printf("Com: Read 0 byte\n");
#endif
		}
		else 
		{
#ifdef DEBUG_LEVEL_2
			fprintf(stderr,"Com: Read %d bytes\n", ret);
#endif
		//将接收的数据送回类解析
			pSCom->DataRecv(Buffer,ret);
		}
		//fprintf(stderr,"read %d bytes\n",ret);
	}
	return NULL;
}

// static void free_buffer(void *arg)
// {
// 	CMDBUF *p = (CMDBUF *)arg;
// 	delete [] p->pCmdBuf;
// 	delete p;
// }

void *CSerialCom::SendingThread(void *pParam)
{
	CSerialCom *pSCom = (CSerialCom *)pParam;
	pthread_detach(pthread_self());
	//CMDBUF *p;
	unsigned int nWait = 0;
	//////////////////////////////////////////////////////////////////////////
	//debug
//  	unsigned char lastcmd[32];
//  	int nlastlen = 0;
	//////////////////////////////////////////////////////////////////////////
	
	while (!pSCom->bToStop)
	{
		/************************************************************************/
		/*                                列表缓冲                              */
		/************************************************************************/
		/*p = pSCom->m_cmdlist.pop();	
		pthread_cleanup_push(free_buffer, (void *)p);

		#ifdef DEBUG_LEVEL_2
		printf("com: sending %d bytes\n", p->nLen);
		#endif

		//wait for cmdrespone
		while(pSCom->m_bCmdRsp == false)
		{
			nWait ++;
			//usleep(500);
			//////////////////////////////////////////////////////////////////////////
			//debug
// 			if (nWait > pSCom->nMaxCnt)
// 			{
// 				pSCom->nMaxCnt = nWait;
// 			}
// 			pSCom->nCurCnt++;
			//////////////////////////////////////////////////////////////////////////

			if (nWait > pSCom->nWaitMax)
			{
				break;
			}
		}
		//////////////////////////////////////////////////////////////////////////
		//debug
// 		if (nWait > pSCom->nWaitMax)
// 		{		
// 			fprintf(stderr,"time out cmd is");
// 			for (int i=0;i<nlastlen;i++)
// 			{
// 				fprintf(stderr," %.2x",lastcmd[i]);
// 			}
// 			fprintf(stderr,"\n");
//  		}
		//////////////////////////////////////////////////////////////////////////

		nWait = 0;

		//////////////////////////////////////////////////////////////////////////
		//debug
// 		pSCom->nCurCnt = 0;
// 		pSCom->nSendCnt++;
		//////////////////////////////////////////////////////////////////////////

		sendBuffer(pSCom->m_iCom, p->pCmdBuf, p->nLen);
		pSCom->m_bCmdRsp = false;

		//////////////////////////////////////////////////////////////////////////
		//debug
// 		memcpy(lastcmd,p->pCmdBuf, p->nLen);
// 		nlastlen = (int)p->nLen;
		//////////////////////////////////////////////////////////////////////////

		pthread_cleanup_pop(1);*/
//		usleep(50000);

		/************************************************************************/
		/*                                环形缓冲                              */
		/************************************************************************/
		/*/wait for cmdrespone
		nWait = 0;
		while(pSCom->m_bCmdRsp == false)
		{
			nWait ++;
			
			if (nWait > pSCom->nWaitMax)
			{
				break;
			}
		}
		//////////////////////////////////////////////////////////////////////////*/

		if (pSCom->m_nGet >= RINGLEN)
		{
			pSCom->m_nGet = 0;
		}
		
		if (pSCom->m_nGet != pSCom->m_nPut)
		{
// 			if(pSCom->m_nGet%20 == 0)
// 			{
// 				fprintf(stderr,"m_nGet = %d\n",pSCom->m_nGet);
// 			}
			sendBuffer(pSCom->m_iCom, (pSCom->m_ringBuf+(pSCom->m_nGet*RBUFLEN)), pSCom->m_nRCmdLen[pSCom->m_nGet]);
			//////////////////////////////////////////////////////////////////////////
			if (pSCom->bDisplaySendByte == true)
			{
				unsigned char *pbuf = (pSCom->m_ringBuf+(pSCom->m_nGet*RBUFLEN));
				unsigned int len = pSCom->m_nRCmdLen[pSCom->m_nGet];
				fprintf(stderr,"[Debug] sendBuffer:");
				for (int i=0;i<(int)len;i++)
				{
					fprintf( stderr," %.2x",*pbuf);
					pbuf ++;
				}
				fprintf(stderr,"\n");
			}
			//////////////////////////////////////////////////////////////////////////
			//fprintf(stderr,"Get data in %d   Then Get++\n",pSCom->m_nGet);
			//////////////////////////////////////////////////////////////////////////
//			usleep(10);
			pSCom->m_nRCmdLen[pSCom->m_nGet] = 0;
			pSCom->m_nGet++;
			//pSCom->m_bCmdRsp = false;
		}
		/************************************************************************/
		/*                                 环形缓冲                             */
		/************************************************************************/
	}	
	return NULL;
}

void CSerialCom::ResetCnt()
{
	nSendCnt = 0;
	nRspCnt = 0;
}

bool CSerialCom::BufEmpty()
{
	if (m_nGet == m_nPut)
	{
		return true;
	}
	return false;
}


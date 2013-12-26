
#include "background.h"

#include "CamVision/capture.h"
#include "Command/command.h"
//#include "SR/speechrecogn.h"
#include "TCP/TCPLin.h"
//#include "Mp3_Play/mp3_play.h"
#include "Dev/sound.h"
//#include "RS422/MFRS422.h"
#include "UDP/UDPBasic.h"
#include "UDP/VideoTransit.h"
#include <signal.h>

/************************************************************************/

static CTCPLin MF_Report;

static CCapture MF_Cap;

static CMFCmd MF_Cmd;

//static SpeechRecognition MF_SR;

//static CMFRS422 MF_RS422;

static CUDPBasic MF_Udp;

static CVideoTransit MF_VideoTransit;

//mp3
static char mp3_file[64];

//servo
//static int tmpServo;

/************************************************************************/
//////////////////////////////////////////////////////////////////////////
void MFInit()
{
	fprintf(stderr,"[MFInit] Begin...\n");
	MF_Cmd.Create(2,B115200);				//与单片机通讯

//	MF_RS422.Create(1,B115200);				//rs422

	MF_Report.Listen(1000);			//打开网络后台监听	
	MF_Cap.pReport = &MF_Report;

	//MF_VideoTransit.Listen(1100);

//	MF_Cmd.SetServoPos(0xfe,512,1);

// 	MF_Cmd.ScheUpdateAD(100);
// 	MF_Cmd.ScheUpdateInput(100);
// 	MF_Cmd.ScheUpdateServoPos(100);
//	MF_RS422.ScheUpdateUSonic(200);

//	MF_Cmd.ScheUpdateAll(200);
	MFADInit(100);
	Sleep(200);
	MFDigiInit(100);
	
	fprintf(stderr,"[MFInit] End!\n\n");
}

//////////////////////////////////////////////////////////////////////////
//LCD
void MF2X4LCD(const char*item,int insize)
{
//	MF_RS422.Printf2X4LCD(item,insize);
}

//////////////////////////////////////////////////////////////////////////
//USonic
void MFUSonicEnable()
{
//	MF_RS422.USonicEnable();
//	MF_RS422.ScheUpdateUSonic(100);
}

int MFGetUltrasonic()
{	
//	fprintf(stderr,"cmd.maxcnt = %d  422.maxcnet = %d!\n",MF_Cmd.maxcnt,MF_RS422.maxcnt);
//	return MF_RS422.Get53USonic();
}

//////////////////////////////////////////////////////////////////////////
//Delay
void DelayMS(int inMS)
{
	Sleep(inMS*1000);
}

// void AlarmHandler(int iAlarm)
// {
//         return;
// }

/*static struct itimerval tnew;
static struct itimerval told;
static long savemask;
void DelayMS(int inMS)
{

        (void)signal(SIGALRM,AlarmHandler);
        savemask=sigblock(sigmask(SIGALRM));
        tnew.it_value.tv_sec=0;
        tnew.it_value.tv_usec=inMS;
        tnew.it_interval.tv_sec=0;
        tnew.it_interval.tv_usec=0;
        setitimer(ITIMER_REAL,&tnew,&told);
        sigpause(savemask);
        sigsetmask(0);
        return;
}*/


// static struct timeval tv;
// void DelayMS(int inMS) 
// {
// 	tv.tv_sec = 0;
// 	tv.tv_usec = inMS*1000;
// 	select(0, NULL, NULL, NULL, &tv);
// 	return;
// }

//////////////////////////////////////////////////////////////////////////
void MFComSend(const void *pBuffer, const int iLength)
{
	MF_Cmd.Send(pBuffer,iLength);
}

//////////////////////////////////////////////////////////////////////////
//Digi. IO
void MFDigiInit(int interval)
{
	//MF_Cmd.ScheUpdateInput(interval);
	MF_Cmd.ASynUpdateInput(interval);
}

void MFSetPortDirect(unsigned int inData)
{
	unsigned char direction[2];
	direction[1] = (unsigned char)(0x000000ff&inData);
	direction[0] = (unsigned char)(0x000000ff&(inData>>8));

	MF_Cmd.SetDigiDirection(direction);
}

int MFGetDigiInput(int inID)
{
	bool ret = MF_Cmd.GetDigiInputVal(inID);

	if (ret == true)
	{
		return 1;
	} 
	else
	{
		return 0;
	}
}

void MFDigiOutput(int inID,int inVal)
{
	MF_Cmd.PutDigiOutput(inID,inVal);
}

//////////////////////////////////////////////////////////////////////////
//AD
void MFADInit(int inID,int interval)
{
	//MF_Cmd.ScheUpdateAD(interval);
	MF_Cmd.ASynUpdateAD(inID,interval);
}

void MFADInit(int interval)
{
	//MF_Cmd.ScheUpdateAD(interval);
	MF_Cmd.ASynUpdateAD(interval);
}

void MFADEnable(int inID)
{
	MF_Cmd.ADEnable(inID);
}

int MFGetAD(int inID)
{
	return MF_Cmd.GetADInputVal(inID);
}

//////////////////////////////////////////////////////////////////////////
//Servo
void MFServoInit(int interval)
{
	MF_Cmd.ScheUpdateServoPos(interval);
}

void MFSetServoMode(int inID,int inMode)
{
	MF_Cmd.SetServoMode(inID,inMode);
}

void MFSetServoPos(int inID,int inPos,int inSpeed)
{
	//同步模式
	//MF_Cmd.SetServoPos(inID,inPos,inSpeed);

	//异步模式
	MF_Cmd.ASynSetServoPos(inID,inPos,inSpeed);
}

void MFSetServoRotaSpd(int inID,int inSpeed)
{
	//同步模式
	//MF_Cmd.SetServoRotaSpd(inID,inSpeed);

	//异步模式
	MF_Cmd.ASynSetServoRotaSpd(inID,inSpeed);
}

void MFServoAction()
{
	//MF_Cmd.ServoAction();
	MF_Cmd.ASynServoAction();
}

int MFGetServoPos(int inID)
{
	return MF_Cmd.GetServoPos(inID);
}

//////////////////////////////////////////////////////////////////////////
bool MFCapOpen()
{
	MF_Cap.Create();
	DelayMS(1000);
	fprintf(stderr,"[MFCapOpen]\n");
	return true;
}

void MFCapClose()
{
	MF_Cap.Stop();
	fprintf(stderr,"[MFCapClose]\n");
}

void MFCapSetH(int inHmax,int inHmin)
{
	MF_Cap.SetHMax(inHmax);
	MF_Cap.SetHMin(inHmin);
}

void MFCapSetS(int inSmax,int inSmin)
{
	MF_Cap.SetSMax(inSmax);
	MF_Cap.SetSMin(inSmin);
}

void MFCapSetI(int inImax,int inImin)
{
	MF_Cap.SetIMax(inImax);
	MF_Cap.SetIMin(inImin);
}

void MFCapSetMode(int inMode)
{
	MF_Cap.SetMode(inMode);
}

int MFCapGetCenterX()
{
	return MF_Cap.m_x;
}

int MFCapGetCenterY()
{
	return (240 - MF_Cap.m_y);
}

int MFCapGetSum()
{
	return MF_Cap.m_sum;
}

void MFCapImgCallback(PIMGREC inFunc)
{
	MF_Cap.pBufferCB = inFunc;
	MF_Cap.SetMode(2);
}

void MFCapConnect(char* inTarIP,int inPort)
{
	MF_Cap.pVT = &MF_VideoTransit;
	//debug
	MF_VideoTransit.Connect(inTarIP,inPort);
}
//////////////////////////////////////////////////////////////////////////

void MFSRStart()
{
/*
	usleep(200000);
	fprintf(stderr,"MFSRStart!!!!!\n");
	MF_SR.start();
	usleep(1000000);
*/
}

void MFSRStop()
{
/*
	MF_SR.stop();
	usleep(1000000);
*/
}

bool MFSRPause()
{
/*
	bool ret = MF_SR.Pause();
	usleep(200000);
	return ret;
*/
}

bool MFSRContinue()
{
/*
	usleep(100000);
	bool ret = MF_SR.Continue();
	usleep(100000);
	return ret;
*/
}

bool MFSRInsertText(int index,const char*item,int insize)
{
/*
	MF_SR.insertItem(index,item,insize);
*/	return true;

}

int MFSRGetResIndex()
{

	int res = -1;
	/*if(isMP3Playing() == 0)
	{
		res = MF_SR.GetResultIndex();
	}*/
	return res;
}

void MFSRClearItems()
{
//	MF_SR.clear();
}

bool MFSRisStarted()
{
//	return MF_SR.isStarted();
return true;
}

//////////////////////////////////////////////////////////////////////////
void MFMp3Play(const char*item,int insize)
{
/*
	MFSRPause();
	if (strcmp(mp3_file,item) == 0)
	{
		//is the same file
		if (isMP3Playing() == 0)
		{
			//replay it
			player_init();
			play(mp3_file);
		}
		return;
	} 
	else
	{
		//not the same file, stop
		if (isMP3Playing() != 0)
		{
			//MFCloseSound();
			stop_play(3000000);
		} 

		player_init();
		memcpy(mp3_file,item,insize);
		mp3_file[insize] = '\0';
		//play
		play(mp3_file);
	}
*/
}

void MFMp3Stop()
{
/*
	//MFCloseSound();
	stop_play(100000);

	//restart sr,if necessary
	if (MFSRisStarted() == true)
	{
		bool MFSRContinue();
	}
*/
}
//////////////////////////////////////////////////////////////////////////
//UDP
int UDPConnect(char *inTarIP, int inTarPort)
{
	return MF_Udp.Connect(inTarIP,inTarPort);
}

void UDPSend(unsigned char *inbuf, int inLen)
{
	MF_Udp.Send(inbuf,inLen);
}

int UDPListen(int inPort)
{
	return MF_Udp.Listen(inPort);
}

void UDPRecvCallback(PBUFREC inFunc)
{
	MF_Udp.pRec = inFunc;
}

//////////////////////////////////////////////////////////////////////////
//EXT
int MFGetFromAddr(unsigned char inAddr)
{	
	return MF_Cmd.GetFromAddr(inAddr);
}

void MFSetToAddr(unsigned char inAddr,unsigned char inVal)
{
	MF_Cmd.SetValTo(inAddr,inVal);
}

//////////////////////////////////////////////////////////////////////////
int GetCmdVaildprc()
{
	if (MF_Cmd.nSendCnt == 0)
	{
		return -1;
	}
	int ret = (int)(MF_Cmd.nRspCnt*100/MF_Cmd.nSendCnt);
	MF_Cmd.ResetCnt();
	return ret;
}

int GetCmdWaitMax()
{
	return MF_Cmd.nMaxCnt;
}

int GetCmdCurCnt()
{
	return MF_Cmd.nCurCnt;
}

bool ComCreat(int inNo,unsigned int inBaudrate)
{
	bool ret=false;
	switch(inNo)
	{
	case 1:
		//MF_RS422.Close();
		//ret = MF_RS422.Create(1,inBaudrate);
		break;

	case 2:
		MF_Cmd.Close();
		ret = MF_Cmd.Create(2,inBaudrate);
		break;
	}
	return ret;
}

void DisplaySendFrame(bool inFlag)
{
	MF_Cmd.bDisplaySendFrame = inFlag;
}

void DisplaySendBytes(bool inFlag)
{
	MF_Cmd.bDisplaySendByte = inFlag;
}

void DisplayRecvFrame(bool inFlag)
{
	MF_Cmd.bDisplayRecvFrame = inFlag;
}

void DisplayRecvBytes(bool inFlag)
{
	MF_Cmd.bDisplayRecvByte = inFlag;
}

int GetRecvFPS(int inSec)
{
	//fprintf(stderr,"%d \n",MF_Cmd.nRspCnt);
	MF_Cmd.nRspCnt = 0;
	DelayMS(inSec*1000);

	int rsFPS = MF_Cmd.nRspCnt / inSec;

	return rsFPS;
}

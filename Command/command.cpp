//////////////////////////////////////////////////////////////////////////
// for MultiflexNG
// http://robot.up-tech.com 
// zwj_uptech@126.com
//////////////////////////////////////////////////////////////////////////
#include <string.h>
#include "command.h"

CMFCmd::CMFCmd()
{

#ifdef DEBUG_LEVEL_2
	fprintf(stderr,"MFCmd: creat!\n");
#endif

	//包头
	sendbuf[0] = 0x55;
	sendbuf[1] = 0xaa;
	sendbuf[2] = 0x71;
	sendbuf[3] = 0x24;

	//指令地址和内容
	for (int i=4;i<MFBUFLEN;i++)
	{
		sendbuf[i] = 0;
	}

	//buf
	m_bFrameStart = false;
	m_nFrameLength = 0;
	m_nRcvIndex = 0;
	m_last = 0;

	//data init
	m_inputInterval = 0;
	m_ADInterval = 0;
	m_ServoInterval = 0;
	m_nUpdateAllInterval = 0;

	m_dataDigiInput[0] = 0xff;
	m_dataDigiInput[1] = 0xff;
	m_dataDigiOutput[0] = 0xff;
	m_dataDigiOutput[1] = 0xff;

	for (int i=0;i<MFIOMAX;i++)
	{
		m_nBakOupVal[i] = 2;
	}

	for (int i=0;i<MFADNO;i++)
	{
		m_dataADInputH[i] = 0;
		m_dataADInputL[i] = 0;
		m_arADEnable[i] = false;
	}

	for (int i=0;i<MFSERVOMAX;i++)
	{
		m_dataServoPosH[i] = 0;
		m_dataServoPosL[i] = 0;
		m_dataServoLoadH[i] = 0;
		m_dataServoLoadL[i] = 0;
		m_arServoEnable[i] = false;

		m_nBakSerPos[i] = 0;
		m_nBakSerPosSpd[i] = 0;
		m_nBakSerRotSpd[i] = 0;

		m_nAsynServoMode[i] = -1;
		m_nAsynSerPosToSet[i] = 0;
		m_nAsynSerSpdToSet[i] = 0;
		m_bAsynServoChanged[i] = false;
	}
	for (int i=0;i<MFEXTDATA;i++)
	{
		m_ExtData[i] = 0;
		m_ExtEnable[i] = false;
	}

	m_bCmdRsp = true;
	nWaitMax = 5000000;
	m_bDigiInputEnable = false;
}

CMFCmd::~CMFCmd()
{
	pthread_cancel(pidInputThread);
	pthread_cancel(pidADThread);
	pthread_cancel(pidServoThread);
}

void CMFCmd::DataRecv(unsigned char *inBuf, int inLen)
{
	if (bDisplayRecvByte == true)
	{
		fprintf(stderr,"[Debug] MFCmd: recv Date :");
		for (int j=0;j<inLen;j++)
		{
			fprintf(stderr," %.2x",inBuf[j]);
		}
		fprintf(stderr,"\n\n");
	}

	for (int i=0;i<inLen;i++)
	{
		m_ParseByte(inBuf[i]);
	}
}

void CMFCmd::Servo(int *inAngle, int *inSpeed)
{
// 	#ifdef DEBUG_LEVEL_2
// 	fprintf(stderr,"MFCmd: servo\n");
// 	#endif
// 	//参数检查
// 	for (int i=0;i<MFSERVONO;i++)
// 	{
// 		if (inAngle[i]>90)
// 		{
// 			inAngle[i] = 90;
// 		}
// 		if (inAngle[i]<-90)
// 		{
// 			inAngle[i] = -90;
// 		}
// 		if (inSpeed[i] <0)
// 		{
// 			inSpeed[i] = 0;
// 		}
// 		if (inSpeed[i] > 0xff)
// 		{
// 			inSpeed[i] = 0xff;
// 		}
// 	}
// 	//生成控制指令
// 	sendbuf[3] = 0x18;
// 	sendbuf[4] = 0x01;
// 	for (int i=0;i<MFSERVONO;i++)
// 	{
// 		sendbuf[5+2*i] = (char)(inAngle[i]+90);
// 		sendbuf[6+2*i] = (char)(inSpeed[i]);
// 	}
//	Send(sendbuf,30);
}

void CMFCmd::DCMotors(int* inRotSpd, int* inDur)
{
	#ifdef DEBUG_LEVEL_2
	fprintf(stderr,"MFCmd: DCMotor\n");
	#endif
	//参数检查
	for (int i=0;i<MFMOTORNO;i++)
	{
		if (inRotSpd[i] > 128)
		{
			inRotSpd[i] = 128;
		}
		if (inRotSpd[i] < -126)
		{
			inRotSpd[i] = -126;
		}
		if (inDur[i] < 0)
		{
			inDur[i] = 0;
		}
		if (inDur[i] > 0xff)
		{
			inDur[i] = 0xff;
		}
	}
	//生成指令
	sendbuf[3] = 0x08;
	sendbuf[4] = 0x02;
	for (int i=0;i<MFMOTORNO;i++)
	{
		if (inRotSpd[i] < 0)
		{
			sendbuf[5+2*i] = ((char)(-inRotSpd[i]))|0x80;
		}
		else
		{
			sendbuf[5+2*i] = (char)(inRotSpd[i]);
		}
		sendbuf[6+2*i] = (char)(inDur[i]);
	}
	Send(sendbuf,13);
}

void CMFCmd::m_ParseByte(unsigned char inchar)
{
	//fprintf(stderr,"CMFCmd::m_ParseByte %.2x\n",inchar);

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
		if (m_nRcvIndex>=MFBUFLEN) 
		{
			m_ResetRcvBuf();
			m_nFrameLength = 5;
			m_bFrameStart = false;
		}
	}
	else
		m_last=inchar;
}

void CMFCmd::m_ParseFrame(unsigned char *inBuf, int inLen)
{
// 	if (!m_ValidFrame(inBuf,inLen))
// 	{
// 		m_ResponseError();
// 		return;
// 	}
	nRspCnt ++;
	//fprintf(stderr,"nRspCnt ++;  %d\n",nRspCnt);
	m_bCmdRsp = true;
	//////////////////////////////////////////////////////////////////////////
	if (bDisplayRecvFrame == true)
	{
		fprintf(stderr,"[Debug] CMFCmd::m_ParseFrame ");
		for (int i=0;i<inLen;i++)
		{
			fprintf(stderr,"%.2x ",inBuf[i]);
		}
		fprintf(stderr," \n");
	}
	//////////////////////////////////////////////////////////////////////////

	bool Devcodevalid=true;
	switch (inBuf[2]) //Dev ID
	{
	case 0x05:	
		break;

	case 0x06:	
		break;

	case 0x30:	//Multiflex
		m_Parse0x30Dev();
		break;

	default:
		{
			Devcodevalid=false;
			break;
		}
	}

// 	if (ctrlcodevalid)
// 		m_Response();
// 	else
// 		m_ResponseError();
}

void CMFCmd::m_ResetRcvBuf()
{
	for (int i=0;i<MFBUFLEN;i++)
	{
		parsbuf[i] = 0;
	}
}

void CMFCmd::ScheUpdateInput(int interval)
{
	//printf("MFCmd: ScheUpdateInput\n");

	if (interval < 0)
	{
		interval = 0;
	}

	m_inputInterval = interval*1000;

	pthread_create(&pidInputThread,NULL,updateInputThread,(void*)this);
}

void CMFCmd::ASynUpdateInput(int interval)
{
	if (interval < 0)
	{
		return;
	}

	sendbuf[2] = 0x30;		//ID
	sendbuf[3] = 0x01;		//Len of Param
	sendbuf[4] = 0x06;		//Function modules(GPIO)
	sendbuf[5] = 0x60;		//Method
	
	//Param
	sendbuf[6] = interval/10;

	//cal sum
	m_CalBufSum(sendbuf,7+sendbuf[3]);
	
	Send(sendbuf,7+sendbuf[3]);
}

void *CMFCmd::updateInputThread(void *pParam)
{
	CMFCmd* pcmd = (CMFCmd*) pParam;
	int nInputInterval = pcmd->m_inputInterval;
	while (pcmd->m_inputInterval > 0 && pcmd->bToStop == false && nInputInterval == pcmd->m_inputInterval)
	{
		pcmd->UpdateInput();
		Sleep(pcmd->m_inputInterval);
	}
	return NULL;
}

void CMFCmd::UpdateInput()
{
	//fprintf(stderr,"MFCmd: UpdateInput\n");
	if (m_bDigiInputEnable == false)
	{
		return;
	}

	sendbuf[2] = 0x30;		//ID
	sendbuf[3] = 0x00;		//Len of Param
	sendbuf[4] = 0x06;		//Function modules(GPIO)
	sendbuf[5] = 0x00;		//Method

	//cal sum
	m_CalBufSum(sendbuf,7);

	Send(sendbuf,7);
}

bool CMFCmd::GetDigiInputVal(int inNo)
{
	if (inNo <0 || inNo > MFIOMAX-1)
	{
		return false;
	}

	unsigned char temp;
	int i = 0;
	if (inNo > 7)
	{
		temp = m_dataDigiInput[0];
		i = inNo-8;
	} 
	else
	{
		temp = m_dataDigiInput[1];
		i = inNo;
	}
	
	//fprintf(stderr,"input%d temp = %.2x",inNo,temp);
	
	temp = temp>>i;

	if ((temp&0x01) == 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

int CMFCmd::GetADInputVal(int inNo)
{
	if (inNo <0 && inNo >7)
	{
		return 0;
	}
	m_wTemp = (WORD)m_dataADInputH[inNo];
	m_wTemp <<= 8;
	m_wTemp &= 0xff00;
	m_wTemp |= (0x00ff & (WORD)m_dataADInputL[inNo]);
	
	int adval = (int)m_wTemp;

	//debug
	//fprintf(stderr,"ad=%d, m_wtemp=%d !!\n",adval,m_wTemp);

	return adval;
}

void CMFCmd::UpdateAD(int inID)
{

	//fprintf(stderr,"MFCmd: UpdateAD %d\n",inID);
	
	if (inID<0 || inID>(MFADNO-1))
	{
		return;
	}

	if (m_arADEnable[inID] == false)
	{
		return;
	}

	sendbuf[2] = 0x30;		//ID
	sendbuf[3] = 0x01;		//Len of Param
	sendbuf[4] = 0x07;		//Function modules
	sendbuf[5] = 0x00;		//Method
	sendbuf[6] = (unsigned char) inID;		//param (ADID)

	//cal sum
	m_CalBufSum(sendbuf,8);

	Send(sendbuf,8);
}

void CMFCmd::ScheUpdateAD(int interval)
{
	//fprintf(stderr,"MFCmd: ScheUpdateAD\n");
	
	if (interval < 0)
	{
		interval = 0;
	}

	m_ADInterval = interval*1000;

	pthread_create(&pidADThread,NULL,updateADThread,(void*)this);
}

void* CMFCmd::updateADThread(void *pParam)
{
	CMFCmd* pcmd = (CMFCmd*) pParam;
	int i;
	int nADinterval = pcmd->m_ADInterval;
	//fprintf(stderr,"pcmd->m_ADInterval=%d   pcmd->bToStop = %d",pcmd->m_ADInterval,pcmd->bToStop);

	while (pcmd->m_ADInterval > 0 && pcmd->bToStop == false && nADinterval == pcmd->m_ADInterval)
	{
		//fprintf(stderr,"for (i=0;i<MFADNO;i++)",i);
		
		for (i=0;i<MFADNO;i++)
		{
			
		//	fprintf(stderr,"pcmd->UpdateAD(%d)\n",i);

			pcmd->UpdateAD(i);
			if (pcmd->m_ADInterval <= 0 || pcmd->bToStop == true || nADinterval != pcmd->m_ADInterval)
			{
					
				//fprintf(stderr,"pcmd->m_ADInterval <= 0 || pcmd->bToStop == true\n");

				return NULL;
			}
		}
		//fprintf(stderr,"to Sleep(%d)\n",pcmd->m_ADInterval);
		Sleep(nADinterval);
	}
	return NULL;
}

void CMFCmd::ASynUpdateAD(int inID,int interval)
{
	if (inID<0 || inID>(MFADNO-1))
	{
		return;
	}
	
// 	if (m_arADEnable[inID] == false)
// 	{
// 		return;
// 	}
	
	sendbuf[2] = 0x30;		//ID
	sendbuf[3] = 0x02;		//Len of Param
	sendbuf[4] = 0x07;		//Function modules
	sendbuf[5] = 0x60;		//Method
	sendbuf[6] = (unsigned char) inID;		//param (ADID)
	sendbuf[7] = interval/10;
	
	//cal sum
	m_CalBufSum(sendbuf,sendbuf[3]+7);
	
	Send(sendbuf,sendbuf[3]+7);
}

void CMFCmd::ASynUpdateAD(int interval)
{
	//fprintf(stderr,"CMFCmd::ASynUpdateAD(int interval)\n");
	sendbuf[2] = 0x30;		//ID
	sendbuf[3] = 0x02;		//Len of Param
	sendbuf[4] = 0x07;		//Function modules
	sendbuf[5] = 0x61;		//Method

	//channel id config
	unsigned char ch = 0;
	for (int i=0;i<8;i++)
	{
		ch>>=1;
		if (m_arADEnable[i] == true)
		{
			//fprintf(stderr,"AD %d enable\n",i);
			ch |= 0x80;
		}
	}
	sendbuf[6] = ch;
	sendbuf[7] = interval/10;
	
	//cal sum
	m_CalBufSum(sendbuf,sendbuf[3]+7);
	
	Send(sendbuf,sendbuf[3]+7);
}

void CMFCmd::m_CalBufSum(unsigned char *inBuf, int inLen)
{
	inBuf[inLen-1] = 0;
	for (int i=0;i<inLen-1;i++)
	{
		inBuf[inLen-1]+= inBuf[i];
	}
}

void CMFCmd::m_Parse0x30Dev()
{
	switch(parsbuf[6])	//状态位
	{
	case 0x00:
		break;

	case 0x09:	//error!
// 		fprintf(stderr,"recieve err msg!    ");
// 		for (int i=0;i<m_nRcvIndex;i++)
// 		{
// 			fprintf(stderr,"%.2x ",parsbuf[i]);
// 		}
// 		fprintf(stderr," \n");
		return;
		break;

	default :
	//	fprintf(stderr,"CMFCmd::m_Parse0x30Dev : unknow Status = %d\n\n",parsbuf[6]);
		break;
	}
	
	switch (parsbuf[4]) //功能模块
	{
	case 0x00:	//系统信息
		break;

	case 0x01:	//系统状态
		break;

	case 0x02:	//保留
		break;

	case 0x03:	//保留
		break;

	case 0x04:	//RS232
		break;

	case 0x05:	//RS422
		break;

	case 0x06:	//GPIO
		switch (parsbuf[5])
		{
		case 0x00:
			m_dataDigiInput[0] = parsbuf[7];
			m_dataDigiInput[1] = parsbuf[8];
			
			//fprintf(stderr,"MFCmd: DigiInput value is %.2x%.2x\n",m_dataDigiInput[0],m_dataDigiInput[1]);
			break;

		case 0x60:	//异步读取
			m_dataDigiInput[0] = parsbuf[7];
			m_dataDigiInput[1] = parsbuf[8];
			
			//fprintf(stderr,"MFCmd: DigiInput value is %.2x%.2x\n",m_dataDigiInput[0],m_dataDigiInput[1]);
			break;
		}
		//////////////////////////////////////////////////////////////////////////
// 		fprintf(stderr,"MFCmd: DigiInput ");
// 		for (int i=0;i<parsbuf[3]+8;i++)
// 		{
// 				fprintf(stderr,"%.2x ",parsbuf[i]);
// 		}
// 		fprintf(stderr," \n");
		//////////////////////////////////////////////////////////////////////////

		break;

	case 0x07:	//ADC
		//fprintf(stderr,"MFCmd: ADC\n");
		switch(parsbuf[5])
		{
		case 0x00:	//读取
			m_dataADInputH[parsbuf[7]] = parsbuf[8];			
			m_dataADInputL[parsbuf[7]] = parsbuf[9];

			//fprintf(stderr,"MFCmd: ADC-Input\n");
			break;

		case 0x60:	//异步读取
			m_dataADInputH[parsbuf[7]] = parsbuf[8];			
			m_dataADInputL[parsbuf[7]] = parsbuf[9];
			
			//fprintf(stderr,"MFCmd: Asyn ADC-Input\n");
			break;

		case 0x61:	//多路异步读取
			//fprintf(stderr,"MFCmd: Asyn Multi-ADC-Input\n");
			unsigned char ch = parsbuf[7];
			unsigned char tmp;
			int dataIndex = 8;
			//fprintf(stderr,"MFCmd: Asyn Multi-ADC-Input: ch=%x\n",ch);
			for(int i=0;i<8;i++)
			{
				tmp = (ch&0x01);
				//fprintf(stderr,"MFCmd: Asyn Multi-ADC-Input: ch=%x  tmp=%x\n",ch,tmp);
				if (tmp != 0)
				{
					//fprintf(stderr,"AD %d recv \n",i);
					m_dataADInputH[i] = parsbuf[dataIndex];			
					m_dataADInputL[i] = parsbuf[dataIndex+1];
					//m_arADEnable[i] = true;
					dataIndex += 2;
				}
				ch >>= 1;
			}
			break;
		}
		break;
	
	case 0x08:	//Servo
		if (parsbuf[7] > MFSERVOMAX-1)
		{
			break;
		}
		switch (parsbuf[5])
		{
		case 0x00:	//读取
			m_dataServoPosH[parsbuf[7]] = parsbuf[8];
			m_dataServoPosL[parsbuf[7]] = parsbuf[9];
			//fprintf(stderr,"MFCmd: ServoPos: ID=%d   Pos = %d\n",parsbuf[7],GetServoPos(parsbuf[7]));
			break;
			
		case 0x02:		
			m_dataServoLoadH[parsbuf[7]] = parsbuf[8];
			m_dataServoLoadL[parsbuf[7]] = parsbuf[9];
			//fprintf(stderr,"MFCmd: ServoLoad: ID=%d   Load = %d\n",parsbuf[7],GetServoLoad(parsbuf[7]));
			break;
		}
		break;


	default:
		break;
	}
}

void CMFCmd::PutDigiOutput(int inID, int inVal)
{
	if (inID <0 || inID >= MFIOMAX)
	{
		return;
	}
	if (inVal > 0)
	{
		inVal = 1;
	}
	else
	{
		inVal = 0;
	}

	if (m_nBakOupVal[inID] == inVal)
	{
		return;
	}
	m_nBakOupVal[inID] = inVal;

	unsigned char temp = 0x01;
	int cnt = inID;

	if (inID > 7)
	{
		cnt -= 8;
	}

	temp = temp << cnt;

	if (inVal == 0)
	{
		if (inID > 7)
		{
			m_dataDigiOutput[0] &= ~(temp);
		} 
		else
		{			
			m_dataDigiOutput[1] &= ~(temp);
		}
	} 
	else
	{		
		if (inID > 7)
		{
			m_dataDigiOutput[0] |= temp;
		} 
		else
		{			
			m_dataDigiOutput[1] |= temp;
		}
	}

	sendbuf[2] = 0x30;		//ID
	sendbuf[3] = 0x02;		//Len of Param
	sendbuf[4] = 0x06;		//Function modules(GPIO)
	sendbuf[5] = 0x21;		//Method(out)
	
	memcpy(&sendbuf[6],m_dataDigiOutput,2);		//param (Digi.Output)

	//cal sum
	m_CalBufSum(sendbuf,9);

	Send(sendbuf,9);
}

void CMFCmd::SetDigiDirection(unsigned char *inDir)
{	
	//fprintf(stderr,"CMFCmd::SetDigiDirection\n");
	sendbuf[2] = 0x30;		//ID
	sendbuf[3] = 0x02;		//Len of Param
	sendbuf[4] = 0x06;		//Function modules(GPIO)
	sendbuf[5] = 0x20;		//Method(Direction)
	
	memcpy(&sendbuf[6],inDir,2);		//param (Digi.Output)

	//cal sum
	m_CalBufSum(sendbuf,9);

	Send(sendbuf,9);

	if (inDir[0]&0x0f!=0x0f || inDir[1]!=0xff)
	{
		m_bDigiInputEnable = true;
	}
	else
	{
		m_bDigiInputEnable = false;
	}
}

void CMFCmd::ScheUpdateServoPos(int interval)
{	
	//printf("MFCmd: ScheUpdateServoPos\n");
	
	if (interval < 0)
	{
		interval = 0;
	}

	m_ServoInterval = interval*1000;

	pthread_create(&pidServoThread,NULL,updateServoPosThread,(void*)this);
}

void CMFCmd::UpdateServoPos(int inID)
{
	sendbuf[2] = 0x30;		//ID
	sendbuf[3] = 0x01;		//Len of Param
	sendbuf[4] = 0x08;		//Function modules (Servo)
	sendbuf[5] = 0x00;		//Method
	sendbuf[6] = (unsigned char) inID;		//param (ServoID)

	//cal sum
	m_CalBufSum(sendbuf,8);

	Send(sendbuf,8);

	//debug
	//fprintf(stderr,"UpdateServoPos(%d)\n",inID);
}

void CMFCmd::UpdateServoLoad(int inID)
{
	sendbuf[2] = 0x30;		//ID
	sendbuf[3] = 0x01;		//Len of Param
	sendbuf[4] = 0x08;		//Function modules (Servo)
	sendbuf[5] = 0x02;		//Method
	sendbuf[6] = (unsigned char) inID;		//param (ServoID)
	
	//cal sum
	m_CalBufSum(sendbuf,8);
	
	Send(sendbuf,8);

	//debug
	//fprintf(stderr,"UpdateServoLoad(%d)\n",inID);
}

void* CMFCmd::updateServoPosThread(void *pParam)
{
	CMFCmd* pcmd = (CMFCmd*) pParam;
	int nServoInterval = pcmd->m_ServoInterval;
	int curID = 0;
	while (pcmd->m_ServoInterval > 0 && pcmd->bToStop == false && nServoInterval==pcmd->m_ServoInterval)
	{
		if (pcmd->m_arServoEnable[curID] == true)
		{
			pcmd->UpdateServoPos(curID);
			//fprintf(stderr,"CMFCmd::updateServoPosThread: sleep %d \n",nServoInterval);
			Sleep(nServoInterval);
		}

		curID ++;
		if (curID > 0xff)
		{
			curID = 1;
		}
	}
	fprintf(stderr,"updateServoPosThread end! something wrong?\n");
	return NULL;
}

void CMFCmd::ServoEnable(int inID)
{
	//fprintf(stderr,"CMFCmd::ServoEnable \n");

	m_arServoEnable[inID] = true;
}

void CMFCmd::SetServoMode(int inID, int inMode)
{
	if (inID<0)
	{
		return;
	}
	//fprintf(stderr,"CMFCmd::SetServoMode ID=%d mode = %d\n",inID,inMode);
	//////////////////////////////////////////////////////////////////////////
	//保护现场
	bool bInpBak = m_bDigiInputEnable;
	m_bDigiInputEnable = false;
	bool bADBak[MFADNO];
	for (int i=0;i<MFADNO;i++)
	{
		bADBak[i] = m_arADEnable[i];
		m_arADEnable[i] = false;
	}
	//ServoEnable(inID);
	bool bServoBak[MFSERVOMAX];
	for (int i=0;i<MFSERVOMAX;i++)
	{
		bServoBak[i] = m_arServoEnable[i];
		m_arServoEnable[i] = false;
	}
	//usleep(10*1000);
	/*/////////////////////////////////////////////////////////////////////////
	//初始值
	switch (inMode)
	{
	case 0:
		SetServoPos(inID,512,1);
		break;
	case 1:
		SetServoRotaSpd(inID,1);
		break;
	case 2:		//卸载模式，开启位置查询
		bServoBak[inID] = true;
		break;
	}
	/////////////////////////////////////////////////////////////////////////*/

	sendbuf[2] = 0x30;		//ID
	sendbuf[3] = 0x02;		//Len of Param
	sendbuf[4] = 0x08;		//Function modules
	sendbuf[5] = 0x20;		//Method
	sendbuf[6] = (unsigned char) inID;		//param (Servo ID)
	
	sendbuf[7] = (unsigned char)inMode;

	//cal sum
	m_CalBufSum(sendbuf,9);
	
	Send(sendbuf,9);

	//////////////////////////////////////////////////////////////////////////	
	usleep(10*1000);
	//////////////////////////////////////////////////////////////////////////

	//恢复现场
	m_bDigiInputEnable = bInpBak ;	
	for (int i=0;i<MFADNO;i++)
	{
		m_arADEnable[i] = bADBak[i];
	}
	for (int i=0;i<MFSERVOMAX;i++)
	{
		m_arServoEnable[i] = bServoBak[i];
	}
	//////////////////////////////////////////////////////////////////////////
}

void CMFCmd::SetServoPos(int inID, int inPos, int inSpeed)
{
	if (inID<0 || inPos<0 )
	{
		return;
	}

	//check change?
	if (inPos == m_nBakSerPos[inID] && inSpeed == m_nBakSerPosSpd[inID])
	{
		return;
	}
	m_nBakSerPos[inID] = inPos;
	m_nBakSerPosSpd[inID] = inSpeed;

	sendbuf[2] = 0x30;		//ID
	sendbuf[3] = 0x05;		//Len of Param
	sendbuf[4] = 0x08;		//Function modules
	sendbuf[5] = 0x21;		//Method
	sendbuf[6] = (unsigned char) inID;		//param (Servo ID)
	//position
	m_Split2Bytes(ABS(inPos),&sendbuf[7]);

	//speed
	m_Split2Bytes(ABS(inSpeed),&sendbuf[9]);
	
	//cal sum
	m_CalBufSum(sendbuf,12);

	//fprintf(stderr,"CMFCmd::SetServoPos ID=%d \n",inID);	
	Send(sendbuf,12);

}

void CMFCmd::SetServoRotaSpd(int inID, int inSpeed)
{
	if (inID<0)
	{
		return;
	}

	if (m_nBakSerRotSpd[inID] == inSpeed)
	{
		return;
	}
	m_nBakSerRotSpd[inID] = inSpeed;
	
	sendbuf[2] = 0x30;		//ID
	sendbuf[3] = 0x04;		//Len of Param
	sendbuf[4] = 0x08;		//Function modules
	sendbuf[5] = 0x22;		//Method
	sendbuf[6] = (unsigned char) inID;		//param (Servo ID)
	if (inSpeed > 0)
	{
		sendbuf[7] = 0x00;
	} 
	else
	{
		sendbuf[7] = 0x01;
	}
	
	//speed
	m_Split2Bytes(ABS(inSpeed),&sendbuf[8]);
	
	//cal sum
	m_CalBufSum(sendbuf,11);
	
	//fprintf(stderr,"CMFCmd::SetServoRotaSpd ID=%d \n",inID);
	Send(sendbuf,11);
}

void CMFCmd::ServoAction()
{
	sendbuf[2] = 0x30;		//ID
	sendbuf[3] = 0x01;		//Len of Param
	sendbuf[4] = 0x08;		//Function modules
	sendbuf[5] = 0x40;		//Method
	sendbuf[6] = 0xfe;		//param (All Servo)
	
	//cal sum
	m_CalBufSum(sendbuf,sendbuf[3]+7);
	
	Send(sendbuf,sendbuf[3]+7);
}

void CMFCmd::ASynUpdateServoPos(int interval)
{
	sendbuf[2] = 0x30;		//ID
	sendbuf[3] = 0x00;		//Len of Param
	sendbuf[4] = 0x08;		//Function modules (Servo)
	sendbuf[5] = 0x00;		//Method

	int nIndex = 6;

	for (int i=1;i<MFSERVOMAX;i++)
	{
		if (m_arServoEnable[i] == true)
		{
			sendbuf[nIndex] = i;
			nIndex ++;
		}
	}
	sendbuf[nIndex] = nIndex-6;
	sendbuf[3] = nIndex-6+1;		//Len of Param

	m_CalBufSum(sendbuf,sendbuf[3]+7);
	
	Send(sendbuf,sendbuf[3]+7);
}

void CMFCmd::ASynSetServoPos(int inID,int inPos,int inSpeed)
{
	m_nAsynServoMode[inID] = 0;
	if (m_nAsynSerPosToSet[inID] != inPos  || m_nAsynSerSpdToSet[inID] != inSpeed)
	{
		m_nAsynSerPosToSet[inID] = inPos;
		m_nAsynSerSpdToSet[inID] = inSpeed;
		m_bAsynServoChanged[inID] = true;
	}
}
	
void CMFCmd::ASynSetServoRotaSpd(int inID,int inSpeed)
{	
	m_nAsynServoMode[inID] = 1;
	if (m_nAsynSerSpdToSet[inID] != inSpeed)
	{
		m_nAsynSerSpdToSet[inID] = inSpeed;
		m_bAsynServoChanged[inID] = true;
	}
}

void CMFCmd::ASynServoAction()
{
	static int nServoNum;
	//1)舵机模式
	nServoNum = 0;
	sendbuf[2] = 0x30;		//ID
	sendbuf[4] = 0x08;		//Function modules (Servo)
	sendbuf[5] = 0x71;		//Method
	for (int i=1;i<MFSERVOMAX;i++)
	{
		if (m_nAsynServoMode[i] == 0)
		{
			//id
			sendbuf[6+nServoNum*5] = i;

			//position
			m_Split2Bytes(ABS(m_nAsynSerPosToSet[i]),&sendbuf[6+nServoNum*5+1]);

			//speed
			m_Split2Bytes(ABS(m_nAsynSerSpdToSet[i]),&sendbuf[6+nServoNum*5+3]);

			nServoNum ++;
		}
	}
	if (nServoNum > 0)
	{
		sendbuf[3] = nServoNum*5;		//Len of Param
		
		//cal sum
		m_CalBufSum(sendbuf,sendbuf[3]+7);
		
		//fprintf(stderr,"CMFCmd::ASynServoAction ServoMode\n");	
		Send(sendbuf,sendbuf[3]+7);
	}

	//2)电机模式
	nServoNum = 0;
	sendbuf[2] = 0x30;		//ID
	sendbuf[4] = 0x08;		//Function modules (Servo)
	sendbuf[5] = 0x72;		//Method
	for (int i=1;i<MFSERVOMAX;i++)
	{
		if (m_nAsynServoMode[i] == 1)
		{
			//id
			sendbuf[6+nServoNum*4] = i;
			
			//dirct
			if (m_nAsynSerSpdToSet[i] >= 0)
			{
				sendbuf[6+nServoNum*4+1] = 0;
			} 
			else
			{
				sendbuf[6+nServoNum*4+1] = 1;
			}
			
			//speed
			m_Split2Bytes(ABS(m_nAsynSerSpdToSet[i]),&sendbuf[6+nServoNum*4+2]);

			//fprintf(stderr,"ID:%d spd:%d",i,m_nAsynSerSpdToSet[i]);
			
			nServoNum ++;
		}
	}
	if (nServoNum > 0)
	{
		sendbuf[3] = nServoNum*4;		//Len of Param

		//cal sum
		m_CalBufSum(sendbuf,sendbuf[3]+7);
		
		//fprintf(stderr,"CMFCmd::ASynServoAction ServoMode\n");	
		Send(sendbuf,sendbuf[3]+7);
	}

	for(int i=1;i<MFSERVOMAX;i++)
	{
		m_bAsynServoChanged[i] = false;
	}
}

int CMFCmd::GetServoPos(int inID)
{
	//使能舵机查询	
	ServoEnable(inID);

	unsigned int ret = 0;

	ret = (0x000000ff & (unsigned int)m_dataServoPosH[inID]);
	ret <<= 8;
	ret &= 0x0000ff00;
	ret |= (0x000000ff & (unsigned int)m_dataServoPosL[inID]);
	return ret;
}

int CMFCmd::GetServoLoad(int inID)
{
	//使能舵机查询
	ServoEnable(inID);

	int ret = 0;
	
	ret = (0x000000ff & (unsigned int)m_dataServoLoadH[inID]);
	ret <<= 8;
	ret &= 0x0000ff00;
	ret |= (0x000000ff & (unsigned int)m_dataServoLoadL[inID]);

	//符号判断
	unsigned int tflag = (unsigned int)ret;
	tflag = 0x01&(tflag>>10);
	if (tflag != 0)
	{
		//负数
		ret = ret & 0xfbff;
		ret = - ret;
	}
	return ret;
}


void CMFCmd::UpdateAddr(unsigned char inAddr)
{
	//fprintf(stderr,"CMFCmd::UpdateAddr = %d\n",inAddr);
	
	sendbuf[2] = 0x30;		//ID
	sendbuf[3] = 0x01;		//Len of Param
	sendbuf[4] = 0xA0;		//Function modules
	sendbuf[5] = 0x00;		//Method
	sendbuf[6] = inAddr;	//param
	
	//cal sum
	m_CalBufSum(sendbuf,sendbuf[3]+7);
	
	Send(sendbuf,sendbuf[3]+7);
}

int CMFCmd::GetFromAddr(unsigned char inAddr)
{
	if (inAddr < 64)
	{
		m_ExtEnable[inAddr] = true;
		return (int)m_ExtData[inAddr];
	} 
	else
	{
		return 0;
	}
}

void CMFCmd::SetValTo(unsigned char inAddr,unsigned char inVal)
{
	sendbuf[2] = 0x30;		//ID
	sendbuf[3] = 0x02;		//Len of Param
	sendbuf[4] = 0xA0;		//Function modules
	sendbuf[5] = 0x20;		//Method
	sendbuf[6] = inAddr;
	sendbuf[7] = inVal;		
	
	//cal sum
	m_CalBufSum(sendbuf,sendbuf[3]+7);
	
	Send(sendbuf,sendbuf[3]+7);
}

void CMFCmd::m_Split2Bytes(unsigned int inSrc, unsigned char *inTarg)
{
	if (inTarg == NULL)
	{
		return;
	}
	unsigned short temp = (unsigned short)0x0000ffff&inSrc;
	inTarg[1] = (unsigned char)temp&0x00ff;

	temp >>= 8;

	inTarg[0] = (unsigned char)temp&0x00ff;
}

void CMFCmd::ScheUpdateAll(int interval)
{	
	if (interval < 0)
	{
		interval = 0;
	}
	m_nUpdateAllInterval = interval*1000;
	
	pthread_create(&pidUpdateAllThread,NULL,updateAllThread,(void*)this);
}

void* CMFCmd::updateAllThread(void *pParam)
{
	CMFCmd* pcmd = (CMFCmd*) pParam;
	int i;
	
	while (pcmd->m_nUpdateAllInterval > 0 && pcmd->bToStop == false)
	{
		//AD
		for (i=0;i<MFADNO;i++)
		{		
			pcmd->UpdateAD(i);
			if (pcmd->m_nUpdateAllInterval <= 0 || pcmd->bToStop == true)
			{				
				return NULL;
			}
		}
		
		//Input
		pcmd->UpdateInput();
		if (pcmd->m_nUpdateAllInterval <= 0 || pcmd->bToStop == true)
		{				
			return NULL;
		}

		//Servo
		for (i=0;i<255;i++)
		{
			if (pcmd->m_arServoEnable[i] == true)
			{
				pcmd->UpdateServoPos(i);
				//pcmd->UpdateServoLoad(i);
				if (pcmd->m_nUpdateAllInterval <= 0 || pcmd->bToStop == true)
				{				
					return NULL;
				}
			}
		}

		//extdata
		for (i=0;i<MFEXTDATA;i++)
		{
			if (pcmd->m_ExtEnable[i] == true)
			{
				pcmd->UpdateAddr(i);
			}
			
			if (pcmd->m_nUpdateAllInterval <= 0 || pcmd->bToStop == true)
			{				
				return NULL;
			}
		}
		Sleep(pcmd->m_nUpdateAllInterval);
	}
	return NULL;
}


void CMFCmd::ADEnable(int inID)
{
	if (inID<0 || inID>MFADNO-1)
	{
		return;
	}

	m_arADEnable[inID] = true;
}

void CMFCmd::ServoAction(int inID)
{
	sendbuf[2] = 0x30;		//ID
	sendbuf[3] = 0x01;		//Len of Param
	sendbuf[4] = 0x08;		//Function modules
	sendbuf[5] = 0x40;		//Method
	sendbuf[6] = (unsigned char)inID;		//param (Servo ID)
	
	//cal sum
	m_CalBufSum(sendbuf,sendbuf[3]+7);
	
	Send(sendbuf,sendbuf[3]+7);
}

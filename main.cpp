/***********************************************************************
** ʾ��������ͷͼ����
** ���ܣ�
***********************************************************************/
 #include  "background.h"
#include "UDP/UDPBasic.h"

static unsigned char endflag[3];
static unsigned char imgbuf[160*120*3];
static unsigned char* pSrc;
static unsigned char* pTar;
static int i;
static unsigned char recDone;

static int tmp;

//�̲߳����ṹ��
struct ThParam 
{
	int nBegin;
	int nEnd;
	int nLen;
	unsigned char* pBuf;

	bool bSend;
	bool bDone;

	CUDPBasic udp;
	unsigned char data[3+450];
};

static ThParam param[8];
static pthread_t pidThread[8];

//�����̺߳���
void *ImgSending(void *pParam)
{
	ThParam* thparam = (ThParam*)pParam;

	thparam->data[0] = 0x55;
	thparam->data[1] = 0xaa;

	while(1)
	{
		//�ȴ����ݸ���
		while(thparam->bSend == false)
		{
			usleep(1);
		}
		thparam->bDone = false;

		int i;
		for (i = thparam->nBegin;i<thparam->nEnd;i++)
		{
			thparam->data[2] = i;
			memcpy(&(thparam->data[3]),thparam->pBuf+450*i,450);
			thparam->udp.Send(thparam->data,453);
		}

		thparam->bDone = true;
		thparam->bSend = false;
	}
}

bool SendDone()
{
	for (int i=0;i<8;i++)
	{
		if (param[i].bDone == false)
		{
			return false;
		}
	}
	return true;
}

//UDP���պ�����
void recv(char* inBuf, int inLen)
{
	fprintf(stderr,"rec %s \n",inBuf); 		//��ʾ���յ����ַ�
	recDone = *inBuf;
}

//����ͼ������
void img(unsigned char* inbuf,int inWidth,int inHeight)
{
	//�ʵ����ͷֱ���
	pSrc = inbuf;
	pTar = imgbuf;

	fprintf(stderr,"buf img...\n");
	for (i=0;i<inWidth*inHeight*3;i+=6)
	{
		memcpy(pTar,pSrc,3);
		pSrc += 6;
		pTar += 3;
		if (i%(inWidth*3) == 0)
		{
			pSrc += inWidth*3;
			i += inWidth*3;
		}
	}

	fprintf(stderr,"send img \n");
	for (int i=0;i<8;i++)
	{
		param[i].bSend = true;
	}

	//�ȴ��������
	while(SendDone() == false)
	{
		usleep(1);
	}

	//end flag
	endflag[0] = 0x55;
	endflag[1] = 0xaa;
	endflag[2] = 0xff;
	param[7].udp.Send(endflag,3);

	while(recDone == 0)
	{
		usleep(1);
	}
	recDone = 0;

	
	fprintf(stderr,"all send done! %d times\n",tmp);
	tmp ++;
}

int main(int argc, char * argv[])
{
	MFInit();					//��ʼ��Ӳ��

	recDone = 0;
	UDPRecvCallback(recv);		//����UDP���պ�����	
	UDPListen(2010);			//�򿪼����˿�

	MFCapImgCallback(&img);		//���ô�����ָ��
	
	tmp = 0;
	
	//��ʼ���̲߳���
	for (int i=0;i<8;i++)
	{
		param[i].bDone = false;
		param[i].bSend = false;
		param[i].nLen = 450;
		param[i].pBuf = imgbuf;

		param[i].nBegin = i*(57600/(8*450));
		param[i].nEnd = (i+1)*(57600/(8*450));

		//���ն�PC��IP						
		param[i].udp.Connect((char*)&("192.168.1.101"),1101+i);

		pthread_create(&pidThread[i], NULL, ImgSending, (void *)&param[i]);
	}

	MFCapOpen();				//����Ƶ

	while (1)
	{
		DelayMS(1000);
	}
}


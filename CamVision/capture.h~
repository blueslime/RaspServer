//////////////////////////////////////////////////////////////////////////
// for MultiflexNG
// http://robot.up-tech.com 
// zwj_uptech@126.com
//////////////////////////////////////////////////////////////////////////
#ifndef CAPTURE_H_H
#define CAPTURE_H_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "fb.h"
#include <linux/videodev.h>

#include "../TCP/TCPLin.h"
#include "../UDP/VideoTransit.h"
#include "../type.h"
#include "../SerialCom/serial.h"

//#define _GNU_SOURCE
#include <getopt.h>

struct capture_info{
	int width, height;
	char device[256];
};

struct fb_dev
{
	//for frame buffer
	int fb;
	void *fb_mem;	//frame buffer mmap
	int fb_width, fb_height, fb_line_len, fb_size;
	int fb_bpp;
	//src must be RGB24 format
	void (*fb_draw)(struct fb_dev *fbdev, void* src, int x, int y, int width, int height);
};

class CCapture
{
public:
	void SetMode(int inMode);
	CCapture();
	virtual ~CCapture();
	void Create();
	void Stop();
	static void *Capting(void *pParam);
	void SetHMax(int inMax);
	void SetHMin(int inMin);
	void SetSMax(int inMax);
	void SetSMin(int inMin);
	void SetIMin(int inMin);
	void SetIMax(int inMax);
	CTCPLin* pReport;		//TCP图像传输
	CVideoTransit* pVT;		//UDP图像传输
	PIMGREC pBufferCB;		//自定义处理函数
	bool m_exit;
	
	// output
	int m_x;
	int m_y;
	int m_sum;
	
	unsigned char* m_pVbuf;
	int m_nWaitToSend;		//启动后等待延时几帧
	CSerialCom* pscom;

private:
	pthread_t pidCapThread;
	int spread(int x,int y);
	int ConvertPix(int x,int y);
	int m_lastx;
	int m_lasty;
	int m_nWidth;
	int m_nHeight;
	int m_nMode;	
};

#endif

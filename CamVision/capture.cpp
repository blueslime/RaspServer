//////////////////////////////////////////////////////////////////////////
// for MultiflexNG
// http://robot.up-tech.com 
// zwj_uptech@126.com
//////////////////////////////////////////////////////////////////////////
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>

#include "capture.h"
#include "grab-ng.h"

static pthread_mutex_t  capinit_lock;
static void* caphandle;
static struct ng_video_fmt fmt;

static char *default_framebuffer="/dev/fb0";

//static struct capture_info capinfo={320, 240, "/dev/video0"};
static struct capture_info capinfo={320, 240, "/dev/video1"};
static struct fb_dev fbdev;
static char* fb_dev_name=NULL;

#define NUM_CAPBUFFER	32

//HSI 
struct HSI{
	int h, s, i;
};

static struct HSI ret_hsi = {0,0,0};
static int max,min;

//Color Temp Value
static int target_x,target_y,p_sum;
static int Hmax,Hmin,Smax,Smin,Imax,Imin;
static unsigned char map[320*240];

static __u8 *psrc;
static int r,g,b;

int getmax(int r,int g,int b)
{
	int max = b;
	if (max < g)
	{
		max = g;
	}
	if (max < r)
	{
		max = r;
	}
	return max;
}

int getmin(int r,int g,int b)
{
	int min = b;
	if (min > g)
	{
		min = g;
	}
	if (min > r)
	{
		min = r;
	}
	return min;
}

void convert(int r,int g,int b)
{
	max = getmax(r,g,b);
	min = getmin(r,g,b);

	//H
	if(max!=min)
	{
		if(max==r)
		{		
			if(g<b)
				ret_hsi.h=60*(g-b)/(max-min)+360;
			else
				ret_hsi.h=60*(g-b)/(max-min);
		}
		if(max==g)
			ret_hsi.h=60*(b-r)/(max-min)+120;
		if(max==b)
			ret_hsi.h=60*(r-g)/(max-min)+240;
	}
	else
	ret_hsi.h=0;

	//S
	ret_hsi.s = max-min;

	//I
	ret_hsi.i = max;

	return;
}

void fb_draw16bpp(struct fb_dev *fbdev, void* src, int x, int y, int width, int height)
{
	/*int i, j;
	int fb_line_len = fbdev->fb_line_len;
	__u8 *psrc= (__u8*)src;
	__u16* pdsc = (__u16*)fbdev->fb_mem;
	__u16 tmp, tmp1;
			
	__u8 r,g,b;

	pdsc+=y*fb_line_len/2 + x;

	for(i=0; i<height; i++)
	{
		
		//直接显示
		for(j=0;j<width;j++)
		{
			b = (*psrc);psrc++;
			g = (*psrc);psrc++;
			r = (*psrc);psrc++;

			tmp = r>>3;		tmp<<=11;
			tmp1 = g>>2;	tmp|=(tmp1<<5);
			tmp |= b>>3;
			pdsc[j] =  tmp;
		}
		//////////////////////////////////////////////////////////////////////////		

		pdsc+=fb_line_len/2;
	}
*/
}

void fb_draw12bpp(struct fb_dev *fbdev, void* src, int x, int y, int width, int height)
{
	/*int i, j;
	int fb_line_len = fbdev->fb_line_len;
	__u8 *psrc= (__u8*)src;
	__u8* pdsc = (__u8*)fbdev->fb_mem;
	__u8 tmp;

	//fixed me! x must be even
	pdsc+=y*fb_line_len + x*3/2;

	for(i=0; i<height; i++){
		for(j=0; j<width*3/2;){
			tmp = psrc[2]&0xf0;
			tmp |=(psrc[1]>>4);
			pdsc[j++] = tmp;

			tmp = psrc[0]&0xf0;
			tmp |=(psrc[5]>>4);
			pdsc[j++] = tmp;

			tmp = psrc[4]&0xf0;
			tmp |=(psrc[3]>>4);
			pdsc[j++] = tmp;

			psrc+=6;
		}
		pdsc+=fb_line_len;
	}*/
}

int framebuffer_open(void)
{
	/*int fb;
	struct fb_var_screeninfo fb_vinfo;
	struct fb_fix_screeninfo fb_finfo;
	
	if (!fb_dev_name && !(fb_dev_name = getenv("FRAMEBUFFER")))
		fb_dev_name=default_framebuffer;

	fb = open (fb_dev_name, O_RDWR);
	if(fb<0){
		printf("device %s open failed\n", fb_dev_name);
		return -1;
	}
	
	if (ioctl(fb, FBIOGET_VSCREENINFO, &fb_vinfo)) {
		printf("Can't get VSCREENINFO: %s\n", strerror(errno));
		close(fb);
		return -1;
	}

	if (ioctl(fb, FBIOGET_FSCREENINFO, &fb_finfo)) {
		printf("Can't get FSCREENINFO: %s\n", strerror(errno));
		return 1;
	}

	fbdev.fb_bpp = fb_vinfo.red.length + fb_vinfo.green.length +
		fb_vinfo.blue.length + fb_vinfo.transp.length;

	fbdev.fb_width = fb_vinfo.xres;
	fbdev.fb_height = fb_vinfo.yres;
	fbdev.fb_line_len = fb_finfo.line_length;
	fbdev.fb_size = fb_finfo.smem_len;

	printf("frame buffer: %dx%d,  %dbpp, 0x%xbyte\n", 
		fbdev.fb_width, fbdev.fb_height, fbdev.fb_bpp, fbdev.fb_size);

	switch(fbdev.fb_bpp){
	case 16:
		fbdev.fb_draw = fb_draw16bpp;
		break;
	case 12:
		fbdev.fb_draw = fb_draw12bpp;
		break;
	default:
		printf("Can't support %d bpp draw\n", fbdev.fb_bpp);
		return -1;
	}


	fbdev.fb_mem = mmap (NULL, fbdev.fb_size, PROT_READ|PROT_WRITE,MAP_SHARED,fb,0);
	if(fbdev.fb_mem==NULL || (int)fbdev.fb_mem==-1){
		fbdev.fb_mem=NULL;
		printf("mmap failed\n");
		close(fb);
		return -1;
	}
	

	fbdev.fb=fb;
	memset (fbdev.fb_mem, 0x0, fbdev.fb_size);

	return 0;
*/
}

void framebuffer_close()
{
	/*if(fbdev.fb_mem){
		munmap(fbdev.fb_mem, fbdev.fb_size);
		fbdev.fb_mem=NULL;
	}

	if(fbdev.fb){
		close(fbdev.fb);
		fbdev.fb=0;
	}
*/
}

//class
CCapture::CCapture()
{
	Hmax = 50;
	Hmin = 0;
	Smax = 255;
	Smin = 40;
	Imax = 255;
	Imin = 0;
	m_exit = true;
	//pParser = NULL;

	m_lastx = 160;
	m_lasty = 120;
	m_nWidth = 320;
	m_nHeight = 240;

	pReport = 0;
	pVT = 0;
	m_nWaitToSend = 10;
	pscom = 0;

	m_pVbuf = NULL;
	m_pVbuf = new unsigned char[m_nWidth*m_nHeight*3];
	pBufferCB = 0;
	m_nMode = 1;

	pthread_mutex_init(&capinit_lock,NULL);
}

CCapture::~CCapture()
{
	/*Stop();
	sleep(100);
	if (pidCapThread)
		pthread_join(pidCapThread, NULL);

	delete[] m_pVbuf;*/
}

int CCapture::spread(int x,int y)
{
	/*if (x<0 || x >= m_nWidth || y<0 || y>= m_nHeight)
	{
		return 0;
	}
	unsigned int m_lastsum = 0;
	
	int range;
	for (range=1;range<m_nWidth;range++)
	{
		m_lastsum = p_sum;
		int i;

		//UP
		for (i=m_lastx-range;i<m_lastx+range;i++)
		{
			ConvertPix(i,m_lasty+range);
		}

		//down
		for (i=m_lastx-range;i<m_lastx+range;i++)
		{
			ConvertPix(i,m_lasty-range);
		}

		//left
		for (i=m_lasty-range;i<m_lasty+range;i++)
		{
			ConvertPix(m_lastx-range,i);
		}

		//right
		for (i=m_lasty-range;i<m_lasty+range;i++)
		{
			ConvertPix(m_lastx+range,i);
		}

		if (p_sum > 100 && p_sum == m_lastsum) //no increase
		{
			break;
		}
	}
	return 1;
*/
}
int CCapture::ConvertPix(int x,int y)
{
	if (x<0 || x>=m_nWidth || y<0 || y>=m_nHeight)
	{
		return false;
	}
	
	int base = y*m_nWidth*3+x*3;
	b = m_pVbuf[base];
	g = m_pVbuf[base+1];
	r = m_pVbuf[base+2];

	convert(r,g,b);

	
	//filter
	bool flag = false;
	if (Hmax > Hmin)
	{
		//if ((ret_hsi.h < Hmax+1 && ret_hsi.h > Hmin-1) && (ret_hsi.s< Smax+1 && ret_hsi.s > Smin-1) && ret_hsi.i>(Imin-1) && ret_hsi.i<(Imax+1))
		if (ret_hsi.h>(Hmin-1) && ret_hsi.h<(Hmax+1) && ret_hsi.s>(Smin-1) && ret_hsi.s<(Smax+1) && ret_hsi.i>(Imin-1) && ret_hsi.i<(Imax+1))
		{
			flag = true;
		}
		else
		{
			flag = false;
		}
	}
	else
	{//Hmax < Hmin

		/*if ((ret_hsi.h < Hmax+1 || ret_hsi.h > Hmin-1) && (ret_hsi.s< Smax+1 && ret_hsi.s > Smin-1))
		{
			flag = true;
		} 
		else
		{
			flag = false;
		}*/

		//////////////////////////////////////////////////////////////////////////
		if (ret_hsi.h>-1 && ret_hsi.h<(Hmax+1) && (ret_hsi.s>(Smin-1) && ret_hsi.s<(Smax+1)) && (ret_hsi.i>(Imin-1) && ret_hsi.i<(Imax+1)))
		{
			flag = true;
		}
		
		if (ret_hsi.h>(Hmin-1) && ret_hsi.h<361 && ret_hsi.s>(Smin-1) && ret_hsi.s<(Smax+1) && ret_hsi.i>(Imin-1) && ret_hsi.i<(Imax+1))
		{
			flag = true;
		}
		//////////////////////////////////////////////////////////////////////////
	}
					
	if(flag == true)
	{
		target_x += x;
		target_y += y;
		p_sum ++;

		//map[y*320+x] = 1;	//checked point & validiate

		m_pVbuf[base] = 0;		//b = 
		m_pVbuf[base+1] = 0;	//g = 
		m_pVbuf[base+2] = 0xff;	//r = 

		return true;
	}
	else
	{
		return false;
	}
}

void *CCapture::Capting(void *pParam)
{
	//主体对象指针
	/*CCapture *pCapture = (CCapture*) pParam;

	//线程终止标志
	pCapture->m_exit = false;
	//fprintf(stderr,"CCapture::Capting m_exit = false\n");

	//report header
	unsigned char header[2] = {0x55,0xaa};

	//v4l_driver在drv-v4l.c中定义了
	struct ng_vid_driver *cap_driver = &v4l_driver;

	//开启framebuffer（显示）
// 	if(framebuffer_open()<0){
// 		return NULL;
// 	}

	//视频捕捉开始
	cap_driver->startvideo(caphandle, 25,  NUM_CAPBUFFER);	//该函数指针指向drv-v4l.c(1067):v4l_startvideo

	{
		//调整显示范围，使其显示在屏幕中央
		struct ng_video_buf* pvideo_buf;
		int x, y, width, height;
		int diff_width, diff_height;

		diff_width = fbdev.fb_width - fmt.width;
		diff_height = fbdev.fb_height - fmt.height;

		if(diff_width>0){
			x =  diff_width/2;
			width = fmt.width;
		}
		else{
			x = 0;
			width = fbdev.fb_width;
		}

		if(diff_height>0){
			y =  diff_height/2;
			height = fmt.height;
		}
		else{
			y = 0;
			height = fbdev.fb_height;
		}

		
		//视频捕捉循环开始
		//bool flag = false;	//flag for result
		while(pCapture->m_exit == false)
		{	
			pvideo_buf=cap_driver->nextframe(caphandle); //指针调用的是v4l_nextframe函数
			/////////////////////////////////////////////////////////////////image process
			//清空像素阈值标记数组
			//memset(map,0,320*240);
			
			//fprintf(stderr,"Hmax = %d  ;  Hmin = %d\n",Hmax,Hmin);
			p_sum = target_x = target_y = 0;
			psrc= (__u8*)(pvideo_buf->data);

			//将图像内容缓冲一下
			memcpy(pCapture->m_pVbuf,psrc,320*240*3); 
			//fprintf(stderr,"H(%d,%d) S(%d,%d) I(%d,%d)\n",Hmax,Hmin,Smax,Smin,Imax,Imin);

			//图像处理
			switch(pCapture->m_nMode)
			{
			case 1:
				//模式一的时候是进行扩散阈值处理
				//////////////////////////////////////////////////////////////////////////
				//扩散扫描，运算量小
				pCapture->spread(pCapture->m_lastx,pCapture->m_lasty);
				//////////////////////////////////////////////////////////////////////////
				
				
				//cal the x and y
				if (p_sum > 0)
				{
					//printf("caled x y \n");
					target_x = target_x/p_sum;
					target_y = target_y/p_sum;
					pCapture->m_lastx = target_x;
					pCapture->m_lasty = target_y;
					//var to output
					pCapture->m_x = target_x;
					pCapture->m_y = target_y;
					pCapture->m_sum = p_sum;
				}
				break;

			case 2:
				if (pCapture->pBufferCB != 0)
				{
					(*(pCapture->pBufferCB))(pCapture->m_pVbuf,fmt.width,fmt.height);
				}
				break;

			default:
				break;
			}
			
			/////////////////////////////////////////////////////////////////
			//视频图像显示
			//fbdev.fb_draw(&fbdev, pCapture->m_pVbuf, x, y, width, height);
			//printf("fb_drawed \n");
			//ng_release_video_buf(pvideo_buf);

			//图像发送到host(TCP方式)
			if (pCapture->pReport != 0)
			{
				if (pCapture->pReport->m_ListenP.m_bConnected == true)
				{
					//fprintf( stderr, "CCapture::Capting: Reporting img!\n");
					pCapture->pReport->Send(header,2);
					pCapture->pReport->Send(pCapture->m_pVbuf,320*240*3);
				}
			}
			//图像发送到host(UDP方式)
			if (pCapture->pVT != 0)
			{
				if (pCapture->pVT->m_bEnable == true)
				{
					pCapture->pVT->ImgTrans(pCapture->m_pVbuf,320,240);
				}
			}

			//为串口发送让步
			if (pCapture->pscom != 0)
			{
				while(pCapture->pscom->BufEmpty() == false)
				{
					usleep(1000);
				}
			}
			
			////////////////////////////////////////////////////////////////
			int release;

			pthread_mutex_lock(&pvideo_buf->lock);
			pvideo_buf->refcount--;
			release = (pvideo_buf->refcount == 0);
			pthread_mutex_unlock(&pvideo_buf->lock);
			if (release && NULL != pvideo_buf->release)
			pvideo_buf->release(pvideo_buf);
			////////////////////////////////////////////////////////////////
		}
	}
	
	//fprintf(stderr,"CCapture::Capting m_exit = true\n");

	//framebuffer_close();
	cap_driver->stopvideo(caphandle);
	cap_driver->close(caphandle);
	return 0;*/
} 

void CCapture::Create()
{
	/*if (m_exit == false)
	{
		return;
	}
	
	//v4l_driver在drv-v4l.c中定义了
	struct ng_vid_driver *cap_driver = &v4l_driver;
	
	//开启framebuffer（显示）
// 	if(framebuffer_open()<0){
// 		fprintf(stderr,"Framebuffer_open false!\n");
// 		return;
// 	}
	
	//开启摄像头设备
	//void* caphandle;
	caphandle=cap_driver->open(capinfo.device);	//capinfo.device 为 "/dev/video1"
	
	if(!caphandle){
		printf("failed to open video for linux interface!\n");
		return ;
	}
	
	//设置摄像头视频格式和分辨率
//	struct ng_video_fmt fmt;
	
	fmt.fmtid = VIDEO_BGR24;
	
	fmt.width = capinfo.width;
	fmt.height = capinfo.height;
	
	if(cap_driver->setformat(caphandle, &fmt)){
		printf("failed to set video format!\n");
		return;
	}

	pthread_create(&pidCapThread, NULL, Capting, (void *)this);*/
}

void CCapture::Stop()
{
	m_exit = true;
	Sleep(500000);
}

void CCapture::SetHMax(int inMax)
{
	Hmax = inMax;

	//规范h阈值范围
	while (Hmax > 360)
	{
		Hmax -= 360;
	}
	while (Hmax < 0)
	{
		Hmax += 360;
	}
}

void CCapture::SetHMin(int inMin)
{
	Hmin = inMin;
	while (Hmin > 360)
	{
		Hmin -= 360;
	}
	while (Hmin < 0)
	{
		Hmin += 360;
	}
}
	
void CCapture::SetSMax(int inMax)
{
	Smax = inMax;
}

void CCapture::SetSMin(int inMin)
{
	Smin = inMin;
}

void CCapture::SetIMax(int inMax)
{
	Imax = inMax;
}

void CCapture::SetIMin(int inMin)
{
	Imin = inMin;
}

void CCapture::SetMode(int inMode)
{
	m_nMode = inMode;
}

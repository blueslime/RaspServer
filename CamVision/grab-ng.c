/*
 * next generation[tm] xawtv capture interfaces
 *
 * (c) 2001 Gerd Knorr <kraxel@bytesex.org>
 *
 */

#define NG_PRIVATE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include <fnmatch.h>
#include <errno.h>
#include <ctype.h>
#include <inttypes.h>
#include <sys/time.h>
#include <sys/types.h>

#include <dlfcn.h>
#ifndef RTLD_NOW
# define RTLD_NOW RTLD_LAZY
#endif

#include "grab-ng.h"

//int  ng_debug          = 3;
int  ng_debug          = 0;		//add by quentin
int  ng_chromakey      = 0x00ff00ff;
int  ng_jpeg_quality   = 75;
int  ng_ratio_x        = 4;
int  ng_ratio_y        = 3;

char ng_v4l_conf[256]  = "v4l-conf";

/* --------------------------------------------------------------------- */

const unsigned int ng_vfmt_to_depth[] = {
    0,               /* unused   */
    8,               /* RGB8     */
    8,               /* GRAY8    */
    16,              /* RGB15 LE */
    16,              /* RGB16 LE */
    16,              /* RGB15 BE */
    16,              /* RGB16 BE */
    24,              /* BGR24    */
    32,              /* BGR32    */
    24,              /* RGB24    */
    32,              /* RGB32    */
    16,              /* LUT2     */
    32,              /* LUT4     */
    16,		     /* YUYV     */
    16,		     /* YUV422P  */
    12,		     /* YUV420P  */
    0,		     /* MJPEG    */
    0,		     /* JPEG     */
    16,		     /* UYVY     */
};

const char* ng_vfmt_to_desc[] = {
    "none",
    "8 bit PseudoColor (dithering)",
    "8 bit StaticGray",
    "15 bit TrueColor (LE)",
    "16 bit TrueColor (LE)",
    "15 bit TrueColor (BE)",
    "16 bit TrueColor (BE)",
    "24 bit TrueColor (LE: bgr)",
    "32 bit TrueColor (LE: bgr-)",
    "24 bit TrueColor (BE: rgb)",
    "32 bit TrueColor (BE: -rgb)",
    "16 bit TrueColor (lut)",
    "32 bit TrueColor (lut)",
    "16 bit YUV 4:2:2 (packed, YUYV)",
    "16 bit YUV 4:2:2 (planar)",
    "12 bit YUV 4:2:0 (planar)",
    "MJPEG (AVI)",
    "JPEG (JFIF)",
    "16 bit YUV 4:2:2 (packed, UYVY)",
};

/* --------------------------------------------------------------------- */

const unsigned int   ng_afmt_to_channels[] = {
    0,  1,  2,  1,  2,  1,  2, 0
};
const unsigned int   ng_afmt_to_bits[] = {
    0,  8,  8, 16, 16, 16, 16, 0
};
const char* ng_afmt_to_desc[] = {
    "none",
    "8bit mono",
    "8bit stereo",
    "16bit mono (LE)",
    "16bit stereo (LE)",
    "16bit mono (BE)",
    "16bit stereo (BE)",
    "mp3 compressed audio",
};

/* --------------------------------------------------------------------- */

const char* ng_attr_to_desc[] = {
    "none",
    "norm",
    "input",
    "volume",
    "mute",
    "audio mode",
    "color",
    "bright",
    "hue",
    "contrast",
};

/* --------------------------------------------------------------------- */

void ng_init_video_buf(struct ng_video_buf *buf)
{
    memset(buf,0,sizeof(*buf));
    pthread_mutex_init(&buf->lock,NULL);    
    pthread_cond_init(&buf->cond,NULL);
}

void ng_release_video_buf(struct ng_video_buf *buf)
{
    int release;

    pthread_mutex_lock(&buf->lock);
    buf->refcount--;
    release = (buf->refcount == 0);
    pthread_mutex_unlock(&buf->lock);
    if (release && NULL != buf->release)
	buf->release(buf);
}

void ng_wakeup_video_buf(struct ng_video_buf *buf)
{
    pthread_cond_signal(&buf->cond);
}

void ng_waiton_video_buf(struct ng_video_buf *buf)
{
    pthread_mutex_lock(&buf->lock);
    while (buf->refcount)
	pthread_cond_wait(&buf->cond, &buf->lock);
    pthread_mutex_unlock(&buf->lock);
}

static void ng_free_video_buf(struct ng_video_buf *buf)
{
    free(buf->data);
    free(buf);
}

struct ng_video_buf*
ng_malloc_video_buf(struct ng_video_fmt *fmt, int size)
{
    struct ng_video_buf *buf;

    buf = malloc(sizeof(*buf));
    if (NULL == buf)
	return NULL;
    ng_init_video_buf(buf);
    buf->fmt  = *fmt;
    buf->size = size;
    buf->data = malloc(size);
    if (NULL == buf->data) {
	free(buf);
	return NULL;
    }
    buf->refcount = 1;
    buf->release  = ng_free_video_buf;
    return buf;
}

/* --------------------------------------------------------------------- */

struct ng_audio_buf*
ng_malloc_audio_buf(struct ng_audio_fmt *fmt, int size)
{
    struct ng_audio_buf *buf;

    buf = malloc(sizeof(*buf)+size);
    memset(buf,0,sizeof(*buf));
    buf->fmt  = *fmt;
    buf->size = size;
    buf->data = (char*)buf + sizeof(*buf);
    return buf;
}

/* --------------------------------------------------------------------- */

struct ng_attribute*
ng_attr_byid(struct ng_attribute *attrs, int id)
{
    if (NULL == attrs)
	return NULL;
    for (;;) {
	if (NULL == attrs->name)
	    return NULL;
	if (attrs->id == id)
	    return attrs;
	attrs++;
    }
}

struct ng_attribute*
ng_attr_byname(struct ng_attribute *attrs, char *name)
{
    if (NULL == attrs)
	return NULL;
    for (;;) {
	if (NULL == attrs->name)
	    return NULL;
	if (0 == strcasecmp(attrs->name,name))
	    return attrs;
	attrs++;
    }
}

const char*
ng_attr_getstr(struct ng_attribute *attr, int value)
{
    int i;
    
    if (NULL == attr)
	return NULL;
    if (attr->type != ATTR_TYPE_CHOICE)
	return NULL;

    for (i = 0; attr->choices[i].str != NULL; i++)
	if (attr->choices[i].nr == value)
	    return attr->choices[i].str;
    return NULL;
}

int
ng_attr_getint(struct ng_attribute *attr, char *value)
{
    int i,val;
    
    if (NULL == attr)
	return -1;
    if (attr->type != ATTR_TYPE_CHOICE)
	return -1;

    for (i = 0; attr->choices[i].str != NULL; i++) {
	if (0 == strcasecmp(attr->choices[i].str,value))
	    return attr->choices[i].nr;
    }

    if (isdigit(value[0])) {
	/* Hmm.  String not found, but starts with a digit.
	   Check if this is a valid number ... */
	val = atoi(value);
	for (i = 0; attr->choices[i].str != NULL; i++)
	    if (val == attr->choices[i].nr)
		return attr->choices[i].nr;
	
    }
    return -1;
}

void
ng_attr_listchoices(struct ng_attribute *attr)
{
    int i;
    
    fprintf(stderr,"valid choices for \"%s\": ",attr->name);
    for (i = 0; attr->choices[i].str != NULL; i++)
	fprintf(stderr,"%s\"%s\"",
		i ? ", " : "",
		attr->choices[i].str);
    fprintf(stderr,"\n");
}

int
ng_attr_int2percent(struct ng_attribute *attr, int value)
{
    int range,percent;

    range   = attr->max - attr->min;
    percent = (value - attr->min) * 100 / range;
    if (percent < 0)
	percent = 0;
    if (percent > 100)
	percent = 100;
    return percent;
}

int
ng_attr_percent2int(struct ng_attribute *attr, int percent)
{
    int range,value;

    range = attr->max - attr->min;
    value = percent * range / 100 + attr->min;
    if (value < attr->min)
	value = attr->min;
    if (value > attr->max)
	value = attr->max;
    return value;
}

int
ng_attr_parse_int(struct ng_attribute *attr, char *str)
{
    int value,n;

    if (0 == sscanf(str,"%d%n",&value,&n))
	/* parse error */
	return attr->defval;
    if (str[n] == '%')
	value = ng_attr_percent2int(attr,value);
    if (value < attr->min)
	value = attr->min;
    if (value > attr->max)
	value = attr->max;
    return value;
}

/* --------------------------------------------------------------------- */

void
ng_ratio_fixup(int *width, int *height, int *xoff, int *yoff)
{
    int h = *height;
    int w = *width;

    if (0 == ng_ratio_x || 0 == ng_ratio_y)
	return;
    if (w * ng_ratio_y < h * ng_ratio_x) {
	*height = *width * ng_ratio_y / ng_ratio_x;
	if (yoff)
	    *yoff  += (h-*height)/2;
    } else if (w * ng_ratio_y > h * ng_ratio_x) {
	*width  = *height * ng_ratio_x / ng_ratio_y;
	if (yoff)
	    *xoff  += (w-*width)/2;
    }
}

void
ng_ratio_fixup2(int *width, int *height, int *xoff, int *yoff,
		int ratio_x, int ratio_y, int up)
{
    int h = *height;
    int w = *width;

    if (0 == ratio_x || 0 == ratio_y)
	return;
    if ((!up  &&  w * ratio_y < h * ratio_x) ||
	(up   &&  w * ratio_y > h * ratio_x)) {
	*height = *width * ratio_y / ratio_x;
	if (yoff)
	    *yoff  += (h-*height)/2;
    } else if ((!up  &&  w * ratio_y > h * ratio_x) ||
	       (up   &&  w * ratio_y < h * ratio_x)) {
	*width  = *height * ratio_x / ratio_y;
	if (yoff)
	    *xoff  += (w-*width)/2;
    }
}

int64_t
ng_tofday_to_timestamp(struct timeval *tv)
{
    long long ts;

    ts  = tv->tv_sec;
    ts *= 1000000;
    ts += tv->tv_usec;
    ts *= 1000;
    return ts;
}

int64_t
ng_get_timestamp()
{
    struct timeval tv;

    gettimeofday(&tv,NULL);
    return ng_tofday_to_timestamp(&tv);
}


/* --------------------------------------------------------------------- */

static void clip_dump(char *state, struct OVERLAY_CLIP *oc, int count)
{
    int i;

    fprintf(stderr,"clip: %s - %d clips\n",state,count);
    for (i = 0; i < count; i++)
	fprintf(stderr,"clip:   %d: %dx%d+%d+%d\n",i,
		oc[i].x2 - oc[i].x1,
		oc[i].y2 - oc[i].y1,
		oc[i].x1, oc[i].y1);
}

static void clip_drop(struct OVERLAY_CLIP *oc, int n, int *count)
{
    (*count)--;
    memmove(oc+n, oc+n+1, sizeof(struct OVERLAY_CLIP) * (*count-n));
}

void ng_check_clipping(int width, int height, int xadjust, int yadjust,
		       struct OVERLAY_CLIP *oc, int *count)
{
    int i,j;

    if (ng_debug > 1) {
	fprintf(stderr,"clip: win=%dx%d xa=%d ya=%d\n",
		width,height,xadjust,yadjust);
	clip_dump("init",oc,*count);
    }
    for (i = 0; i < *count; i++) {
	/* fixup coordinates */
	oc[i].x1 += xadjust;
	oc[i].x2 += xadjust;
	oc[i].y1 += yadjust;
	oc[i].y2 += yadjust;
    }
    if (ng_debug > 1)
	clip_dump("fixup adjust",oc,*count);

    for (i = 0; i < *count; i++) {
	/* fixup borders */
	if (oc[i].x1 < 0)
	    oc[i].x1 = 0;
	if (oc[i].x2 < 0)
	    oc[i].x2 = 0;
	if (oc[i].x1 > width)
	    oc[i].x1 = width;
	if (oc[i].x2 > width)
	    oc[i].x2 = width;
	if (oc[i].y1 < 0)
	    oc[i].y1 = 0;
	if (oc[i].y2 < 0)
	    oc[i].y2 = 0;
	if (oc[i].y1 > height)
	    oc[i].y1 = height;
	if (oc[i].y2 > height)
	    oc[i].y2 = height;
    }
    if (ng_debug > 1)
	clip_dump("fixup range",oc,*count);

    /* drop zero-sized clips */
    for (i = 0; i < *count;) {
	if (oc[i].x1 == oc[i].x2 || oc[i].y1 == oc[i].y2) {
	    clip_drop(oc,i,count);
	    continue;
	}
	i++;
    }
    if (ng_debug > 1)
	clip_dump("zerosize done",oc,*count);

    /* try to merge clips */
 restart_merge:
    for (j = *count - 1; j >= 0; j--) {
	for (i = 0; i < *count; i++) {
	    if (i == j)
		continue;
	    if (oc[i].x1 == oc[j].x1 &&
		oc[i].x2 == oc[j].x2 &&
		oc[i].y1 <= oc[j].y1 &&
		oc[i].y2 >= oc[j].y1) {
		if (ng_debug > 1)
		    fprintf(stderr,"clip: merge y %d,%d\n",i,j);
		if (oc[i].y2 < oc[j].y2)
		    oc[i].y2 = oc[j].y2;
		clip_drop(oc,j,count);
		if (ng_debug > 1)
		    clip_dump("merge y done",oc,*count);
		goto restart_merge;
	    }
	    if (oc[i].y1 == oc[j].y1 &&
		oc[i].y2 == oc[j].y2 &&
		oc[i].x1 <= oc[j].x1 &&
		oc[i].x2 >= oc[j].x1) {
		if (ng_debug > 1)
		    fprintf(stderr,"clip: merge x %d,%d\n",i,j);
		if (oc[i].x2 < oc[j].x2)
		    oc[i].x2 = oc[j].x2;
		clip_drop(oc,j,count);
		if (ng_debug > 1)
		    clip_dump("merge x done",oc,*count);
		goto restart_merge;
	    }
	}
    }
    if (ng_debug)
	clip_dump("final",oc,*count);
}


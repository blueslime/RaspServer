#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <unistd.h>
#include "sound.h"

//#define    SOUND_DEV        "/dev/sound/dsp"
#define    SOUND_DEV        "/dev/dsp"
#define    MIXER_DEV		"/dev/mixer"

static int sound_fd=0;
static int mixer_fd=0;
static int snd_vol = 0xff3f;;

int MFOpenSound()
{	
	if(!sound_fd)
	{
		if ( (sound_fd = open(SOUND_DEV,O_RDWR)) < 0 )
		{
			perror("ERROR OPENING DSP DEVICE\n");
			sound_fd = 0;
        }

		//mix
		if ((mixer_fd = open(MIXER_DEV,O_RDWR,0)) == -1) 
		{
			fprintf(stderr,"mixer open fail\n");
		}
		ioctl(mixer_fd, MIXER_WRITE(SOUND_MIXER_VOLUME),&snd_vol);
	}
	return sound_fd;
}

int MFGetSoundFD()
{
	if (sound_fd == 0)
	{
		MFOpenSound();
	}
	return sound_fd;
}

void MFCloseSound()
{
	close(sound_fd);
	sound_fd = 0;
}

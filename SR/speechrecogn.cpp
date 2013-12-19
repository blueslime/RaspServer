#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#include "speechrecogn.h"
#include "voice.h"
# include "../Dev/sound.h"
extern SpeechRecognition *sr;

SpeechRecognition::SpeechRecognition(): 
	m_bIsStarted(false),
   	m_nItem(0)//, m_channel(channel)
{
#ifdef _DEBUG
	//printf("create speech recognition\n");
#endif
// 	if (dsp != 0)
// 		strncpy(m_szDsp, dsp, 32);
// 	else
// 		m_szDsp[0] = '\0';
 	memset(m_szVoc, 0 , sizeof(m_szVoc));
	if (sr != NULL)
		delete sr;
	sr = this;

	m_nResIndex = -1;
}

SpeechRecognition::~SpeechRecognition()
{
	sr = NULL;
	voice_exit();
}

void SpeechRecognition::start()
{
	if (m_bIsStarted == false)
	{
		if (msr_init() < 0)
		{
			fprintf(stderr, "MSR init failed\n");
			return;
		}
	
		addVocabularyToEngine();
		if (::voice_init() < 0)
		{
			fprintf(stderr, "Start speech recognition failed\n");
			return;
		}
		else
		{
#ifdef _DEBUG
			printf("Start speech recognition success\n");
#endif
		}
	}
	else
	{
		addVocabularyToEngine();
		engine_continue();
		voice_continue(10);
	}
	MFGetSoundFD();
	m_bIsStarted = true;
}

void SpeechRecognition::stop()
{
	if (m_bIsStarted)
	{
		voice_stop(10);
		engine_stop_clear();
	}
}

void SpeechRecognition::addVocabularyToEngine()
{
	int i = 0;
	for (; i< MAX_INDEX; ++i)
	{
		if (strlen(m_szVoc[i]) > 0)
		{
			engine_add_word(m_szVoc[i]);
#ifdef _DEBUG
			printf("Add item: %s\n", m_szVoc[i]);
#endif
		}
	}
}

void SpeechRecognition::insertItem(int index, const char *item, int size)
{
	if (index < 0 || index >= MAX_INDEX 
			|| item == NULL 
			|| size < 0 || size > MAX_ITEM_SIZE)
	{
		fprintf(stderr, "insert item failed\n");
		return;
	}
#ifdef _DEBUG
	printf("Add one item: %s\n", item);
#endif
	strncpy(m_szVoc[index], item, MAX_ITEM_SIZE);
}

void SpeechRecognition::deleteItem(int index)
{
	if (index < 0 || index >= MAX_INDEX)
	{
		fprintf(stderr, "deleteItem failed: invalid index\n");
		return;
	}
#ifdef _DEBUG
	printf("Delete item: %d\n", index);
#endif
	memset(m_szVoc[index], 0, MAX_ITEM_SIZE);
}

void SpeechRecognition::replaceItem(int index, const char *item, int size)
{
	if (index < 0 || index >= MAX_INDEX 
			|| item == NULL
			|| size < 0 || size > MAX_ITEM_SIZE)
	{
		fprintf(stderr, "replace item failed\n");
		return;
	}
#ifdef _DEBUG
	printf("replace item %d to: %s\n", index, item);
#endif
	memset((void *)m_szVoc[index], 0, MAX_ITEM_SIZE);
	strncpy(m_szVoc[index], item, MAX_ITEM_SIZE);
}

void SpeechRecognition::deleteItem(int first, int last)
{
	if (first < 0 || first > last || last >= MAX_INDEX)	
	{
		fprintf(stderr, "Delete items from %d to %d failed\n", first, last);
		return;
	}
	while (first <= last)
	{
		deleteItem(first);
		++first;
	}
}

void SpeechRecognition::clear()
{
#ifdef _DEBUG
	printf("Clear vocabulary\n");
#endif
	memset(m_szVoc, 0, sizeof(m_szVoc));
}

void SpeechRecognition::textReturned(const char *text)
{
	m_nResIndex = -1;
	int i = 0;
#ifdef _DEBUG
	printf("Text returned: %s\n", text);
#endif
	for (; i<MAX_INDEX; ++i)
	{
		if (strcmp(m_szVoc[i], text) == 0)
		{
			//sendResult(i);
			m_nResIndex = i;
			break;
		}
	}
}

void SpeechRecognition::sendResult(int index)
{
#ifdef _DEBUG
	printf("Send result: %d\n", index);
#endif
// 	if (m_pCmd != NULL)
// 	{
// 		m_pCmd->textReturn(m_channel, index);
// 	}
}

#if 0
void * SpeechRecognition::voiceThread(void * arg)
{
	int sound_fd;
    unsigned int value;
// 	if ( (sound_fd = open(SOUND_DEV,O_RDWR,0x666)) < 0 ) {
// 		perror("ERROR OPENING DSP DEVICE\n");
// 		exit(1);            /* Open error*/
// 	}
	sound_fd = MFGetSoundFD();

    value=8192;
  	if ( ioctl(sound_fd,SNDCTL_DSP_GETBLKSIZE,&value) < 0 ) {
        perror("Setting block size error\n");
        exit(1);
	}

  	value=16;
  	if ( ioctl(sound_fd,SNDCTL_DSP_SAMPLESIZE,&value) < 0 ) {
        perror("Setting sample size error\n");
        exit(1);
	}

  	value=0;
  	if ( ioctl(sound_fd,SNDCTL_DSP_STEREO,&value) < 0 ) {
        perror("Setting stereo/mono mode error\n");
        exit(1);
	}

  	value=2;
  	if ( ioctl(sound_fd,SNDCTL_DSP_CHANNELS,&value) < 0 ) {
        perror("Setting mono mode error\n");
        exit(1);
	}

  	value=8000;
  	if ( ioctl(sound_fd,SNDCTL_DSP_SPEED,&value) < 0 ) {
        perror("Setting speed error\n");
        exit(1);
	}

    while(1)
    {
        int n=0;
//int file_fd=0;
        short temp_buf[1024];
        short msr_buf[512];
        char zerobuf[80];
        int flag_record2=0;
        
        memset(zerobuf,0,sizeof(zerobuf));
 
        //file_fd = open("bcwav.wav",O_WRONLY | O_CREAT | O_TRUNC,0666);

        for(;;)
        {
            running=0;
            pthread_mutex_lock(&voice_lock);
            pthread_mutex_unlock(&voice_lock);
            running=1;

            pthread_mutex_lock(&mutex);
            flag_record2 = flag_record;
            pthread_mutex_unlock(&mutex);

            if(flag_record2 == 4)
                break;
            else if(flag_record2 == 0)
            {
                if((n=read(sound_fd,(char *)temp_buf,sizeof(temp_buf))) < 0)
                    perror("read /dsp may be errori1\n");
                continue; //not start record now.
            }
            else if(flag_record2 == 1)
            {
                if((n=read(sound_fd,(char *)temp_buf,sizeof(temp_buf))) < 0)
                    perror("read /dev/dsp may be error\n");
	
                {
                    int i = 0;
                    int index;
                    for(index = 0;index < n/2;index++)
                    {
                        msr_buf[i++]=temp_buf[index++];
                    }	   
                    MSR_SendData((char *)msr_buf, 2*i);	   
                    //index = write(file_fd,(char *)msr_buf, 2*i);	   
                }
                continue;
            }
            else if(flag_record2 == 2)
            {  
                if((n=read(sound_fd,(char *)temp_buf,sizeof(temp_buf))) < 0)
                    perror("read /dev/dsp may be error3\n");
                MSR_SendData(zerobuf, sizeof(zerobuf));
                //usleep(100000);
                continue;
            }
            else if(flag_record2 == 3)
            {
	
                if((n=read(sound_fd,(char *)temp_buf,sizeof(temp_buf))) < 0)
                    perror("read /dev/dev may be error3\n");
                continue;
            }
            else
            {
                printf("record thread status maybe error,%d\n",flag_record2);
                break;
            }
        }

    }
    
    return NULL;
}
#endif

int SpeechRecognition::GetResultIndex()
{
	return m_nResIndex;
}

void SpeechRecognition::ResetResIndex()
{
	m_nResIndex = -1;
}

bool SpeechRecognition::isStarted()
{
	return m_bIsStarted;
}

bool SpeechRecognition::Continue()
{
	if (m_bIsStarted == false)
	{
		return false;
	}
	else
	{
//		engine_continue();
		voice_continue(10);
		return true;
	}
}

bool SpeechRecognition::Pause()
{
	if (m_bIsStarted == false)
	{
		return false;
	}
	else
	{
		voice_stop(10);
		return true;
	}
}

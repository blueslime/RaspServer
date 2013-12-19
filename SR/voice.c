#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/soundcard.h>
#include <sys/ioctl.h>

#include "status.h"
#include "../Dev/sound.h"

#include "common.h"
#include "msr_api.h"

#define DEBUG tty
#if (DEBUG==tty)
#define DP( s, arg... )  printf( "<voice main>:\t" s , ##arg )
#define DE( s, arg... )  fprintf( stderr , "<voice main>:\t" s , ##arg )
#else
#define DP( x... )
#define DE( x... )
#endif
#include "speechrecogn.h"
SpeechRecognition *sr = NULL;
static pthread_t              th_voice,th_test;
static pthread_mutex_t        voice_lock;
static pthread_mutex_t        engine_lock;

static int running=0;
static int engine_running=0;

MSR_VOCABULARYHANDLE hVoc = NULL;

unsigned int g_message;
long g_wParam;
long g_lParam;

BOOL g_blnHasMessage = FALSE;

MSR_ERROR err_flag;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int flag_record = 0;

static int sound_fd;

void processhandler(unsigned int message, long wParam, long lParam)
{
    printf("MSR: is calling used defined functions (message=%x)\n", message);
    
    g_blnHasMessage = TRUE;
    
    g_message = message;
    g_wParam  = wParam;
    g_lParam  = lParam;
}

static void config_snd()
{
    unsigned int value;

 //   if(!sound_fd)return;
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
}


char TestResult[10][MAX_WORD_LENGTH];

void * TestThread(void * attr)
{
    int TestResultNbest = 10;
    
    int i;
    int rejection[10];
    unsigned int message;    
    int retVal;
	pthread_detach(pthread_self());

    while(1)
    {
        for (i = 0; i < 10; i++)
        {
            rejection[i] = 0;
        }
    
        TestResultNbest = 10;
    
        for (i = 0; i < 10; i++) TestResult[i][0] = 0;
   
        MSR_Stop();

        retVal = MSR_Start();
        if ( retVal != MSR_SUCCESS )
        {
            printf("start asr failed: retVal = %d.\n", retVal);
            return NULL;
        }

        printf("please speaking:\n");
     
        pthread_mutex_lock(&mutex);    
        flag_record = 1;
        pthread_mutex_unlock(&mutex);

        while(1)
        {
            int detect_flag = 0;

            engine_running=0;
            pthread_mutex_lock(&engine_lock);
            pthread_mutex_unlock(&engine_lock);
            engine_running=1;

            MSR_STATE state = MSR_State();               
        
            if ( state == RUN )
            {
                if ( detect_flag < 0 )
                {
                    printf("engine internal error: code=%d\n",detect_flag);
                    exit(-1);
                }
                if ( detect_flag <= 1 )
                {
                    detect_flag = MSR_Detect();
                }
                if ( detect_flag >= 1 && detect_flag <= 2 )
                    detect_flag = MSR_Recognize();
            }
        
            if (!g_blnHasMessage)
            {
                continue;
            }
        
            g_blnHasMessage = FALSE;
        
            message = g_message;
            message -= MSR_EVENT_START;
        
            if (message == MSR_EVENT_STARTPOINTDETECTED)
            {
                printf("channel: start point detected...Waiting\n");
                continue;
            }
            else if (message == MSR_EVENT_ENDPOINTDETECTED)
            {   
                pthread_mutex_lock(&mutex);    
                flag_record = 2;
                pthread_mutex_unlock(&mutex);
                printf("channel: end point detected...Waiting\n");
                continue;
            }
            else if (message == MSR_EVENT_SPEECHTOOSHORT)
            {
                strcpy(TestResult[0], "REJECT");
                TestResultNbest = 1;
                MSR_Stop();
                pthread_mutex_lock(&mutex);    
                flag_record = 3;
                pthread_mutex_unlock(&mutex);
                printf("message == MSR_EVENT_SPEECHTOOSHORT\n");
            }
            else if (message == MSR_EVENT_NOSPEECH)
            {
                strcpy(TestResult[0], "REJECT");
                TestResultNbest = 1;
                MSR_Stop();
                pthread_mutex_lock(&mutex);    
                flag_record = 3;
                pthread_mutex_unlock(&mutex);
                printf("message == MSR_EVENT_NOSPEECH\n");
            }
            else if (message == MSR_EVENT_RESULT)
            {
                int err;
            
#ifdef _GetCM_
                err = MSR_GetResult(TestResult, &TestResultNbest, &cm);
#else
                err = MSR_GetResult(TestResult, &TestResultNbest);
#endif
                if ( err != MSR_SUCCESS )
                {
                    exit(-1);
                }
            
                if ( TestResultNbest == 0 )
                { 
                    TestResultNbest = 1;
                    strcpy(TestResult[0], "REJECT");
                }
                else
                {
                    int v;
                    for ( v = 0; v < TestResultNbest; v++ )
                    {
                        printf("%s\n", TestResult[v]);
                    }
                }
                pthread_mutex_lock(&mutex);    
                flag_record = 3;
                pthread_mutex_unlock(&mutex);
                printf("======== Sentence Result ========\n");         
            }
            else if (message != MSR_EVENT_RESULT)
            {
                pthread_mutex_lock(&mutex);    
                flag_record = 3;
                pthread_mutex_unlock(&mutex);
                printf("message != MSR_EVENT_RESULT\n");
            }
            break;
        }
   //     do_event(EV_SPEECH,TestResult[0]);
		pthread_testcancel();
#ifdef _DEBUG
		printf("TestResult[0]: %s\n", TestResult[0]);
#endif
   		if (sr != NULL)
			sr->textReturned(TestResult[0]);
//        config_snd();
        
        pthread_mutex_lock(&mutex);    
        flag_record = 0;
        pthread_mutex_unlock(&mutex);

    }
}


static void * voice(void * attr)
{
 //   if(!sound_fd)

//         if ( (sound_fd = open(SOUND_DEV,O_RDWR,0x666)) < 0 ) {
//             perror("ERROR OPENING DSP DEVICE\n");
//             exit(1);            /* Open error*/
//         }
	sound_fd = MFGetSoundFD();
    config_snd();
	pthread_detach(pthread_self());

    while(1)
    {
        int n=0;
//int file_fd=0;
        short temp_buf[1024];
        short msr_buf[512];
        unsigned char zerobuf[80];
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
                    perror("read /dev/sound/dsp may be errori1\n");
                continue; //not start record now.
            }
            else if(flag_record2 == 1)
            {
                if((n=read(sound_fd,(char *)temp_buf,sizeof(temp_buf))) < 0)
                    perror("read /dev/sound/dsp may be error\n");
	
                {
                    int i = 0;
                    int index;
                    for(index = 0;index < n/2;index++)
                    {
                        msr_buf[i++]=temp_buf[index++];
                    }	   
                    MSR_SendData((unsigned char *)msr_buf, 2*i);	   
                    //index = write(file_fd,(char *)msr_buf, 2*i);	   
                }
                continue;
            }
            else if(flag_record2 == 2)
            {  
                if((n=read(sound_fd,(char *)temp_buf,sizeof(temp_buf))) < 0)
                    perror("read /dev/sound/dev may be error3\n");
                MSR_SendData(zerobuf, sizeof(zerobuf));
                //usleep(100000);
                continue;
            }
            else if(flag_record2 == 3)
            {
	
                if((n=read(sound_fd,(char *)temp_buf,sizeof(temp_buf))) < 0)
                    perror("read /dev/sound/dev may be error3\n");
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

void * TestThread(void * attr);

int msr_init()
{
    err_flag = MSR_Init();
        
    if ( err_flag != MSR_SUCCESS )
    {
        printf("SYS initial fail.\n");
        return -1;
    }
        
    printf("---- System Initial OK. ----\n");
    
    hVoc = MSR_CreateVocabulary(512);
    if ( hVoc == NULL )
        return -1;
    
    if ( MSR_Open (processhandler) != MSR_SUCCESS )
    {
        printf("Invalid msr handle.\n");
        exit(-1);
    }
    printf("---- Open MSR OK ----\n");
	return 0;
}

int voice_init()
{
  /*  err_flag = MSR_Init();
        
    if ( err_flag != MSR_SUCCESS )
    {
        printf("SYS initial fail.\n");
        return -1;
    }
        
    printf("---- System Initial OK. ----\n");
    
    hVoc = MSR_CreateVocabulary(512);
    if ( hVoc == NULL )
        return -1;
    
    if ( MSR_Open (processhandler) != MSR_SUCCESS )
    {
        printf("Invalid msr handle.\n");
        exit(-1);
    }
    printf("---- Open MSR OK ----\n");
*/
    err_flag = MSR_SetVocabularyToDecoder(hVoc);
    if ( err_flag != MSR_SUCCESS )
    {
        DE("Set vocabulary to decoder fail.\n");
        return -1;
    }

    pthread_mutex_init(&voice_lock,NULL);
    pthread_mutex_init(&engine_lock,NULL);
    
    pthread_create(&th_voice,NULL,&voice,NULL);

    pthread_create(&th_test,NULL,&TestThread,NULL);
    return 0;
}

void voice_exit()
{
//    pthread_join(th_voice,NULL);
	pthread_cancel(th_voice);
	pthread_cancel(th_test);
    pthread_mutex_destroy(&voice_lock);
    pthread_mutex_destroy(&engine_lock);
	pthread_mutex_destroy(&mutex);

    return ;
}

int engine_add_word(char * str)
{
    static int i=0;

    if(!str)
        return -1;

    MSR_AddActiveWord(hVoc,str,NULL);
    
    return 0;
}


int engine_stop_clear()
{
    if(MSR_State()==RUN)
        MSR_Stop();
    err_flag = MSR_RemoveVocabularyFromDecoder(hVoc);
    
    if ( err_flag != MSR_SUCCESS )
    {
        DE("Remove vocabulary from decoder fail.\n");
        return -1;
    }

    MSR_DestroyVocabulary(hVoc);
    hVoc = MSR_CreateVocabulary(512);
    if ( hVoc == NULL )
    {
        DE("Failed creating a new vocabulary");
        return -1;
    }

    DP("Sound Engine cleared\n");
    
    return 0;
}

int engine_continue()
{
    err_flag = MSR_SetVocabularyToDecoder(hVoc);
    if ( err_flag != MSR_SUCCESS )
    {
        DE("Set vocabulary to decoder fail.\n");
        return -1;
    }
    
    if(MSR_State()!=RUN)
        MSR_Start();
    
    DP("Sound Engine Restarted\n");

    return 0;
}

int voice_stop(int wait)
{
    int i=0;

    pthread_mutex_trylock(&voice_lock);

    for(i=0;running&&(i<wait);i++)
    {
        usleep(1);
    }

    if(!(wait-=i))
        return -1;

    wait-=i;
    
    pthread_mutex_trylock(&engine_lock);

    for(i=0;engine_running&&(i<wait);i++)
    {
        usleep(1);
    }

    if(!(wait-=i))
        return -1;
    
    if(MSR_State()==RUN)
        MSR_Stop();

	//close(sound_fd);

    return 0;
}

int voice_continue(int wait)
{
    int i;
   	//close(sound_fd);
	//MFCloseSound();
	sleep(1);
// 	if ( (sound_fd = open(SOUND_DEV,O_RDWR,0x666)) < 0 ) {
//             perror("ERROR OPENING DSP DEVICE\n");
//             exit(1);            /* Open error*/
//         }
	sound_fd = MFGetSoundFD();

    config_snd();
    
    if(MSR_State()!=RUN)
        MSR_Start();

    pthread_mutex_unlock(&engine_lock);

    for(i=0;(!engine_running)&&(i<wait);i++)
    {
        usleep(1);
    }

    if(!(wait-=i))
        return -1;

    pthread_mutex_unlock(&voice_lock);

    for(i=0;(!running)&&(i<wait);i++)
    {
        if(running)
            return 0;
        usleep(1);
    }
    
    if(!(wait-=i))
        return -1;
    
    return 0;
}
#ifdef __cplusplus
extern "C"
{
#endif
void *my_malloc(int sz)
{
    return malloc(sz);
}

void my_free(void *ptr)
{
    return free(ptr);
}

void msr_lock()
{
}

void msr_unlock()
{
}
#ifdef __cplusplus
}
#endif

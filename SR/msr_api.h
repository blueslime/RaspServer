/*******************************************************************************
 *
 *                   THINKIT PROPRIETARY INFORMATION
 *
 * This software is supplied under the terms of the license agreement
 * or nondisclosure agreement with THINKIT and may not be copied
 * or disclosed except in accordance with the terms of that agreement.
 *
 *          Copyright (c) 2002-2004 THINKIT. All Rights Reserved.
 *
 *******************************************************************************/

#ifndef _MSRAPI_H_
#define _MSRAPI_H_

#ifdef  __cplusplus
extern "C" {
#endif
    
#if !defined SHORT
typedef short SHORT;
#endif

#if !defined INT32
typedef int INT32;
#endif

#if !defined UCHAR
typedef unsigned char UCHAR;
typedef unsigned char uchar;
#endif

/* ----- MSR function return values ----- */
#define      MSR_SUCCESS                0       /* function success return value */
#define      MSR_INVALID_HANDLE         -1
#define      MSR_ERROR_UNKNOWN          -100
#define      MSR_SYSFILE_OPEN_ERR       -101
#define      MSR_STATE_ERR              -102
#define      MSR_POINTER_ERR            -103
#define      MSR_SAVE_TO_PDB_ERR        -104
#define      MSR_INSERT_TO_PDB_ERR      -105
#define      MSR_WORDLENGTH_TOOLONG     -106
#define      MSR_MEMALLOC_ERR           -107
#define      MSR_OPEN_ERR               -108
#define      MSR_PARAMRANGE_ERR         -109
#define      MSR_WORDNOTINVOC           -110
#define      MSR_START_ERR              -111
#define      MSR_POSTMESSAGE_ERR        -112
#define      MSR_GETWIDX_FROM_PDB_ERROR -113
#define      MSR_SAVE_TREE_IN_MEM_ERROR -114
#define      MSR_SEND_DATA_LENGTH_ERROR -115
#define      MSR_SEND_DATA_NULL_POINTER -116
#define      MSR_SEND_DATA_IGNORED      -117
#define      MSR_SEND_TOOMORE_DATA_ONCE -118
#define      MSR_UNKNOWN_CODING_FORMAT  -119
#define      MSR_VAD_INIT_ERROR         -120
#define      MSR_TOO_MANY_WORDS         -121

/* ----- parameter type value definition ----- */
#define     MSR_PARAM_VOICE_CODING      0
    /* speech format code */
    #define     MSR_ALAW_PCM            0   /* 8k 8bit a-law */
    #define     MSR_ULAW_PCM            1   /* 8k 8bit u-law */
    #define     MSR_LINEAR_PCM          2   /* 8k 8bit linear pcm, centered at value 128 */
    #define     MSR_RAW_16              4   /* 8k 16 bit raw data */
    #define     MSR_16K_16              8   /* 16k 16 bit raw data */

/* ----- input sppech feature ----- */
#define     MSR_FEATURE                 0x1000  
#define     MSR_FEATURE_MFCC            0x1001  
#define     MSR_FEATURE_MFCC_E          0x1002  
#define     MSR_FEATURE_FVQCODE         0x1003  

/* ----- MSR event type defiition ----- */
#define     MSR_EVENT_START                 0x8000

#define     MSR_RESULT                      (MSR_EVENT_START + 1)
#define     MSR_SPEECHTOOSHORT              (MSR_EVENT_START + 2)
#define     MSR_WAIT_SPEECH_TOO_LONG        (MSR_EVENT_START + 3)
#define     MSR_SPEECH_START_DETECTED       (MSR_EVENT_START + 4)
#define     MSR_SPEECH_END_DETECTED         (MSR_EVENT_START + 5)
#define     MSR_RESULT_REJECTION            (MSR_EVENT_START + 6)
#define     MSR_NOISE_ENERGY                (MSR_EVENT_START + 7)

#define     MSR_EVENT_RESULT                1   
#define     MSR_EVENT_SPEECHTOOSHORT        2
#define     MSR_EVENT_NOSPEECH              3
#define     MSR_EVENT_STARTPOINTDETECTED    4
#define     MSR_EVENT_ENDPOINTDETECTED      5
#define     MSR_EVENT_NOISEENERGY           7

/* ----- internal message ----- */
#define     MSR_STOP                        (MSR_EVENT_START + 64)
#define     MSR_VOICE                       (MSR_EVENT_START + 65)
#define     MSR_CLOSE                       (MSR_EVENT_START + 66)

#define     MSR_API_VERSION                 "EMBEDDED_C_FIX_API_v2.02.0023"

typedef enum _MSR_STATE_  
{
    BOOT = 0,
    IDLE,
    RUN,
    RECOVER
} MSR_STATE;      
    
typedef int     MSR_ERROR;
typedef void*   MSR_VOCABULARYHANDLE;
#define     MAX_WORD_LENGTH                 32      /* max length for single word in dictionary */

// #define _GetCM_

/* ----- callback function definition -----*/
typedef void (*MSR_MessageCallBack) (unsigned int message, long wParam, long lParam);

/* ----- system functions ----- */
int         MSR_Init(void);
int         MSR_InitWFS(char *pdbfile);
int         MSR_Exit(void);
int         MSR_Open(MSR_MessageCallBack lpMessageProc);
int         MSR_Close(void);
int         MSR_Start(void);
int         MSR_Stop(void);

#ifndef _GetCM_
int         MSR_GetResult(char pResult[][MAX_WORD_LENGTH], int *nbest);
#else
int         MSR_GetResult(char pResult[][MAX_WORD_LENGTH], int *nbest, float *cm);
#endif

int         MSR_SendData(unsigned char *buffer, int len);
int         MSR_AddActiveWord(MSR_VOCABULARYHANDLE hVoc, char *word_input, char *word_name /*= NULL*/);
int         MSR_DeleteActiveWord(MSR_VOCABULARYHANDLE hVoc, char *word_input);
MSR_STATE   MSR_State(void);
int         MSR_Recognize(void);
int         MSR_Detect(void);
MSR_VOCABULARYHANDLE MSR_CreateVocabulary(int flash_size/*=128*/);
MSR_VOCABULARYHANDLE MSR_CreateVocabularyWFS(int flash_size/*=128*/, char *treenodefn, char *treesuccfn);
int         MSR_DestroyVocabulary(MSR_VOCABULARYHANDLE hVoc);
int         MSR_SetVocabularyToDecoder(MSR_VOCABULARYHANDLE hVoc);
MSR_VOCABULARYHANDLE MSR_GetVocabularyFromDecoder(void);
int         MSR_RemoveVocabularyFromDecoder(MSR_VOCABULARYHANDLE hVoc);

void        MSR_EnableNoiseDetection(int blnEnable);
void        MSR_SetNoiseEnergy(int energy);
void        MSR_SetNoiseEnergy(int energy);

#ifdef  __cplusplus
}
#endif

#endif


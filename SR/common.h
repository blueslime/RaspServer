/*==========================================================================*/
/* This is basic type definition header file for whole fix_point C project  */
/*                                                                          */
/*                           Mar 10, 2004                                   */
/*                           --Liu Jian--                                   */
/*==========================================================================*/

#ifndef __FXP_COMMON_H__
#define __FXP_COMMON_H__

#include "typedefs.h"

#ifndef _CONSOLE
#define _EPSON_C33           /* ----- EPSON PLATFORM ----- */
#endif

#include <stdio.h>
#include <stdlib.h>

#ifndef  _EPSON_C33
#include <memory.h>
#endif

#include <string.h>
#include <math.h>

/* this is a subset of common.h, since public datastructure should be less than internal used */
#include "msr_api.h" 

#ifndef bool
#define bool    int
#define true    1
#define false   0
#endif
#ifndef BOOL
#define BOOL    int
#endif

#ifndef FALSE
#define FALSE   0
#endif
#ifndef TRUE
#define TRUE    1
#endif
/*
#ifndef max
#define max(x, y)   ((x > y) ? x : y)
#endif

#ifndef min
#define min(x, y)   ((x < y) ? x : y)
#endif
*/
/*configuration for program*/
#ifdef _GetCM_
#define SENONE_SEG      /* with cm enabled or disabled */
#endif
#define _MEM_OPTIMAL_   /*tree bode structure size reduced*/

#define _P4_PLATFORM_ /* pure c++ code */
/* #define _ARM_PLATFORM_  /* for Intel xscale platform */

/* Definition of constant values */
#define TOP_N_BEST              10      /*maximum nbest result available from decoder*/

#define MAX_PRONS_PER_WORD      128     /* max number of mono phone for single word in dictionary */
#define MAX_PRONS_HALF_WORD     (MAX_PRONS_PER_WORD / 2)
#define MAX_MP_NUMBER_PER_WORD  32      /* max MP number allowed for one word */

#define     fxpRndZero      0x0
#define     fxpRndNear      0x1

/* Definition of Types */
#ifdef _CONSOLE

//typedef unsigned short  ushort;
//typedef unsigned int    uint;

#else

//#ifndef _EPSON_C33
//#include <stdio.h>
///#define _MAX_FNAME FILENAME_MAX
//#else
//typedef unsigned short  ushort;
//typedef unsigned int    uint;
//#endif

#endif

typedef int             phnenc;     /* for phone encoding */

/* type define for feature obv */
#define OBVTYPE         Word16  /*16 bit singed int*/
#define ACCOBVTYPE      Word32      /*32 bit signed int*/
/* type define for prob */
#define PROBTYPE        Word16
#define ACCPROBTYPE     Word32
/* type define for critical integer date type inside decoder */
#define INTTYPE         ushort

#define FXP_MAX_8U     ( 0xFF )
#define FXP_MAX_16U    ( 0xFFFF )
#define FXP_MAX_32U    ( 0xFFFFFFFF )
#define FXP_MIN_8S     (-128 )
#define FXP_MAX_8S     ( 127 )
#define FXP_MIN_16S    (-32768 )
#define FXP_MAX_16S    ( 32767 )
#define FXP_MIN_32S    (-2147483647 - 1 )
#define FXP_MAX_32S    ( 2147483647 )

#define LW_SIGN (long)0x80000000       /* sign bit */
#define LW_MIN (long)0x80000000
#define LW_MAX (long)0x7fffffff

/* basic type definition */
/* for GVQ */
typedef struct _aGVQHeader
{
   short fvqcbkSize; /* the size of fvq code book for a feature stream */
   short cbkSize;   /* the size of gvq code book for a feature stream */
   short streamNum; /* the stream number of feature vector */
   short streamLen;
   short scaleFactor;
   OBVTYPE *code; /* start pointer for GVQ codebook content */
} aGVQHeader;

/* for SDC HMM Param */
#define SDCHMM_MIXTURE_NUM  8       /*only support this value!!*/
#define SDCHMM_STREAM_NUM   24      /*only support this value!!*/

typedef ushort aHMMState ;
typedef struct _SDCHMMPARAM
{
    uchar vecSize;            /* the feature vector size */
    uchar maxStateNum;        /* the maximum number of states per hmm */
    ushort exStateNum;        /* the exStateSet number, total state number */
    ushort wgtNum;            /* total number of weights */
    ushort mixNum;            /* total number of mixtures */
    aHMMState *exStateSet;
    uchar *eIdxSet;
    PROBTYPE *wgtSet;
} SDCHMMPARAM;

/* for HMM MAP decision-tree based */
/* for editing state phone based lexical tree dynamically */
typedef struct _aHMMDcsTreeNode
{
   short left,right;        /* child nodes */
   short quest;             /* index of the best question */
   short exstate;           /* index of exStateSet in hmm.param */
} aHMMDcsTreeNode;

typedef struct _DecisionTreeHeader
{
   int nmono;             /* mono-phone number */
   int maxStateNum;       /* maximum state number */
   int nodeNum;           /* number of tree nodes */
   int qNum;              /* number of questions */
   int subqNum;           /* number of sub-questions */
}DecisionTreeHeader;

/* for CM MAP */
#define MAPFORONE2ONE   0x1 /* only support this kind now*/
#define MAPFORDECISION  0x2 /* not support */
#define MAPONEONONETSK  0x3 /* not support */
typedef struct _CMMap
{
   ushort mapType;  /* one of above */
   ushort monoLen;
   ushort monoTee;
   ushort taskStateNum;
   ushort *monoMapTbl;
   /*ushort *monoStateTbl;*/
} CMMap;

/* for LexTree */
typedef struct _aLexTreeHeader
{
    INTTYPE silStartStateIdx;   /* the first state index of silence model */
    INTTYPE teeStateIdx;        /* the state index of tee model */
    int     wordSetNum;         /* the number of iname set */
    int     wordNum;            /* the number of lname set */
    int     nodeNum;            /* the number of tree nodes */
    int     maxchildren;        /* the number of maxium children number */
    int     StatePhoneNum;      /* the number of all state index in the tree */
} aLexTreeHeader;

/* for dictionary header */
typedef enum _PronDictType 
{
   forGraphDecoder = 0, /* "prons" field of PronDict structure is filled with tri-phone expansion index */
   forTreeDecoder,      /* "prons" field of PronDict structure is omitted since PronDict Tree file contain such info. */
   forLookupOnly,       /* "prons" field don't do TriphoneExpansion, store mono phone idx */
   UNKNOWN
} PronDictType;

typedef struct _PronDictHeader
{
    int wordNum;        /* LM name number */
    int wordSetNum;     /* Iname number */
    int wordStartIdx;   /* Iname start index exclude "<enter>" , equal to 1 */
    int endStartIdx;    /* Iname end index, "<exit>", equal wordSetNum=1 */
    int phnSpecNum;     /* Special phone number, e.g. "garbage", "sil", "tee" ... */
    int wordPhnNum;     /* total mono phone number of all words in dictionary file */
    int bytePhnNum;     /* indicate how many bits should be used for maximum mono phone number, 8bit-->1, 16bit-->2 */
    PronDictType type;  /* see definition for "PronDictType" */
} PronDictHeader;

/*********** for treedecoder *****************************/
#ifdef SENONE_SEG
typedef struct _aSenoneTrace
{
    struct _aSenoneTrace *next;
    short time;             /* time stamp in senone level */
    INTTYPE use;
    INTTYPE senoneIdx;      /* corresponding to state index */
    PROBTYPE p;             /* prob likelihood at the end of the state phone */
} aSenoneTrace;
#endif

typedef struct _aTceToken
{
    struct _aTrace *trace;      /* trace information */
#ifdef SENONE_SEG
    aSenoneTrace *senoneTrace;  /* senonoe trace corresponding to the current word index */
#endif
    PROBTYPE p;                 /* maximum likelihood of the local path */
} aTceToken;  

typedef struct _aTrace
{
    short   time;                /* time frame */
    INTTYPE   use;               /* use counter */
    INTTYPE    widx;             /* word node */
    INTTYPE    nattach;          /* the number of attaches */
    struct _aTceToken attachs[1];/* attached trace */
} aTrace;

typedef struct _aTokenLnk
{
    struct _aTokenLnk *next;
    struct _aTrace *trace;       /* history link of the token */
#ifdef SENONE_SEG
    aSenoneTrace *senoneTrace;   /* current senone trace until current patial path of the current word */
#endif
    PROBTYPE p;                  /* likelihood */
}aTokenLnk;

typedef struct _aTreeNode
{
    struct _aTreeNode *lnk;      /* active link */
    struct _aTokenLnk *tklnk;    /* passing tokens */
    union
    {
        struct
        {
            INTTYPE triphnIdx;         /*state phone index */
            INTTYPE nsucc;             /* number of child nodes */
            struct _aTreeNode **succs; /* child nodes */
        } node;
        struct
        {
            INTTYPE latt;              /* lattice index */
        } word;
    } is;
    
#ifndef _MEM_OPTIMAL_
    INTTYPE wenum;                   /* word end node number */
    INTTYPE rnum;                    /* regular node nubmer */
#endif
    
    INTTYPE depth;               /* tree depth */
    INTTYPE option;              /* node type tag */
}aTreeNode;
/*********** for treedecoder end *****************************/

/* basic structure for tree online operation */
typedef struct _aPronStateDict
{
    INTTYPE *sidx;              /* state index expansion for all tri-phones of this word */
    INTTYPE nstate;             /* number of states of this word including "sp" after extended */
    INTTYPE *option;            /* see definition in "option.h" */
} aPronStateDict;

typedef struct _oneStrm
{
    int strmLen;
    int *strmStruct;
} oneStrm; 

typedef struct _aTrans
{
    struct _aTrans *next;        /* next transcription */
    INTTYPE widx;                /* lattice index */
    ushort t;                    /* time alignment for that word */
    INTTYPE use;                 /* use count */
    PROBTYPE p;                  /* base probability */
    PROBTYPE dp;                 /* probability increasement */
#ifdef SENONE_SEG
    struct _aSenoneTrace *senoneTrace;       /* store phone trace to the word with index widx */
#endif
}aTrans;

typedef struct _aTransSet
{
    struct _aTrace *prev;        /* next trace back */
    struct _aTrans *next;        /* the word link */
    PROBTYPE p;                  /* total probability */
}aTransSet;

typedef struct _NodeCounts
{
    int ncnt;                   /* nodes counter */
    int wencnt;                 /* word end nodes counter */
    int nsncnt;                 /* non-sole child nodes counter */
}NodeCounts;

/* for Lable new word */
typedef struct _LableName
{
    char name[MAX_WORD_LENGTH];
    uchar prons[MAX_PRONS_PER_WORD];    /* mono idx number must be smaller 256 */
} LableName;

/* for state phone based confidence measure */
typedef struct _aSenoneHypo
{
    int time;                   /* time stamp in senone level */
    int dur;
    int senoneIdx;              /* corresponding to state index */
    float csv;                  /* confidence score value in senone value */
    struct _aSenoneHypo *next;
}aSenoneHypo;
  
typedef struct _aWordHypo
{
    int time;                   /* time frame */
    int dur;
    int widx;                   /* word index */
    float csv;                  /* confidence score value in word level */
    struct _aSenoneHypo *sHypo;             /* senone trace within the current phone */
}aWordHypo;

typedef struct _WORDLINK
{
    char word[MAX_WORD_LENGTH];
    float confidence;
    aWordHypo *wHypo;                   /* for store state phone trace informance for current word */
    struct _WORDLINK *next;
} WORDLINK;

typedef struct
{
    int NBest;
    WORDLINK **sen;
} MSR_SENRESULT;

typedef struct _TreeDecoder             TreeDecoder;
typedef struct _CMemChunk               CMemChunk;
typedef struct _SDTBeam                 SDTBeam;
typedef struct _iLexTree                iLexTree;
typedef struct _iTreeDecoder            iTreeDecoder;

/* global functions */
#define ownCopy_16s(src, desc, len)   memcpy(desc, src, (len) * sizeof(Word16))
#define ownZero_16s(src, len)         memset(src, 0, (len) * sizeof(Word16))
#define ownZero_32s(src, len)         memset(src, 0, (len) * sizeof(Word32))
#define ownZero_8u(src,len)           memset(src, 0, (len) * sizeof(Word8u))

/* defined in CodecConvert.c */
short ulaw2linear(unsigned char ulawbyte);
short alaw2linear(unsigned char ulawbyte);

/* defined in chunk.c */
void Exit(int x);
void GetTimeLog(char * szMsg);

#define CAL_MALLOC_SPACE
#ifdef  CAL_MALLOC_SPACE
#define MAX_MALLOC_SIZE     (1024 * 1024 * 32)
extern unsigned long g_iTotalMalloc;
#endif

int mystricmp(char *s, char *t);
#include "chunk.h"

#define SEG_LEN                         4   /* must be 4*n integer */

#endif

/*
2.4 kbps MELP Proposed Federal Standard speech coder

Fixed-point C code, version 1.0

Copyright (c) 1998, Texas Instruments, Inc.  
*/
/* typedef statements of types used in all routines */

#include <stdio.h>
#include <stdlib.h>

#ifndef __TYPEDEFS
#define __TYPEDEFS

#define LW_SIGN (long)0x80000000       /* sign bit */
#define LW_MIN (long)0x80000000
#define LW_MAX (long)0x7fffffff

#define SW_SIGN (short)0x8000          /* sign bit for Word16 type */
#define SW_MIN (short)0x8000           /* smallest Ram */

#ifdef  SW_MAX 
#undef  SW_MAX 
#endif

#define SW_MAX (short)0x7fff           /* largest Ram */

/* Definition of Types *
 ***********************/

typedef long int Word32;             /* 32 bit "accumulator" (L_*) */
typedef short int Word16;           /* 16 bit "register"  (sw*) */

typedef short int ShortwordRom;        /* 16 bit ROM data    (sr*) */
typedef long int LongwordRom;          /* 32 bit ROM data    (L_r*)  */

typedef unsigned short UShortword;     /* 16 bit unsigned data */
typedef unsigned long ULongword;       /* 32 bit unsigned data */

struct NormSw {                        /* normalized Word16 fractional
                                        * number snr.man precedes snr.sh
                                        * (the shift count)i */
  Word16 man;                       /* "mantissa" stored in 16 bit
                                        * location */
  Word16 sh;                        /* the shift count, stored in 16 bit
                                        * location */
};


/* Global constants *
 ********************/


#define NP 10                          /* order of the lpc filter */
#define N_SUB 4                        /* number of subframes */
#define F_LEN 160                      /* number of samples in a frame */
#define S_LEN 40                       /* number of samples in a subframe */
#define A_LEN 170                      /* LPC analysis length */
#define OS_FCTR 6                      /* maximum LTP lag oversampling
                                        * factor */
#define OVERHANG 8                      /* comfort noise parameter:
                  number of frames averaged to 
                  CN generate reference */
#define OH_SHIFT 3                      /* shift corresponding to 
                  OVERHANG */
#define PN_INIT_SEED (Word32)0x1091988L /* initial seed for Comfort
                    noise pn-generator */

#define strStr strStr16

/* global variables */
/********************/

extern int giFrmCnt;                   /* 0,1,2,3,4..... */
extern int giSfrmCnt;                  /* 0,1,2,3 */
extern int giOverflow;
extern int giCarry;
extern int giRandomRF;


/* Primary comfort noise/DTX state variable */
extern Word16 gswDTXFrmCnt; /* encoders DTX frame counter */
extern Word16 gswLastDTXFrmCnt; /* gswDTXFrmCnt delayed by 1 frame */


/* !! upd !! PHILIPS #1*/
extern int giDTXon;   /* DTX Mode on/off */
/* !! end of update !! PHILIPS #1 */

extern int  giWriteFile;
extern FILE *gfpUtil;

#define max(a,b)  (((a) > (b)) ? (a) : (b))
#define min(a,b)  (((a) < (b)) ? (a) : (b))

#endif


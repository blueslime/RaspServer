/*******************************************************************************
 *
 *                       HCCL PROPRIETARY INFORMATION
 *
 * This software is supplied under the terms of the license agreement
 *   or nondisclosure agreement with HCCL and may not be copied
 * or disclosed except in accordance with the terms of that agreement.
 *
 *                Copyright (c) 2002 HCCL. All Rights Reserved.
 *
 *******************************************************************************/

/*
This file implements memory management for lists consisting of elements of same size.
The requirements are as follow:
    1. Avoid overhead for each element;
    2. Can grow automatically;
    3. Keep the old interface as in <mem.h>
*/

#ifndef _CHUNK_H_
#define _CHUNK_H_

#ifdef __cplusplus
extern "C" 
{
#endif

#ifndef BOOL
#define BOOL        int
#endif

typedef struct _aChunkCell
{
    struct _aChunkCell *nextCell;
}aChunkCell;

typedef struct _aMemChunkHeader
{
    struct _aMemChunkHeader *next;  /* points to next chunk */
    struct _aMemChunkHeader *prev;  /* points to prev chunk */
    int nCells;                     /* number of cells in this chunk */
    void *memData;
}aMemChunk;

void *Malloc(int sz,BOOL clear/*=false*/);
//void *Malloc_1D(int n1,int itsz,BOOL clear/*=false*/);
//void **Malloc_2D(int n1,int n2,int itsz,BOOL clear/*=false*/);
//void ***Malloc_3D(int n1,int n2,int n3,int itsz,BOOL clear/*=false*/);
void *Malloc32(int sz,BOOL clear/*=false*/);
// void *Malloc32_1D(int n1,int itsz,BOOL clear/*=false*/);
void Free(void *ptr); 
void Free32(void *ptr);
//void Free_1D(void **ptr,int n1);
//void Free_2D(void ***ptr,int n1,int n2);

//void *LoadFile(char *file,int *length/*=NULL*/);
//void *LoadHeader(char *file, int *length);
//void *LoadFile32(char *file,int shift,int *length/*=NULL*/);
// int MemUsed(void);

struct _CMemChunk
{
    aMemChunk *mcHead, *mcTail;             /* header and tail pointers for memory chunk list */
    struct _aChunkCell *fcHead, *fcTail;    /* header and tail pointer of free cell list */

    int au;         /* initial unit number in chunk */
    int cellSize;   /* bytes per cell */
    int inc;        /* number of units incremented */
};

BOOL CMemChunk_AddChunk(CMemChunk *self, BOOL clear/* = false*/);
void *CMemChunk_New(CMemChunk *self, BOOL clear/* = false*/);
void CMemChunk_Delete_P2(CMemChunk *self, void *ptr);
BOOL CMemChunk_Init(CMemChunk *self, BOOL clear/* = false*/);
void CMemChunk_ResetMem(CMemChunk *self);
void CMemChunk_CheckMem(CMemChunk *self);

CMemChunk *CMemChunk_New_P3(int au/* = 128*/, int cellSize/* = 16*/, int inc/* = 64*/);
void CMemChunk_Delete(CMemChunk *self);

#ifdef __cplusplus
}
#endif

#endif

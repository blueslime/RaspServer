#ifndef STRUCT_FOR_MF270_H
#define STRUCT_FOR_MF270_H

typedef struct tagMF_Threshold
{
	unsigned char flag;
	int major;
	int minor;
}STMfth;

//flag
#define MF_HTH	1
#define MF_STH	2
#define MF_ITH	3
#define MF_MODE 4

#endif

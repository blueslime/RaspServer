#ifndef TYPE_H_H
#define TYPE_H_H


#ifdef _DEBUG
#define dprintStr(s) \
	fprintf(stderr, s": %d(%s)\n", __LINE__, __FILE__)
#else
#define dprintStr(s) 
#endif

typedef unsigned char UCHAR;
typedef unsigned char BYTE;
typedef unsigned int UINT;
typedef void * LPVOID;
typedef bool BOOL;
//typedef int SOCKET;
typedef short WORD;
typedef long DWORD;
#define FALSE false
#define TRUE true
#define WINAPI
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1

//自定义UDP接收函数指针
typedef void (*PBUFREC)(char* , int);
typedef void (*PIMGREC)(unsigned char* , int , int);
#define ABS(x)((x)<0?-(x):(x))

#define Sleep usleep
#define WSAGetLastError() errno
#define closesocket close

#ifdef _DEBUG
#define ASSERT(expr, debug, ret) \
{\
	if (!expr)\
	{\
		debug;\
		return ret;\
	}\
}
#else
#define ASSERT() 
#endif

#if 0
#include <IOSTREAM>
class CError
{
	static const char * const errDesc[];/* = {"Link disconnected",
											"Not enough memory"};*/
	enum {ERROR_DISCONNECTION = 0, ERROR_NOT_ENOUGH_MEM};
	int errno;
public:
	CError(int err): errno(err){}
	virtual ~CError(){}
	int getErrorCode() const
	{
		return errno;
	}
	const char *getErrorDescription() const
	{
		if ((errno >= 0) && (errno < sizeof(errDesc)))
		{
			return errDesc[errno];
		}
		else
		{
			return NULL;
		}
	}
	friend std::ostream &operator<<(std::ostream &os, const CError &err);
};

inline std::ostream &operator<<(std::ostream &os, const CError &err)
{
	return os<< err.getErrorCode()<< " ("<<err.getErrorDescription()<<") ";
}

const char * const CError::errDesc[] = {"Link disconnected",
										"Not enough memory"};
#endif

#endif

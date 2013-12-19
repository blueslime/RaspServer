#ifndef CQUEUE_H_H
#define CQUEUE_H_H
//#include <queue>
#ifdef WIN32
#include <WINDOWS.H>
#endif
#ifdef __LINUX__
#include <pthread.h>
#endif

template <class T, int nsize = 32>
class CQueue
{
//	enum {BufferSize = 32};
	int BufferSize;
#ifdef WIN32
	HANDLE hMutex;
	HANDLE hNotFullEvent, hNotEmptyEvent;
#endif
#ifdef __LINUX__
	pthread_cond_t condNotFullEvent, condNotEmptyEvent;
	pthread_mutex_t mutex;
#endif
	T      tData[nsize];
	int    nReadPos, nWritePos;
public:
	CQueue();
	virtual ~CQueue();
	bool isEmpty() const;
	int size() const;
	void push(T &);
	T& pop();
	bool tryPop(T &);
};
template <class T, int nsize>
CQueue<T, nsize>::CQueue(): nReadPos(0), nWritePos(0)
{
#ifdef WIN32
	hMutex = CreateMutex(NULL,FALSE,NULL);
	hNotFullEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
	hNotEmptyEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
#endif
#ifdef __LINUX__
	pthread_cond_init(&condNotFullEvent, NULL);
	pthread_cond_init(&condNotEmptyEvent, NULL);
	pthread_mutex_init(&mutex, NULL);
#endif
	BufferSize = nsize;
}
template <class T, int nsize>
CQueue<T, nsize>::~CQueue()
{
#ifdef WIN32
	CloseHandle(hMutex);
	CloseHandle(hNotEmptyEvent);
	CloseHandle(hNotFullEvent);
#endif
#ifdef __LINUX__
	pthread_cond_destroy(&condNotEmptyEvent);
	pthread_cond_destroy(&condNotFullEvent);
	pthread_mutex_destroy(&mutex);
#endif
}

template <class T, int nsize>
bool CQueue<T, nsize>::isEmpty() const
{
	bool flag;
#ifdef WIN32
	WaitForSingleObject(hMutex, INFINITE);
#endif
#ifdef __LINUX__
	pthread_mutex_lock(&mutex);
#endif
	flag = (nReadPos==nWritePos) ? true: false;
#ifdef WIN32
	ReleaseMutex(hMutex);
#endif
#ifdef __LINUX__
	pthread_mutex_unlock(&mutex);
#endif
	return flag;
}
template <class T, int nsize>
int CQueue<T, nsize>::size() const
{
	int len;
#ifdef WIN32
	WaitForSingleObject(hMutex, INFINITE);
#endif
#ifdef __LINUX__
	pthread_mutex_lock(&mutex);
#endif
	len = (nWritePos<nReadPos) ? (BufferSize+nWritePos-nReadPos): (nWritePos-nReadPos);
#ifdef WIN32
	ReleaseMutex(hMutex);
#endif
#ifdef __LINUX__
	pthread_mutex_unlock(&mutex);
#endif
	return len;
}
template <class T, int nsize>
void CQueue<T, nsize>::push(T &elem)
{
#ifdef WIN32
	WaitForSingleObject(hMutex, INFINITE);
#endif
#ifdef __LINUX__
	pthread_mutex_lock(&mutex);
#endif
	while ((nWritePos+1)%BufferSize == nReadPos)
	{
#ifdef WIN32
		ReleaseMutex(hMutex);
		WaitForSingleObject(hNotFullEvent, INFINITE);
		WaitForSingleObject(hMutex, INFINITE);
#endif
#ifdef __LINUX__
		pthread_cond_wait(&condNotFullEvent, &mutex);
#endif
	}
	tData[nWritePos++] = elem;
	nWritePos = (nWritePos == BufferSize) ? 0: nWritePos;
#ifdef WIN32
	ReleaseMutex(hMutex);
	PulseEvent(hNotEmptyEvent);
#endif
#ifdef __LINUX__
	pthread_mutex_unlock(&mutex);
	pthread_cond_signal(&condNotEmptyEvent);
#endif
}
template <class T, int nsize>
T &CQueue<T, nsize>::pop()
{
#ifdef WIN32
	WaitForSingleObject(hMutex, INFINITE);
#endif
#ifdef __LINUX__
	pthread_mutex_lock(&mutex);
#endif
	while (nReadPos == nWritePos)
	{
#ifdef WIN32
		ReleaseMutex(hMutex);
		WaitForSingleObject(hNotEmptyEvent, INFINITE);
		WaitForSingleObject(hMutex, INFINITE);
#endif
#ifdef __LINUX__
		pthread_cond_wait(&condNotEmptyEvent, &mutex);
#endif
	}
	T &t = tData[nReadPos++];
	nReadPos = (nReadPos == BufferSize)? 0: nReadPos;
#ifdef WIN32
	ReleaseMutex(hMutex);
	PulseEvent(hNotFullEvent);
#endif
#ifdef __LINUX__
	pthread_mutex_unlock(&mutex);
	pthread_cond_signal(&condNotFullEvent);
#endif
	return t;
}

template <class T, int nsize>
bool CQueue<T, nsize>::tryPop(T &t)
{
#ifdef WIN32
	WaitForSingleObject(hMutex, INFINITE);
#endif
#ifdef __LINUX__
	pthread_mutex_lock(&mutex);
#endif
	if (nReadPos != nWritePos)
	{
		t = tData[nReadPos++];
		nReadPos = (nReadPos == BufferSize)? 0: nReadPos;
#ifdef WIN32
		ReleaseMutex(hMutex);
		PulseEvent(hNotFullEvent);
#endif
#ifdef __LINUX__
		pthread_mutex_unlock(&mutex);
		pthread_cond_signal(&condNotFullEvent);
#endif
		return true;
	}
	else
	{
#ifdef WIN32
		ReleaseMutex(hMutex);
#endif
#ifdef __LINUX__
		pthread_mutex_unlock(&mutex);
#endif
		return false;
	}
}

#endif

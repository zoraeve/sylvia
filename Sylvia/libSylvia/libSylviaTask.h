#pragma once

#include <map>
#include <string>
#include <deque>

#include <time.h>

#include "libSylviaUtility.h"

#ifdef LIBSYLVIA_IN_WINDOWS
#include <pthread.h>
#pragma comment(lib, "pthreadVC2.lib")
#elif defined LIBSYLVIA_IN_LINUX
#include "pthread.h"
#endif

class libSylviaTask
{
public:
	libSylviaTask(const LIBSYLVIA_TASK& task);
	~libSylviaTask(void);

public:
	std::string TI;
	unsigned int TotalFrames;
	unsigned int CompleteParts;
	unsigned int TotolSize;
	float Progress;
	float AvgSpeed;
	float MaxSpeed;
	float MinSpeed;
	float DownloadSize;
	LIBSYLVIA_METHOD Method;
	LIBSYLVIA_STATUS Status;
	time_t Expend;
	std::string URI;
	std::string SaveAs;

public:
	pthread_t tidMaintain;
	pthread_t tidSaveToFile;
	pthread_t threadPool[LIBSLYVIA_THREADPOOLSIZE];
	
public:
	std::deque<int> TaskQ;
	pthread_rwlock_t TaskQLock/* = PTHREAD_RWLOCK_INITIALIZER*/;

public:
	std::deque<LIBSYLVIA_CONTENT> ContentsQ;
	pthread_rwlock_t ContentsQLock/* = PTHREAD_RWLOCK_INITIALIZER*/;

public:
	int Start();
	int Stop();

public:
	int GetHttpContentLength();

public:
	int Query(LIBSYLVIA_INFO& info);
	int Pause();
	int Resume();
	int Cancel();
};


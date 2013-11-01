#pragma once

#include <map>
#include <string>
#include <deque>

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
	libSylviaTask(void);
	~libSylviaTask(void);

public:
	unsigned int TotalFrames;
	unsigned int CompleteParts;
	unsigned int TotolSize;
	float Progress;
	float DownloadSpeed;
	float DownloadSize;
	LIBSYLVIA_METHOD Method;
	LIBSYLVIA_TASK_STATUS Status;
	std::string URI;
	std::string SaveAs;
	
public:
	std::deque<int> TaksQ;
	pthread_rwlock_t TaskQLock/* = PTHREAD_RWLOCK_INITIALIZER*/;

public:
	std::deque<LIBSYLVIA_CONTENT> ContentsQ;
	pthread_rwlock_t ContentsQLock/* = PTHREAD_RWLOCK_INITIALIZER*/;

public:
	int Start();
	int Stop();

public:
	int Query(LIBSYLVIA_STATUS& status);
	int Pause();
	int Resume();
	int Cancel();
};


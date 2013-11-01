#pragma once

#include "libSylviaUtility.h"
#include "libSylviaLogger.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <deque>
#include <map>
using namespace std;

#ifdef LIBSYLVIA_IN_WINDOWS
#include <pthread.h>
#pragma comment(lib, "pthreadVC2.lib")
#elif defined LIBSYLVIA_IN_LINUX
#include "pthread.h"
#endif

class libSylviaEngine
{
public:
	libSylviaEngine(void);
	~libSylviaEngine(void);

public:
	std::string currentURI;

public:
	int AddTask(LIBSYLVIA_TASK& task);
	int cleanup();

public:
	LIBSYLVIA_TASK task;

public:
	pthread_t tidWorker;
	pthread_t tidSaveToFile;
	pthread_t threadPool[LIBSLYVIA_THREADPOOLSIZE];

public:
	int GetHttpContentLength();

public:
	int query(LIBSYLVIA_STATUS& status);
	int pause();
	int resume();
	int cancel();
};


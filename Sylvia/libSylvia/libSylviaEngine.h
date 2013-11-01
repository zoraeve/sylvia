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

class libSylvia_engine
{
public:
	libSylvia_engine(void);
	~libSylvia_engine(void);

public:
	std::string currentURI;

#if 0
public:
	bool busy;
	bool bBusy();
	int SaveToFile(const char* szSaveAs);
	std::map<int, std::string> contents;
#endif

public:
	int AddTask(LIBSYLVIA_TASK& task);
	int cleanup();

public:
	LIBSYLVIA_TASK task;

	pthread_t tidWorker;
	pthread_t tidSaveToFile;

public:
	std::deque<int> taskQ;
	pthread_rwlock_t taskQLock/* = PTHREAD_RWLOCK_INITIALIZER*/;
	pthread_t threadPool[LIBSLYVIA_THREADPOOLSIZE];
	
public:
	std::deque<LIBSYLVIA_CONTENT> contentsQ;
	pthread_rwlock_t contentsQLock/* = PTHREAD_RWLOCK_INITIALIZER*/;
	
public:
	int GetHttpContentLength();

public:
	int query(LIBSYLVIA_STATUS& status);
	int pause();
	int resume();
	int cancel();
};


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

#include <pthread.h>
#pragma comment(lib, "pthreadVC2.lib")

#define interval 10
#define threadPoolSize 10
#define segment (1024 * 1024)

class libSylvia_engine
{
public:
	libSylvia_engine(void);
	~libSylvia_engine(void);

public:
	std::string currentURI;

public:
	bool bBusy() const;
	int addTask(LIBSYLVIA_TASK& task);
	int cleanup();

public:
	bool busy;
	LIBSYLVIA_TASK task;

	pthread_t thread;

public:
	std::deque<int> taskQ;
	pthread_rwlock_t taskQLock/* = PTHREAD_RWLOCK_INITIALIZER*/;
	pthread_t threadPool[threadPoolSize];
	std::map<int, std::string> contents;

	bool bReady2Exit/* = false*/;

	std::vector<pthread_t> threadVector;

public:
	int GetHttpContentLength();
	int SaveToFile(const char* szSaveAs);
};


#pragma once

#include "libSylviaUtility.h"
#include "libSylviaLogger.h"

#include "libSylviaTask.h"

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
	static libSylviaEngine* Instance()
	{
		if (NULL == ins)
		{
			ins = new libSylviaEngine();
		}
		return ins;
	}

private:
	static libSylviaEngine* ins;

	libSylviaEngine(void);

public:
	std::string currentURI;

public:
	int post(const LIBSYLVIA_TASK& task);
	int cleanup();

public:
	std::deque<LIBSYLVIA_TASK> taskQ;
	pthread_rwlock_t taskQLock;

public:
	std::map<std::string, libSylviaTask*> taskMap;

public:
	pthread_t tidMaintain;

public:
	bool bExit;

public:
	int query(const char* index, LIBSYLVIA_INFO& info);
	int pause(const char* index);
	int resume(const char* index);
	int cancel(const char* index);

	int onNotify(const char* index);
};


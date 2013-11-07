#include "libSylviaEngine.h"
#include "libSylviaLogger.h"
#include "libSylviaUtility.h"

#include <time.h>

#ifdef LIBSYLVIA_IN_WINDOWS
#include <curl/curl.h>
#pragma comment(lib, "libcurl_imp.lib")
#elif defined LIBSYLVIA_IN_LINUX
#include "curl/curl.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#endif


void* thdMaintain(void* lparam)
{
	if (NULL == lparam)
	{
		return NULL;
	}
	libSylviaEngine* p = reinterpret_cast<libSylviaEngine*>(lparam);

	while(!p->bExit)
	{
		int iRet = pthread_rwlock_rdlock(&p->taskQLock);
		if (0 != iRet)
		{
			libSylvia_sleep(LIBSYLVIA_INTERVAL);
			continue;
		}
		if (p->taskQ.size() > 0)
		{
			pthread_rwlock_unlock(&p->taskQLock);

			iRet = pthread_rwlock_wrlock(&p->taskQLock);
			if (0 != iRet)
			{
				libSylvia_sleep(LIBSYLVIA_INTERVAL);
				continue;
			}

			if (0 >= p->taskQ.size())
			{
				pthread_rwlock_unlock(&p->taskQLock);
				libSylvia_sleep(LIBSYLVIA_INTERVAL);
				continue;
			}

			LIBSYLVIA_TASK t = p->taskQ.front();
			p->taskQ.pop_front();
			pthread_rwlock_unlock(&p->taskQLock);

			libSylviaTask* task = new libSylviaTask(t);
			p->taskMap.insert(std::make_pair(t.Index, task));
		}
		else
		{
			pthread_rwlock_unlock(&p->taskQLock);
		}

		libSylvia_sleep(LIBSYLVIA_INTERVAL);
	}

	return NULL;
}

libSylviaEngine::libSylviaEngine(void)
{
	bExit = false;
	taskQ.clear();
	pthread_rwlock_init(&taskQLock, NULL);

	CURLcode cRet = curl_global_init(CURL_GLOBAL_ALL);
	if (CURLE_OK != cRet)
	{
		exit(-1);
	}

	pthread_create(&tidMaintain, NULL, &thdMaintain, this);
}

libSylviaEngine::~libSylviaEngine(void)
{
	curl_global_cleanup();
}

int libSylviaEngine::post( const LIBSYLVIA_TASK& t )
{
	int nRet = pthread_rwlock_wrlock(&taskQLock);
	while(0 != nRet)
	{
		libSylvia_sleep(LIBSYLVIA_INTERVAL);
		nRet = pthread_rwlock_wrlock(&taskQLock);
	}
	taskQ.push_back(t);
	pthread_rwlock_unlock(&taskQLock);

	return 0;
}

int libSylviaEngine::cleanup()
{
	bExit = true;

	return 0;
}

int libSylviaEngine::query(const char* index, LIBSYLVIA_INFO& info)
{
	std::map<std::string, libSylviaTask*>::iterator iter = taskMap.find(index);
	if (taskMap.end() == iter)
	{
		return -1;
	}

	return iter->second->Query(info);

	return 0;
}

int libSylviaEngine::cancel(const char* index)
{
	std::map<std::string, libSylviaTask*>::iterator iter = taskMap.find(index);
	if (taskMap.end() == iter)
	{
		return -1;
	}

	return iter->second->Cancel();

	return 0;
}

int libSylviaEngine::pause( const char* index )
{
	std::map<std::string, libSylviaTask*>::iterator iter = taskMap.find(index);
	if (taskMap.end() == iter)
	{
		return -1;
	}

	return iter->second->Pause();

	return 0;
}

int libSylviaEngine::resume( const char* index )
{
	std::map<std::string, libSylviaTask*>::iterator iter = taskMap.find(index);
	if (taskMap.end() == iter)
	{
		return -1;
	}

	return iter->second->Resume();

	return 0;
}

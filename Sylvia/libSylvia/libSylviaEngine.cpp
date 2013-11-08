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

libSylviaEngine* libSylviaEngine::ins = new libSylviaEngine();

int cbTaskNotify(void* p)
{
	libSylviaEngine::Instance()->onNotify((const char*)p);

	return 0;
}

void* thdMaintain(void* lparam)
{
	if (NULL == lparam)
	{
		return NULL;
	}

	while(!libSylviaEngine::Instance()->bExit)
	{
		int iRet = pthread_rwlock_rdlock(&libSylviaEngine::Instance()->taskQLock);
		if (0 != iRet)
		{
			libSylvia_sleep(LIBSYLVIA_INTERVAL);
			continue;
		}
		if (libSylviaEngine::Instance()->taskQ.size() > 0)
		{
			pthread_rwlock_unlock(&libSylviaEngine::Instance()->taskQLock);

			iRet = pthread_rwlock_wrlock(&libSylviaEngine::Instance()->taskQLock);
			if (0 != iRet)
			{
				libSylvia_sleep(LIBSYLVIA_INTERVAL);
				continue;
			}

			if (0 >= libSylviaEngine::Instance()->taskQ.size())
			{
				pthread_rwlock_unlock(&libSylviaEngine::Instance()->taskQLock);
				libSylvia_sleep(LIBSYLVIA_INTERVAL);
				continue;
			}

			LIBSYLVIA_TASK t = libSylviaEngine::Instance()->taskQ.front();
			libSylviaEngine::Instance()->taskQ.pop_front();
			pthread_rwlock_unlock(&libSylviaEngine::Instance()->taskQLock);

			libSylviaTask* task = new libSylviaTask(t);
			task->Start(&cbTaskNotify);
			libSylviaEngine::Instance()->taskMap.insert(std::make_pair(task->TI, task));
		}
		else
		{
			pthread_rwlock_unlock(&libSylviaEngine::Instance()->taskQLock);
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

	void* pRet;
	pthread_join(tidMaintain, &pRet);

	curl_global_cleanup();

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

int libSylviaEngine::onNotify( const char* index )
{
	return 0;
}

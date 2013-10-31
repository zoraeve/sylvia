#include "libSylvia.h"
#include "libSylviaUtility.h"
#include "libSylviaLogger.h"
#include "libSylvia_engine.h"

#include <string>
#include <deque>
#include <time.h>

#ifdef LIBSYLVIA_IN_WINDOWS
#include <pthread.h>
#include <Windows.h>
#elif defined LIBSYLVIA_IN_LINUX
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "pthread.h"
#endif

#ifdef LIBSYLVIA_IN_WINDOWS
#pragma comment(lib, "pthreadVC2.lib")
#endif


std::deque<LIBSYLVIA_TASK> libSylvia_taskQ;
pthread_t libSylvia_thread;
LIBSYLVIA_TASK libSylvia_currentTask;
bool libSylvia_exit = true;

libSylvia_engine* libSylvia_Engine; 

void* libSylvia_maintain(void* lparam)
{
	if (NULL == lparam)
	{
		return NULL;
	}
	libSylvia_engine* pEngine = reinterpret_cast<libSylvia_engine*>(lparam);

	while(!libSylvia_exit)
	{
		if (pEngine->bBusy())
		{
#ifdef LIBSYLVIA_IN_WINDOWS
			Sleep(LIBSYLVIA_INTERVAL);
#elif defined LIBSYLVIA_IN_LINUX
			usleep(LIBSYLVIA_INTERVAL);
#endif
			continue;
		}

		if (libSylvia_taskQ.size())
		{
			libSylvia_currentTask = libSylvia_taskQ.front();
			libSylvia_taskQ.pop_front();

			pEngine->addTask(libSylvia_currentTask);
		}
		else
		{
#ifdef LIBSYLVIA_IN_WINDOWS
			Sleep(LIBSYLVIA_INTERVAL);
#elif defined LIBSYLVIA_IN_LINUX
			usleep(LIBSYLVIA_INTERVAL);
#endif
		}
	}

	pEngine->cleanup();

	return NULL;
}

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_ini()
{
	libSylvia_Engine = new libSylvia_engine();
	libSylvia_exit = false;

	libSylvia_logger_ini(libSylvia_exit);

	pthread_create(&libSylvia_thread, NULL, &libSylvia_maintain, (void*)libSylvia_Engine);

	return 0;
}

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_fin()
{
	libSylvia_exit = true;

	libSylvia_logger_fin(libSylvia_exit);

	delete libSylvia_Engine;

	return 0;
}

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_httpGet(const char* szURI, const char* szSaveAs)
{
	LIBSYLVIA_TASK task;
	task.URI = szURI;

	libSylvia_guessWhat(szURI, task.SaveAs);

	task.Method = _LIBSYLVIA_METHOD_HTTP_;

	libSylvia_taskQ.push_back(task);

	return 0;
}

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_httpsGet(const char* szURI, const char* szSaveAs)
{
	LIBSYLVIA_TASK task;
	task.URI = szURI;
	task.SaveAs = szSaveAs;
	task.Method = _LIBSYLVIA_METHOD_HTTPS_;

	libSylvia_taskQ.push_back(task);

	return 0;
}

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_ftpGet(const char* szURI, const char* szSaveAs)
{
	LIBSYLVIA_TASK task;
	task.URI = szURI;
	task.SaveAs = szSaveAs;
	task.Method = _LIBSYLVIA_METHOD_FTP_;

	libSylvia_taskQ.push_back(task);

	return 0;
}

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_sftpGet(const char* szURI, const char* szSaveAs)
{
	LIBSYLVIA_TASK task;
	task.URI = szURI;
	task.SaveAs = szSaveAs;
	task.Method = _LIBSYLVIA_METHOD_SFTP_;

	libSylvia_taskQ.push_back(task);

	return 0;
}

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_query(const int index, LIBSYLVIA_STATUS& status)
{
	libSylvia_Engine->query(status);

	status.nRemainTasks = libSylvia_taskQ.size();

	return 0;
}

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_pause(const int index)
{
	return 0;
}

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_resume(const int index)
{
	return 0;
}

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_cancel(const int index)
{
	libSylvia_Engine->cancel();

	return 0;
}

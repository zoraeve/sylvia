#include "libSylvia.h"
#include "libSylviaUtility.h"
#include "libSylviaLogger.h"

#include <string>
#include <deque>

#include <pthread.h>
#pragma comment(lib, "pthreadVC2.lib")

#include "libSylvia_engine.h"

#ifdef LIBSYLVIA_IN_WINDOWS
#include <Windows.h>
#endif

#include <time.h>

std::deque<LIBSYLVIA_TASK> libSylvia_taskQ;
pthread_t libSylvia_thread;
LIBSYLVIA_TASK libSylvia_currentTask;
bool libSylvia_exit = true;

libSylvia_engine libSylvia_Engine; 

void* libSylvia_maintain(void* lparam)
{
	if (lparam == nullptr)
	{
		return nullptr;
	}
	libSylvia_engine* pEngine = reinterpret_cast<libSylvia_engine*>(lparam);

	while(!libSylvia_exit)
	{
		if (pEngine->bBusy())
		{
#ifdef LIBSYLVIA_IN_WINDOWS
			Sleep(10);
#else
			sleep(10);
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
			Sleep(10);
#else
			sleep(10);
#endif
		}
	}

	pEngine->cleanup();

	return nullptr;
}

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_ini()
{
	libSylvia_exit = false;

	libSylvia_logger_ini(libSylvia_exit);

	pthread_create(&libSylvia_thread, nullptr, &libSylvia_maintain, (void*&)libSylvia_Engine);

	return 0;
}

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_fin()
{
	libSylvia_exit = true;

	libSylvia_logger_fin(libSylvia_exit);

	return 0;
}

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_httpGet(const char* szURI, const char* szSaveAs)
{
	LIBSYLVIA_TASK task;
	task.URI = szURI;

	if (nullptr == szSaveAs)
	{
		std::string s = szURI;
		if (std::string::npos == s.find("/"))
		{
			char szName[128] = {0};
			srand(time(NULL));
			int name = rand();
			sprintf(szName, "%d.save", name);
			task.SaveAs = szName;
		}
		else
		{
			task.SaveAs =s.substr(s.find_last_of("/") + 1).c_str();
		}
	}
	else
	{
		task.SaveAs = szSaveAs;
	}

	task.method = _LIBSYLVIA_METHOD_HTTP_;

	libSylvia_taskQ.push_back(task);

	return 0;
}

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_httpsGet(const char* szURI, const char* szSaveAs)
{
	LIBSYLVIA_TASK task;
	task.URI = szURI;
	task.SaveAs = szSaveAs;
	task.method = _LIBSYLVIA_METHOD_HTTPS_;

	libSylvia_taskQ.push_back(task);

	return 0;
}

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_ftpGet(const char* szURI, const char* szSaveAs)
{
	LIBSYLVIA_TASK task;
	task.URI = szURI;
	task.SaveAs = szSaveAs;
	task.method = _LIBSYLVIA_METHOD_FTP_;

	libSylvia_taskQ.push_back(task);

	return 0;
}

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_sftpGet(const char* szURI, const char* szSaveAs)
{
	LIBSYLVIA_TASK task;
	task.URI = szURI;
	task.SaveAs = szSaveAs;
	task.method = _LIBSYLVIA_METHOD_SFTP_;

	libSylvia_taskQ.push_back(task);

	return 0;
}

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_query(const int index, LIBSYLVIA_STATUS& status)
{
	libSylvia_Engine.query(status);

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
	libSylvia_Engine.cancel();

	return 0;
}

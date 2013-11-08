#include "libSylvia.h"
#include "libSylviaUtility.h"
#include "libSylviaLogger.h"
#include "libSylviaEngine.h"

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

bool libSylvia_exit = true;

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_task(LIBSYLVIA_TASK& task, char* index)
{
	libSylvia_uuid(index);
	task.Index = index;

	libSylviaEngine::Instance()->post(task);

	return 0;
}

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_ini()
{
#ifdef LIBSYLVIA_IN_WINDOWS
	if (FAILED(CoInitialize(NULL)))
	{
		return -1;
	}
#endif

	libSylvia_exit = false;

	libSylvia_logger_ini(libSylvia_exit);

	return 0;
}

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_fin()
{
#ifdef LIBSYLVIA_IN_WINDOWS
	CoUninitialize();
#endif

	libSylvia_exit = true;

	libSylvia_logger_fin(libSylvia_exit);

	return 0;
}

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_httpGet(const char* szURI, const char* szSaveAs, char* index)
{
	LIBSYLVIA_TASK task;
	task.URI = szURI;

	libSylvia_guessWhat(szURI, task.SaveAs);

	task.Method = _LIBSYLVIA_METHOD_HTTP_;

	libSylvia_task(task, index);

	return 0;
}

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_httpsGet(const char* szURI, const char* szSaveAs, char* index)
{
	LIBSYLVIA_TASK task;
	task.URI = szURI;
	task.SaveAs = szSaveAs;
	task.Method = _LIBSYLVIA_METHOD_HTTPS_;

	libSylvia_task(task, index);

	return 0;
}

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_ftpGet(const char* szURI, const char* szSaveAs, char* index)
{
	LIBSYLVIA_TASK task;
	task.URI = szURI;
	task.SaveAs = szSaveAs;
	task.Method = _LIBSYLVIA_METHOD_FTP_;

	libSylvia_task(task, index);

	return 0;
}

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_sftpGet(const char* szURI, const char* szSaveAs, char* index)
{
	LIBSYLVIA_TASK task;
	task.URI = szURI;
	task.SaveAs = szSaveAs;
	task.Method = _LIBSYLVIA_METHOD_SFTP_;

	libSylvia_task(task, index);

	return 0;
}

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_query(const char* index, LIBSYLVIA_INFO& info)
{
	if (NULL == index)
	{
		return -1;
	}

	return libSylviaEngine::Instance()->query(index, info);

	return 0;
}

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_pause(const char* index)
{
	return libSylviaEngine::Instance()->pause(index);

	return 0;
}

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_resume(const char* index)
{
	return libSylviaEngine::Instance()->resume(index);

	return 0;
}

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_cancel(const char* index)
{
	return libSylviaEngine::Instance()->cancel(index);

	return 0;
}

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_status( LIBSYLVIA_OPERATION op, const char* index, LIBSYLVIA_INFO& info )
{
	return 0;
}


LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_bt();
LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_magnet();
LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_ed2k();

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_service();
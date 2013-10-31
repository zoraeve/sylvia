#include "libSylviaLogger.h"
#include "libSylviaUtility.h"

#include <iostream>
#include <deque>
#include <stdarg.h>

#ifdef LIBSYLVIA_IN_WINDOWS
#include <Windows.h>
#ifdef GLOG_SUPPORT
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#pragma comment(lib, "libglog.lib")
#endif
#include <pthread.h>
#pragma comment(lib, "pthreadVC2.lib")
#elif defined LIBSYLVIA_IN_LINUX
#include "pthread.h"
#include "glog/logging.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#endif


typedef struct _logUnit_
{
	int level;
	std::string logDetail;
}logUnit;
std::deque<_logUnit_> logQ;

bool libSylvia_logger_flag = true;
pthread_t libSylvia_logger_thread;
// pthread_cond_t libSylvia_logger_cond;
// pthread_mutex_t libSylvia_logger_mutex;

void* logger(void* lparam)
{
	while(1)
	{
		if (logQ.size())
		{
#ifdef GLOG_SUPPORT
			logUnit lu = logQ.front();
			logQ.pop_front();

			switch(lu.level)
			{
			case 0:
				LOG(INFO) << lu.logDetail << std::endl;
				break;
			case 1:
				LOG(WARNING) << lu.logDetail << std::endl;
				break;
			case 2:
				LOG(ERROR) << lu.logDetail << std::endl;
				break;
			case 3:
				LOG(FATAL) << lu.logDetail << std::endl;
				break;
			default:
				LOG(INFO) << lu.logDetail << std::endl;
				break;
			}
#endif
		}
		else
		{
			if (!libSylvia_logger_flag)
			{
				//pthread_cond_timedwait();
#ifdef LIBSYLVIA_IN_WINDOWS
				Sleep(LIBSYLVIA_INTERVAL);
#elif defined LIBSYLVIA_IN_LINUX
				usleep(LIBSYLVIA_INTERVAL);
#endif
				continue;
			}
			break;
		}
	}
	return NULL;
}

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_logger_ini(bool flag)
{
#ifdef GLOG_SUPPORT
	google::InitGoogleLogging("libSylvia");
	google::SetLogDestination(google::GLOG_INFO, "./libSylvia_");
#endif

	libSylvia_logger_flag = flag;

// 	pthread_mutex_init(&libSylvia_logger_mutex, NULL);
// 	pthread_cond_init(&libSylvia_logger_cond, NULL);

	pthread_create(&libSylvia_logger_thread, NULL, logger, NULL);

	return 0;
}

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_logger_fin(bool flag)
{
	libSylvia_logger_flag = flag;

	void* thdRet;
	pthread_join(libSylvia_logger_thread, &thdRet);

#ifdef GLOG_SUPPORT
	google::FlushLogFiles(0);
#endif

	return 0;
}

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_log(int level, const char* format, ...)
{
	va_list pArg;
	va_start(pArg, format);

	try
	{
		char* pBuf = new char[4096]; // how about just new once only
		memset(pBuf, 0, 4096);
		vsprintf(pBuf, format, pArg);

		va_end(pArg);

		logUnit lu;
		lu.level = level;
		lu.logDetail = pBuf;
		logQ.push_back(lu);

		delete[] pBuf;
		pBuf = NULL;
	}
	catch (...)
	{
		;
	}
	
	return 0;
}
#pragma once
#include "libSylvia.h"

#include <string>
#include <deque>

#ifdef LIBSYLVIA_IN_WINDOWS
#include <pthread.h>
#pragma comment(lib, "pthreadVC2.lib")
#elif defined LIBSYLVIA_IN_LINUX
#include "pthread.h"
#endif

typedef enum _LIBSYLVIA_METHOD_
{
	_LIBSYLVIA_METHOD_UNKNOWN_ = 0,
	_LIBSYLVIA_METHOD_HTTP_ = 1,
	_LIBSYLVIA_METHOD_HTTPS_ = 2,
	_LIBSYLVIA_METHOD_FTP_ = 3,
	_LIBSYLVIA_METHOD_SFTP_ = 4
}LIBSYLVIA_METHOD;

typedef enum _LIBSYLVIA_TASK_STATUS_
{
	_LIBSYLVIA_STATUS_TASK_UNKNOWN_ = 0,
	_LIBSYLVIA_STATUS_TASK_PAUSED_ = 1,
	_LIBSYLVIA_STATUS_TASK_WAITING_ = 2,
	_LIBSYLVIA_STATUS_TASK_RUNNING_ = 3,
	_LIBSYLVIA_STATUS_TASK_CANCELED_ = 4,
	_LIBSYLVIA_STATUS_TASK_DONE_ = 5
}LIBSYLVIA_TASK_STATUS;

typedef struct _LIBSYLVIA_CONTENT_ 
{
	unsigned int index;
	std::string sData;
}LIBSYLVIA_CONTENT;

typedef struct _LIBSYLVIA_TASK_ 
{
	unsigned int TotalFrames;
	unsigned int CompleteParts;
	unsigned int TotolSize;
	float Progress;
	float DownloadSpeed;
	float DownloadSize;
	LIBSYLVIA_METHOD Method;
	LIBSYLVIA_TASK_STATUS Status;
	pthread_rwlock_t ContentsQLock;
	pthread_rwlock_t TaskQLock;
	std::string URI;
	std::string SaveAs;
	std::deque<int> TaskQ;
	std::deque<LIBSYLVIA_CONTENT> ContentsQ;

	_LIBSYLVIA_TASK_()
	{
		URI = "";
		SaveAs = "";
		Method = _LIBSYLVIA_METHOD_UNKNOWN_;
		TaskQ.clear();
		ContentsQ.clear();
		CompleteParts = 0;
		DownloadSize = 0;
		DownloadSpeed = 0;
		Progress = 0;
		TotalFrames = 0;
		TotolSize = 0;
		Status = _LIBSYLVIA_STATUS_TASK_UNKNOWN_;

		pthread_rwlock_init(&TaskQLock, NULL);
		pthread_rwlock_init(&ContentsQLock, NULL);
	}

	~_LIBSYLVIA_TASK_()
	{
		URI = "";
		SaveAs = "";
		Method = _LIBSYLVIA_METHOD_UNKNOWN_;
		TaskQ.clear();
		ContentsQ.clear();
		CompleteParts = 0;
		DownloadSize = 0;
		DownloadSpeed = 0;
		Progress = 0;
		TotalFrames = 0;
		TotolSize = 0;
		Status = _LIBSYLVIA_STATUS_TASK_UNKNOWN_;
	}
}LIBSYLVIA_TASK;

#define LIBSYLVIA_INTERVAL 10 * 1000 // 10'000 microseconds

#define LIBSLYVIA_THREADPOOLSIZE 10
#define LIBSYLVIA_SEGMENTSIZE (1024 * 1024)


#define LIBSYLVIA_BLOCK_4K (4 * 1024)
#define LIBSYLVIA_BLOCK_1M (1024 * 1024)


int libSylvia_guessWhat(const char* szURI, std::string& sSavedAs);
int libSylvia_preAllocation(const unsigned int nSizeOfBytes, const char* pSavedAs);
int libSylvia_randomWrite(const char* pSavedAs, unsigned int nPos, std::string sData);
int libSylvia_sleep(unsigned long microseconds);
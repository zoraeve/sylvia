#pragma once
#include "libSylvia.h"

#include <string>
#include <deque>

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
	_LIBSYLVIA_STATUS_UNKNOWN_ = 0,
	_LIBSYLVIA_STATUS_PAUSED_ = 1,
	_LIBSYLVIA_STATUS_WAITING_ = 2,
	_LIBSYLVIA_STATUS_RUNNING_ = 3,
	_LIBSYLVIA_STATUS_CANCELED_ = 4,
	_LIBSYLVIA_STATUS_DONE_ = 5
}LIBSYLVIA_TASK_STATUS;

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
	std::string URI;
	std::string SaveAs;
	std::deque<int> TaksQ;
}LIBSYLVIA_TASK;

typedef struct _LIBSYLVIA_CONTENT_ 
{
	unsigned int index;
	std::string sData;
}LIBSYLVIA_CONTENT;

#define LIBSYLVIA_INTERVAL 10 * 1000 // 10'000 microseconds

#define LIBSLYVIA_THREADPOOLSIZE 10
#define LIBSYLVIA_SEGMENTSIZE (1024 * 1024)


#define LIBSYLVIA_BLOCK_4K (4 * 1024)
#define LIBSYLVIA_BLOCK_1M (1024 * 1024)


int libSylvia_guessWhat(const char* szURI, std::string& sSavedAs);
int libSylvia_preAllocation(const unsigned int nSizeOfBytes, const char* pSavedAs);
int libSylvia_randomWrite(const char* pSavedAs, unsigned int nPos, std::string sData);
int libSylvia_sleep(unsigned long microseconds);
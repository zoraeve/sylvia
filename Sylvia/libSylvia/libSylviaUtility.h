#pragma once

#include <string>

typedef enum _LIBSYLVIA_METHOD_
{
	_LIBSYLVIA_METHOD_UNKNOWN_ = 0,
	_LIBSYLVIA_METHOD_HTTP_ = 1,
	_LIBSYLVIA_METHOD_HTTPS_ = 2,
	_LIBSYLVIA_METHOD_FTP_ = 3,
	_LIBSYLVIA_METHOD_SFTP_ = 4
}LIBSYLVIA_METHOD;

typedef struct _LIBSYLVIA_TASK_ 
{
	std::string URI;
	std::string SaveAs;
	LIBSYLVIA_METHOD Method;
}LIBSYLVIA_TASK;

typedef struct _LIBSYLVIA_CONTENT_ 
{
	unsigned int index;
	std::string sData;
}LIBSYLVIA_CONTENT;


#define LIBSYLVIA_BLOCK_4K (4 * 1024)
#define LIBSYLVIA_BLOCK_1M (1024 * 1024)


int libSylvia_guessWhat(const char* szURI, std::string& sSavedAs);
int libSylvia_preAllocation(const unsigned int nSizeOfBytes, const char* pSavedAs);
int libSylvia_randomWrite(const char* pSavedAs, unsigned int nPos, std::string sData);
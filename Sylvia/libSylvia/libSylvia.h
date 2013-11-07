#pragma once

#ifndef __LIBSYLIVIA__
#define __LIBSYLIVIA__

#ifndef __LIBSYLIVIA_H__
#define __LIBSYLIVIA_H__

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef LIBSYLVIA_API
#if defined(_WIN32) || defined(_WIN64)
#define LIBSYLVIA_IN_WINDOWS
#ifdef LIBSYLVIA_IMPORT
#define LIBSYLVIA_API __declspec(dllimport)
#else
#define LIBSYLVIA_API __declspec(dllexport)
#endif
#define LIBSYLVIA_CALLBACK __stdcall
#elif defined(__linux__)
#define LIBSYLVIA_IN_LINUX 
#define LIBSYLVIA_API
#define LIBSYLVIA_CALLBACK
#endif
#endif


	typedef enum _LIBSYLVIA_STATUS_
	{
		_LIBSYLVIA_STATUS_TASK_UNKNOWN_ = 0,
		_LIBSYLVIA_STATUS_TASK_PAUSED_ = 1,
		_LIBSYLVIA_STATUS_TASK_WAITING_ = 2,
		_LIBSYLVIA_STATUS_TASK_RUNNING_ = 3,
		_LIBSYLVIA_STATUS_TASK_CANCELED_ = 4,
		_LIBSYLVIA_STATUS_TASK_DONE_ = 5,
		_LIBSYLVIA_STATUS_TASK_NOT_EXIST_ = 6
	}LIBSYLVIA_STATUS;

	typedef struct _LIBSYLVIA_INFO_
	{
		char szURI[4096];
		char szSaveAs[256];
		float Progress;
		LIBSYLVIA_STATUS status;
	}LIBSYLVIA_INFO;

	typedef enum _LIBSYLVIA_OPERATION_
	{
		_LIBSYLVIA_OPERATION_UNKNOWN_ = 0,
		_LIBSYLVIA_OPERATION_ADD_ = 1,
		_LIBSYLVIA_OPERATION_CANCEL_ = 2,
		_LIBSYLVIA_OPERATION_PAUSE_ = 3,
		_LIBSYLVIA_OPERATION_RESUME_ = 4,
		_LIBSYLVIA_OPERATION_QUERY_ = 5,

// 	 	_LIBSYLVIA_OPERATION_HTTP_GET_ = 101,
// 	 	_LIBSYLVIA_OPERATION_HTTPS_GET_ = 102,
// 	 	_LIBSYLVIA_OPERATION_FTP_GET_ = 103,
// 	 	_LIBSYLVIA_OPERATION_SFTP_GET_ = 104
	}LIBSYLVIA_OPERATION;


	LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_ini();
	LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_fin();

	LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_httpGet(const char* szURI, const char* szSaveAs, char* index);
	LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_httpsGet(const char* szURI, const char* szSaveAs, char* index);

	LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_ftpGet(const char* szURI, const char* szSaveAs, char* index);
	LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_sftpGet(const char* szURI, const char* szSaveAs, char* index);

// LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_status(LIBSYLVIA_OPERATION op, const char* index, LIBSYLVIA_STATUS& status);

	LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_query(const char* index, LIBSYLVIA_INFO& info);
	LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_pause(const char* index);
	LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_resume(const char* index);
	LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_cancel(const char* index);


#ifdef __cplusplus
}
#endif

#endif // __LIBSYLIVIA_H__
#endif // __LIBSYLIVIA__

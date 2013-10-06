#pragma once

#ifndef __LIBSYLIVIA__
#define __LIBSYLIVIA__

#ifndef __LIBSYLIVIA_H__
#define __LIBSYLIVIA_H__


#ifndef LIBSYLVIA_API
#if defined(_WIN32) || defined(_WIN64)
#define LIBSYLVIA_IN_WINDOWS
#ifdef LIBSYLVIA_IMPORT
#define LIBSYLVIA_API __declspec(dllimport)
#else
#define LIBSYLVIA_API __declspec(dllexport)
#endif
#define LIBSYLVIA_CALLBACK __stdcall
#endif
#elif defined(__linux__)
#define LIBSYLVIA_IN_LINUX
#define LIBSYLVIA_API
#define LIBSYLVIA_CALLBACK _cdecl
#endif

typedef struct _LIBSYLVIA_STATUS_
{
	char szCurrentURI[4096];
	char szCurrentSaveAs[256];
	float currentProgress;
	unsigned int nRemainTasks;
}LIBSYLVIA_STATUS;

#ifdef __cplusplus
extern "C"
{
#endif

	LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_ini();
	LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_fin();

	LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_httpGet(const char* szURI, const char* szSaveAs);
	LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_httpsGet(const char* szURI, const char* szSaveAs);

	LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_ftpGet(const char* szURI, const char* szSaveAs);
	LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_sftpGet(const char* szURI, const char* szSaveAs);

	LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_query(const int index, LIBSYLVIA_STATUS& status);
	LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_pause(const int index);
	LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_resume(const int index);
	LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_cancel(const int index);

#ifdef __cplusplus
}
#endif


#endif // __LIBSYLIVIA_H__
#endif // __LIBSYLIVIA__
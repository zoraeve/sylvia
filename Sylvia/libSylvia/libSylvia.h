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

#ifdef __cplusplus
extern "C"
{
#endif

	LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_ini();
	LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_fin();

#ifdef __cplusplus
}
#endif


#endif // __LIBSYLIVIA_H__
#endif // __LIBSYLIVIA__
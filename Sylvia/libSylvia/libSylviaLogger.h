
#include "libSylvia.h"

#define GLOG_SUPPORT

#ifdef GLOG_SUPPORT
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#pragma comment(lib, "libglog.lib")
#endif

#ifdef LOGCXX_SUPPORT
#endif

typedef enum _LIBSYLVIA_LOG_LEVEL_
{
	_LIBSYLVIA_LOG_LEVEL_INFO_  = 0,
	_LIBSYLVIA_LOG_LEVEL_WARN_  = 1,
	_LIBSYLVIA_LOG_LEVEL_ERROR_ = 2,
	_LIBSYLVIA_LOG_LEVEL_FATAL_ = 3
}LIBSYLVIA_LOG_LEVEL;


LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_logger_ini();
LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_logger_fin();
LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_log(int level, const char* format, ...);


#ifdef LIBSYLVIA_IN_WINDOWS
#define LIBSYLVIA_LOG_INFO(fmt, ...)  libSylvia_log(_LIBSYLVIA_LOG_LEVEL_INFO_,  "<%s>\t<%d>\t<%s>,"fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LIBSYLVIA_LOG_WARN(fmt, ...)  libSylvia_log(_LIBSYLVIA_LOG_LEVEL_WARN_,  "<%s>\t<%d>\t<%s>,"fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LIBSYLVIA_LOG_ERROR(fmt, ...) libSylvia_log(_LIBSYLVIA_LOG_LEVEL_ERROR_, "<%s>\t<%d>\t<%s>,"fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LIBSYLVIA_LOG_FATAL(fmt, ...) libSylvia_log(_LIBSYLVIA_LOG_LEVEL_FATAL_, "<%s>\t<%d>\t<%s>,"fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define LIBSYLVIA_LOG_INFO(fmt, ...)  libSylvia_log(_LIBSYLVIA_LOG_LEVEL_INFO_,  "<%s>|<%d>|<%s>,"fmt, __FILE__, __LINE__, __FUNCTION__, ##args)
#define LIBSYLVIA_LOG_WARN(fmt, ...)  libSylvia_log(_LIBSYLVIA_LOG_LEVEL_WARN_,  "<%s>|<%d>|<%s>,"fmt, __FILE__, __LINE__, __FUNCTION__, ##args)
#define LIBSYLVIA_LOG_ERROR(fmt, ...) libSylvia_log(_LIBSYLVIA_LOG_LEVEL_ERROR_, "<%s>|<%d>|<%s>,"fmt, __FILE__, __LINE__, __FUNCTION__, ##args)
#define LIBSYLVIA_LOG_FATAL(fmt, ...) libSylvia_log(_LIBSYLVIA_LOG_LEVEL_FATAL_, "<%s>|<%d>|<%s>,"fmt, __FILE__, __LINE__, __FUNCTION__, ##args) 
#endif



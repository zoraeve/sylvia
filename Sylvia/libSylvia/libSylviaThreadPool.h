#pragma once

#ifdef LIBSYLVIA_IN_WINDOWS
#include <pthread.h>
#pragma comment(lib, "pthreadVC2.lib")
#elif defined LIBSYLVIA_IN_LINUX
#include "pthread.h"
#endif

class libSylviaThreadPool
{
public:
	libSylviaThreadPool(void);
	~libSylviaThreadPool(void);
};


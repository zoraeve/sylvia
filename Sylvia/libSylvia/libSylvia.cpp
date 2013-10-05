#include "libSylvia.h"
#include "libSylviaLogger.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <deque>
#include <map>
using namespace std;

#include <time.h>

#include <curl/curl.h>
#pragma comment(lib, "libcurl_imp.lib")

#include <pthread.h>
#pragma comment(lib, "pthreadVC2.lib")


#define interval 10
#define threadPoolSize 10
#define segment (1024 * 1024)

std::deque<int> taskQ;
pthread_rwlock_t taskQLock = PTHREAD_RWLOCK_INITIALIZER;
pthread_t threadPool[threadPoolSize];
std::map<int, std::string> contents;

bool bReady2Exit = false;

std::vector<pthread_t> threadVector;


LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_ini()
{
	libSylvia_logger_ini(false);

	return 0;
}

LIBSYLVIA_API int LIBSYLVIA_CALLBACK libSylvia_fin()
{
	libSylvia_logger_fin(true);

	return 0;
}




#include <iostream>
using namespace std;

#ifdef __linux__
#include "pthread.h"
#include "libSylvia.h"
#else
#include "..\libSylvia\libSylvia.h"
#ifdef _DEBUG
#pragma comment(lib, "../Debug/libSylvia.lib")
#else
#pragma comment(lib, "../Release/libSylvia.lib")
#endif
#include <Windows.h>
#include <process.h>
#endif

#ifdef __linux__
void* demo(void* lparam)
#else
unsigned int __stdcall demo(LPVOID lparam)
#endif
{
	int cnt = 0;

	while(1)
	{
		LIBSYLVIA_STATUS s = {0};
		libSylvia_query(0, s);

		if (s.currentProgress == 100.0)
		{
#ifdef __linux__
			return NULL;
#else
			return 0;
#endif
		}

#ifdef __linux__
		sleep(1);
#else
		Sleep(1000);
#endif
		
		if (++cnt == 1)
		{
			libSylvia_httpGet("http://dldir1.qq.com/qqfile/qq/QQ2013/QQ2013SP3/8548/QQ2013SP3.exe", NULL);
		}
	}
#ifdef __linux__
	return NULL;
#else
	return 0;
#endif
}

int main(int argc, char* argv[])
{
	libSylvia_ini();

#ifdef __linux__
	pthread_t tid;
	pthread_create(&tid, NULL, demo, NULL);
#else
	HANDLE h = (HANDLE)_beginthreadex(nullptr, 0, &demo, nullptr, 0, nullptr);
#endif

#ifdef __linux__
		void* p;
		pthread_join(tid, &p);
#else
		WaitForSingleObject(h, INFINITE);
#endif

	libSylvia_fin();

	return 0;
}
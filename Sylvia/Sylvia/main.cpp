

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
	if (NULL == lparam)
	{
#ifdef __linux__
		return NULL;
#else
		return 0;
#endif
	}
	char** argv = reinterpret_cast<char**>(lparam);

	char szIndex[64] = {0};
	int cnt = 0;

	while(1)
	{
		LIBSYLVIA_INFO s = {0};
		libSylvia_query(szIndex, s);

		if (s.Progress >= 100.0)
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
// #ifdef _DEBUG
// 			libSylvia_httpGet("http://oa.hikvision.com.cn/hikvision/Bulletin.nsf/de7f496c716ae0564825765500153eda/a95c410fec3adde548257b49002aaa35/$FILE/%E6%B3%95%E5%BE%8B%E5%92%A8%E8%AF%A2%E5%B9%B3%E5%8F%B0%E6%93%8D%E4%BD%9C%E7%AE%80%E4%BB%8B.docx", NULL, szIndex);
// //			libSylvia_httpGet("http://117.21.189.48/cdn.baidupcs.com/file/6949bf3029e81553a546e97d7348d77f?xcode=5e8e5a37e56cf86d041c91a25aa7cb673e1748317ed8c839&fid=1410197977-250528-949775881&time=1383189038&sign=FDTAXER-DCb740ccc5511e5e8fedcff06b081203-sW88WzuvvDGrLLQ125%2BAFUXOJVc%3D&to=cb&fm=N,B,T,t&expires=8h&rt=sh&r=985737545&logid=2522427617&sh=1&fn=%E5%95%83%E6%85%A2%E4%BA%8C.rar&wshc_tag=0&wsiphost=ipdbm", NULL, szIndex);
// #else
// 			libSylvia_httpGet((const char*)(*argv)[0], (const char*)(*argv)[1], NULL);
// #endif
			libSylvia_httpGet("http://oa.hikvision.com.cn/hikvision/Bulletin.nsf/de7f496c716ae0564825765500153eda/a95c410fec3adde548257b49002aaa35/$FILE/%E6%B3%95%E5%BE%8B%E5%92%A8%E8%AF%A2%E5%B9%B3%E5%8F%B0%E6%93%8D%E4%BD%9C%E7%AE%80%E4%BB%8B.docx", NULL, szIndex);
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
// #ifndef _DEBUG
// 	if (3 != argc)
// 	{
// #ifdef LIBSYLVIA_IN_WINDOWS
// 		cout << "usage: Sylvia.exe URI SAVEAS" << endl;
// 		cout << "example: Sylvia.exe http://xxx.xxx.xxx/xxx.xxx SAVEAS" << endl;
// #else
// 		cout << "usage: ./Sylvia URI SAVEAS" << endl;
// 		cout << "example: ./Sylvia http://xxx.xxx.xxx/xxx.xxx SAVEAS" << endl;
// #endif
// 		return -1;
// 	}
// #endif

	libSylvia_ini();

#ifdef __linux__
	pthread_t tid;
	const char* sz= "http://oa.hikvision.com.cn/hikvision/Bulletin.nsf/de7f496c716ae0564825765500153eda/a95c410fec3adde548257b49002aaa35/$FILE/%E6%B3%95%E5%BE%8B%E5%92%A8%E8%AF%A2%E5%B9%B3%E5%8F%B0%E6%93%8D%E4%BD%9C%E7%AE%80%E4%BB%8B.docx";
	pthread_create(&tid, NULL, demo, (void*)sz);
#else
	HANDLE h = (HANDLE)_beginthreadex(nullptr, 0, &demo, &argv, 0, nullptr);
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
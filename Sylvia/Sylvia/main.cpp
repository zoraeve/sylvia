


#include <iostream>
using namespace std;

#include "..\libSylvia\libSylvia.h"
#ifdef _DEBUG
#pragma comment(lib, "../Debug/libSylvia.lib")
#else
#pragma comment(lib, "../Release/libSylvia.lib")
#endif

#include <Windows.h>
#include <process.h>

unsigned int __stdcall demo(LPVOID lparam)
{
	int cnt = 0;

	while(1)
	{
		LIBSYLVIA_STATUS s = {0};
		libSylvia_query(0, s);

		Sleep(1000);
		
		if (++cnt == 1)
		{
			libSylvia_httpGet("http://dldir1.qq.com/qqfile/qq/QQ2013/QQ2013SP3/8548/QQ2013SP3.exe", nullptr);
		}
	}

	return 0;
}

int main()
{
	libSylvia_ini();

	_beginthreadex(nullptr, 0, &demo, nullptr, 0, nullptr);

	while(1)
	{
		Sleep(10000);
	}

	libSylvia_fin();

	return 0;
}
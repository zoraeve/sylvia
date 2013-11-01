#include "libSylviaUtility.h"
#include "libSylviaLogger.h"

#include <time.h>

#include <sstream>
#include <fstream>
using namespace std;


#ifdef LIBSYLVIA_IN_WINDOWS
#include <Windows.h>
#elif defined LIBSYLVIA_IN_LINUX
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#endif

int libSylvia_guessWhat(const char* szURI, std::string& sSavedAs)
{
	std::string sURI(szURI);

	int slash_pos = sURI.find_last_of('/');
	if (std::string::npos == slash_pos)
	{
		srand(time(NULL));
		int name = rand();

		stringstream ss;
		ss << name;
		sSavedAs = ss.str();

		return 1;
	}

	sURI = sURI.substr(slash_pos + 1);

	int dot_pos = sURI.find_last_of('.');
	if (std::string::npos == dot_pos)
	{
		srand(time(NULL));
		int name = rand();

		stringstream ss;
		ss << name;
		sSavedAs = ss.str();

		return 2;
	}

	std::string sName = sURI.substr(0, dot_pos);
	sURI = sURI.substr(dot_pos + 1);

	std::string::iterator iter = sURI.begin();
	for ( ; iter != sURI.end(); ++iter)
	{
		if ((*iter >= 48 && *iter <= 57) || (*iter >= 65 && *iter <= 90) || (*iter >= 97 && *iter <= 122))
		{
			continue;
		}
		else
		{
			break;
		}
	}

	std::string sExt = (sURI.end() == iter) ? sURI : sURI.substr(0, sURI.find_first_of(*iter));

	sSavedAs = sName + "." + sExt;

	return 0;
}

int libSylvia_preAllocation(const unsigned int nSizeOfBytes, const char* pSavedAs)
{
	if (NULL == pSavedAs)
	{
		LIBSYLVIA_LOG_ERROR("Invaild file name: NULL, please give a vaild name or call \"libSylvia_guessWhat\" first");
		return -1;
	}

	ofstream ofile;
	ofile.open(pSavedAs, ios::out | ios::binary);
	if (!ofile.is_open())
	{
		LIBSYLVIA_LOG_ERROR("Create file %s failed", pSavedAs);
		return -2;
	}

	std::string block(LIBSYLVIA_BLOCK_1M, 0x0);
	std::string restBlock(nSizeOfBytes % LIBSYLVIA_BLOCK_1M, 0x0);

	int seg = nSizeOfBytes / LIBSYLVIA_BLOCK_1M;

	LIBSYLVIA_LOG_INFO("preallocation start");

	for (int cnt = 0; cnt < seg; ++cnt)
	{
		ofile << block;
	}
	ofile << restBlock;

	ofile.flush();
	ofile.close();

	LIBSYLVIA_LOG_INFO("preallocation done");

	return 0;
}

int libSylvia_randomWrite(const char* pSavedAs, unsigned int nPos, std::string sData)
{
	ofstream ofile;
	ofile.open(pSavedAs, ios::in | ios::out | ios::binary | ios::ate );

	ofile.seekp(0, ios::beg);
	ofile.seekp(nPos, ios::beg);

	ofile << sData;

	ofile.flush();
	ofile.close();

	return 0;
}

int libSylvia_sleep( unsigned long microseconds )
{
#ifdef LIBSYLVIA_IN_WINDOWS
	Sleep(microseconds / 1000);
#elif defined LIBSYLVIA_IN_LINUX
	usleep(microseconds);
#endif

	return 0;
}

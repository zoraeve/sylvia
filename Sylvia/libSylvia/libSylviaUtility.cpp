#include "libSylviaUtility.h"
#include "libSylviaLogger.h"
#include "libSylviaMD5.h"

#include <stdio.h>
#include <time.h>

#include <sstream>
#include <fstream>
using namespace std;

#ifdef LIBSYLVIA_IN_WINDOWS
#include <Windows.h>
#include <ObjBase.h>
#elif defined LIBSYLVIA_IN_LINUX
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <uuid.h>
#include <iconv.h>
#endif


int libSylvia_guessWhat(const char* pURI, std::string& sSavedAs)
{
	std::string sURI(pURI);

	int slash_pos = sURI.find_last_of('/');
	if (std::string::npos == slash_pos)
	{
		srand(time(NULL));
		int name = rand();

		stringstream ss;
		ss << name << ".save";
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
		ss << name << ".save";
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
	
	char* tmp = new char[sSavedAs.length() * 2 + 1];
	memset(tmp, 0, sSavedAs.length() * 2 + 1);
	if (0 != libSylvia_urlDecode(sSavedAs.c_str(), tmp, sSavedAs.length() * 2))
	{
		delete[] tmp;
		return 0;
	}

	sSavedAs.clear();
	sSavedAs = tmp;
	delete[] tmp;

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

int libSylvia_md5(const void* pSrcBuf, const unsigned int nSrcLen, char* pDstBuf)
{
	LIBSYLVIA_MD5_CTX ctx;
	unsigned char tmp[16] = {0};
	libSylvia_MD5Init(&ctx);
	libSylvia_MD5Update(&ctx, (unsigned char*)pSrcBuf, nSrcLen);
	libSylvia_MD5Final(tmp, &ctx);

	sprintf((char*)pDstBuf, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
		tmp[0], tmp[1], tmp[2], tmp[3], tmp[4], tmp[5], tmp[6], tmp[7], 
		tmp[8], tmp[9] ,tmp[10], tmp[11], tmp[12], tmp[13], tmp[14], tmp[15]);

	return 0;
}

int libSylvia_uuid( char uuid[LIBSYLVIA_UUID_LENGTH] )
{
#ifdef LIBSYLVIA_IN_WINDOWS
	GUID id;
	HRESULT hr = CoCreateGuid(&id);
	if (SUCCEEDED(hr))
	{
		_snprintf(uuid, 36, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X", 
			id.Data1, id.Data2, id.Data3,
			id.Data4[0], id.Data4[1], id.Data4[2], id.Data4[3], 
			id.Data4[4], id.Data4[5], id.Data4[6], id.Data4[7]);
	}
	else
	{
		LIBSYLVIA_LOG_ERROR("Create GUID FAILED");
	}
#elif defined LIBSYLVIA_IN_LINUX
	uuid_t* id;
	uuid_rc_t rc = uuid_create(&id);
	if (UUID_RC_OK != rc)
	{
		LIBSYLVIA_LOG_ERROR("uuid_create error %d", rc);
		return -1;
	}

	rc = uuid_make(id, UUID_MAKE_V4, NULL, NULL);
	if (UUID_RC_OK != rc)
	{
		LIBSYLVIA_LOG_ERROR("uuid_make error %d", rc);
		return -2;
	}

	void* ptr = uuid;
	size_t len = LIBSYLVIA_UUID_LENGTH;
	rc = uuid_export(id, UUID_FMT_STR, &ptr, &len);
	if (rc != UUID_RC_OK)
	{
		LIBSYLVIA_LOG_ERROR("uuid_export error %d", rc);
		return -3;
	}
#endif

	return 0;
}

int libSylvia_urlEncode(const char* szSrc, char* pBuf, int cbBufLen, bool bUpperCase)
{
	if(szSrc == NULL || pBuf == NULL || cbBufLen <= 0)
		return -1;

	size_t cbMultiBytes = strlen(szSrc);
	if(cbMultiBytes == 0)
	{
		pBuf[0] = 0;
		return 0;
	}

#ifdef LIBSYLVIA_IN_WINDOWS
	int cchWideChar = MultiByteToWideChar(CP_ACP, 0, szSrc, cbMultiBytes, NULL, 0);
	LPWSTR pUnicode = (LPWSTR)malloc((cchWideChar + 1) * sizeof(WCHAR));
	if(pUnicode == NULL)
		return -2;
	MultiByteToWideChar(CP_ACP, 0, szSrc, cbMultiBytes, pUnicode, cchWideChar + 1);

	int cbUTF8 = WideCharToMultiByte(CP_UTF8, 0, pUnicode, cchWideChar, NULL, 0, NULL, NULL);
	LPSTR pUTF8 = (LPSTR)malloc((cbUTF8 + 1) * sizeof(CHAR));
	if(pUTF8 == NULL)
	{
		free(pUnicode);
		return -3;
	}
	WideCharToMultiByte(CP_UTF8, 0, pUnicode, cchWideChar, pUTF8, cbUTF8 + 1, NULL, NULL);
	pUTF8[cbUTF8] = '\0';
#elif defined LIBSYLVIA_IN_LINUX
	int cbUTF8 = strlen(szSrc) + 1;
	char* pUTF8 = (char*)malloc(cbUTF8);
	strncpy(pUTF8, szSrc, strlen(szSrc));
#endif

	char baseChar = bUpperCase ? 'A' : 'a';
	unsigned char c;
	int cbDest = 0;
	unsigned char *pSrc = (unsigned char*)pUTF8;
	unsigned char *pDest = (unsigned char*)pBuf;
	while(*pSrc && cbDest < cbBufLen - 1)
	{
		c = *pSrc;
		if(isalpha(c) || isdigit(c) || c == '-' || c == '.' || c == '~')
		{
			*pDest = c;
			++pDest;
			++cbDest;
		}
		else if(c == ' ')
		{
			*pDest = '+';
			++pDest;
			++cbDest;
		}
		else
		{
			if(cbDest + 3 > cbBufLen - 1)
				break;
			pDest[0] = '%';
			pDest[1] = (c >= 0xA0) ? ((c >> 4) - 10 + baseChar) : ((c >> 4) + '0');
			pDest[2] = ((c & 0xF) >= 0xA)? ((c & 0xF) - 10 + baseChar) : ((c & 0xF) + '0');
			pDest += 3;
			cbDest += 3;
		}
		++pSrc;
	}

	*pDest = '\0';
#ifdef LIBSYLVIA_IN_WINDOWS
	free(pUnicode);
#endif
	free(pUTF8);
	return 0;
}

int libSylvia_urlDecode(const char* szSrc, char* pBuf, int cbBufLen)
{
	if(szSrc == NULL || pBuf == NULL || cbBufLen <= 0)
		return -1;

	size_t len_ascii = strlen(szSrc);
	if(len_ascii == 0)
	{
		pBuf[0] = 0;
		return 0;
	}

	char *pUTF8 = (char*)malloc(len_ascii + 1);
	if(pUTF8 == NULL)
		return -2;

	int cbDest = 0;
	unsigned char *pSrc = (unsigned char*)szSrc;
	unsigned char *pDest = (unsigned char*)pUTF8;
	while(*pSrc)
	{
		if(*pSrc == '%')
		{
			*pDest = 0;
			if(pSrc[1] >= 'A' && pSrc[1] <= 'F')
				*pDest += (pSrc[1] - 'A' + 10) * 0x10;
			else if(pSrc[1] >= 'a' && pSrc[1] <= 'f')
				*pDest += (pSrc[1] - 'a' + 10) * 0x10;
			else
				*pDest += (pSrc[1] - '0') * 0x10;

			if(pSrc[2] >= 'A' && pSrc[2] <= 'F')
				*pDest += (pSrc[2] - 'A' + 10);
			else if(pSrc[2] >= 'a' && pSrc[2] <= 'f')
				*pDest += (pSrc[2] - 'a' + 10);
			else
				*pDest += (pSrc[2] - '0');

			pSrc += 3;
		}
		else if(*pSrc == '+')
		{
			*pDest = ' ';
			++pSrc;
		}
		else
		{
			*pDest = *pSrc;
			++pSrc;
		}
		++pDest;
		++cbDest;
	}

	*pDest = '\0';
	++cbDest;

#ifdef LIBSYLVIA_IN_WINDOWS
	int cchWideChar = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)pUTF8, cbDest, NULL, 0);
	LPWSTR pUnicode = (LPWSTR)malloc(cchWideChar * sizeof(WCHAR));
	if(pUnicode == NULL)
	{
		free(pUTF8);
		return -3;
	}
	MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)pUTF8, cbDest, pUnicode, cchWideChar);
	WideCharToMultiByte(CP_ACP, 0, pUnicode, cchWideChar, pBuf, cbBufLen, NULL, NULL);
#elif defined LIBSYLVIA_IN_LINUX
	strncpy(pBuf, pUTF8, (strlen(pUTF8) < cbBufLen ? strlen(pUTF8) : cbBufLen));
#endif
	
	free(pUTF8);
#ifdef LIBSYLVIA_IN_WINDOWS
	free(pUnicode);
#endif

	return 0;
}
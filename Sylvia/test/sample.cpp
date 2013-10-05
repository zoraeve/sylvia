


#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <deque>
#include <map>
using namespace std;

#include <time.h>
#include <stdarg.h>

#include <curl/curl.h>
#pragma comment(lib, "libcurl_imp.lib")

#include <pthread.h>
#pragma comment(lib, "pthreadVC2.lib")

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#pragma comment(lib, "libglog.lib")

#define interval 10
#define threadPoolSize 10
#define segment (1024 * 1024)

std::deque<int> taskQ;
pthread_rwlock_t taskQLock = PTHREAD_RWLOCK_INITIALIZER;
pthread_t threadPool[threadPoolSize];
std::map<int, std::string> contents;

bool bReady2Exit = false;

#define ASYNC_LOG
#ifdef ASYNC_LOG
typedef struct _logUnit_
{
	int level;
	std::string logDetail;
}logUnit;
std::deque<_logUnit_> logQ;

void logger2(int level, std::string sLog)
{
	logUnit lu;
	lu.level = level;
	lu.logDetail = sLog;
	logQ.push_back(lu);

	return ;
}
#endif 

std::vector<pthread_t> threadVector;

void logger(int level, const char* format, ...)
{
	va_list pArg;
	va_start(pArg, format);

	char pBuf[1024] = {0};
	vsprintf(pBuf, format, pArg);

	va_end(pArg);

#ifndef ASYNC_LOG
	switch(level)
	{
	case 0:
		LOG(INFO) << pBuf << endl;
		break;
	case 1:
		LOG(WARNING) << pBuf << endl;
		break;
	case 2:
		LOG(ERROR) << pBuf << endl;
		break;
	case 3:
		LOG(FATAL) << pBuf << endl;
		break;
	default:
		LOG(INFO) << pBuf << endl;
		break;
	}
#else
	logger2(level, pBuf);
#endif
	
	return ;
}
#if defined(_WIN32) || defined(_WIN64)
#define LOG_INFO(fmt, ...)  logger(0, "<%s>\t<%d>\t<%s>,"fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  logger(1, "<%s>\t<%d>\t<%s>,"fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) logger(2, "<%s>\t<%d>\t<%s>,"fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...) logger(3, "<%s>\t<%d>\t<%s>,"fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define LOG_INFO(fmt, ...)  logger(0, "<%s>|<%d>|<%s>,"fmt, __FILE__, __LINE__, __FUNCTION__, ##args)
#define LOG_WARN(fmt, ...)  logger(1, "<%s>|<%d>|<%s>,"fmt, __FILE__, __LINE__, __FUNCTION__, ##args)
#define LOG_ERROR(fmt, ...) logger(2, "<%s>|<%d>|<%s>,"fmt, __FILE__, __LINE__, __FUNCTION__, ##args)
#define LOG_FATAL(fmt, ...) logger(3, "<%s>|<%d>|<%s>,"fmt, __FILE__, __LINE__, __FUNCTION__, ##args) 
#endif

size_t HttpHeaderHandler(void* ptr, size_t size, size_t nmemb, void* stream)
{
	if (nullptr == stream)
	{
		return -1;
	}

	std::string* pStr = reinterpret_cast<std::string*>(stream);

	char* p = (char*)ptr;

	pStr->append(p, size * nmemb);

	return size * nmemb;
}

size_t HttpContentHandler(void* ptr, size_t size, size_t nmemb, void* stream)
{
	if (nullptr == stream)
	{
		return -1;
	}

	std::string* pStr = reinterpret_cast<std::string*>(stream);

	char* p = (char*)ptr;

	pStr->append(p, size * nmemb);

	return size * nmemb;
}

void* thdLogger(void* lparam)
{
	while(1)
	{
		if (logQ.size())
		{
			logUnit lu = logQ.front();
			logQ.pop_front();
			switch(lu.level)
			{
			case 0:
				LOG(INFO) << lu.logDetail << endl;
				break;
			case 1:
				LOG(WARNING) << lu.logDetail << endl;
				break;
			case 2:
				LOG(ERROR) << lu.logDetail << endl;
				break;
			case 3:
				LOG(FATAL) << lu.logDetail << endl;
				break;
			default:
				LOG(INFO) << lu.logDetail << endl;
				break;
			}
		}
		else
		{
			if (!bReady2Exit)
			{
				Sleep(10);
				continue;
			}
			break;
		}
	}

	return nullptr;
}

int judgement()
{
	return 0;
}

int GetHttpContentLength(const char* szURI)
{
	logger(0, "%s\n%s", "get content length for: ", szURI);

	if (nullptr == szURI)
	{
		return -1;
	}

	CURL* pCurl = nullptr;
	pCurl = curl_easy_init();
	if (nullptr == pCurl)
	{
//		logger(ERROR, "%s", "curl_easy_init failed");
		LOG_ERROR("curl_easy_init failed");
		return -2;
	}

	curl_easy_reset(pCurl);

	std::string sHeader;

	curl_easy_setopt(pCurl, CURLOPT_URL, szURI);
	curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(pCurl, CURLOPT_NOBODY, 1L);
	curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 30);
	curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, 30);
	curl_easy_setopt(pCurl, CURLOPT_HEADERFUNCTION, &HttpHeaderHandler);
	curl_easy_setopt(pCurl, CURLOPT_WRITEHEADER, &sHeader);


	CURLcode cRet = curl_easy_perform(pCurl);
	if (CURLE_OK == cRet)
	{
		logger(0, "%s", "Get Http Head Done");

		int pos = sHeader.find("Content-Length: ");
		if (std::string::npos != pos)
		{
			std::string sTmp = sHeader.substr(pos + strlen("Content-Length: "));
			pos = sTmp.find("\r\n");
			if (std::string::npos != pos)
			{
				curl_easy_cleanup(pCurl);
				return atoi(sTmp.substr(0, pos).c_str());
			}
			else
			{
				curl_easy_cleanup(pCurl);
				logger(ERROR, "%s", "parse http head failed, invalid format");
				return -5;
			}
		}
		else
		{
			curl_easy_cleanup(pCurl);
			logger(ERROR, "%s", "parse http head failed, cannot find Content-Length: ");
			return -4;
		}
	}
	else
	{
		curl_easy_cleanup(pCurl);
		logger(ERROR, "%s%d", "get http head failed, error: ", cRet);
		return -3;
	}
}


int GetHttpContent(const char* szURI)
{
	if (nullptr == szURI)
	{
		return -1;
	}

	CURL* pCurl = nullptr;
	pCurl = curl_easy_init();
	if (nullptr == pCurl)
	{
		logger(ERROR, "%s", "curl_easy_init failed");
		return -2;
	}

	curl_easy_reset(pCurl);

	std::string sData;

	curl_easy_setopt(pCurl, CURLOPT_URL, szURI);
	curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);
//	curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, 30);
	curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, &HttpContentHandler);
	curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &sData);

	CURLcode cRet = curl_easy_perform(pCurl);
	if (CURLE_OK == cRet)
	{
		ofstream file;
		file.open("sample", ios::binary);
		file << sData;
		file.flush();
		file.close();
	}

	curl_easy_cleanup(pCurl);

	return 0;
}

void* thdGetHttpContent(void* lparam)
{
	if (nullptr == lparam)
	{
		return nullptr;
	}

	const char* szURI = reinterpret_cast<char*>(lparam);

	CURL* pCurl = nullptr;
	pCurl = curl_easy_init();
	if (nullptr == pCurl)
	{
		logger(ERROR, "%s", "curl_easy_init failed");
		return nullptr;
	}

	while(true)
	{
		curl_easy_reset(pCurl);

		pthread_rwlock_rdlock(&taskQLock);
		if (0 >= taskQ.size())
		{
			pthread_rwlock_unlock(&taskQLock);
			break;
		}
		pthread_rwlock_unlock(&taskQLock);

		int nRet = pthread_rwlock_wrlock(&taskQLock);
		if (0 != nRet)
		{
			Sleep(interval);
			continue;
		}
		if (0 >= taskQ.size())
		{
			pthread_rwlock_unlock(&taskQLock);
			break;
		}
		int index = 0;
		index = taskQ.front();
		taskQ.pop_front();
		pthread_rwlock_unlock(&taskQLock);

		logger(0, "%s%d%s", "part ", index, " start");

		std::string sData;

		curl_easy_setopt(pCurl, CURLOPT_URL, szURI);
		curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);
//		curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1L);
		curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, 10);

		curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 60);
		curl_easy_setopt(pCurl, CURLOPT_LOW_SPEED_LIMIT, 1);
		curl_easy_setopt(pCurl, CURLOPT_LOW_SPEED_TIME, 30);

		char szRange[128] = {0};
		sprintf(szRange, "%d-%d", index * segment, (index + 1) * segment - 1);
		curl_easy_setopt(pCurl, CURLOPT_RANGE, szRange);
		curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, &HttpContentHandler);
		curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &sData);

		CURLcode cRet = curl_easy_perform(pCurl);
		if (CURLE_OK == cRet)
		{
			double speed, size;
			curl_easy_getinfo(pCurl, CURLINFO_SIZE_DOWNLOAD, &size);
			curl_easy_getinfo(pCurl, CURLINFO_SPEED_DOWNLOAD, &speed);

			logger(0, "%s%d%s%f%s%f%s", "part ", index, " done, download size: ", size, "bytes, Avg speed: ", speed, "bytes-per-sec");

			contents[index] = sData;
		}
		else
		{
			logger(ERROR, "%s%d%s%d%s", "part " , index, " failed for ", cRet, ", reset this part");

			nRet = pthread_rwlock_wrlock(&taskQLock);
			while(0 != nRet)
			{
				Sleep(interval);
				nRet = pthread_rwlock_wrlock(&taskQLock);
			}
			taskQ.push_back(index);
			pthread_rwlock_unlock(&taskQLock);
		}
	}

	curl_easy_cleanup(pCurl);

	return nullptr;
}

int SaveToFile(const char* szSaveAs)
{
	char szName[128] = {0};

	if (nullptr == szSaveAs)
	{
		srand(time(NULL));
		int name = rand();

		sprintf(szName, "%d.save", name);
	}
	else
	{
		sprintf(szName, szSaveAs);
	}

	logger(0, "%s%s", "save to file with name: ", szName);

	ofstream file;
	file.open(szName, ios::binary);

	if (!file.is_open())
	{
		logger(ERROR, "%s", "create file failed");
		return -1;
	}

	for(int cnt = 0; cnt < contents.size(); ++cnt)
	{
		file << contents[cnt];
	}

	file.flush();
	file.close();

	return 0;
}

int Process(const char* szURI, const char* szSaveAs = nullptr)
{
	std::cout << "Processing: " << endl << szURI << endl;

	logger(0, "%s\n%s", "processing: ", szURI);

	if (nullptr == szURI)
	{
		return -1;
	}

	int nSize = GetHttpContentLength(szURI);

	//	GetHttpContent(szURI);

	if (0 >= nSize)
	{
		logger(ERROR, "%s", "get http content length error");
		return -2;
	}

//	logger(0, "%s%d", "http content length: ", nSize);
	LOG_INFO("%s%d", "http content length: ", nSize);

	int nFrames = nSize / segment + 1;
	for (int cnt = 0; cnt < nFrames; ++cnt)
	{
		taskQ.push_back(cnt);
	}

	logger(0, "%s%d%s", "Split to ", nFrames, " pieces");

	for (int cnt = 0; cnt < threadPoolSize; ++cnt)
	{
		pthread_create(&threadPool[cnt], nullptr, &thdGetHttpContent, (void*)szURI);
	}

	do 
	{
		float progress = ( (float)contents.size() / (float)nFrames ) * 100.0;
		cout << progress << "% Complete" << endl;
		Sleep(1000);
	} while (contents.size() < nFrames);

	cout << "100% Complete" << endl;

	if (nullptr == szSaveAs)
	{
		char szName[128] = {0};
		std::string s = szURI;
		if (std::string::npos == s.find("/"))
		{
			return SaveToFile(szURI);
		}
		else
		{
			return SaveToFile(s.substr(s.find_last_of("/") + 1).c_str());
		}
	}
	return SaveToFile(szSaveAs);
}

int main(int argc, char* argv[])
{
	google::InitGoogleLogging(argv[0]);
	google::SetLogDestination(google::GLOG_INFO, "./Sylvia_");

	pthread_rwlock_init(&taskQLock, nullptr);

#ifdef ASYNC_LOG
	pthread_t thdLog;
	pthread_create(&thdLog, nullptr, thdLogger, nullptr);
#endif

#ifdef PRODUCT_RELEASE
	switch(argc)
	{
	case 2:
		Process(argv[1], nullptr);
		break;
	case 3:
		Process(argv[1], argv[2]);
		break;
	default:
		{
			std::cout << "Usage: " << endl;
			std::cout << "test.exe URI File" << endl;
			std::cout << "Sample: " << endl;
			std::cout << "test.exe http://sample.com/sample.txt sample.txt" << endl;
		}
		break;
	}
#else
	// 	int cnt = 5;
	// 	while(--cnt)
	{
		taskQ.clear();
		contents.clear();

		cout << "================================= start: =================================" << endl;
//		Process("http://optimate.dl.sourceforge.net/project/udt/udt/4.11/udt.sdk.4.11.tar.gz", nullptr);
//		Process("http://dldir1.qq.com/qqfile/qq/QQ2013/QQ2013SP2/8178/QQ2013SP2.exe", nullptr);
		Process("http://www.wholetomato.com/binaries/VA_X_Setup2001.exe", nullptr);
		cout << "================================= done:  =================================" << endl;
	}
#endif

#ifdef ASYNC_LOG
	bReady2Exit = true;
	void* ptrRet;
	pthread_join(thdLog, &ptrRet);
#endif

	google::FlushLogFiles(0);

	cout << endl << "cleanup" << endl;
	system("pause");

	return 0;
}
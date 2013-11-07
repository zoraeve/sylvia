#include "libSylviaTask.h"
#include "libSylviaLogger.h"
#include "libSylviaUtility.h"

#include <iostream>
using namespace std;
#include <time.h>

#ifdef LIBSYLVIA_IN_WINDOWS
#include <curl/curl.h>
#pragma comment(lib, "libcurl_imp.lib")
#elif defined LIBSYLVIA_IN_LINUX
#include "curl/curl.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#endif


size_t thdHttpHeaderHandler(void* ptr, size_t size, size_t nmemb, void* stream)
{
	if (NULL == stream)
	{
		return -1;
	}
	std::string* pStr = reinterpret_cast<std::string*>(stream);

	char* p = (char*)ptr;

	pStr->append(p, size * nmemb);

	return size * nmemb;
}

size_t thdHttpContentHandler(void* ptr, size_t size, size_t nmemb, void* stream)
{
	if (NULL == stream)
	{
		return -1;
	}
	std::string* pStr = reinterpret_cast<std::string*>(stream);

	char* p = (char*)ptr;

	pStr->append(p, size * nmemb);

	return size * nmemb;
}

void* thdGetHttpContent(void* lparam)
{
	if (NULL == lparam)
	{
		return NULL;
	}
	libSylviaTask* p = reinterpret_cast<libSylviaTask*>(lparam);

	CURL* pCurl = NULL;
	pCurl = curl_easy_init();
	if (NULL == pCurl)
	{
		LIBSYLVIA_LOG_ERROR("curl_easy_init failed");
		return NULL;
	}

	while(true)
	{
		curl_easy_reset(pCurl);

		int nRet = pthread_rwlock_rdlock(&p->TaskQLock);
		if (0 != nRet)
		{
			libSylvia_sleep(LIBSYLVIA_INTERVAL);
			continue;
		}

		if (0 >= p->TaskQ.size())
		{
			pthread_rwlock_unlock(&p->TaskQLock);
			break;
		}
		pthread_rwlock_unlock(&p->TaskQLock);

		nRet = pthread_rwlock_wrlock(&p->TaskQLock);
		if (0 != nRet)
		{
			libSylvia_sleep(LIBSYLVIA_INTERVAL);
			continue;
		}
		if (0 >= p->TaskQ.size())
		{
			pthread_rwlock_unlock(&p->TaskQLock);
			break;
		}
		int index = 0;
		index = p->TaskQ.front();
		p->TaskQ.pop_front();
		pthread_rwlock_unlock(&p->TaskQLock);

		LIBSYLVIA_LOG_INFO("%s%d%s", "part ", index, " start");

		std::string sData;

		curl_easy_setopt(pCurl, CURLOPT_URL, p->URI.c_str());
		curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);
//		curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1L);
		curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, 10);

		curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 60);
		curl_easy_setopt(pCurl, CURLOPT_LOW_SPEED_LIMIT, 1);
		curl_easy_setopt(pCurl, CURLOPT_LOW_SPEED_TIME, 30);

		char szRange[128] = {0};
		sprintf(szRange, "%d-%d", index * LIBSYLVIA_SEGMENTSIZE, (index + 1) * LIBSYLVIA_SEGMENTSIZE - 1);
		curl_easy_setopt(pCurl, CURLOPT_RANGE, szRange);
		curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, &thdHttpContentHandler);
		curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &sData);

		CURLcode cRet = curl_easy_perform(pCurl);
		if (CURLE_OK == cRet)
		{
			double speed, size;
			curl_easy_getinfo(pCurl, CURLINFO_SIZE_DOWNLOAD, &size);
			curl_easy_getinfo(pCurl, CURLINFO_SPEED_DOWNLOAD, &speed);

			LIBSYLVIA_LOG_INFO("%s%d%s%f%s%f%s", "part ", index, " done, download size: ", size, "bytes, Avg speed: ", speed, "bytes-per-sec");

			LIBSYLVIA_CONTENT c;
			c.index = index * LIBSYLVIA_SEGMENTSIZE;
			c.sData = sData;
			++p->CompleteParts;
			nRet = pthread_rwlock_wrlock(&p->ContentsQLock);
			while(0 != nRet)
			{
				libSylvia_sleep(LIBSYLVIA_INTERVAL);
				nRet = pthread_rwlock_wrlock(&p->ContentsQLock);
			}
			p->ContentsQ.push_back(c);
			pthread_rwlock_unlock(&p->ContentsQLock);
		}
		else
		{
			LIBSYLVIA_LOG_ERROR("%s%d%s%d%s", "part " , index, " failed for ", cRet, ", reset this part");

			nRet = pthread_rwlock_wrlock(&p->TaskQLock);
			while(0 != nRet)
			{
				libSylvia_sleep(LIBSYLVIA_INTERVAL);
				nRet = pthread_rwlock_wrlock(&p->TaskQLock);
			}
			p->TaskQ.push_back(index);
			pthread_rwlock_unlock(&p->TaskQLock);
		}
	}

	curl_easy_cleanup(pCurl);

	return NULL;
}

void* thdSaveToFile(void* lparam)
{
	if (NULL == lparam)
	{
		return NULL;
	}
	libSylviaTask* p = reinterpret_cast<libSylviaTask*>(lparam);

	while(p->CompleteParts != p->TotalFrames)
	{
		int nRet = pthread_rwlock_rdlock(&p->ContentsQLock);
		if (0 != nRet)
		{
			libSylvia_sleep(LIBSYLVIA_INTERVAL);
			continue;
		}
		if(p->ContentsQ.size() > 0)
		{
			pthread_rwlock_unlock(&p->ContentsQLock);

			nRet = pthread_rwlock_wrlock(&p->ContentsQLock);
			if (0 != nRet)
			{
				libSylvia_sleep(LIBSYLVIA_INTERVAL);
				continue;
			}
			if (0 >= p->ContentsQ.size())
			{
				pthread_rwlock_unlock(&p->ContentsQLock);
				libSylvia_sleep(LIBSYLVIA_INTERVAL);
				continue;
			}
			LIBSYLVIA_CONTENT c;
			c = p->ContentsQ.front();
			p->ContentsQ.pop_front();
			pthread_rwlock_unlock(&p->ContentsQLock);

			libSylvia_randomWrite(p->SaveAs.c_str(), c.index, c.sData);
		}
		else
		{
			pthread_rwlock_unlock(&p->ContentsQLock);
		}

		libSylvia_sleep(LIBSYLVIA_INTERVAL);
	}

	return NULL;
}

void* thdWorker(void* lparam)
{
	if (NULL == lparam)
	{
		return NULL;
	}
	libSylviaTask* p = reinterpret_cast<libSylviaTask*>(lparam);

	if (0 == p->URI.length())
	{
		return NULL;
	}

	p->TotolSize = p->GetHttpContentLength();
	if (0 >= p->TotolSize)
	{
		LIBSYLVIA_LOG_ERROR("get http content length error");
		return NULL;
	}
	LIBSYLVIA_LOG_INFO("%s%d", "http content length: ", p->TotolSize);

	libSylvia_preAllocation(p->TotolSize, p->SaveAs.c_str());

	p->TotalFrames = p->TotolSize / LIBSYLVIA_SEGMENTSIZE + 1;
	for (int cnt = 0; cnt < p->TotalFrames; ++cnt)
	{
		p->TaskQ.push_back(cnt);
	}
	LIBSYLVIA_LOG_INFO("%s%d%s", "Split to ", p->TotalFrames, " pieces");

	for (int cnt = 0; cnt < LIBSLYVIA_THREADPOOLSIZE; ++cnt)
	{
		pthread_create(&p->threadPool[cnt], NULL, &thdGetHttpContent, lparam);
	}
	pthread_create(&p->tidSaveToFile, NULL, &thdSaveToFile, lparam);

	while (p->CompleteParts < p->TotalFrames)
	{
		p->Progress = ((float)(p->CompleteParts) / (float)(p->TotalFrames)) * 100.0;
#ifdef _DEBUG
		cout << p->Progress << "% Complete" << endl;
#else
		LIBSYLVIA_LOG_INFO("%f Complete", p->Progress);
#endif
		libSylvia_sleep(LIBSYLVIA_INTERVAL * 1000);
	}

	void* pRet = NULL;
	pthread_join(p->tidSaveToFile, &pRet);

	while(0 < p->ContentsQ.size())
	{
		p->Progress = ( ((float)p->TotalFrames - (float)p->ContentsQ.size() + 1.0) / (float)p->TotalFrames ) * 100.0;

		LIBSYLVIA_CONTENT c;
		c = p->ContentsQ.front();
		p->ContentsQ.pop_front();
		libSylvia_randomWrite(p->SaveAs.c_str(), c.index, c.sData);

		cout << p->Progress << "% Complete" << endl;
	}
	cout << "100% Complete" << endl;

	return NULL;
}

libSylviaTask::libSylviaTask( const LIBSYLVIA_TASK& task )
{
	pthread_rwlock_init(&TaskQLock, NULL);
	pthread_rwlock_init(&ContentsQLock, NULL);

	TI = task.Index;
	URI = task.URI;
	SaveAs = task.SaveAs;
	Method = task.Method;
	Status = _LIBSYLVIA_STATUS_TASK_WAITING_;
	TotalFrames = 0;
	CompleteParts = 0;
	TotolSize = 0;
	Progress = 0;
	AvgSpeed = 0;
	MaxSpeed = 0;
	MinSpeed = 0;
	DownloadSize = 0;
	Expend = time(NULL);

	Start();
}

libSylviaTask::~libSylviaTask(void)
{
}

int libSylviaTask::Start()
{
	pthread_create(&tidMaintain, NULL, &thdWorker, this);

	return 0;
}

int libSylviaTask::GetHttpContentLength()
{
	LIBSYLVIA_LOG_INFO("get content length for: %s", URI.c_str());

	if (0 == URI.length())
	{
		return -1;
	}

	CURL* pCurl = NULL;
	pCurl = curl_easy_init();
	if (NULL == pCurl)
	{
		LIBSYLVIA_LOG_ERROR("curl_easy_init failed");
		return -2;
	}
	curl_easy_reset(pCurl);

	std::string sHeader;

	curl_easy_setopt(pCurl, CURLOPT_URL, URI.c_str());
	curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(pCurl, CURLOPT_NOBODY, 1L);
	curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 30);
	curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, 30);
	curl_easy_setopt(pCurl, CURLOPT_HEADERFUNCTION, &thdHttpHeaderHandler);
	curl_easy_setopt(pCurl, CURLOPT_WRITEHEADER, &sHeader);

	CURLcode cRet = curl_easy_perform(pCurl);
	if (CURLE_OK == cRet)
	{
		int pos = sHeader.find("Content-Length: ");
		if (std::string::npos != pos)
		{
			std::string sTmp = sHeader.substr(pos + strlen("Content-Length: "));
			pos = sTmp.find("\r\n");
			if (std::string::npos != pos)
			{
				curl_easy_cleanup(pCurl);
				LIBSYLVIA_LOG_INFO("Get Http Head Done");
				return atoi(sTmp.substr(0, pos).c_str());
			}
			else
			{
				curl_easy_cleanup(pCurl);
				LIBSYLVIA_LOG_ERROR("parse http head failed, invalid format");
				return -5;
			}
		}
		else
		{
			curl_easy_cleanup(pCurl);
			LIBSYLVIA_LOG_ERROR("parse http head failed, cannot find Content-Length");
			return -4;
		}
	}
	else
	{
		curl_easy_cleanup(pCurl);
		LIBSYLVIA_LOG_ERROR("%s%d", "get http head failed, error: ", cRet);
		return -3;
	}
}

int libSylviaTask::Query( LIBSYLVIA_INFO& info )
{
	strncpy(info.szURI, URI.c_str(), URI.length() > 4095 ? 4095 : URI.length());
	strncpy(info.szSaveAs, SaveAs.c_str(), SaveAs.length() > 255 ? 255 : SaveAs.length());
	info.Progress = Progress;
	info.status = Status;

	return 0;
}

int libSylviaTask::Cancel()
{
	return 0;
}

int libSylviaTask::Resume()
{
	return 0;
}

int libSylviaTask::Pause()
{
	return 0;
}

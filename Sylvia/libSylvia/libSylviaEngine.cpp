#include "libSylviaEngine.h"
#include "libSylviaLogger.h"
#include "libSylviaUtility.h"

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
	libSylvia_engine* p = reinterpret_cast<libSylvia_engine*>(lparam);

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

		pthread_rwlock_rdlock(&p->task.TaskQLock);
		if (0 >= p->task.TaskQ.size())
		{
			pthread_rwlock_unlock(&p->task.TaskQLock);
			break;
		}
		pthread_rwlock_unlock(&p->task.TaskQLock);

		int nRet = pthread_rwlock_wrlock(&p->task.TaskQLock);
		if (0 != nRet)
		{
			libSylvia_sleep(LIBSYLVIA_INTERVAL);
			continue;
		}
		if (0 >= p->task.TaskQ.size())
		{
			pthread_rwlock_unlock(&p->task.TaskQLock);
			break;
		}
		int index = 0;
		index = p->task.TaskQ.front();
		p->task.TaskQ.pop_front();
		pthread_rwlock_unlock(&p->task.TaskQLock);

		LIBSYLVIA_LOG_INFO("%s%d%s", "part ", index, " start");

		std::string sData;

		curl_easy_setopt(pCurl, CURLOPT_URL, p->task.URI.c_str());
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
			++p->task.CompleteParts;
			nRet = pthread_rwlock_wrlock(&p->task.ContentsQLock);
			while(0 != nRet)
			{
				libSylvia_sleep(LIBSYLVIA_INTERVAL);
				nRet = pthread_rwlock_wrlock(&p->task.ContentsQLock);
			}
			p->task.ContentsQ.push_back(c);
			pthread_rwlock_unlock(&p->task.ContentsQLock);
		}
		else
		{
			LIBSYLVIA_LOG_ERROR("%s%d%s%d%s", "part " , index, " failed for ", cRet, ", reset this part");

			nRet = pthread_rwlock_wrlock(&p->task.TaskQLock);
			while(0 != nRet)
			{
				libSylvia_sleep(LIBSYLVIA_INTERVAL);
				nRet = pthread_rwlock_wrlock(&p->task.TaskQLock);
			}
			p->task.TaskQ.push_back(index);
			pthread_rwlock_unlock(&p->task.TaskQLock);
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
	libSylvia_engine* p = reinterpret_cast<libSylvia_engine*>(lparam);

	while(p->task.CompleteParts != p->task.TotalFrames)
	{
		while(p->task.ContentsQ.size() > 0)
		{
			pthread_rwlock_rdlock(&p->task.ContentsQLock);
			if (0 >= p->task.ContentsQ.size())
			{
				pthread_rwlock_unlock(&p->task.ContentsQLock);
				continue;
			}
			pthread_rwlock_unlock(&p->task.ContentsQLock);

			int nRet = pthread_rwlock_wrlock(&p->task.ContentsQLock);
			if (0 != nRet)
			{
				libSylvia_sleep(LIBSYLVIA_INTERVAL);
				continue;
			}
			if (0 >= p->task.ContentsQ.size())
			{
				pthread_rwlock_unlock(&p->task.ContentsQLock);
				break;
			}
			LIBSYLVIA_CONTENT c;
			c = p->task.ContentsQ.front();
			p->task.ContentsQ.pop_front();
			pthread_rwlock_unlock(&p->task.ContentsQLock);

			libSylvia_randomWrite(p->task.SaveAs.c_str(), c.index, c.sData);
		}
		libSylvia_sleep(LIBSYLVIA_INTERVAL);
	}

	return NULL;
}

void* worker(void* lparam)
{
	if (NULL == lparam)
	{
		return NULL;
	}
	libSylvia_engine* p = reinterpret_cast<libSylvia_engine*>(lparam);

	if (0 == p->task.URI.length())
	{
		return NULL;
	}

	p->task.TotolSize = p->GetHttpContentLength();

	if (0 >= p->task.TotolSize)
	{
		LIBSYLVIA_LOG_ERROR("get http content length error");
		return NULL;
	}

	LIBSYLVIA_LOG_INFO("%s%d", "http content length: ", p->task.TotolSize);

	libSylvia_preAllocation(p->task.TotolSize, p->task.SaveAs.c_str());

	p->task.TotalFrames = p->task.TotolSize / LIBSYLVIA_SEGMENTSIZE + 1;
	for (int cnt = 0; cnt < p->task.TotalFrames; ++cnt)
	{
		p->task.TaskQ.push_back(cnt);
	}

	LIBSYLVIA_LOG_INFO("%s%d%s", "Split to ", p->task.TotalFrames, " pieces");

	p->task.Progress = 0.0f;

	for (int cnt = 0; cnt < LIBSLYVIA_THREADPOOLSIZE; ++cnt)
	{
		pthread_create(&p->threadPool[cnt], NULL, &thdGetHttpContent, lparam);
	}

	pthread_create(&p->tidSaveToFile, NULL, &thdSaveToFile, lparam);

	do 
	{
		p->task.Progress = ((float)(p->task.CompleteParts) / (float)(p->task.TotalFrames)) * 100.00;
#ifdef _DEBUG
		cout << p->task.Progress << "% Complete" << endl;
#else
		LIBSYLVIA_LOG_INFO("%f Complete", p->task.Progress);
#endif
		libSylvia_sleep(LIBSYLVIA_INTERVAL * 100);
	} while (p->task.CompleteParts < p->task.TotalFrames);

	void* pRet = NULL;
	pthread_join(p->tidSaveToFile, &pRet);

	while(0 < p->task.ContentsQ.size())
	{
		p->task.Progress = ( ((float)p->task.TotalFrames - (float)p->task.ContentsQ.size() + 1.0) / (float)p->task.TotalFrames ) * 100.0;

		LIBSYLVIA_CONTENT c;
		c = p->task.ContentsQ.front();
		p->task.ContentsQ.pop_front();
		libSylvia_randomWrite(p->task.SaveAs.c_str(), c.index, c.sData);

		cout << p->task.Progress << "% Complete" << endl;
	}
	cout << "100% Complete" << endl;

	return NULL;
}

libSylvia_engine::libSylvia_engine(void)
{
	task.URI = "";
	task.SaveAs = "";
	task.Method = _LIBSYLVIA_METHOD_UNKNOWN_;
	task.TaskQ.clear();
	task.ContentsQ.clear();

	pthread_rwlock_init(&task.TaskQLock, NULL);
	pthread_rwlock_init(&task.ContentsQLock, NULL);
}

libSylvia_engine::~libSylvia_engine(void)
{
}

int libSylvia_engine::AddTask( LIBSYLVIA_TASK& t )
{
	task = t;

	pthread_create(&tidWorker, NULL, worker, this);

	return 0;
}

int libSylvia_engine::cleanup()
{
	return 0;
}

int libSylvia_engine::GetHttpContentLength()
{
	LIBSYLVIA_LOG_INFO("%s\n%s", "get content length for: ", task.URI.c_str());

	if (0 == task.URI.length())
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

	curl_easy_setopt(pCurl, CURLOPT_URL, task.URI.c_str());
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
		LIBSYLVIA_LOG_INFO("Get Http Head Done");

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

int libSylvia_engine::query(LIBSYLVIA_STATUS& status)
{
	strncpy(status.szCurrentURI, task.URI.c_str(), task.URI.length());
	strncpy(status.szCurrentSaveAs, task.SaveAs.c_str(), task.SaveAs.length());
	status.currentProgress = task.Progress;

	return 0;
}

int libSylvia_engine::cancel()
{
	return 0;
}

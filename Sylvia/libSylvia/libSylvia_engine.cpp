#include "libSylvia_engine.h"
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


size_t httpHeaderHandler(void* ptr, size_t size, size_t nmemb, void* stream)
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

size_t httpContentHandler(void* ptr, size_t size, size_t nmemb, void* stream)
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

		pthread_rwlock_rdlock(&p->taskQLock);
		if (0 >= p->taskQ.size())
		{
			pthread_rwlock_unlock(&p->taskQLock);
			break;
		}
		pthread_rwlock_unlock(&p->taskQLock);

		int nRet = pthread_rwlock_wrlock(&p->taskQLock);
		if (0 != nRet)
		{
#ifdef LIBSYLVIA_IN_WINDOWS
			Sleep(LIBSYLVIA_INTERVAL);
#elif defined LIBSYLVIA_IN_LINUX
			usleep(LIBSYLVIA_INTERVAL);
#endif
			continue;
		}
		if (0 >= p->taskQ.size())
		{
			pthread_rwlock_unlock(&p->taskQLock);
			break;
		}
		int index = 0;
		index = p->taskQ.front();
		p->taskQ.pop_front();
		pthread_rwlock_unlock(&p->taskQLock);

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
		curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, &httpContentHandler);
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
			++p->finishedBlocks;
			nRet = pthread_rwlock_wrlock(&p->contentsQLock);
			while(0 != nRet)
			{
#ifdef LIBSYLVIA_IN_WINDOWS
				Sleep(LIBSYLVIA_INTERVAL);
#elif defined LIBSYLVIA_IN_LINUX
				usleep(LIBSYLVIA_INTERVAL);
#endif
				nRet = pthread_rwlock_wrlock(&p->contentsQLock);
			}
			p->contentsQ.push_back(c);
			pthread_rwlock_unlock(&p->contentsQLock);
#if 0
			p->contents[index] = sData;
#endif
		}
		else
		{
			LIBSYLVIA_LOG_ERROR("%s%d%s%d%s", "part " , index, " failed for ", cRet, ", reset this part");

			nRet = pthread_rwlock_wrlock(&p->taskQLock);
			while(0 != nRet)
			{
#ifdef LIBSYLVIA_IN_WINDOWS
				Sleep(LIBSYLVIA_INTERVAL);
#elif defined LIBSYLVIA_IN_LINUX
				usleep(LIBSYLVIA_INTERVAL);
#endif
				nRet = pthread_rwlock_wrlock(&p->taskQLock);
			}
			p->taskQ.push_back(index);
			pthread_rwlock_unlock(&p->taskQLock);
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

	while(!p->bComplete)
	{
		while(p->contentsQ.size() > 0)
		{
			pthread_rwlock_rdlock(&p->contentsQLock);
			if (0 >= p->contentsQ.size())
			{
				pthread_rwlock_unlock(&p->contentsQLock);
				continue;
			}
			pthread_rwlock_unlock(&p->contentsQLock);

			int nRet = pthread_rwlock_wrlock(&p->contentsQLock);
			if (0 != nRet)
			{
#ifdef LIBSYLVIA_IN_WINDOWS
				Sleep(LIBSYLVIA_INTERVAL);
#elif defined LIBSYLVIA_IN_LINUX
				usleep(LIBSYLVIA_INTERVAL);
#endif
				continue;
			}
			if (0 >= p->contentsQ.size())
			{
				pthread_rwlock_unlock(&p->contentsQLock);
				break;
			}
			LIBSYLVIA_CONTENT c;
			c = p->contentsQ.front();
			p->contentsQ.pop_front();
			pthread_rwlock_unlock(&p->contentsQLock);

			libSylvia_randomWrite(p->task.SaveAs.c_str(), c.index, c.sData);
		}
#ifdef LIBSYLVIA_IN_WINDOWS
		Sleep(LIBSYLVIA_INTERVAL);
#elif defined LIBSYLVIA_IN_LINUX
		usleep(LIBSYLVIA_INTERVAL);
#endif
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
		p->busy = false;
		return NULL;
	}

	int nSize = p->GetHttpContentLength();

	if (0 >= nSize)
	{
		LIBSYLVIA_LOG_ERROR("get http content length error");
		p->busy = false;
		return NULL;
	}

	LIBSYLVIA_LOG_INFO("%s%d", "http content length: ", nSize);

	libSylvia_preAllocation(nSize, p->task.SaveAs.c_str());

	int nFrames = nSize / LIBSYLVIA_SEGMENTSIZE + 1;
	for (int cnt = 0; cnt < nFrames; ++cnt)
	{
		p->taskQ.push_back(cnt);
	}

	LIBSYLVIA_LOG_INFO("%s%d%s", "Split to ", nFrames, " pieces");

	p->progress = 0.0f;

	for (int cnt = 0; cnt < LIBSLYVIA_THREADPOOLSIZE; ++cnt)
	{
		pthread_create(&p->threadPool[cnt], NULL, &thdGetHttpContent, lparam);
	}

	pthread_create(&p->tidSaveToFile, NULL, &thdSaveToFile, lparam);

	do 
	{
		p->progress = ((float)(p->finishedBlocks) / (float)(nFrames)) * 100.00;
#if 0
		p->progress = ( (float)p->contents.size() / (float)nFrames ) * 100.0;
#endif
		cout << p->progress << "% Complete" << endl;
#ifdef LIBSYLVIA_IN_WINDOWS
		Sleep(LIBSYLVIA_INTERVAL);
#elif defined LIBSYLVIA_IN_LINUX
		usleep(LIBSYLVIA_INTERVAL);
#endif
	} while (p->contents.size() < nFrames);

	void* pRet = NULL;
	pthread_join(p->tidSaveToFile, &pRet);

	while(0 < p->contentsQ.size())
	{
		float progress = ( ((float)nFrames - (float)p->contentsQ.size() + 1.0) / (float)nFrames ) * 100.0;

		LIBSYLVIA_CONTENT c;
		c = p->contentsQ.front();
		p->contentsQ.pop_front();
		libSylvia_randomWrite(p->task.SaveAs.c_str(), c.index, c.sData);

		cout << progress << "% Complete" << endl;
	}

	cout << "100% Complete" << endl;

#if 0
	if (0 == p->task.SaveAs.length())
	{
		char szName[128] = {0};
		std::string s = p->task.URI;
		if (std::string::npos == s.find("/"))
		{
			p->SaveToFile(p->task.URI.c_str());
			p->busy = false;
			return NULL;
		}
		else
		{
			p->SaveToFile(s.substr(s.find_last_of("/") + 1).c_str());
			p->busy = false;
			return NULL;
		}
	}
	p->SaveToFile(p->task.SaveAs.c_str());
	p->busy = false;
#endif

	return NULL;
}

libSylvia_engine::libSylvia_engine(void)
{
	busy = false;
	task.URI = "";
	task.SaveAs = "";
	task.Method = _LIBSYLVIA_METHOD_UNKNOWN_;
	progress = 0.0;
	finishedBlocks = 0;
	bComplete = false;

	contents.clear();
	contentsQ.clear();

	pthread_rwlock_init(&taskQLock, NULL);
	pthread_rwlock_init(&contentsQLock, NULL);
}


libSylvia_engine::~libSylvia_engine(void)
{
}

bool libSylvia_engine::bBusy()
{
	return busy;
}

int libSylvia_engine::addTask( LIBSYLVIA_TASK& t )
{
	busy = true;
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
	curl_easy_setopt(pCurl, CURLOPT_HEADERFUNCTION, &httpHeaderHandler);
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

int libSylvia_engine::SaveToFile( const char* szSaveAs )
{
	char szName[128] = {0};

	if (NULL == szSaveAs)
	{
		srand(time(NULL));
		int name = rand();

		sprintf(szName, "%d.save", name);
	}
	else
	{
		sprintf(szName, szSaveAs);
	}

	LIBSYLVIA_LOG_INFO("%s%s", "save to file with name: ", szName);

	ofstream file;
	file.open(szName, ios::binary);

	if (!file.is_open())
	{
		LIBSYLVIA_LOG_ERROR("create file failed");
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

int libSylvia_engine::query(LIBSYLVIA_STATUS& status)
{
	strncpy(status.szCurrentURI, task.URI.c_str(), task.URI.length());
	strncpy(status.szCurrentSaveAs, task.SaveAs.c_str(), task.SaveAs.length());
	status.currentProgress = progress;

	return 0;
}

int libSylvia_engine::cancel()
{
	return 0;
}

#include "stdafx.h"
#include "datapusher.h"
#include <functional>
#include <algorithm>
#include "evdi-http-post.h"
#include "get_client_ip.h"
#include "cJSON.h"
#include <curl/curl.h>
#include <iostream>  
#include <fstream>  
#include <stdlib.h>  
#include <condition_variable>

#define SafeNode(T)(NULL != T->valuestring && strlen(T->valuestring) > 0)
#define IGNORE_EVENT 1
#define SHARE_EVENT 2
#define TAB_EVENT 3
#define MOVE_EVENT 4
#define UPDATE_EVENT 5
static datapusher  *_inst = nullptr;
int volatile datapusher::portguard::_81Used;
int volatile datapusher::portguard:: _3490Used;

datapusher::datapusher()
{ 
	char client_ip[50] = { 0 };
	get_client_ip(client_ip);
	mtermip = client_ip;
	mAvalaiblePort = 0;

	InitWhiteList();
	//mtermip = "10.1.40.144";
}


datapusher * datapusher::getInstance()
{

	if(_inst  == nullptr)
	{
		_inst = new datapusher();
	}

	return _inst;
}

void datapusher::doPush(int port ,int type)
{
	portguard  pguard(port);

	char *outdata = (char*)malloc(1024);
	if ( NULL == outdata )
	{
		return  ;
	}

	memset(outdata, 0, 1024);
	char url[50] = { 0 }; 
	_snprintf(url, 50 ,"http://%s:%d/request/flash/share_url", mtermip.c_str(),port);

	int nret = evdi_http_post(url, mdata.c_str(), outdata);
	printf("print_json == %s \n", mdata.c_str());

	if (nret == 0)
	{
		mAvalaiblePort = port;
	}

	cJSON *args;
	args = cJSON_Parse(outdata);

	if ( outdata )
	{
		free(outdata);
	}	 
			 
	 
}

static int empty_count = 0;
string  datapusher::pushRequestStatus(const char *data)
{

	/*
	if (mAvalaiblePort  == 81)
	{
		return  "{\"flag\":true}";
	}*/

	if (mAvalaiblePort  == 0)
	{
		return  "{\"flag\":false}";
	}

	if (mcurposturl.empty())
	{
		empty_count++;
		if (empty_count < 5)
			return  "{\"flag\":true}";
		else
		{
			empty_count = 0;
			return "{\"flag\":false}";
		}
	}

	empty_count = 0;
	if (!mcurposturl.empty())
	{
		cJSON *cjData = cJSON_Parse(data);
		if (NULL != cjData)
		{
			cJSON *jsUrl = cJSON_GetObjectItem(cjData, "url");
			if (NULL != jsUrl)
			{
				if (mcurposturl.compare(jsUrl->valuestring) != 0)
				{
					cJSON_Delete(cjData);
					return "{\"flag\":true}";
				}
			}

			cJSON_Delete(cjData);
		}
	}

	char *outdata = (char*)alloca(1024); //already in error status,refuse to work.
	if (NULL == outdata)
	{
		return "{\"flag\":false}";
	}

	memset(outdata, 0, 1024);

	char url[50] = { 0 };
	_snprintf(url, 50, "http://%s:%d/request/flash/getstatus", mtermip.c_str(), mAvalaiblePort);

	int nret = evdi_http_post(url, data, outdata,1000);

	if (nret == CURLE_OPERATION_TIMEDOUT) //look timeout as true till get status successfully.
	{
		strcpy(outdata, "{\"flag\":true}");
	}
	//if (strlen(outdata) > 0)
	printf("print_json == %s \n", outdata);

	return string(outdata);

}

void datapusher::InitWhiteList()
{
	TCHAR filename[255] = {0};
	::GetModuleFileName(0,filename,255);
	::PathRemoveFileSpec(filename);

	std::wstring strPath = filename;
	strPath += L"\\plugin\\math\\whitelist.cf";
	 
	if (!::PathFileExists(strPath.c_str()))
	{
		char *webnames[] = { "youku.com", "tudou.com", "v.qq.com", "iqiyi.com", "tv.sohu.com","youtube.com" };
		for (int i = 0; i < sizeof(webnames) / sizeof(char *);i++)
			mWhiteList.push_back(std::string(webnames[i]));

		return;
	}

	try{
		std::string  strAnsiPath = CW2A(strPath.c_str()).m_psz;
		std::ifstream in(strAnsiPath);
		if (!in.is_open())
		{
			::OutputDebugString(L"Error opening file");
			return;
		}

		char buffer[255] = { 0 };
		while (!in.eof())
		{
			in.getline(buffer, 255);
			std::string strline(buffer);
			
			if (!strline.empty())
				mWhiteList.push_back(strline);
		}
	}
	catch (exception *e){
		::OutputDebugString(CA2W(e->what()));
	}

}

bool datapusher::parseandstoreurl(const char *data)
{

	JsonHolder  jh(data);
	cJSON * cjData = jh.getPtr();

	//mcurposturl.clear();

	if (NULL != cjData)
	{
		cJSON *cjSharedUrl = cJSON_GetObjectItem(cjData,"share_url");
		if (NULL != cjSharedUrl)
		{
			if (NULL != cjSharedUrl->valuestring && strlen(cjSharedUrl->valuestring) > 0)
			{
				mcurposturl = cjSharedUrl->valuestring;
			}
		}

		cJSON * cjType = cJSON_GetObjectItem(cjData, "type");
		if (NULL != cjType)
		{
			if (NULL != cjType->valuestring && strlen(cjType->valuestring) > 0)
			{
				std::string strVal = cjType->valuestring;
				if (strVal.compare("tabevent") == 0)
				{
					cJSON *cjname = cJSON_GetObjectItem(cjData,"name");
					if (NULL != cjname)
					{
						if (NULL != cjname->valuestring && strlen(cjname->valuestring) > 0)
						{
							std::string strName = cjname->valuestring;
							if (strName.compare("onActivated") == 0)
							{
								cJSON *jsTab = cJSON_GetObjectItem(cjData, "tab");
								if (NULL != jsTab)
								{
									cJSON *jsUrl = cJSON_GetObjectItem(jsTab, "url");
									if (NULL != jsUrl)
									{
										if (NULL != jsUrl->valuestring && strlen(jsUrl->valuestring) > 0)
										{
											mcurposturl = jsUrl->valuestring;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	 
	return false;
}

bool datapusher::isWiteListEnabled(std::string strUrl)
{
	auto s = mWhiteList.begin();
	while (s != mWhiteList.end())
	{
		int pos = strUrl.find(*s);
		if (pos >= 0)
			return true;
		s++;
	}

	return false;
}

bool datapusher::isMessageCanbePushed(const char *data ,int &type)
{
	std::string strData = data;

	type = IGNORE_EVENT;
	int pos = strData.find("\"name\":\"onUpdated\"");
	if (pos >= 0) //ignore update
		return false;


	pos = strData.find("\"name\":\"onCreated\"");
	if (pos >= 0)
		return false;

	pos = strData.find("share_url");
	if (pos >= 0)
	{
		pos = strData.find("\"current\":false");
		if (pos >= 0)
			return false;
	}
	
	JsonHolder  jh(data);
	cJSON * cjData = jh.getPtr();
	cJSON *cjSharedUrl = cJSON_GetObjectItem(cjData, "share_url");
	if (NULL != cjSharedUrl)
	{
		type = SHARE_EVENT;
		if (SafeNode(cjSharedUrl))
		{
			std::string strUrl = cjSharedUrl->valuestring;
			return isWiteListEnabled(strUrl);
		}
	}
	else
	{

		cJSON  *jsName = cJSON_GetObjectItem(cjData, "name");
		if (NULL != jsName)
		{
			if (SafeNode(jsName))
			{
				std::string strName = jsName->valuestring;
				if (strName.compare("onActivated") == 0)
				{
					type = TAB_EVENT;
					cJSON *jsTab = cJSON_GetObjectItem(cjData, "tab");
					if (NULL != jsTab)
					{
						cJSON * jsUrl = cJSON_GetObjectItem(jsTab, "url");
						if (NULL != jsUrl)
						{
							if (SafeNode(jsUrl))
							{
								std::string strUrl = jsUrl->valuestring;
								return isWiteListEnabled(strUrl);
							}
						}
					}

				}

			}
		}

		cJSON  *jsType = cJSON_GetObjectItem(cjData, "type");
		if (NULL != jsType)
		{
			if (SafeNode(jsType))
			{
				std::string  strType = jsType->valuestring;
				if (strType.compare("middle") == 0)
				{
					type = MOVE_EVENT;
				}
			}
		}
		else
		{
			cJSON  *jsName = cJSON_GetObjectItem(cjData, "name");
			if (NULL == jsName)
			{
				type = UPDATE_EVENT;
			}
		}

	}
	 

	return true;
}

//char databuffer[4096] = { 0 };
bool  datapusher::push(const char * data)
{
	int type = 0;
	bool benable = isMessageCanbePushed(data ,type);

	if (benable == false)
	{
		if (type == TAB_EVENT)  //tab onActivated
			mdata = "{\"type\":\"tabevent\",\"name\":\"onActivated\",\"tab\":{\"url\":\"\"},\"playerStatus\":\"failed\",\"video\":null,\"current\":true}";
		else
			return true;
	}else
		mdata = data;

	//msgQ for scroll event
	if (type == MOVE_EVENT)
	{
		this->scrlmmsgQ.insertQ(mdata);
		return true;
	}


	//msgQ for scroll event
	if (type == UPDATE_EVENT)
	{
		this->udmmsgQ.insertQ(mdata);
		return true;
	}

	::OutputDebugString(CA2W(("\nenable push:" + mdata).c_str()));

	parseandstoreurl(mdata.c_str());
	if (mAvalaiblePort == 3490 && portguard::_3490Used != 0)
	{
		//mMsgBuffer.push_back(mdata);
		mmsgQ.insertQ(mdata);
	}

	if (mAvalaiblePort == 81 && portguard::_81Used != 0)
	{
		//mMsgBuffer.push_back(mdata);
		mmsgQ.insertQ(mdata);
	}

	if (mAvalaiblePort == 0) // try port
	{
		new std::thread(std::bind(&datapusher::doPush, this, 3490, type));
		new std::thread(std::bind(&datapusher::doPush, this, 81, type));
	}
	else
	{
		if (mAvalaiblePort == 3490 && portguard::_3490Used == 0)
		{ 
			mmsgQ.clearQ();
			new std::thread(std::bind(&datapusher::doPush, this, 3490, type));
		}

		if (mAvalaiblePort == 81 && portguard::_81Used == 0)
		{
			//mmsgQ.clearQ();
			mmsgQ.clearQ();
			new std::thread(std::bind(&datapusher::doPush, this, 81, type));
		}
	}
 
	return true;
}
std::string datapusher::getip()
{
	return this->mtermip;
}

void datapusher::showmsg()
{
	::MessageBox(NULL, L"tst", L"DEMO", MB_OK);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
datapusher::messageQ::messageQ(int waitTime) :pQTrd(NULL)
{
	mwaitTime = waitTime;
}

datapusher::messageQ::~messageQ()
{}
	 
void datapusher::messageQ::clearQ()
{
	std::lock_guard<std::mutex> lck(mqtx);
	mMsgBuffer.clear();
}

void datapusher::messageQ::pushQ(std::string strData)
{
	char outData[1024] = {};
	evdi_http_post("http://127.0.0.1:81/request/window/info", strData.c_str(),outData);
}
 
void datapusher::messageQ::RunQ(void)
{
	while (1)
	{
		std::unique_lock<std::mutex> ulck(metx);
		mcv.wait(ulck);
		{

			//2 secs buffer
			std::this_thread::sleep_for(chrono::milliseconds(mwaitTime));
			std::lock_guard<std::mutex> lck(mqtx);

			::OutputDebugString(L"\n RunQ :excute once....");
			if (mMsgBuffer.size() > 0)
			{
				int port = datapusher::getInstance()->getAvalaiblePort();
				if ((portguard::_3490Used == 0 && port == 3490) || (portguard::_81Used == 0 && port==81))
				{
					std::string str = *mMsgBuffer.begin();
					::OutputDebugString(CA2W(("\n RunQ:" + str).c_str()).m_psz);
					pushQ(str);
					mMsgBuffer.clear();
				}
			}
		}
	}
}

void datapusher::messageQ::insertQ(std::string strM)
{
	std::lock_guard<std::mutex> lck(mqtx);

	if (NULL == pQTrd)
	{
		pQTrd = new std::thread(std::bind(&datapusher::messageQ::RunQ, this));
	}

	mMsgBuffer.push_front(strM);
	mcv.notify_all();
}
	 
//////////////////////////////////////////////////////////////////////////////////////////////
//scroll message Q
std::string datapusher::scrollmessageQ::getUrl()
{
	int port = datapusher::getInstance()->getAvalaiblePort();
	std::string termip = datapusher::getInstance()->getip();

	if (termip.empty())
		return "";
	char url[50] = { 0 };
	std::string urlFormat = this->getUrlFormat();
	_snprintf(url, 50, urlFormat.c_str(), termip.c_str(), port);

	return std::string(url);
}
void datapusher::scrollmessageQ::pushQ(std::string strData)
{
	char *outdata = (char*)malloc(1024);
	if (NULL == outdata)
	{
		return;
	}

	memset(outdata, 0, 1024);
	std::string url = getUrl();
	if (url.empty())
		return;

	int nret = evdi_http_post(url.c_str(), strData.c_str(), outdata);
	printf("print_json == %s \n", strData.c_str());

	cJSON *args;
	args = cJSON_Parse(outdata);

	if (outdata)
	{
		free(outdata);
	}

}
 
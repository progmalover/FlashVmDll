#include "stdafx.h"
#include <stdio.h>
#include <thread>
#include <string>
#include <list>
#include "cJSON.h"
#include <list>
#include <mutex>   
#include <condition_variable>

class datapusher{
	class JsonHolder{
	public:
		JsonHolder(const char *data)
		{
			mJsonObject = cJSON_Parse(data);
		}
		
		cJSON *getPtr(){ return mJsonObject; };
		~JsonHolder()
		{
			if (NULL != mJsonObject)
				cJSON_Delete(mJsonObject);
		}
	private:
		cJSON *mJsonObject;
	};

	class portguard{
	public:
		portguard(int port = 81){
			mport = port;
			if (port == 3490)
				_3490Used = 1;
			if (port == 81)
				_81Used = 1;
		}
		~portguard(){
			
			if (mport == 3490)
				_3490Used = 0;
			if (mport == 81)
				_81Used = 0;
		}

		static volatile int  _81Used;
		static volatile int _3490Used;
	private:
		int mport;
	};

public:
	class messageQ{
		friend class datapusher;
	
	public:
		messageQ(int waitTime=2000);
		~messageQ();

		virtual void pushQ(std::string strData);
	protected:
		void clearQ();
		void insertQ(std::string strM);
		virtual void RunQ(void);
	private:
		std::list<std::string> mMsgBuffer;
		std::mutex mqtx;
		std::mutex metx;
		std::condition_variable mcv;
		std::thread *pQTrd;
		int mwaitTime;
	};

	class scrollmessageQ:public messageQ{
	public:
		scrollmessageQ() :messageQ(250)
		{}
		std::string getUrl();
		virtual std::string getUrlFormat(){
			return "http://%s:%d/request/flash/flash_move";
		};
		void pushQ(std::string strData);
	};

	class updatemessageQ :public scrollmessageQ{
	public:
		updatemessageQ() :scrollmessageQ()
		{}
		virtual std::string getUrlFormat(){ 
			return "http://%s:%d/request/flash/flash_update"; 
		};
	};
public:
	datapusher();
protected:
	void doPush(int port,int type);
	void InitWhiteList();
public: 
	bool push(const char * data);
	bool parseandstoreurl(const char *data);
	bool isMessageCanbePushed(const char *data ,int &type);
	bool isWiteListEnabled(std::string strUrl);
	std::string pushRequestStatus(const char *data);
	std::string getip();
	int getAvalaiblePort(){ return mAvalaiblePort; };
	static datapusher * getInstance();
	void showmsg();
private:
	std::string mtermip;	
	std::string mdata;
	std::string mcurposturl;
	int volatile mAvalaiblePort;
	std::list<std::string> mWhiteList;
	messageQ  mmsgQ;
	scrollmessageQ  scrlmmsgQ; 
	updatemessageQ  udmmsgQ;
	//std::string mrqdata;
};
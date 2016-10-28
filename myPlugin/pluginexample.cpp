
/*
author : wangchen
*/

#include "stdafx.h"
#include "pluginexample.h"
#include "cJSON.h"
#include "evdi-http-post.h"
#include "get_client_ip.h"
#include "datapusher.h"
#include "heartbeatreceiver.h"

void Reverse(char *s)
{
	for(int i = 0, j = (int)strlen(s) - 1; i < j; ++i, --j) {

		char c = s[i];

		s[i] = s[j];

		s[j] = c;
	}
}

class MyPluginexample :
	public IPluginModInterface
{
public:
	MyPluginexample()
	{
		m_resinterface = NULL;
	}
	string getnode_name(){return m_node_name;}
	string getservice_name(){return m_service_name;}
	int _install(const char *node_name);	
	int _uninstall(unsigned int uninstallmagickey);
	int _run(IResourceInterface *resinterface);
	
	static MyPluginexample *get_instance()
	{
		static MyPluginexample g_instance;
		return &g_instance;
	}

	static int URIFACE_CALL _buffer_free(char *resultbuf)
	{
		free(resultbuf);
		return 0;
	}

	static int URIFACE_CALL _res_provider(const char *sessionkey, const char *module, const char *method, char bremote, char brequest, 
		const char *parambuf, int len, /* out */char** resultbuf, /* out */int *retlen)
	{
		if (parambuf && len)
		{
			string str1;
			str1.assign(parambuf, len);
			Reverse((char *)str1.c_str());
			string computername;
			computername = get_instance()->getservice_name();			
			string str = "[";
			str += computername.c_str();
			str += "]";
			str += get_instance()->getnode_name();
			str += ":";
			str += str1;
			*retlen = (int)str.size()+1;
			*resultbuf = (char *)malloc(*retlen);
			memcpy(*resultbuf, str.c_str(), *retlen);
		}
		else
			return 101;
		return 0;
	}

	static int URIFACE_CALL _web_hander(const char *sessionkey, const char *module, const char *method, char bremote, char brequest, 
		const char *parambuf, int len, /* out */char** resultbuf, /* out */int *retlen)
	{
		string struri = method;
		if ("binary"==struri)
		{
			*retlen = 80;
			*resultbuf = (char *)malloc(*retlen);
			memcpy(*resultbuf, &module, *retlen);
			return 0;
		}
		string computername;
		computername = get_instance()->getservice_name();

		string str = "[";
		str += computername.c_str();
		str += "]";
		str += get_instance()->getnode_name();
		str += ":handle by pluginexample";
		*retlen = (int)str.size()+1;
		
		*resultbuf = (char *)malloc(*retlen);
		memcpy(*resultbuf, str.c_str(), *retlen);
		
		return 0;
	}

	 
	static int URIFACE_CALL _heartbeat(const char *sessionkey, const char *module, const char *method, char bremote, char brequest,
		const char *parambuf, int len, /* out */char** resultbuf, /* out */int *retlen)
	{
		*retlen = 0;
	 
		SYSTEMTIME time;
		::GetLocalTime(&time);
		TCHAR txt[50] = { 0 };
		wsprintf(txt, L"\n cur time hour:%d,min:%d,sec:%d:", time.wHour,time.wMinute,time.wSecond);

		::OutputDebugString(txt);

		heartbeatreceiver *inst = heartbeatreceiver::getInstance();
		if (inst->isDead())
		{
			inst->Start();
		}

		inst->Increment();

		return true;
	}

	static int URIFACE_CALL _get_status(const char *sessionkey, const char *module, const char *method, char bremote, char brequest,
		const char *parambuf, int len, /* out */char** resultbuf, /* out */int *retlen)
	{
		//::MessageBox(NULL, CA2W(parambuf),L"request",MB_OK);
		std::string strData;
		std::string retStr;

		heartbeatreceiver *inst = heartbeatreceiver::getInstance();
		if (!inst->isDead())
			inst->Increment();

		if (NULL != parambuf || strlen(parambuf) > 0)
		{
			strData = parambuf;

			::OutputDebugString(CA2W(("\n_get_status:" + strData).c_str()));
			datapusher &dpusher = *datapusher::getInstance();
			retStr = dpusher.pushRequestStatus(parambuf);

			if (retStr.empty())
				retStr = "{\"status\":false}";
			::OutputDebugString(CA2W(("\n_get_status ret:" + retStr).c_str()));

		}
		 
		*retlen = (int)retStr.size();
		*resultbuf = (char *)calloc(*retlen,sizeof(char));
		memcpy(*resultbuf, retStr.c_str(), retStr.size());
		
		return 0;
	}

	static int URIFACE_CALL _share_url(const char *sessionkey, const char *module, const char *method, char bremote, char brequest, 
		const char *parambuf, int len, /* out */char** resultbuf, /* out */int *retlen)
	{

		//string strlog = parambuf;
		//::OutputDebugString(CA2W(("\n get event:" + strlog).c_str()));
		heartbeatreceiver *inst = heartbeatreceiver::getInstance();
		if (!inst->isDead())
			inst->Increment();
		datapusher &dpusher = *datapusher::getInstance();
		dpusher.push(parambuf);
   

		string str("success");
		*retlen = (int)str.size();
		*resultbuf = (char *)calloc(*retlen,sizeof(char));
		memcpy(*resultbuf, str.c_str(), str.size());


		return 0;
	}

#ifdef _WIN32
	static CStringW CStrA2CStrW(const CStringA &cstrSrcA)
	{
		int len = MultiByteToWideChar(CP_ACP, 0, LPCSTR(cstrSrcA), -1, NULL, 0);
		wchar_t *wstr = new wchar_t[len];
		memset(wstr, 0, len*sizeof(wchar_t));
		MultiByteToWideChar(CP_ACP, 0, LPCSTR(cstrSrcA), -1, wstr, len);
		CStringW cstrDestW = wstr;
		delete[] wstr;

		return cstrDestW;
	}
	static void ExecuteExternalFile(char **buffer, int *len, CStringA csArguments)
	{
		SECURITY_ATTRIBUTES secattr; 
		ZeroMemory(&secattr,sizeof(secattr));
		secattr.nLength = sizeof(secattr);
		secattr.bInheritHandle = TRUE;

		HANDLE rPipe, wPipe;

		//Create pipes to write and read data
		CreatePipe(&rPipe,&wPipe,&secattr,0);
		//
		STARTUPINFOW sInfo; 
		ZeroMemory(&sInfo,sizeof(sInfo));
		PROCESS_INFORMATION pInfo; 
		ZeroMemory(&pInfo,sizeof(pInfo));
		sInfo.cb=sizeof(sInfo);
		sInfo.dwFlags=STARTF_USESTDHANDLES;
		sInfo.hStdInput=NULL; 
		sInfo.hStdOutput=wPipe; 
		sInfo.hStdError=wPipe;
		
		
		CStringW tmp;
		CStringW argd;
		argd = CStrA2CStrW(csArguments);
		tmp.Format(L"cmd /C %ws",argd.GetBuffer());
		

		//Create the process here.
		CreateProcessW(0,tmp.GetBuffer(),0,0,TRUE, NORMAL_PRIORITY_CLASS|CREATE_NO_WINDOW,0,0,&sInfo,&pInfo);
		CloseHandle(wPipe);

		//now read the output pipe here.
		char *buf = (char *)malloc(90000);
		DWORD reDword=0;
		int nres;
		while (1)
		{
			DWORD count = 0;
			nres = ::ReadFile(rPipe,buf+reDword,90000-reDword,&count,0);
			if (count==0)
			{
				break;
			}
			reDword += count;
		}
		
		*len = reDword;
		*buffer = buf;		
		return;
	}

	static int URIFACE_CALL _cmd_hander(const char *sessionkey, const char *module, const char *method, char bremote, char brequest, 
		const char *parambuf, int len, /* out */char** resultbuf, /* out */int *retlen)
	{
		if (!parambuf || !len)
		{
			return 101;
		}
		CStringA csArguments;
		csArguments.Append(parambuf, len);
		ExecuteExternalFile(resultbuf, retlen, csArguments);		
		return 0;
	}
#endif

private:
	// 资源接口
	IResourceInterface *m_resinterface;
	string m_node_name;
	string m_service_name;
};


int MyPluginexample::_install(const char *node_name)
{
	WriteWorkLogA("plugin example install");
	return 0;
}
int MyPluginexample::_uninstall(unsigned int uninstallmagickey)
{
	WriteWorkLogA("plugin example uninstall");
	return 0;
}
int MyPluginexample::_run(IResourceInterface *resinterface)
{
	m_resinterface = resinterface;

	char result[30] = {0};
	int retlen = 30;
	retlen = 30;
	m_resinterface->_get_node_name(result, &retlen);
	m_node_name = result;
	retlen = 30;
	m_resinterface->_get_service_name(result, &retlen);
	m_service_name = result;
	if (m_service_name.size()==0)
	{
		oshelper::getcomputername(m_service_name);
	}


	// 例子演示
	string modulename = "math1";
	if (!m_resinterface->_is_masternode())
	{
		if (m_node_name=="nodeuser")
		{
			modulename = "math2";
		}
		else
		{
			modulename = "math3";
		}
	}
	
	
	m_resinterface->_register(modulename.c_str(), "add", &MyPluginexample::_res_provider, &MyPluginexample::_buffer_free);
	m_resinterface->_register(modulename.c_str(), "web/web", &MyPluginexample::_web_hander, &MyPluginexample::_buffer_free);
	m_resinterface->_register(modulename.c_str(), "binary", &MyPluginexample::_web_hander, &MyPluginexample::_buffer_free);


	m_resinterface->_register("flash", "getstatus", &MyPluginexample::_get_status, &MyPluginexample::_buffer_free);

	m_resinterface->_register("flash", "share_url", &MyPluginexample::_share_url, &MyPluginexample::_buffer_free);
	///request/flash/update
	m_resinterface->_register("flash", "update", &MyPluginexample::_share_url, &MyPluginexample::_buffer_free);
	
	m_resinterface->_register("window", "info", &MyPluginexample::_share_url, &MyPluginexample::_buffer_free);
	//http://127.0.0.1:81/request/window/scroll
	m_resinterface->_register("window", "scroll", &MyPluginexample::_share_url, &MyPluginexample::_buffer_free);
	///request/window/heartbeat
	m_resinterface->_register("window", "heartbeat", &MyPluginexample::_heartbeat, &MyPluginexample::_buffer_free);

#ifdef _WIN32
	m_resinterface->_register(modulename.c_str(), "cmd", &MyPluginexample::_cmd_hander, &MyPluginexample::_buffer_free);
#endif

	// 注册http服务
	if (m_resinterface->_is_masternode())
	{
		// 只有master节点提供http服务
		m_resinterface->_register_http_handler("http", &MyPluginexample::_res_provider, &MyPluginexample::_buffer_free);
	}
	
	return 0;
}

PLUGINEXAMPLE_API int query(/* [in] */const char *interfacename, /* [out] */IPluginModInterface **plugininterface)
{
	int nRet = 0;
	if (!plugininterface || !interfacename)
	{
		return 1;
	}
	string facename = interfacename;
	if (facename == INTERFACE_BASENAME)
	{
		*plugininterface = MyPluginexample::get_instance();
	}
	else
	{
		nRet = 2;
		*plugininterface = 0;
	}	
	return nRet;
}

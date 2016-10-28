
#pragma once

#include <string>
using namespace std;

#ifdef _WIN32
#include <windows.h>
#include <atlstr.h>
#else
#include <dlfcn.h>
#include <unistd.h>
#endif

class oshelper
{
public:
	static void _setthreadname(unsigned int dwThreadId, const char* threadName)
	{
#ifdef _WIN32
#pragma pack(push,8)
		typedef struct tagTHREADNAME_INFO
		{
			DWORD dwType; // Must be 0x1000.
			LPCSTR szName; // Pointer to name (in user addr space).
			DWORD dwThreadID; // Thread ID (-1=caller thread).
			DWORD dwFlags; // Reserved for future use, must be zero.
		} THREADNAME_INFO;
#pragma pack(pop)

		THREADNAME_INFO info;
		info.dwType = 0x1000;
		info.szName = threadName;
		info.dwThreadID = dwThreadId;
		info.dwFlags = 0;

		__try
		{
			RaiseException(0x406D1388, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info);
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
		}
#else
#endif
	}
	template<typename A>
	static void setthreadname(A threadid, const char* threadName)
	{
#ifdef _DEBUG
		unsigned int dwThreadId;
		std::stringstream ss;
		ss << std::hex << threadid;
		ss >> dwThreadId;
		_setthreadname(dwThreadId, threadName);
#endif
	}

	static void *loadfuncaddr(string &modfullpath, const char *funcname)
	{
		void *funcaddr = 0;
#ifdef _WIN32
		HMODULE hLib = LoadLibraryA(modfullpath.c_str());
		if (!hLib)
		{
			WriteErrorLogA("loadlib %s error", modfullpath.c_str());
			return funcaddr;
		}
		
		(FARPROC &)funcaddr = GetProcAddress(hLib, funcname);
		if (!funcaddr)
		{
			FreeLibrary(hLib);
			WriteErrorLogA("getlib run proc %s error", modfullpath.c_str());
			return funcaddr;
		}
		return funcaddr;
#else

		void* dp = dlopen(modfullpath.c_str(), RTLD_NOW);	
		if (!dp)
		{
			WriteErrorLogA("loadlib %s error.", modfullpath.c_str());
			return funcaddr;
		}

		(void * &)funcaddr = dlsym(dp, funcname);
		if (!funcaddr)
		{
			dlclose(dp);
			WriteErrorLogA("get funcaddr %s error. %s", funcname, modfullpath.c_str());
			return funcaddr;
		}		
#endif
		return funcaddr;
	}

	static void getcomputername(string &computername)
	{
		// int gethostname(char *name, size_t len);
		computername.clear();
#ifdef _WIN32
		CStringA strComputerid;
		DWORD alen = 100;
		GetComputerNameA(strComputerid.GetBuffer(alen), &alen);
		strComputerid.ReleaseBuffer();
		computername = strComputerid.GetBuffer();
#else
		char buffer[100] = {0};
		gethostname(buffer, 100);
		computername = buffer;
#endif
	}
};

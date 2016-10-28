
/*
author : wangchen
*/

#pragma once

#ifdef _WIN32

#include <atlstr.h>


	/*

	ʹ�÷�����

	using namespace WriteLogLineEx;

	// ������־�ļ���exe����Ŀ¼
	WriteErrorLog(_T("code=%d"), GetLastError());
	WriteWorkLog(_T("runtime long"));

	// ������־�ļ���windowsĿ¼
	WriteErrorLogEx(_T("code=%d"), GetLastError());
	WriteWorkLogEx(_T("runtime long"));
	
	*/

#define _f_in WriteWorkLogA("--->" __FUNCTION__)
#define _f_out WriteWorkLogA("<---" __FUNCTION__)

	#ifdef UNICODE
		#define WriteErrorLog	WriteErrorLogW
		#define WriteErrorLogEx	WriteErrorLogExW
		#define WriteWorkLog	WriteWorkLogW
		#define WriteWorkLogEx	WriteWorkLogExW
	#else
		#define WriteErrorLog	WriteErrorLogA
		#define WriteErrorLogEx	WriteErrorLogExA
		#define WriteWorkLog	WriteWorkLogA
		#define WriteWorkLogEx	WriteWorkLogExA
	#endif	

	class MyLogHelper
	{
	public:
		static int RealWriteLog(const char *SrcFile, int SrcLine, char *LogStr, int Mode, int bError)
		{
#ifdef NO_WRITE_LOG
			return 0;
#endif

			CStringW FileName;
			CStringW exeproc;
			if (Mode == 1)
			{
				// ��ȡϵͳWindowsĿ¼	
				GetWindowsDirectoryW(FileName.GetBuffer(600), 600);
				FileName.ReleaseBuffer();
			}
			else
			{
				// ��ȡӦ�ó����Ŀ¼	
				GetModuleFileNameW(NULL, FileName.GetBuffer(600), 600);
				FileName.ReleaseBuffer();
				int nPos = FileName.ReverseFind(L'\\');				
				exeproc = FileName.Right(FileName.GetLength()-nPos-1);
				FileName.SetAt(nPos, 0);
				FileName.ReleaseBuffer();
			}
			exeproc.AppendFormat(L":%d", GetCurrentProcessId());
			FileName.TrimRight(L'\\');
			FileName += L"\\log.txt";

			HANDLE hFile = INVALID_HANDLE_VALUE;
			DWORD errorcode = ERROR_SHARING_VIOLATION;
			while (hFile == INVALID_HANDLE_VALUE && errorcode==ERROR_SHARING_VIOLATION)
			{
				// ��ռ����
				hFile = CreateFileW(FileName, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, 
					OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (hFile==INVALID_HANDLE_VALUE)
				{
					errorcode = GetLastError();
				}
			}

			if (hFile==INVALID_HANDLE_VALUE)
			{
				return 0;
			}


			LARGE_INTEGER liSize = {0};
			LARGE_INTEGER offset = {0};
			liSize.LowPart = GetFileSize(hFile, (DWORD *)&liSize.HighPart);	
			if (liSize.LowPart > 1024 * 1024 * 4)
			{
				// ����һ����֮ǰ��־
				DWORD savelen = 512*1024;
				offset.QuadPart = liSize.QuadPart-savelen;
				char *SaveLogBuf = (char *)malloc(savelen);
				SetFilePointer(hFile, offset.LowPart, &offset.HighPart, FILE_BEGIN);
				ReadFile(hFile, SaveLogBuf, savelen, &savelen, NULL);
				DWORD i = 0;
				while (SaveLogBuf[i] != 0x0D && i<savelen-1)
				{
					i++;
				}

				if (i<savelen-2)
				{
					// ����0x0A
					i += 2;
				}

				offset.QuadPart = 0;
				SetFilePointer(hFile, offset.LowPart, &offset.HighPart, FILE_BEGIN);
				SetEndOfFile(hFile);
				WriteFile(hFile, SaveLogBuf+i, savelen-i, &savelen, NULL);
				free(SaveLogBuf);
			}
			offset.QuadPart = 0;
			SetFilePointer(hFile, offset.LowPart, &offset.HighPart, FILE_END);

			// ��ȡ��ǰʱ��
			SYSTEMTIME SysTime; 
			CStringA CurrDateTime;
			GetLocalTime(&SysTime); 
			CurrDateTime.Format("%4d/%02d/%02d %02d:%02d:%02d", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute, SysTime.wSecond);

			CStringA writestr;
			writestr.Format("%s(%d) :[%ws]", SrcFile, SrcLine, exeproc.GetBuffer());
			if (bError)
			{
				writestr += "[ERROR] ";
			}
			writestr += LogStr;
			writestr.AppendFormat(" <%s>\x0D\x0A\x0", CurrDateTime);
			// д��־���ļ�
			DWORD writelen = 0;
			WriteFile(hFile, writestr.GetBuffer(), writestr.GetLength(), &writelen, NULL);
			CloseHandle(hFile);
			return 0;
		}

		static char *UnicodeToMB(CStringW &str)
		{
			int needlen = WideCharToMultiByte(CP_ACP, 0, str, str.GetLength(), NULL, 0, NULL, NULL);
			needlen += 4;
			char *buf = (char *)malloc(needlen);
			if (buf)
			{
				memset(buf, 0, needlen);
				WideCharToMultiByte(CP_ACP, 0, str, str.GetLength(), buf, needlen, NULL, NULL);
			}
			return buf;
		}
	protected:
	private:
	};


	// д������־���ļ����÷���printfһ������־�ļ����浽EXEĿ¼
	#define WriteErrorLogA(...) \
	{ \
		CStringA LogStr_WriteLogLineEx; \
		LogStr_WriteLogLineEx.Format(__VA_ARGS__); \
		MyLogHelper::RealWriteLog(__FILE__, __LINE__, LogStr_WriteLogLineEx.GetBuffer(), 0, 1); \
	}

	#define WriteErrorLogW(...) \
	{ \
		CStringW LogStr_WriteLogLineEx; \
		LogStr_WriteLogLineEx.Format(__VA_ARGS__); \
		char * aLogStr_WriteLogLineEx = MyLogHelper::UnicodeToMB(LogStr_WriteLogLineEx); \
		MyLogHelper::RealWriteLog(__FILE__, __LINE__, aLogStr_WriteLogLineEx, 0, 1); \
		free(aLogStr_WriteLogLineEx); \
	}
	
	// д������־���ļ����÷���printfһ������־�ļ����浽WindowsĿ¼
	#define WriteErrorLogExA(...) \
	{ \
		CStringA LogStr_WriteLogLineEx; \
		LogStr_WriteLogLineEx.Format(__VA_ARGS__); \
		MyLogHelper::RealWriteLog(__FILE__, __LINE__, LogStr_WriteLogLineEx.GetBuffer(), 1, 1); \
	}

	#define WriteErrorLogExW(...) \
	{ \
		CStringW LogStr_WriteLogLineEx; \
		LogStr_WriteLogLineEx.Format(__VA_ARGS__); \
		char * aLogStr_WriteLogLineEx = MyLogHelper::UnicodeToMB(LogStr_WriteLogLineEx); \
		MyLogHelper::RealWriteLog(__FILE__, __LINE__, aLogStr_WriteLogLineEx, 1, 1); \
		free(aLogStr_WriteLogLineEx); \
	}

	// д������־���ļ����÷���printfһ������־�ļ����浽EXEĿ¼
	#define WriteWorkLogA(...) \
	{ \
		CStringA LogStr_WriteLogLineEx; \
		LogStr_WriteLogLineEx.Format(__VA_ARGS__); \
		MyLogHelper::RealWriteLog(__FILE__, __LINE__, LogStr_WriteLogLineEx.GetBuffer(), 0, 0); \
	}

	#define WriteWorkLogW(...) \
	{ \
		CStringW LogStr_WriteLogLineEx; \
		LogStr_WriteLogLineEx.Format(__VA_ARGS__); \
		char * aLogStr_WriteLogLineEx = MyLogHelper::UnicodeToMB(LogStr_WriteLogLineEx); \
		MyLogHelper::RealWriteLog(__FILE__, __LINE__, aLogStr_WriteLogLineEx, 0, 0); \
		free(aLogStr_WriteLogLineEx); \
	}

	// д������־���ļ����÷���printfһ������־�ļ����浽WindowsĿ¼
	#define WriteWorkLogExA(...) \
	{ \
		CStringA LogStr_WriteLogLineEx; \
		LogStr_WriteLogLineEx.Format(__VA_ARGS__); \
		MyLogHelper::RealWriteLog(__FILE__, __LINE__, LogStr_WriteLogLineEx.GetBuffer(), 1, 0); \
	}

	#define WriteWorkLogExW(...) \
	{ \
		CStringW LogStr_WriteLogLineEx; \
		LogStr_WriteLogLineEx.Format(__VA_ARGS__); \
		char * aLogStr_WriteLogLineEx = MyLogHelper::UnicodeToMB(LogStr_WriteLogLineEx); \
		MyLogHelper::RealWriteLog(__FILE__, __LINE__, aLogStr_WriteLogLineEx, 1, 0); \
		free(aLogStr_WriteLogLineEx); \
	}

#else

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>

class MyLogHelper
{
public:
	static char *getlinelog(const char* format, ...)
	{
		va_list arg;
		va_start(arg, format);
		int len = 512;
		char *buffer = (char*)malloc(len);
		vsnprintf(buffer, len, format, arg);
		va_end(arg);
		return buffer;
	}
	static void printlog(const char *SrcFile, int SrcLine, char* logstr)
	{
		// todo,֧���ļ���־����¼��ǰʱ��
		time_t now;
		struct tm *timenow;
		time(&now);
		timenow = localtime(&now);
		printf("%s(%d):[%d] %s :%s", SrcFile, SrcLine, getpid(), logstr, asctime(timenow));
	}
};
/*
#define debug(format, ...) fprintf(stderr, format, ## __VA_ARGS__)
#define debug(format, ...) fprintf(stderr, format, __VA_ARGS__)
*/	

#define WriteErrorLogA(...)  \
{\
	char *LogStr_WriteLogLineEx = MyLogHelper::getlinelog(__VA_ARGS__); \
	MyLogHelper::printlog(__FILE__, __LINE__, LogStr_WriteLogLineEx); \
	free(LogStr_WriteLogLineEx); \
}

#define WriteWorkLogA(...)  \
{\
	char *LogStr_WriteLogLineEx = MyLogHelper::getlinelog(__VA_ARGS__); \
	MyLogHelper::printlog(__FILE__, __LINE__, LogStr_WriteLogLineEx); \
	free(LogStr_WriteLogLineEx); \
}

#endif

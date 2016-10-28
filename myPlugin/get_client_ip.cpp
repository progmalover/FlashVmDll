#include "StdAfx.h"
#include "get_client_ip.h"


#include <Windows.h>
#include "cJSON.h"
#include <WinDef.h>
#include <WinIoCtl.h>

// auoto_setup.cpp : Defines the entry point for the console application.
//
// testTravseDir.cpp : Defines the entry point for the console application.
//



#include <string>

using namespace std;

#define PIPE_MAX_BUFFER 4096
#ifndef _DEBUG
//#define PIPE_NAME  _T("\\\\.\\pipe\\GuestToolSvr")
#else
//#define PIPE_NAME  _T("\\\\.\\Global\\com.morningcloud.tcloud")
//#define PIPE_NAME  _T("\\\\.\\Global\\com.morningcloud.tcloud.multimedia")
#define PIPE_NAME  _T("\\\\.\\d:")
#endif

#define NT_DEVICE_NAME          L"\\Device\\Ndisprot"
#define DOS_DEVICE_NAME         L"\\Global??\\Ndisprot"

#define RING3_DOS_DEVICE_NAME         L"\\\\.\\Ndisprot"

#define PIPE_NAME				RING3_DOS_DEVICE_NAME

#define FSCTL_NDISPROT_BASE      FILE_DEVICE_NETWORK

#define _NDISPROT_CTL_CODE(_Function, _Method, _Access)  \
	CTL_CODE(FSCTL_NDISPROT_BASE, _Function, _Method, _Access)

#define IOCTL_NDISPROT_OPEN_DEVICE   \
	_NDISPROT_CTL_CODE(0x200, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_NDISPROT_QUERY_OID_VALUE   \
	_NDISPROT_CTL_CODE(0x201, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_NDISPROT_SET_OID_VALUE   \
	_NDISPROT_CTL_CODE(0x205, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_NDISPROT_QUERY_BINDING   \
	_NDISPROT_CTL_CODE(0x203, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_NDISPROT_BIND_WAIT   \
	_NDISPROT_CTL_CODE(0x204, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_NDISPROT_GET_CLIENT_IP   \
	_NDISPROT_CTL_CODE(0x800, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)


#define MYTRACE

static HANDLE g_hPipe = INVALID_HANDLE_VALUE;
static HANDLE g_hEvent = NULL;
static BOOL g_bExit = FALSE;
//功能执行是否成功计数
static int g_nSuccessCount = 0;

//打开管道
BOOL OpenPipe()
{
	static BOOL bLog = FALSE;
	g_hPipe = CreateFile(PIPE_NAME, 
		GENERIC_READ|GENERIC_WRITE ,
		0,
		NULL,
		OPEN_EXISTING,
		/*FILE_FLAG_OVERLAPPED*/FILE_ATTRIBUTE_SYSTEM,
		NULL);

	if(g_hPipe == INVALID_HANDLE_VALUE)
	{
		printf(" open pipe failed \n");

		DWORD dwErr = GetLastError();
		printf("error == %d",dwErr);
		if(!bLog) {
			bLog = TRUE;
			MYTRACE(_T("打开设备%s失败:%d"), PIPE_NAME, GetLastError());
		}
		return FALSE;
	}
	MYTRACE(_T("打开设备%s成功"), PIPE_NAME);
	return TRUE;
}

bool get_client_ip_from_buffer(char* json_data, char * client_ip)
{

	bool bret = false;

	cJSON *args;
	args = cJSON_Parse(json_data);

	if ( !args ) 
	{
		printf(" error == null");
	}
	else
	{
		printf("args ret == %s \n", cJSON_Print(args));

		cJSON *result_json = NULL;
		result_json = cJSON_GetObjectItem(args, "result");

		if ( NULL == result_json )
		{
			printf(" NULL == result_json ");
		}
		else
		{
			printf(" NULL != result_json ");

			if ( cJSON_GetObjectItem(result_json, "client_ip") )
			{
				printf(" client_ip == %s ", cJSON_GetObjectItem(result_json, "client_ip")->valuestring);
				sprintf(client_ip, cJSON_GetObjectItem(result_json, "client_ip")->valuestring);
				bret = true;
			}
			else
			{
				printf(" client_ip == null");
			}
		}
	}

	return bret;
}

//读取命令string
int PipeReadCmdString(char* szBuffer, int nLen)
{
	if(g_hPipe == INVALID_HANDLE_VALUE) return -1;

	int nCount = 0;
	int nSize = 0;
	BOOL bGetData = FALSE;
	int i = 0;

	int nRet = 0;
	char szTemp[1] = {};
	DWORD nRead = 0;
	BOOL bHeadFind = FALSE;
	OVERLAPPED ol = {};
	ol.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	while (TRUE)
	{
		if ( i != 0 )
		{
			::ReadFile(g_hPipe, szBuffer, nCount, (LPDWORD)&nRead, &ol);
		}
		else
		{
			::ReadFile(g_hPipe, &nSize, sizeof(int), (LPDWORD)&nRead, &ol);
		}

		//::ReadFile(g_hPipe, szTemp, 1, &nRead, &ol);

		DWORD dwRet = WaitForSingleObject(ol.hEvent, 5000);
		switch(dwRet)
		{
		case WAIT_TIMEOUT:
			{
				//MYTRACE(_T("读取设备5s超时"));
				printf(" read time out \n");
				CloseHandle(ol.hEvent);
				return -2;
			}
			break;
		case WAIT_OBJECT_0:
			{
				printf("get data WAIT_OBJECT_0 \n");
				if(!GetOverlappedResult(g_hPipe, &ol, &nRead, FALSE))
				{
					MYTRACE(_T("GetOverlappedResult failed:%d"), GetLastError());
					CloseHandle(ol.hEvent);
					//return -1;
					//AfxThrowUserException();
				}
				else
				{
					if ( i == 0 )
					{
						printf("get data == %d \n", nSize);
						nCount = nSize;
					}
					else
					{
						printf("get all data  == %s \n", szBuffer );

						char client_ip[20] = { 0 };
						get_client_ip_from_buffer(szBuffer, client_ip);
						return 0;
					}


					bGetData = TRUE;
					++i;
				}


			}
			break;
		default:
			{
				MYTRACE(_T("WaitForsingleObject failed:%d"), GetLastError());
				CloseHandle(ol.hEvent);
				return -1;
			}
			break;
		}
	}

	MYTRACE(_T("读取数据：%s"), szBuffer);
	CloseHandle(ol.hEvent);
	return nRet;
}

BOOL PipeWriteCmdString(LPCSTR szCmd)
{
	if(g_hPipe == INVALID_HANDLE_VALUE)
		return FALSE;

	char writedata[2048] = { 0 };

	*((int*)writedata) = strlen(szCmd);

	memcpy(writedata + 4, szCmd, strlen(szCmd));

	DWORD dwNumberOfBytesTransfer = 0;
	OVERLAPPED ol = {};
	ol.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	WriteFile(g_hPipe, writedata, 4 + strlen(szCmd), &dwNumberOfBytesTransfer, &ol);
	BOOL bRet = TRUE;
	if(!GetOverlappedResult(g_hPipe, &ol, &dwNumberOfBytesTransfer, TRUE))
	{
		printf(" write err %d \n", GetLastError());
		bRet = FALSE;
	}
	CloseHandle(ol.hEvent);
	return bRet;
}


char * get_string_by_index( int nIndex, char *str, char* subStr)
{
	int i = 0;
	int subBegin = 0;
	int subEnd = 0;
	int nCountFindTag = 0;
	BOOL bFind = FALSE;

	if ( NULL == str || str[0] != '#' || nIndex <= 0 || NULL == subStr)
	{
		return NULL;
	}

	for (  i = 0; i < (int)strlen(str); ++i )
	{
		if ( str[i] == '#' )
		{
			nCountFindTag++;

			if ( nCountFindTag == nIndex )
			{
				subBegin = i;
			}

			if ( nCountFindTag == nIndex + 1)
			{
				subEnd = i;

				bFind = TRUE;
				break;
			}
		}
	}

	if ( bFind)
	{
		strncpy(subStr, str + subBegin + 1, subEnd - subBegin - 1);

		if ( 0 != _stricmp(subStr, "None") && 
			0 != _stricmp(subStr, "Null"))
		{
			return subStr;
		}
	}

	return NULL;
}


bool get_client_ip(char *client_ip)
{
	char ip_data[512] = { 0 };
	DWORD dwRet = 0;
	DWORD dwRead = 512;
	OpenPipe();
	//ReadFile(g_hPipe, ip_data, dwRead, &dwRet, NULL);


	bool bret = DeviceIoControl(g_hPipe, IOCTL_NDISPROT_GET_CLIENT_IP, client_ip, 50, client_ip, 50, &dwRet, NULL);

	printf("lasterror == %d", GetLastError());

	//printf("ip_data == %s", ip_data);

	//get_string_by_index(10, ip_data, client_ip);
	printf("client_ip == %s\n", client_ip);

	if ( INVALID_HANDLE_VALUE != g_hPipe )
	{
		CloseHandle( g_hPipe);
	}

	printf("exit");


	return bret;
}

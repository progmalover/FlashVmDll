
/*
author : wangchen
*/

#pragma once

#ifdef _WIN32
#include <Windows.h>
#endif
#include <string>
using namespace std;

#include "autobuffer.h"

class cmypath
{
public:
	static int getrunpath(string &runpath)
	{
		// 以 '/' 结束
		cautobuffer mybuf(400);
		char pathtag = 0;
		int length = 0;
		// 获取应用程序根目录
#ifdef _WIN32
		DWORD ret = GetModuleFileNameA(NULL, mybuf, mybuf.getlength());
		if (0==ret)
		{
			return 1;
		}
		length = (int)ret;
		pathtag = '\\';
#else
		length = readlink("/proc/self/exe", mybuf, mybuf.getlength());
		if (length<=0)
		{
			return 1;
		}
		pathtag = '/';
#endif
		runpath.assign(mybuf.getbuffer(), length);
		string::size_type pos = runpath.rfind(pathtag);
		runpath = runpath.substr(0, pos+1);
		return 0;
	}

};

/*
author : wangchen
*/

#pragma once

#include "../writeloglineex.h"
#include "cJSON.h"
#include <stdio.h>

class filejsonop
{
public:
	static cJSON *getobjectfromfile(const string &jsonfile)
	{
		cJSON *json = (cJSON *)0;
		FILE *pFile;
		pFile = fopen(jsonfile.c_str(), "rb");
		if (pFile==NULL)
		{
			WriteErrorLogA("load json error %s", jsonfile.c_str());
			return json;
		}
		string jsonconfig;
		while (!feof(pFile))
		{
			size_t count;
			char buf[512] = {0};
			count = fread(buf, 1, sizeof(buf), pFile);
			if (count==0)
			{
				break;
			}
			jsonconfig.append(buf, count);
		}
		fclose(pFile);
		
		json = cJSON_Parse(jsonconfig.c_str());		
		return json;
	}
	static int saveobjecttofile(cJSON *json, const string &jsonfile)
	{
		char *jsonout = cJSON_Print(json);
		string strjsonout = jsonout;
		free(jsonout);
		
		FILE *pFile;
		pFile = fopen(jsonfile.c_str(), "wb");
		if (pFile==NULL)
		{
			WriteErrorLogA("could'nt create %s", jsonfile.c_str());
			return 1;
		}
		fwrite(strjsonout.c_str(), 1, strjsonout.size(), pFile);		
		fclose(pFile);
		return 0;
	}

};
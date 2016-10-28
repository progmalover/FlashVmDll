
/*
author : wangchen
*/

#pragma once
#include <string>
#include <vector>
using namespace std;

class cmystring
{
public:
	static void trimuri(string &uri)
	{
		uri.erase(0, uri.find_first_not_of("/\\"));
		uri.erase(uri.find_last_not_of("/\\") + 1);
	}
	
	static int split(const string &str, const string &pattern, vector<string> &result)
	{
		string::size_type pos;
		string::size_type size=str.size();		
		for(string::size_type i=str.find_first_not_of(pattern, 0); i<size; i++)
		{
			pos=str.find(pattern,i);
			if (pos==string::npos)
			{
				result.push_back(str.substr(i));
				break;
			}
			if(pos<size)
			{
				result.push_back(str.substr(i,pos-i));
				i=pos+pattern.size()-1;
			}
		}
		return 0;
	}

	static int strcmp(const char *s, const char *t)
	{
		while(*s && *t && *s == *t)
		{
			++s;
			++t;
		}
		return (*s - *t);
	}

protected:
private:
};
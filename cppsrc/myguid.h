
#pragma once

#ifdef _WIN32
#include <windows.h>
#else
//guid
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#endif

class myguid
{
public:
	#ifndef _WIN32
	#define sprintf_s snprintf
	#endif

	static string getguid()
	{
		string str;
#ifdef _WIN32		
		GUID guid;
		CoCreateGuid(&guid);
		char buffer[150] = {0};
		sprintf_s(buffer, sizeof(buffer), "%08lx%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x", 
			guid.Data1, guid.Data2, guid.Data3, 
			guid.Data4[0], guid.Data4[1], guid.Data4[2],
			guid.Data4[3], guid.Data4[4], guid.Data4[5],
			guid.Data4[6], guid.Data4[7]);
		str = buffer;
		
#else
		/*
		uuid_t uuid;
		char str[37] = {0};

		uuid_generate(uuid);
		uuid_unparse(uuid, str);
		*/
		boost::uuids::uuid u;
		boost::uuids::random_generator rgen;  
		u = rgen(); 
		str = boost::uuids::to_string(u);

#endif
		return str;
	}
protected:
private:
};

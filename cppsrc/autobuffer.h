
/*
author : wangchen
*/
#pragma once

#include <stdlib.h>
#include <string.h>
/*
	自动buffer
*/
class cautobuffer
{
public:
	explicit cautobuffer(int size)
	{
		m_bufferbytesize = size;
		m_pbuffer = (char *)malloc(size);
		memset(m_pbuffer, 0, size);
	}
	~cautobuffer()
	{
		free(m_pbuffer);
		m_pbuffer = NULL;
		m_bufferbytesize = 0;
	}
	int getlength()
	{
		return m_bufferbytesize;
	}
	char *getbuffer()
	{
		return m_pbuffer;
	}

	operator char *()  
	{
		return m_pbuffer;
	} 

	operator unsigned char *()  
	{
		return (unsigned char *)m_pbuffer;  
	}

	operator wchar_t *()  
	{
		return (wchar_t *)m_pbuffer;
	}
	
	char * operator +(unsigned int offset)  
	{
		return m_pbuffer+offset;
	}

	/*
	// 赋值转换
	CAutoBuffer& operator =(const char *&pbuf)  
	{
		m_pbuffer = pbuf;  
		return *this;  
	}
	*/
protected:
private:
	char *m_pbuffer;
	int m_bufferbytesize;
};

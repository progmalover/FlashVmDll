/***************************************************************************
 *            evdi-http-post.h
 *
 *  Copyright  2014  os-easy
 *  <user@host>

****************************************************************************/

#ifndef _EVDI_HTTP_POST_H_
#define _EVDI_HTTP_POST_H_

#define TIMEOUT_SECOND 20   //http超时时间
#define RETRY_CONNECT  1    //http重连次数

#ifdef _WIN32
#pragma comment(lib, "libcurl.lib")
#endif

//#include "evdi-common.h"

int evdi_http_get(const char *url,char *out_data);  //has not use

int evdi_http_post(const char *url, const char *post_data, char *out_data ,int timeout = 2000); //timeout:ms

#endif

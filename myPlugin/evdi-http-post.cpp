//#include "evdi-common.h"
#include "stdafx.h"
#include "evdi-http-post.h"
#include <curl/curl.h>


static size_t write_data( void *ptr, size_t size, size_t nmemb, void *userdata)
{
    strcat((char*)userdata, (char*)ptr);
    return size*nmemb;
}

int evdi_http_post(const char *url, const char *post_data, char *out_data, int timeout )
{
	
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    struct curl_slist *headers=NULL; 
    headers = curl_slist_append(headers, "version: 1");
    #ifdef USE_CH
    headers = curl_slist_append(headers, "accept-language: cn");
    #endif
    #ifdef USE_EN
    headers = curl_slist_append(headers, "accept-language: en");
    #endif
    #ifdef USE_KR
    headers = curl_slist_append(headers, "accept-language: kr");
    #endif

    headers = curl_slist_append(headers, "content-type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, out_data);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout);     //设置超时时间
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);     //是否将发送内容显示
    curl_easy_setopt(curl, CURLOPT_HEADER, 0);    //返回的数据格式中加上头部
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);//允许被抓到的链接跳转
    //CURLOPT_COOKIEFILE: 验证该次POST是否成功的原因
    //curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "/home/hahaya/curlposttest.cookie");

    //增加http重连次数
    int i;
    for (i = 0; i < RETRY_CONNECT; ++i)
    {
        res = curl_easy_perform(curl);
        if (CURLE_OK == res)
            break;
    }

	if ( CURLE_OK != res )
	{
		printf(" url == %s  error \n ", url);
	}

    curl_slist_free_all(headers); 
    curl_easy_cleanup(curl);

	return res;
}

int evdi_http_get(const char *url,char *out_data)
{
	
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, out_data);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIMEOUT_SECOND);     //设置超时时间
    curl_easy_setopt(curl, CURLOPT_HEADER, 0);    //返回的数据格式中加上头部
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);//允许被抓到的链接跳转

    //增加http重连次数
    int i;
    for (i = 0; i < RETRY_CONNECT; ++i)
    {
        res = curl_easy_perform(curl);
        if (CURLE_OK == res)
            break;
    }
    curl_easy_cleanup(curl);

	return res;

	return 0;
}

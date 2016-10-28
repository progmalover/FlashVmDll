// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the PLUGINEXAMPLE_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// PLUGINEXAMPLE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef _WIN32

#ifdef PLUGINEXAMPLE_EXPORTS
#define PLUGINEXAMPLE_API __declspec(dllexport)
#else
#define PLUGINEXAMPLE_API __declspec(dllimport)
#endif

#else

#define PLUGINEXAMPLE_API __attribute__((dllexport))

#endif

extern "C"
{
	// 获得插件的操作接口
	PLUGINEXAMPLE_API int query(/* [in] */const char *interfacename, /* [out] */IPluginModInterface **plugininterface);	
};


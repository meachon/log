#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <queue>
#include <thread>


#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _WIN32
#ifdef LIBSLOG_EXPORTS
	#pragma message("#export liblog.dll function interface")
	#define SLOG_API __declspec(dllexport)
#else
	#pragma message("#import liblog.dll function interface")
	#define SLOG_API __declspec(dllimport)
#endif
#elif __linux__
#define SLOG_API
#endif


//log level
#define DEBUG_LEVEL  1
#define INFO_LEVEL   2
#define WARN_LEVEL   3
#define ERROR_LEVEL  4
#define CRITIC_LEVEL 5

void SLOG_API initlog(const char *dir, int file_max_size_mb, int log_level, int log_file_count, bool sync_write_flag, bool console_write_flag, bool file_write_flag);
void SLOG_API writelog(unsigned int loglevel, char* file, const char* function, unsigned int line, const char *fmt, ...);
void SLOG_API stoplog();

//#define NOLOG

#ifndef NOLOG
#define LOG(loglevel, fmt,...) \
	writelog(loglevel,__FILE__, (char*)__FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define LOG(args...)
#endif


#ifdef __cplusplus
}
#endif


#endif // LOG_H

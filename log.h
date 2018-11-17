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


//log level
#define DEBUG_LEVEL  1
#define INFO_LEVEL   2
#define WARN_LEVEL   3
#define ERROR_LEVEL  4
#define CRITIC_LEVEL 5

struct LOG_ST {
	std::string				logLevel;
	std::string				srcFile;
	std::string				funcName;
	int								lineNum;
	std::string				logContent;
	std::string				logTime;
	int								pid;
	std::thread::id   tid;
};


class CLog
{
public:
	CLog();
	~CLog();

	int setLogDirName(std::string dir);
	int setLogMaxSize(int size);
	int setLogLevel(int level);
	int setLogFileCount(int count);
	int setSyncWriteLog(bool syncFlag = false);

	int start();
	int stop();
	bool isStarted();

	void write(unsigned int loglevel, char* file,char* funtion, unsigned int line, const char *fmt, ...);
	void writeLog();
	void writeLogSync(struct LOG_ST log_element);


private:
	int checkLogFileSize();
	void deleteOldLogFiles(std::string cate_dir);

	std::ofstream m_streamLog;

	std::string   m_logfilename;
	std::string   m_logDirName;
	unsigned int  m_logMaxSize;
	unsigned int  m_logLevel;
	bool          m_syncWriteLogFlag;
	unsigned int  m_logFileCount;

	std::thread*  m_pthreadWrite;
	//std::thread  m_threadWrite;
	bool   m_threadStartFlag;

	std::map<int,std::string> m_loglevel;
	std::queue<LOG_ST>        m_queue;
};

extern CLog runlog;


//#define NOLOG

#ifndef NOLOG
#define LOG(loglevel, fmt,...) \
	runlog.write(loglevel,__FILE__, (char*)__FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define LOG(args...)
#endif



#endif // LOG_H

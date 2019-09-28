#include <stdio.h>
#include "log.h"
#include <mutex>
#include <condition_variable>
#include <thread>
#include <time.h>
#include <stdlib.h>
#include <iostream>

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#include <windows.h>
#include <Algorithm>
#include <process.h>
#elif __linux__
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdarg.h>
#include <algorithm>
#include <dirent.h>
#include <string.h>
#endif

std::mutex g_queue_mutex;
std::mutex g_log_console_mutex;


struct LOG_ST {
	std::string				logLevel;
	std::string				srcFile;
	std::string				funcName;
	int						lineNum;
	std::string				logContent;
	std::string				logTime;
	int						pid;
	std::thread::id			tid;
};


class CLog
{
public:
	CLog();
	~CLog();

	int setLogDirName( const std::string& dir);
	int setLogMaxSize(int size);
	int setLogLevel(int level);
	int setLogFileCount(int count);
	int setSyncWriteLog(bool syncFlag = false);
	int setConsoleWrite(bool consoleFlag = false);
	int setFileWrite(bool fileFlag = true);

	int start();
	int stop();
	bool isStarted();

	void write(unsigned int loglevel, char* file, const char* function, unsigned int line, const char *logContent);

private:
	void writeLog();
	void writeLogSync(struct LOG_ST log_element);
	int checkLogFileSize();
	void deleteOldLogFiles(const std::string& cate_dir);

	std::ofstream m_streamLog;

	std::string   m_logfilename;
	std::string   m_logDirName;
	unsigned int  m_logMaxSize;
	unsigned int  m_logLevel;
	bool          m_syncWriteLogFlag;
	unsigned int  m_logFileCount;
	bool          m_consoleWriteFlag;
	bool          m_fileWriteFlag;

	std::thread*  m_pthreadWrite;
	bool   m_threadStartFlag;

	std::map<int,std::string> m_loglevel;
	std::queue<LOG_ST>        m_queue;
};

CLog runlog;


CLog::CLog():m_logfilename("runlog.txt"),m_logDirName("./")
{
#ifdef _WIN32
	m_loglevel[1] = "DEBUG ";
	m_loglevel[2] = "INFO  ";
	m_loglevel[3] = "WARN  ";
	m_loglevel[4] = "ERROR ";
	m_loglevel[5] = "CRITIC";
#else
	m_loglevel[1] = "\e[0;32m DEBUG  \e[0m";
	m_loglevel[2] = "\e[1;32m INFO   \e[0m";
	m_loglevel[3] = "\e[1;33m WARN   \e[0m";
	m_loglevel[4] = "\e[0;31m ERROR  \e[0m";
	m_loglevel[5] = "\e[1;31m CRITIC \e[0m";
#endif
	m_logMaxSize = 10;
	m_logLevel = ERROR_LEVEL;
	m_logFileCount = 10;
	m_syncWriteLogFlag = false;
	m_threadStartFlag = false;
	m_consoleWriteFlag = false;
	m_fileWriteFlag = true;
	m_pthreadWrite = NULL;
}

CLog::~CLog()
{
	if ((isStarted()) && (NULL != m_pthreadWrite))
	{
		m_threadStartFlag = false;
		//m_pthreadWrite->join();
		//delete m_pthreadWrite;
		//m_pthreadWrite = NULL;
	}
}

int CLog::setSyncWriteLog(bool syncFlag)
{
	m_syncWriteLogFlag = syncFlag;
	return 0;
}

int CLog::setLogDirName(const std::string& dir)
{
	m_logDirName = dir;
#ifdef _WIN32
	if (-1 == _access(m_logDirName.c_str(), 0))
	{
		_mkdir(m_logDirName.c_str());
	}
#elif __linux__
	if (-1 == access(m_logDirName.c_str(), 0))
	{
		mkdir(m_logDirName.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
	}
#endif

	return 0;
}

int CLog::setLogMaxSize(int size_mb)
{
	if ((size_mb >= 1) && (size_mb < 100))
	{
		m_logMaxSize = size_mb;
	}

	return 0;
}

int CLog::setLogLevel(int level)
{
	if ((level >= 1) && (level < 5))
	{
		m_logLevel = level;
	}

	return 0;
}

int CLog::start()
{

	if (m_fileWriteFlag)
	{
		m_threadStartFlag = true;
		m_pthreadWrite = new std::thread(&CLog::writeLog, this);
	}

	return 0;
}

int CLog::stop()
{
	if (!m_fileWriteFlag)
		return 0;

	if ((m_fileWriteFlag)&&(!m_syncWriteLogFlag))
	{
		m_threadStartFlag = false;
		m_pthreadWrite->join();
		delete m_pthreadWrite;
		m_pthreadWrite = NULL;
	}

	if (m_streamLog.is_open())
		m_streamLog.close();

	return 0;
}

bool CLog::isStarted()
{
	return m_threadStartFlag;
}

void CLog::write(unsigned int loglevel, char* file, const char* function, unsigned int line, const char *logContent)
{
	if (loglevel < m_logLevel)
		return;
	if (!isStarted() && !m_syncWriteLogFlag && !m_consoleWriteFlag)
		return;
	if (m_logDirName.empty())
		return;
	if ((!m_consoleWriteFlag) && (!m_fileWriteFlag))
		return;

	while (m_queue.size() > 100)
	{
#ifdef _WIN32
		Sleep(1);
#elif __linux__
		usleep(100);
#endif
	}

	time_t rawtime;
	struct tm tmlocal;
	char logtime[20];
	rawtime = time(NULL);
#ifdef _WIN32
	ZeroMemory(&tmlocal,sizeof(tm));
	localtime_s(&tmlocal, &rawtime);
#elif __linux__
	localtime_r(&rawtime, &tmlocal);
#endif
	strftime(logtime, 20, "%Y-%m-%d %X", &tmlocal);

	std::string filename = file;
	int pos = filename.find_last_of('\\');
	filename = filename.substr(pos + 1, filename.length());

	struct LOG_ST log_element;
	log_element.logTime = logtime;
	log_element.logLevel = m_loglevel[loglevel];
#ifdef _WIN32
	log_element.pid = _getpid();
#else
	log_element.pid = getpid();
#endif
	log_element.tid = std::this_thread::get_id();
	if (logContent != NULL)
	{
		log_element.logContent = logContent;
	}
	log_element.funcName = function;
	log_element.srcFile = file;
	log_element.lineNum = line;

	if (m_consoleWriteFlag)
	{
		g_log_console_mutex.lock();

		std::cout
			<< "[" << log_element.logTime << "] [" << log_element.logLevel << "] "
			<< "[" << log_element.pid << "] [" << log_element.tid << "] -- "
			<< log_element.logContent << " [" << log_element.srcFile << "(" << log_element.lineNum << "):"
			<< log_element.funcName << "]" << std::endl;

		g_log_console_mutex.unlock();
	}

	if (m_fileWriteFlag)
	{
		if (m_syncWriteLogFlag)
		{
			writeLogSync(log_element);
		}
		else
		{
			g_queue_mutex.lock();
			m_queue.push(log_element);
			g_queue_mutex.unlock();
		}
	}

	return;
}



int CLog::checkLogFileSize()
{
	std::string filepath = m_logDirName + "/" + m_logfilename;
	if (m_streamLog.is_open())
	{
		m_streamLog.seekp(0, m_streamLog.end);
		//size_t fileSize = m_streamLog.tellp();

		std::streamoff fileSize = m_streamLog.tellp();

		if (m_logMaxSize * 1024 * 1024 < fileSize)
		{
			time_t rawtime;
			struct tm tmlocal;
			char ptime[20];
			rawtime = time(NULL);
#ifdef _WIN32
			ZeroMemory(&tmlocal,sizeof(tm));
			localtime_s(&tmlocal, &rawtime);
#elif __linux__
			localtime_r(&rawtime, &tmlocal);
#endif
			strftime(ptime, 20, "%Y%m%d%H%M%S", &tmlocal);

			int pos = m_logfilename.find_first_of(".", 0);
			std::string filename = m_logfilename.substr(0, pos);
			std::string suffix = m_logfilename.substr(pos, m_logfilename.length());

			std::string newfilepath = m_logDirName + "/" + filename + "_" + ptime + suffix;
			m_streamLog.close();

			rename(filepath.c_str(), newfilepath.c_str());

			deleteOldLogFiles(m_logDirName);
		}
	}

	return 0;
}

void CLog::deleteOldLogFiles(const std::string& cate_dir)
{
	std::vector<std::string> files;

#ifdef _WIN32
	_finddata_t file;
	long lf;
	//�����ļ���·��
	if ((lf = _findfirst(cate_dir.c_str(), &file)) == -1)
	{
		return;
	}
	else
	{
		do
		{
			if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0)
				continue;

			files.push_back(file.name);
		} while (_findnext(lf, &file) == 0);
	}
	_findclose(lf);
#elif __linux__
	//regex_t reg;
	struct dirent *file = NULL; // return value for readdir()
	DIR *dir;					// return value for opendir()
	dir = opendir(cate_dir.c_str());
	if (NULL == dir)
	{
		return;
	}

	/* read all the files in the dir ~ */
	while ((file = readdir(dir)) != NULL)
	{
		// get rid of "." and ".."
		if (strcmp(file->d_name, ".") == 0 ||
			strcmp(file->d_name, "..") == 0)
		{
			continue;
		}

		files.push_back(file->d_name);
	}
	closedir(dir);
#endif
	sort(files.begin(), files.end());

	for (unsigned int index = 0; index + m_logFileCount <= files.size(); index++)
	{
		std::string filename = m_logDirName + "/" + files[index];
		remove(filename.c_str());
	}
}

int CLog::setLogFileCount(int count)
{
	if ((count >= 1) && (count < 100))
	{
		m_logFileCount = count;
	}

	return 0;
}

void CLog::writeLog()
{
	m_streamLog.open(m_logDirName + "/" + m_logfilename, std::fstream::out | std::fstream::app);
	while (m_threadStartFlag)
	{
		if (m_queue.empty())
		{
#ifdef _WIN32
			Sleep(500);
#elif __linux__
			usleep(500);
#endif
			continue;
		}

		if (m_streamLog.is_open())
		{
			if (!m_queue.empty())
			{
				g_queue_mutex.lock();
				struct LOG_ST logelement = m_queue.front();
				m_queue.pop();
				g_queue_mutex.unlock();

				m_streamLog << "[" << logelement.logTime << "] [" << logelement.logLevel << "] "
							<< "[" << logelement.pid << "] [" << logelement.tid << "] -- "
							<< logelement.logContent << " [" << logelement.srcFile << "(" << logelement.lineNum << "):"
							<< logelement.funcName << "]" << std::endl;
			}

			checkLogFileSize();
		}
		else
		{
			m_streamLog.open(m_logDirName + "/" + m_logfilename, std::fstream::out | std::fstream::app);
		}
	}

	if (m_streamLog.is_open())
	{
		m_streamLog.close();
	}
}

void CLog::writeLogSync(struct LOG_ST logelement)
{
	if (!m_streamLog.is_open())
	{
		m_streamLog.open(m_logDirName + "/" + m_logfilename, std::fstream::out | std::fstream::app);
	}

	m_streamLog << "[" << logelement.logTime << "] [" << logelement.logLevel << "] "
				<< "[" << logelement.pid << "] [" << logelement.tid << "] -- "
				<< logelement.logContent << " [" << logelement.srcFile << "(" << logelement.lineNum << "):"
				<< logelement.funcName << "]" << std::endl;

	checkLogFileSize();

	if (m_streamLog.is_open())
		m_streamLog.close();
}

int CLog::setConsoleWrite(bool consoleFlag)
{
	m_consoleWriteFlag = consoleFlag;
	return 0;
}

int CLog::setFileWrite(bool fileFlag)
{
	m_fileWriteFlag = fileFlag;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

//CLog runlog;

void initlog(const char *dir, int file_max_size_mb, int log_level, int log_file_count,  bool sync_write_flag, bool console_write_flag, bool file_write_flag)
{
	runlog.setLogDirName(dir);
	runlog.setLogMaxSize(file_max_size_mb);
	runlog.setLogLevel(log_level);
	runlog.setLogFileCount(log_file_count);
	runlog.setSyncWriteLog(sync_write_flag);
	runlog.setFileWrite(file_write_flag);
	runlog.setConsoleWrite(console_write_flag);
	runlog.start();

	return;
}


void writelog(unsigned int loglevel, char* file, const char* function, unsigned int line, const char *fmt, ...)
{
	va_list args;
	char *log_buf = NULL;
#ifdef _WIN32
	int len = 0;
#endif

	va_start(args, fmt);
#ifdef _WIN32
	len = _vscprintf(fmt, args ) + 1;
	log_buf = new char[len];
	vsprintf_s(log_buf, len, fmt, args);
#elif __linux__
	vasprintf(&log_buf, fmt, args);
#endif
	va_end(args);

	runlog.write(loglevel, file, function, line, log_buf);

#ifdef _WIN32
	delete[] log_buf;
#elif __linux__
	free(log_buf);
#endif
	log_buf = NULL;

	return;
}


void stoplog()
{
	runlog.stop();
}

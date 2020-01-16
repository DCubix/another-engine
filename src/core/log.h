#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <fstream>
#include <string>

#include "termcolor.hpp"

class Logger {
public:
	enum Level {
		Information = 0,
		Warning,
		Error,
		Assert
	};

	void setup(const char* file, const char* function, int line);
	void print(Level level, const std::string& message);

	inline void info(const std::string& message) { print(Level::Information, message); }
	inline void warn(const std::string& message) { print(Level::Warning, message); }
	inline void error(const std::string& message) { print(Level::Error, message); }
	inline void assert(bool test, const std::string& message) {
		if (!test) {
			print(Level::Assert, message);
			std::abort();
		}
	}

	inline static Logger& get() { return log; }
private:
	Logger() = default;

	std::string m_file, m_function;
	int m_line;

	static Logger log;
};

#ifdef __PRETTY_FUNCTION__
#define FN __PRETTY_FUNCTION__
#else
#define FN __FUNCTION__
#endif

#define Log (Logger::get().setup(__FILE__,FN,__LINE__),Logger::get())

#endif // LOG_H
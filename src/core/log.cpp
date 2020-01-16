#include "log.h"

#include <ctime>
#include <algorithm>

Logger Logger::log{};

void Logger::setup(const char* file, const char* function, int line) {
	m_file = std::string(file);
	m_function = std::string(function);
	m_line = line;
}

void Logger::print(Level level, const std::string& message) {
	time_t now = time(0);
	struct tm tstruct;
	char buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%m/%d/%Y %X", &tstruct);

	const std::string prefx = "[" + std::string(buf) + "] ";
	std::string filen = m_file;
	std::replace(filen.begin(), filen.end(), '\\', '/');
	filen = filen.substr(filen.find_last_of('/') + 1);

	std::cout << termcolor::green << termcolor::dark << prefx;
	switch (level) {
		case Level::Information: std::cout << termcolor::cyan << "[INFO.]"; break;
		case Level::Warning: std::cout << termcolor::yellow << "[WARNING]"; break;
		case Level::Error: std::cout << termcolor::red << "[ERROR]"; break;
		case Level::Assert: std::cout << termcolor::magenta << "[ASSERT]"; break;
	}

	std::cout << termcolor::reset;
	std::cout << " [" << filen << "(" << m_function << "@" << m_line << ")] " << message << std::endl;
}

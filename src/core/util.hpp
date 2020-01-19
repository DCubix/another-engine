#ifndef UTIL_HPP
#define UTIL_HPP

#include <string>
#include <sstream>
#include <vector>

namespace ae {
	namespace util {
		inline std::string& ltrim(std::string& s, const char* t = " \t\n\r\f\v") {
			s.erase(0, s.find_last_not_of(t));
			return s;
		}

		inline std::string& rtrim(std::string& s, const char* t = " \t\n\r\f\v") {
			s.erase(s.find_last_not_of(t) + 1);
			return s;
		}

		inline std::vector<std::string> split(const std::string& in, char delim) {
			std::stringstream ss(in);
			std::string item;
			std::vector<std::string> vec;
			while (std::getline(ss, item, delim)) {
				vec.push_back(item);
			}
			return vec;
		}
	}
}

#endif // UTIL_HPP

#ifndef FIXED_STRING_H
#define FIXED_STRING_H

#include <string>
#include <array>
#include <algorithm>
#include <cstdio>

template <int N>
struct FixedString {
	std::array<char, N> desc;
	int size;

	FixedString() :size(0){}

	void set(const char *fmt, ...){
		va_list args;
		va_start(args, fmt);
		char buffer[1024];
		vsprintf_s(buffer, fmt, args);
		va_end(args);
		desc.fill('\0');
		size = std::min((int)strlen(buffer), N);
		std::copy(buffer, buffer + size, desc.begin());
	}

	std::string str() const {
		return (size == 0) ? (std::string()) : std::string(desc.begin(), desc.begin() + size);
	}
};

#endif
#ifndef DESCRIPTION_H
#define DESCRIPTION_H

#include <array>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <sstream>

#include "component.h"
#include "fixed_string.h"

struct ShortDescription: public Component<ShortDescription> {
	static const char* Name(){ return "Short Description"; }

	static const int MAX_SIZE = 32;
	FixedString<MAX_SIZE> shortDescription;
	ID id;
	ID entity;

	ShortDescription(){}
	ShortDescription(const char *fmt, ...){
		va_list args;
		va_start(args, fmt);
		char buffer[1024];
		vsprintf_s(buffer, fmt, args);
		shortDescription.set(buffer);
		va_end(args);
	}

	std::string what() {
		std::ostringstream oss;
		oss << "shortdescription {";
		oss << "shortDescription: \"" << shortDescription.str() << "\"";
		oss << "}";
		return oss.str();
	}
};

struct Description: public Component<Description> {
	static const char* Name(){ return "Description"; }

	static const int MAX_SIZE = 128;
	FixedString<MAX_SIZE> description;

	Description(){}
	Description(const char *fmt, ...){		
		va_list args;
		va_start(args, fmt);
		char buffer[1024];
		vsprintf_s(buffer, fmt, args);
		description.set(buffer);
		va_end(args);
	}

	std::string what() {
		std::ostringstream oss;
		oss << "description {";
		oss << "description: \"" << description.str() << "\"";		
		oss << "}";
		return oss.str();
	}
	
	ID id;
	ID entity;
};

#endif
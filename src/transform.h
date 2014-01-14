#ifndef TRANSFORM_H
#define TRANSFORM_H
#include "component.h"
#include <sstream>

struct Transform: public Component<Transform> {
	static const char* Name(){ return "Transform"; }

	float x, y;

	Transform(float x = 0.f, float y = 0.f) :x(x), y(y){}
	std::string what() {
		std::ostringstream oss;
		oss << "transform {";
		COM_LOG_C(x);
		COM_LOG(y);
		oss << "}";
		return oss.str();
	}
};

#endif

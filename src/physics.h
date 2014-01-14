#ifndef PHYSICS_H
#define PHYSICS_H

#include "component.h"

struct Physics : public Component<Physics> {
	static const char* Name(){ return "Physics"; }

	float vx, vy;
	float oldx, oldy;

	Physics(float vx = 0.f, float vy = 0.f) :vx(vx), vy(vy){}
	std::string what() {
		std::ostringstream oss;
		oss << "physics {";
		COM_LOG_C(vx);
		COM_LOG_C(vy);
		COM_LOG_C(oldx);
		COM_LOG(oldy);
		oss << "}";
		return oss.str();
	}
};

#endif
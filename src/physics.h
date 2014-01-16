#ifndef PHYSICS_H
#define PHYSICS_H

#include "component.h"

struct Physics : public Component<Physics> {
	static const char* Name(){ return "Physics"; }

	float vx, vy;
	float oldx, oldy;

	// Physics now manages a resource
	// so it should implement the rule of 4
	struct Thing { };
	Thing* thing;

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
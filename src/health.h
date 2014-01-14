#ifndef HEALTH_H
#define HEALTH_H
#include "component.h"
#include <sstream>

struct Health: public Component<Health> {
	static const char* Name(){ return "Health"; }

	float health;
	bool poisoned;

	Health(float health = 0.f, bool poisoned = false) :health(health), poisoned(poisoned){}
	std::string what() {
		std::ostringstream oss;
		oss << "health {";
		COM_LOG_C(health);
		COM_LOG(poisoned);
		oss << "}";
		return oss.str();
	}

	ID id;
	ID entity;
};

#endif
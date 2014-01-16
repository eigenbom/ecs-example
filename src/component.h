#ifndef COMPONENT_H
#define COMPONENT_H

#include <ostream>
#include <cassert>

using ID = unsigned int;
static const ID INVALID_ID = 0;
static const int MAX_COMPONENTS = 16;

// Logging shorthand for components
#define COM_LOG_C(var) {oss << (#var) << ": " << std::boolalpha << var << ", ";}
#define COM_LOG(var) {oss << (#var) << ": " << std::boolalpha << var;}

// Uses CRTP to generate ids
// (based on EntityX by AlecThomas)
struct BaseComponent {
protected:
	static unsigned int sComponentIndex;
};

template <typename Derived>
struct Component : public BaseComponent {
	ID id;
	ID entity;

	/// Used internally for registration.
	static int Index(){
		static unsigned int index = sComponentIndex++;
		assert(index < MAX_COMPONENTS);
		return index;
	}

	operator bool(){
		return id!=INVALID_ID;
	}
};


#endif
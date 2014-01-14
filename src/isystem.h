#ifndef ISYSTEM_H
#define ISYSTEM_H

class Entity;
class EntitySystem;
class ISystem {
public:	
	virtual ~ISystem(){};
	
	// returns the 
	virtual bool implements(int componentIndex) = 0;
	virtual void setup(Entity& e){};
	virtual void cleanup(Entity& e){};
	virtual void update(EntitySystem& es, double dt) = 0;
	virtual const char* name() = 0;
};

#endif
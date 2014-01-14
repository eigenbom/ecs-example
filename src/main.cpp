#include <iostream>
#include <initializer_list>
#include <vector>
#include <list>
#include <string>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <tuple>
#include <functional>
#include <algorithm>
#include <cassert>
#include <iomanip>
#include <array>
#include <cstdlib>
#include <ctime>

#include "packedarray.h"
#include "entity.h"
#include "isystem.h"
#include "all_components.h"

using namespace std;

class HealthSystem : public ISystem {
public:
	bool implements(int componentIndex) override {
		return Health::Index()==componentIndex;
	}

	void setup(Entity& e) override {
		std::cout << "HealthSystem::setup entity " << e.id << "\n";
		
		// Initialise derived values, timers, internal structs, or whatever
		e.get<Health>().health = 100;
	}

	void cleanup(Entity& e) override {
		std::cout << "HealthSystem::cleanup entity " << e.id << "\n";

		// Destroy internal things or stuffs etc
	}

	void update(EntitySystem& es, double dt) override {
		for (Health& h : es.components<Health>()){
			if (h.poisoned){
				h.health -= 0.1f * (float)dt;
				if (h.health <= 0){
					// Create KILL EVENT
				}
			}
		}
	}

	const char* name() override {
		return "HealthSystem";
	}
};

class PhysicsSystem : public ISystem {
public:
	bool implements(int componentIndex) override {
		return Physics::Index()==componentIndex;
	}

	void setup(Entity& e) override {
		std::cout << "PhysicsSystem::setup entity " << e.id << "\n";

		// Check that it has a transform
		assert(e.has<Transform>());
	}

	void cleanup(Entity& e) override {
		std::cout << "PhysicsSystem::cleanup entity " << e.id << "\n";

	}

	void update(EntitySystem& es, double dt) override {
		// NB: Usually would split this into 
		// separate read/execute/update passes
		// to allow multithreading over systems

		for (Physics& p : es.components<Physics>()){			
			Entity& e = es.lookup(p.entity);
			Transform& tr = e.get<Transform>();

			p.oldx = tr.x;
			p.oldy = tr.y;
			tr.x += p.vx * (float)dt;
			tr.y += p.vy * (float)dt;
		}
	}

	const char* name() override {
		return "PhysicsSystem";
	}
};

template <typename First> void AttachEntityToSystem(Entity& e, ISystem* sys, const TypeList<First>& tl){	
	if (e.has<First>() && sys->implements(First::Index())){
		sys->setup(e);
	}
}

template <typename First, typename... Rest> void AttachEntityToSystem(Entity& e, ISystem* sys, const TypeList<First, Rest...>& tl){
	if (e.has<First>() && sys->implements(First::Index())){
		sys->setup(e);
	}
	if (sizeof...(Rest)){
		AttachEntityToSystem(e, sys, TypeList<Rest...>());
	}
}


int main(int argc, char** argv[])
{
	std::srand((unsigned int)std::time(NULL));

	EntitySystem es;
	HealthSystem healthSystem;
	PhysicsSystem physicsSystem;

	es.addSystem(&healthSystem);
	es.addSystem(&physicsSystem);

	Entity& e1 = es.create();
	ID id = e1.id;
	int numEyes = 2 + (rand() % 8);

	// TODO: Each of these should generate an event, so e.g., 
	// we can setup Physics when a physics entity is created
	e1.add(Transform(4,5));
	e1.add(Health(10));
	e1.add(Physics(1,0));
	e1.add(ShortDescription("Bob-%d", numEyes));
	e1.add(Description("An angry robot with %d eyes.", numEyes));
	
	// At this point we register the entity with every system 
	// that wants to know about it
	for (ISystem* sys: std::list<ISystem*>{ &healthSystem, &physicsSystem }){
		AttachEntityToSystem(e1, sys, ComponentTypeList());
	}	
	es.sync();

	// component access shorthand
	es.lookup(id).transform() = vec2{ 0, 0 };
	es.lookup(id).physics().vx *= 0.9f; // slow down
	
	// test if entity removal signals systems
	es.remove(id);
	es.sync();

	es.printDebugInfo(std::cout);


	/*
	std::cout << "Test entity creation\n";
	std::cout << e1;

	std::cout << "Test component removal\n";
	e1.remove<Description>();

	std::cout << "before sync\n";
	std::cout << e1;
	es.printDebugInfo(std::cout);

	es.sync();
	std::cout << "after sync\n";
	std::cout << e1;
	es.printDebugInfo(std::cout);

	std::cout << "Test entity removal\n";
	std::cout << "Before sync\n";
	es.remove(id);
	es.printDebugInfo(std::cout);

	es.sync();
	std::cout << "After sync\n";
	es.printDebugInfo(std::cout);

	es.sync();
	std::cout << es.lookup(id);

	// Assignment
	es.lookup(id).get<Transform>() = { 0, 0 };
	std::cout << es.lookup(id);

	// Treasure chests
	vector<const char*> inscriptions = {
		"Don't open this!",
		"Treasure inside!",
		"You stink!"
	};

	Entity& e2 = es.create();
	ID e2id = e2.id;
	e2.add(Transform{ -4.5f, 0.8f });
	e2.add(Inventory{
		{ Item::SWORD },
		{ Item::POTION, 4 },
		{ Item::POTION, 3 },
		{ Item::ARROW, 64 }
	});
	e2.add(Description("An ornate treasure chest with an inscription that reads \"%s\".", inscriptions[rand() % inscriptions.size()]));

	es.sync();


	// Print out all entities
	std::cout << "Print out all entities\n";
	for (Entity& e : es.entities()){
		if (e){
			std::cout << e;
		}
	}

	// Iterate through all transforms, for example
	std::cout << "Iterating over specific components\n";
	for (auto tr : es.components<Transform>()){
		tr.x = 0.5;

		// Also adjust health e.g.,
		Entity& e = es.lookup(tr.entity);
		if (e.has<Health>()){
			Health& h = e.get<Health>();
			h.health += 0.01f;
		}
	}

	// Accessing a non-existent component
	// will return an invalid component
	// You should use Entity::has<Component>() to check
	Entity& someEntityWithoutHealth = es.lookup(e2id);
	Health& health = someEntityWithoutHealth.get<Health>();
	health.health = 666;
	std::cout << someEntityWithoutHealth;

	// Removal
	es.remove(e2id);

	// TODO: Should we add the ability to add/remove
	// components after the entity has been created?
	// TODO: use packed array to store components (use a max size of MAX_ENTITIES*(probability_of_entity_having_this_component<=1))

	// Update a system
	for (int i = 0; i < 1000; i++){
		healthSystem.update(es, 0.01);
	}

	for (Entity& e : es.entities()){
		if (e.has<Health>()){
			cout << e.get<Health>().what() << "\n";
		}
	}

	*/

	return EXIT_SUCCESS;
}

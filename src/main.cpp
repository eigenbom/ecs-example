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
#include "all_components.h"

using namespace std;

class HealthSystem {
public:
	void update(EntitySystem& es, double dt){
		for (Health& h : es.components<Health>()){
			if (h.poisoned){
				h.health -= 0.1f * (float)dt;
				if (h.health <= 0){
					// Create KILL EVENT
				}
			}
		}
	}
};

int main(int argc, char** argv[])
{
	std::srand((unsigned int)std::time(NULL));

	EntitySystem es;
	HealthSystem healthSystem;

	Entity& e1 = es.create();
	ID id = e1.id;
	int numEyes = 2 + (rand() % 8);
	e1.add(Transform{ 4, 5 });
	e1.add(Health{ 99, true });
	e1.add(ShortDescription("Bob-%d", numEyes));
	e1.add(Description("An angry robot with %d eyes.", numEyes));

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
	e2.add(Description("An ornate treasure chest with an inscription that reads \"%s\".", inscriptions[rand()%inscriptions.size()]));

	es.sync();


	// Print out all entities
	std::cout << "Print out all entities\n";
	for (Entity& e: es.entities()){
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

	for(Entity& e: es.entities()){
	if (e.has<Health>()){
	cout << e.get<Health>().what() << "\n";
	}
	}

	return EXIT_SUCCESS;
}

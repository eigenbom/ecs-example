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
#include <chrono>

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

template <typename T>
double testVectorCreation(std::chrono::high_resolution_clock& clock, int sz){
	auto t1 = clock.now();
	std::vector<T> v(sz);
	auto t2 = clock.now();
	std::chrono::duration<double> timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);	
	return timeSpan.count();	
}

using Type = int;

struct SomeData {
	Type t;
};

struct SomeData2 {
	Type t;
	SomeData2(){}
};

struct BaseData {
	int whatever;
};

struct SomeData3: public BaseData{
	Type t;
};

struct BaseData2 {
	int whatever;
};

struct SomeData4 : public BaseData2{
	Type t;
	SomeData4(){}
};

struct BaseData3 {
	int whatever;
	BaseData3(){}
};

struct SomeData5: public BaseData3{
	// std::vector<SomeData5> doesn't initialise all emptys
  // so creation is FAST
	Type t;

	// std::vector<Type> ts; // slows down
	std::array<unsigned char, 8> a; // its ok

	struct InternalStruct {
		int x;
	} is; // seems to be ok!

	/*
	struct InternalStruct2 {
		int x;
		InternalStruct2():x(0){}
	} is2; 
	// slows down vec tests
	*/

	struct InternalStruct3 {
		int x;
		InternalStruct3(){}
	} is3; // still good

	// Some managed thing...
	InternalStruct3* pis3;
	// shared_ptr<InternalStruct3> pis3; // slows down vec tests

	SomeData5():BaseData3(){} // Need blank constructor for vector tests, but not for MyVec tests..
	SomeData5(Type t):t(t),BaseData3(){}
	SomeData5(const SomeData5& sd){
		t = sd.t;
	}
	SomeData5(SomeData5&& sd){
		std::cout << "SomeData5 rvalue copy constructor called\n";
		// should swap the managed thing..

		// std::swap();
		t = sd.t;
	}
};



struct NonCopyable {
protected:
	NonCopyable() = default;
	~NonCopyable() = default;
private:
	NonCopyable(const NonCopyable&){};
	NonCopyable& operator=(const NonCopyable& d){};
};

struct Data : public BaseData3, NonCopyable {	
	int t;	
	std::array<unsigned char, 8> a; // its ok

	struct Foo {
		int x;
	} is;

	// Some managed thing...
  // InternalStruct* pis3;
	// std::unique_ptr<Foo> pfoo; // slows down vec tests
	Foo* pfoo;

	// What happens with a vector<>
	std::vector<int> data;

	struct Something {
		Something(){}
		~Something(){
			std::cout << "Deleting something.\n";
		}
	} something;

	// Always need a default constructor?	
	Data(int _t = 0) :BaseData3(), NonCopyable(), data(_t){
		std::cout << "new Foo\n";
		pfoo = new Foo(); 
		pfoo->x = _t;
		t = _t;
		for (int i = 0; i < data.size(); i++){
			data[i] = t + i;
		}
	};

	~Data(){
		std::cout << "delete Foo\n";
		delete pfoo;

		// Because the memory may still be around
		// we need to null things to be safe
		// even though the user should maintain a 
		// list of which things are valid..
		pfoo = nullptr;

		// Don't have to delete data?
		// data.~vector();
	}
	
	Data(Data&& d){
		// swap uses move
		// semantics where available
		std::swap(pfoo, d.pfoo);
		std::swap(data, d.data);
		std::swap(t, d.t);
	}
};

// A simple vector class that doesn't initialise its entries
// The idea is that the user appropriately initialises things
template <typename T>
class MyVec {
public:		
	using value_type = T;
	static const size_t value_type_size = sizeof(T);
	MyVec(int size):mSize(size),mData(size*value_type_size){}	
	size_t size() const { return mSize; }

	// Warning this is going to be uninitialised
	// So who knows what will happen when copying etc
	T& operator[](unsigned int index){
		return *reinterpret_cast<T*>(&mData[index*value_type_size]);
	}

protected:
	// Need to use a struct with a blank constructor 
	// to avoid initialization
	struct uninitialized_char { unsigned char c; uninitialized_char(){} };
	static_assert(1 == sizeof(uninitialized_char), "");
	size_t mSize;
	std::vector<uninitialized_char> mData; // just some bytes 

	// Another way to handle the bytes
	// std::unique_ptr<char[]>();
};


double testNonVectorCreation(std::chrono::high_resolution_clock& clock, int sz){
	auto t1 = clock.now();
	MyVec<Data> v(sz);
	auto t2 = clock.now();
	std::chrono::duration<double> timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);

	// Access uninitialised stuff
	// Seems to be all zeros..
	std::cout << "Check initial values\n";
	for (int i = 0; i < sz; i+=sz/10){
		std::cout << "[" << i << "]    ";
		std::cout << "0x" << v[i].pfoo << "\n";
	}
	std::cout << "\n";
	
	// Construct (using in-place new)	
	// And remember which things are constructed...
	std::vector<int> constructed;
	std::cout << "Construct\n";
	for (int i = 0; i < sz; i += sz/10){
		std::cout << "[" << i << "]    ";
		new(&v[i])Data(i%7);
		std::cout << "0x" << v[i].pfoo << "->x = " << v[i].pfoo->x << "\n";
		constructed.push_back(i);
	}
	std::cout << "\n";

	// Move constructor (rvalue?)
	std::cout << "Move construct\n";
	for (int i = 1; i < sz - 1; i += sz / 10){
		std::cout << "[" << i << "]    ";
		new(&v[i])Data(std::move(v[i - 1]));
		std::cout << "0x" << v[i].pfoo << " <-> 0x" << v[i - 1].pfoo << "\n";

		// Update constructed list..
		std::replace(constructed.begin(), constructed.end(), i-1, i);
	}
	std::cout << "\n";

	// Destructor
	std::cout << "Destruct\n";
	for (int i: constructed){
		std::cout << "[" << i << "]    \n";
		// Explicitly destroy
		std::cout << "  before = 0x" << v[i].pfoo << "\n";
		std::cout << "       vec 0x" << v[i].data.data() << "\n";

		v[i].~Data();
		std::cout << "  after  = 0x" << v[i].pfoo << "\n";
		std::cout << "       vec 0x" << v[i].data.data() << "\n";
	}
	std::cout << "\n";

	// Call assignment operators 
	/*
	for (int i = 0; i < sz-1; i += sz / 10){
		v[i] = v[i + 1];
		std::cout << v[i].t << " ";
	}
	std::cout << "\n";
	*/

	// v[2] = v[3];

	// In-place new...?

	return timeSpan.count();
}

int main(int argc, char** argv[]){
	// Test speed of initialisation
	auto clock = std::chrono::high_resolution_clock();	
	
	const int NUM_ELEMENTS = 1<<24;

	// Just do the single test
	double s = testNonVectorCreation(clock, NUM_ELEMENTS);
	std::cout << "  [test] took " << (1000*s) << "ms\n";

	/*
	const int NUM_TESTS = 16;
	static const int NUM_RESULTS = 7;
	double results[NUM_RESULTS] = { 0, 0, 0 };
	for (int i = 0; i < NUM_TESTS; i++){
		std::cout << ".";
		results[0] += testVectorCreation<Type>(clock, NUM_ELEMENTS);
		results[1] += testVectorCreation<SomeData>(clock, NUM_ELEMENTS);
		results[2] += testVectorCreation<SomeData2>(clock, NUM_ELEMENTS);
		results[3] += testVectorCreation<SomeData3>(clock, NUM_ELEMENTS);
		results[4] += testVectorCreation<SomeData4>(clock, NUM_ELEMENTS);
		results[5] += testVectorCreation<SomeData5>(clock, NUM_ELEMENTS);
		results[6] += testNonVectorCreation(clock, NUM_ELEMENTS);
	}

	std::cout << "\nResults\n";
	for (int i = 0; i < NUM_RESULTS; i++){
		std::cout << "  [test " << i << "] took " << (1000*(results[i] / NUM_TESTS)) << "ms\n";
	}*/

	return EXIT_SUCCESS;	
}


int main2(int argc, char** argv[])
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

	// Test move semantics etc




	/*

	// component access shorthand
	es.lookup(id).transform() = vec2{ 0, 0 };
	es.lookup(id).physics().vx *= 0.9f; // slow down

	// check validity of component
	std::cout << boolalpha << (bool)(es.lookup(id).get<Inventory>()) << "\n";
	
	// test if entity removal signals systems
	es.remove(id);
	es.sync();

	es.printDebugInfo(std::cout);

	*/

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

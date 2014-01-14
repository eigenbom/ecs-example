#include "entity.h"
#include <iostream>

Entity::Entity() :mES(nullptr), id(INVALID_ID){}

Entity::Entity(EntitySystem* es) : mES(es), id(INVALID_ID) {

}

void Entity::clear(){
	id = INVALID_ID;
	for (int i = 0; i < NUM_COMPONENTS; i++){
		mHasComponent[i] = false;
	}
}

Entity::operator bool(){
	return id != INVALID_ID;
}

template <typename First>
void printComponent(std::ostream& out, Entity& e, const TypeList<First>& tl){
	if (e.has<First>()) out << "- " << e.get<First>().what() << "\n";
}

template <typename First, typename... Rest>
void printComponent(std::ostream& out, Entity& e, const TypeList<First, Rest...>& tl){
	if (e.has<First>()) out << "- " << e.get<First>().what() << "\n";
	if (sizeof...(Rest)){
		printComponent(out, e, TypeList<Rest...>());
	}
}

std::ostream& operator<<(std::ostream& out, Entity& e){
	out << "Entity {id:" << e.id << "}\n";
	printComponent(out, e, ComponentTypeList());
	return out;
}

void Entity::removeAllComponents(bool immediately){
	removeComponents(immediately, ComponentTypeList());
}

EntitySystem::EntityView::Iterator::Iterator(EntitySystem* es, int i) :es(es), i(i){}
EntitySystem::EntityView::Iterator& EntitySystem::EntityView::Iterator::operator++(){
	++i;
	return *this;
}

bool EntitySystem::EntityView::Iterator::operator==(const EntitySystem::EntityView::Iterator& rhs) const {
	return i == rhs.i;
}

bool EntitySystem::EntityView::Iterator::operator!=(const EntitySystem::EntityView::Iterator& rhs) const {
	return i != rhs.i;
}

Entity& EntitySystem::EntityView::Iterator::operator*(){
	return es->mEntities.objects().get(i);
}

const Entity& EntitySystem::EntityView::Iterator::operator*() const {
	return es->mEntities.objects().get(i);
}

EntitySystem::EntityView::EntityView(EntitySystem* es) :es(es){}
EntitySystem::EntityView::Iterator EntitySystem::EntityView::begin() {
	return Iterator(es, 1);
}

EntitySystem::EntityView::Iterator EntitySystem::EntityView::end(){
	return Iterator(es, es->mEntities.size());
}

EntitySystem::EntitySystem(){
	// Setup up the invalid entity
	Entity& invalidEntity = create();
	
	// Create arrays for components
	// Includes an invalid component with id=INVALID_ID
	mComponents = std::vector<PackedArrayBase*>(NUM_COMPONENTS, nullptr);
	setupComponentArrays(ComponentTypeList());

	// component removal cache
	mComponentsToBeRemoved = std::vector<std::vector<ID>>(NUM_COMPONENTS, std::vector<ID>());
}

EntitySystem::~EntitySystem(){
	for (PackedArrayBase* b : mComponents) delete b;
}

void EntitySystem::addSystem(ISystem* system){
	mSystems.push_back(system);
}

/// Create a new entity
Entity& EntitySystem::create(){
	Entity proto(this);
	proto.clear();
	ID id = mEntities.add(proto);
	return mEntities.lookup(id);
}

// Remove an entity
// Won't be removed until sync()ed
void EntitySystem::remove(ID id){
	mEntitiesToBeRemoved.push_back(id);
}

bool EntitySystem::has(ID id){
	if (id == INVALID_ID) return false;
	else return mEntities.has(id);
}

// Retrieve an entity (or get an invalid one)
Entity& EntitySystem::lookup(ID id){
	if (mEntities.has(id)){
		return mEntities.lookup(id);
	}
	else {
		return mEntities.lookup(INVALID_ID);
	}
}

// Get full list of entities
EntitySystem::EntityView EntitySystem::entities(){
	return EntityView(this);
}

template <typename First> void RemoveEntityFromSystem(Entity& e, ISystem* sys, const TypeList<First>& tl){
	if (e.has<First>() && sys->implements(First::Index())){
		sys->cleanup(e);
	}
}

template <typename First, typename... Rest> void RemoveEntityFromSystem(Entity& e, ISystem* sys, const TypeList<First, Rest...>& tl){
	if (e.has<First>() && sys->implements(First::Index())){
		sys->cleanup(e);
	}
	if (sizeof...(Rest)){
		RemoveEntityFromSystem(e, sys, TypeList<Rest...>());
	}
}

void EntitySystem::sync(){	
	for (ID id : mEntitiesToBeRemoved){
		if (mEntities.has(id)){
			Entity& e = mEntities.lookup(id);
			for (ISystem* sys : mSystems){
				RemoveEntityFromSystem(e, sys, ComponentTypeList());
			}
			e.removeAllComponents(true);
			e.clear();
			mEntities.remove(id);
		}
	}
	mEntitiesToBeRemoved.clear();
	
	removeQueuedComponents(ComponentTypeList());
	for (auto v : mComponentsToBeRemoved) v.clear();
}

void EntitySystem::printDebugInfo(std::ostream& out){
	out << "EntitySystem\n";
	out << "------------------------\n";
	out << mEntities.size() << " entities (" << (mEntities.objects().bytes() / 1024) << "kb) " << std::endl;
	printDebugInfoForComponents(out, ComponentTypeList());
	out << "------------------------\n";
}
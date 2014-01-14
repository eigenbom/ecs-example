#ifndef ENTITY_H
#define ENTITY_H

#include <iomanip>

#include "all_components.h"
#include "packedarray.h"
#include "isystem.h"

static const int MAX_ENTITIES = 0xffff;

class EntitySystem;
class Entity {
public:
	ID id;
	Entity(); 

	// Add a component to the entity
	template <typename C>	C& add(C& c = C());

	// Get a component 
	// PRE: entity has() the component
	template <typename C> C& get();

	// Check if entity has a component
	template <typename C>	bool has();

	// Remove component
	template <typename C>	void remove(bool immediately=false);

	// Remove all components
	void removeAllComponents(bool immediately = false);

	// Returns id()!=INVALID_ID
	operator bool();

	// Shorthand for common components
	Transform& transform(){	return get<Transform>(); }
	Health& health(){ return get<Health>(); }
	Physics& physics(){ return get<Physics>(); }
	
protected:

	Entity(EntitySystem* es);
	void clear();
		
	// Helpers to help remove lots of components
	template <typename First> void removeComponents(bool immediately, const TypeList<First>& tl);
	template <typename First, typename... Rest> void removeComponents(bool immediately, const TypeList<First, Rest...>& tl);

	ID mComponents[NUM_COMPONENTS];
	bool mHasComponent[NUM_COMPONENTS];
	EntitySystem* mES;

	friend class EntitySystem;
};

std::ostream& operator<<(std::ostream& out, Entity& e);

class EntitySystem {
protected:
	// Helpers
	
	class EntityView {
	protected:
		class Iterator : public std::iterator<std::input_iterator_tag, Entity>{
		public:
			Iterator(EntitySystem* es, int i);
			Iterator& operator++();
			bool operator==(const Iterator& rhs) const;
			bool operator!=(const Iterator& rhs) const;
			Entity& operator*();
			const Entity& operator*() const;

		protected:
			int i;
			EntitySystem* es;
			friend class EntitySystem;
		};

	public:
		EntityView(EntitySystem* es);
		Iterator begin();
		Iterator end();

	protected:
		EntitySystem* es;
		friend class EntitySystem;
	};

	template <typename C>
	class ComponentView {
	protected:
		class Iterator : public std::iterator<std::input_iterator_tag, C> {
		public:
			Iterator(EntitySystem* es, int i);
			Iterator& operator++();
			bool operator==(const Iterator& rhs) const;
			bool operator!=(const Iterator& rhs) const;
			C& operator*();
			const C& operator*() const;

		protected:
			int i;
			EntitySystem* es;
			friend class EntitySystem;
		};

	public:
		ComponentView(EntitySystem* es);
		Iterator begin();
		Iterator end();

	protected:	

		EntitySystem* es;
		friend class EntitySystem;
	};

public:
	EntitySystem();
	~EntitySystem();

	// Add systems
	// EntitySystem doesn't own it
	void addSystem(ISystem* system);
	
	// Create a new entity immediately
	Entity& create();

	// Remove an entity and its components
	// NB: Won't be removed until sync()ed
	void remove(ID id);
	
	// Check for entity
	bool has(ID id);

	// Lookup an entity
	// NB: Don't retain the ref, it may change after a sync()
	Entity& lookup(ID id);

	// Get full list of entities
	EntityView entities();
	
	// Get all components of a particular type
	template <typename C>
	ComponentView<C> components();

	// TODO: Call sync() at the end of each frame
	// to remove queued entities, components etc
	void sync();
	
	// Info
	void printDebugInfo(std::ostream& out);

protected:
	/// Internal helpers
	template <typename C>
	C& addComponent(ID entityId, C& pc);
		
	template <typename C>
	C& getComponent(ID id);

	template <typename C>
	void removeComponent(ID id);
	
	template <typename First>
	void removeQueuedComponents(const TypeList<First>& tl);
	template <typename First, typename... Rest>
	void removeQueuedComponents(const TypeList<First, Rest...>& tl);

	template <typename C>
	void setupComponentArray();
	template <typename First>
	void setupComponentArrays(const TypeList<First>& tl);
	template <typename First, typename... Rest>
	void setupComponentArrays(const TypeList<First, Rest...>& tl);

	template <typename First>
	void printDebugInfoForComponents(std::ostream& out, const TypeList<First>& tl);
	template <typename First, typename... Rest>
	void printDebugInfoForComponents(std::ostream& out, const TypeList<First, Rest...>& tl);

	template <typename C>
	PackedArray<C>& array();

protected:
	PackedArray<Entity> mEntities;
	std::vector<PackedArrayBase*> mComponents;	
	std::vector<ISystem*> mSystems;
	friend class Entity;

	std::vector<ID> mEntitiesToBeRemoved;
	std::vector<std::vector<ID> > mComponentsToBeRemoved;
};

#include "entity.inl"

#endif
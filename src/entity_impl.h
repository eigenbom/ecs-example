

template <typename C>
C& Entity::add(C& c){
	if (has<C>()){
		// If already has the component then copy it
		// NB: Can we add two components of same type?
		C& oc = get<C>();
		oc = c;
		return oc;
	}
	else {
		C& pc = mES->addComponent<C>(id, c);
		if (pc.id!=INVALID_ID){
			mComponents[C::Index()] = pc.id;
			mHasComponent[C::Index()] = true;
		}
		return pc;
	}
}

template <typename C>
C& Entity::get(){
	return mES->getComponent<C>(mComponents[C::Index()]);
}

template <typename C>
bool Entity::has(){
	return mHasComponent[C::Index()];
}

// Remove component
template <typename C>	
void Entity::remove(){
	if (mHasComponent[C::Index()]){
		mES->removeComponent<C>(mComponents[C::Index()]);
		mHasComponent[C::Index()] = false;
	}
}

template <typename First>
void Entity::removeComponents(const TypeList<First>& tl){
	remove<First>();
}

template <typename First, typename... Rest> 
void Entity::removeComponents(const TypeList<First, Rest...>& tl){
	remove<First>();
	if (sizeof...(Rest)){
		removeComponents(TypeList<Rest...>());
	}
}

template <typename C>
EntitySystem::ComponentView<C>::Iterator::Iterator(EntitySystem* es, int i) :es(es), i(i){}

template <typename C>
typename EntitySystem::ComponentView<C>::Iterator&
EntitySystem::ComponentView<C>::Iterator::operator++(){
	++i;
	return *this;
}

template <typename C>
bool EntitySystem::ComponentView<C>::Iterator::operator==(const typename EntitySystem::ComponentView<C>::Iterator& rhs) const {
	return i == rhs.i; 
}

template <typename C>
bool EntitySystem::ComponentView<C>::Iterator::operator!=(const typename EntitySystem::ComponentView<C>::Iterator& rhs) const { 
	return i != rhs.i; 
}

template <typename C>
C& EntitySystem::ComponentView<C>::Iterator::operator*() {	
	return es->array<C>().objects().get(i);
}

template <typename C>
const C& EntitySystem::ComponentView<C>::Iterator::operator*() const {
	return es->array<C>().objects().get(i);
}

template <typename C>
EntitySystem::ComponentView<C>::ComponentView(EntitySystem* es) :es(es){}

template <typename C>
typename EntitySystem::ComponentView<C>::Iterator EntitySystem::ComponentView<C>::begin(){
	return Iterator(es, 1);
}

template <typename C>
typename EntitySystem::ComponentView<C>::Iterator EntitySystem::ComponentView<C>::end(){
	return Iterator(es, es->array<C>().size());
}

template <typename C>
EntitySystem::ComponentView<C> EntitySystem::components(){
	return EntitySystem::ComponentView<C>(this);
}

// protected

template <typename C>
C& EntitySystem::addComponent(ID entityId, C& pc){
	PackedArray<C>& arr = array<C>(); 
	pc.entity = entityId;
	ID id = arr.add(pc);
	return arr.lookup(id);
}

template <typename C>
C& EntitySystem::getComponent(ID id){
	PackedArray<C>& arr = array<C>();
	if (arr.has(id)){
		return arr.lookup(id);
	}
	else {
		return arr.lookup(INVALID_ID);
	}
}

template <typename C>
void EntitySystem::removeComponent(ID id){
	mComponentsToBeRemoved[C::Index()].push_back(id);
}

template <typename C>
void EntitySystem::setupComponentArray(){	
	if (mComponents[C::Index()] == nullptr){
		mComponents[C::Index()] = new PackedArray<C>();
		PackedArray<C>& arr = array<C>();
		
		// Add invalid component
		C invalid;		
		invalid.entity = INVALID_ID;		
		ID id = arr.add(invalid);
		invalid = arr.lookup(id);
		assert(invalid.id == INVALID_ID);
		
		// Log memory usage etc
		unsigned int bytes = arr.objects().bytes();
		std::cout << "System: allocating " << std::setprecision(8) << (bytes / 1024) << "kb for " << C::Name() << " component." << std::endl;
	}
}

template <typename First>
void EntitySystem::setupComponentArrays(const TypeList<First>& tl){
	setupComponentArray<First>();
}

template <typename First, typename... Rest>
void EntitySystem::setupComponentArrays(const TypeList<First, Rest...>& tl){
	setupComponentArray<First>();
	if (sizeof...(Rest)){
		setupComponentArrays(TypeList<Rest...>());
	}
}

template <typename First>
void EntitySystem::removeQueuedComponents(const TypeList<First>& tl){
	PackedArray<First>& arr = array<First>();
	for (ID id : mComponentsToBeRemoved[First::Index()]){		
		if (arr.has(id)){
			arr.remove(id);
		}
	}
}

template <typename First, typename... Rest>
void EntitySystem::removeQueuedComponents(const TypeList<First, Rest...>& tl){
	PackedArray<First>& arr = array<First>();
	for (ID id : mComponentsToBeRemoved[First::Index()]){
		if (arr.has(id)){
			arr.remove(id);
		}
	}
	if (sizeof...(Rest)){
		removeQueuedComponents(TypeList<Rest...>());
	}
}

template <typename C>
PackedArray<C>& EntitySystem::array(){
	return *static_cast<PackedArray<C>*>(mComponents[C::Index()]);
}

template <typename First>
void EntitySystem::printDebugInfoForComponents(std::ostream& out, const TypeList<First>& tl){
	PackedArray<First>& arr = array<First>();	
	out << "#Component[" << First::Name() << "]: " << arr.size() << " (" << (arr.objects().bytes()/1024) << "kb)" << std::endl;
}

template <typename First, typename... Rest>
void EntitySystem::printDebugInfoForComponents(std::ostream& out, const TypeList<First, Rest...>& tl){
	PackedArray<First>& arr = array<First>();
	out << "#Component[" << First::Name() << "]: " << arr.size() << " (" << (arr.objects().bytes() / 1024) << "kb)" << std::endl;
	if (sizeof...(Rest)){
		printDebugInfoForComponents(out, TypeList<Rest...>());
	}
}
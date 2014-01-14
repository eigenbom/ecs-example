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

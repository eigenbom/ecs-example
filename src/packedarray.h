#ifndef PACKED_ARRAY_H
#define PACKED_ARRAY_H

#include <climits>
#include <vector>
#include <cstdlib>
#include <cassert>

#include "component.h"

// A simple resizable array for PODs
// Unlike vector() doesn't initialise until it wants to
template <typename T>
class StaticArray {
public:
	void accommodate(unsigned int size){
		mData.resize(size * sizeof(T));
	}

	T& get(unsigned int index){
		assert(index*sizeof(T) < mData.size());
		return *(T*)&mData[index * sizeof(T)];
	}

	void set(unsigned int index, const T& t){
		assert(index*sizeof(T) < mData.size());
		// Call the copy constructor to initialise everything
		// ((T*)&mData[index * sizeof(T)])->T(t); 

		// create a new object in the buffer
		// Calls copy constructor
		new(&mData[index * sizeof(T)])T(t);

		// *(T*)&mData[index * sizeof(T)] = t;
	}

	unsigned int capacity() const {
		return (unsigned int)(mData.size() / sizeof(T));
	}

	// Number of bytes used by this array
	unsigned int bytes() const {
		return (unsigned int)mData.size();
	}

protected:
	std::vector<char> mData; // just some bytes 
};

class PackedArrayBase {
public:
	virtual ~PackedArrayBase(){}
};

// PackedArray: stores things in a static array 
// by tightly packing them and using an extra 
// array to track indices etc
// (based on bitsquids packedarray)
// POST: The first ID is always 0
template <typename T>
class PackedArray : public PackedArrayBase {
public:
	PackedArray(){
		mObjects.accommodate(MAX_OBJECTS);
		clear();
	}

	~PackedArray(){

	}

	bool has(ID id) {
		Index &in = mIndices[id & INDEX_MASK];
		return in.id == id && in.index != USHRT_MAX;
	}

	T& lookup(ID id) {
		return mObjects.get(mIndices[id&INDEX_MASK].index);
	}

	// Add a new object 
	// by optionally copying a prototype
	ID add(const T& proto = T()) {
		Index &in = mIndices[mFreelistDequeue];
		mFreelistDequeue = in.next;
		// NB: id is now incremented on entity removal
		// in.id += NEW_OBJECT_ID_ADD;		
		in.index = mNumObjects++;
		mObjects.set(in.index, proto);
		T &o = mObjects.get(in.index);
		// TODO: Do we need to call reset?
		// Call system::reset(id) 
		// o.reset();
		// o = proto;
		o.id = in.id;
		return o.id;
	}

	// Remove an object from this packed array
	// TODO: Don't forget to cleanup object from its systems
	// // this->cleanup(id);
	void remove(ID id) {
		Index &in = mIndices[id&INDEX_MASK];
		T &o = mObjects.get(in.index);
		// increment the version number to avoid ID conflicts
		in.id += NEW_OBJECT_ID_ADD;
		// NB: struct copy here
		// Copy object from end to here
		// TODO: Should do a move instead?
		o = mObjects.get(mNumObjects - 1);
		// Call destructor of object at end
		// Just in case it needs to clean up

		// XXX: This crashes the system when picking up an entity (sprite destructor?)
		mObjects.get(mNumObjects - 1).~T();
		mNumObjects--;

		mIndices[o.id&INDEX_MASK].index = in.index;
		in.index = USHRT_MAX;
		mIndices[mFreelistEnqueue].next = id&INDEX_MASK;
		mFreelistEnqueue = id&INDEX_MASK;
	}

	StaticArray<T>& objects(){
		return mObjects;
	}

	unsigned int size(){
		return mNumObjects;
	}

	void clear(){
		mNumObjects = 0;
		for (unsigned i = 0; i < MAX_OBJECTS; ++i) {
			mIndices[i].id = i;
			mIndices[i].next = i + 1;
			mIndices[i].index = USHRT_MAX;
		}
		mFreelistDequeue = 0;
		mFreelistEnqueue = MAX_OBJECTS - 1;
	}

protected:
	static const int MAX_OBJECTS = 0x10000; //  0xffff;
	static const int INDEX_MASK = 0xffff;
	static const int NEW_OBJECT_ID_ADD = 0x10000;

	using uint16 = unsigned short;
	struct Index {
		ID id;
		uint16 index;
		uint16 next;
	};

	unsigned int mNumObjects;
	StaticArray<T> mObjects;
	Index mIndices[MAX_OBJECTS];

	uint16 mFreelistEnqueue;
	uint16 mFreelistDequeue;
};


#endif

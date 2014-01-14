#ifndef INVENTORY_H
#define INVENTORY_H
#include "component.h"
#include <array>
#include <sstream>

static const int MAX_ITEMS = 64;
enum Item {
	SWORD,
	AXE,
	POTION,
	ARROW
};

std::ostream& operator<<(std::ostream& oss, Item item);

struct Inventory: public Component<Inventory> {
	static const char* Name(){ return "Inventory"; }

	struct ItemAndCount {
		Item item;
		int count;
		ItemAndCount() :count(0){}
		ItemAndCount(Item item) :item(item), count(1){}
		ItemAndCount(Item item, int count) :item(item), count(count){}

	};
	std::array<ItemAndCount, MAX_ITEMS> items;

	Inventory(std::initializer_list<ItemAndCount> contents = std::initializer_list<ItemAndCount>()){
		items.fill(ItemAndCount());
		std::copy(contents.begin(), contents.end(), items.begin());
	}

	std::string what() {
		std::ostringstream oss;
		oss << "inventory {";
		for (auto item : items){
			if (item.count>0){
				oss << item.item;
				if (item.count > 1) oss << " (" << item.count << "), ";
				else oss << ", ";
			}
		}
		oss << "}";
		return oss.str();
	}

	ID id;
	ID entity;
};



#endif

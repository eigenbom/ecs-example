#include "inventory.h"
#include <cassert>

std::ostream& operator<<(std::ostream& oss, Item item){
	switch (item){
	case Item::SWORD: oss << "Sword"; break;
	case Item::AXE: oss << "Axe"; break;
	case Item::POTION: oss << "Potion"; break;
	case Item::ARROW: oss << "Arrow"; break;
	}
	return oss;
}


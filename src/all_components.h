#ifndef ALL_COMPONENTS_H
#define ALL_COMPONENTS_H

#include "transform.h"
#include "health.h"
#include "inventory.h"
#include "description.h"
#include "physics.h"

template <typename... Args> struct TypeList { static const int NUM = sizeof...(Args); };
using ComponentTypeList = TypeList<Transform, Health, Inventory, ShortDescription, Description, Physics>;
static const int NUM_COMPONENTS = ComponentTypeList::NUM;

#endif
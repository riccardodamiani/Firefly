#ifndef ENTITY_H
#define ENTITY_H

#include "engine_exports.h"
typedef unsigned long long EntityName;

ENGINE_API constexpr const EntityName DecodeName(const char* name);

#endif
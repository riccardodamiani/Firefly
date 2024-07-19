#ifndef GAME_ENGINE_STRUCTS_H
#define GAME_ENGINE_STRUCTS_H

#include <mutex>
#include <shared_mutex>

typedef unsigned long long EntityName;
typedef std::shared_mutex RWLock;
typedef std::unique_lock< RWLock >  WriteLock;
typedef std::shared_lock< RWLock >  ReadLock;

enum class GameEvent {
	NO_EVENT,
	GAME_QUIT
};
enum class TransformPivotPoint {
	PARENT_CENTER,
	OBJECT_CENTER
};

#endif
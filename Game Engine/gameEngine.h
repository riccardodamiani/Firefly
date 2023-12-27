#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include "structures.h"
#include "gui.h"
#include "input.h"
#include "multithreadManager.h"

#include <vector>
#include <mutex>
#include <shared_mutex>
#include <map>
#include <atomic>
#include <random>

class GameObject;
class Input;
class GlobalVariable;
class GameEngine;
class Int;
class UInt;
class Bool;
class Double;
class Vector2;
class Void_Ptr;
class Variable;
class Barrier;
class LightObject;
class Scene;
class GUI_Element;

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

class GameEngine {
	enum class GameEngineRequestType {
		LOAD_SCENE,
		REGISTER_GAME_OBJECT,
		REGISTER_LIGHT_OBJECT,
		DESTROY_GAME_OBJECT,
		DESTROY_ALL
	};

	typedef struct gameObjectData {
		EntityName name;
		GameObject* obj;
	}GameObjectData;

	typedef struct requestData {
		GameObjectData objectData;
		Scene *scene;
		GameEngineRequestType requestType;
	}RequestData;

	struct UpdateHelperData {
		std::vector <GameObjectData>* objects;
		double elapsedTime;
	};
	struct DrawHelperData {
		std::vector <GameObjectData>* objects;
		double maxRenderRadius;
		vector2 cameraPos;
	};
	struct PhysicsHelperData {
		double timeElapsed;
		int threads;
	};
public:
	GameEngine();
	void GameEngine_Start(void);

	uint32_t posToZone(vector2 pos);
	int zoneDistance(uint32_t zone1, uint32_t zone2);
	GameObject* FindGameObject(EntityName name);
	EntityName RegisterGameObject(GameObject* obj, EntityName name);
	EntityName RegisterLightObject(LightObject* obj, EntityName name);
	std::vector <LightObject*>* GetLightObjects();
	void DestroyGameObject(EntityName name);
	void CreateScene(Scene *scene);
	void LoadScene(int sceneId);
	void Quit();

	Int* AllocGlobalVariable_Int(EntityName varName, long initVal);
	UInt* AllocGlobalVariable_UInt(EntityName varName, unsigned long initVal);
	Double* AllocGlobalVariable_Double(EntityName varName, double initVal);
	Vector2* AllocGlobalVariable_Vector2(EntityName varName, vector2 initVal);
	Void_Ptr* AllocGlobalVariable_Ptr(EntityName varName, void *initVal);
	Bool* AllocGlobalVariable_Bool(EntityName varName, bool initVal);
	Variable *GetGlobalVariable(EntityName varName);
	void DestroyGlobalVariable(EntityName varName);

	EntityName GenerateRandomName(void);

	vector2 MousePosition();
	vector2 LastClickPosition();
	void updateMouse();
	
	double GetRenderFPS();
	double GetGameFPS();
	void SetGameFPS(double gameFps);

	//internal use
	void _GuiListener(GUI_Element* element, GuiAction action);
private:
	void mainThread();
	void gameThread();

	double limit_fps(double elapsedTime, double maxFPS);
	void ThrowTheGarbage();

	void LoadScene_Internal();
	void ChangeScene_Internal(Scene* scene);
	void ClearGameObjects(void);
	void ClearGameObjects_Internal(void);
	void FreeAllGlobalVars(void);

	void PollRequests(double timeLeft);
	void RegisterGameObject_Internal(GameObject* obj, EntityName name);
	void RegisterLightObject_Internal(GameObject* obj, EntityName name);
	void DestroyGameObject_Internal(EntityName name);
	std::pair <bool, int> FindObject_Internal(EntityName name);

	//helper routines
	static void animation_helper_routine(int start_index, int end_index, void* args);
	static void pre_update_helper_routine(int start_index, int end_index, void* args);
	static void update_helper_routine(int start_index, int end_index, void* args);
	static void post_update_helper_routine(int start_index, int end_index, void* args);
	static void draw_helper_routine(int start_index, int end_index, void* args);
	static void physics_helper_routine(int start_index, int end_index, void* args);

	std::vector <GameObjectData> _objects;
	std::vector <GameObjectData> _lightObj;
	std::atomic <int> _obj_vect_size;
	std::vector < RequestData> _requests;
	std::vector <std::pair <GameObject*, int>> _garbageCollector;
	std::map <EntityName, Variable*> globalVars;

	std::atomic <vector2> _mousePosition;
	std::atomic <vector2> _lastClickPosition;

	//std::mutex object_vector_mutex;
	RWLock object_vector_mutex;		//read write lock for the object vector
	std::mutex global_var_mutex;
	std::mutex scene_loading_mutex;
	std::mutex request_mutex;
	std::mutex scene_mutex;
	
	std::mutex sync_render_mutex;
	std::condition_variable sync_cv;
	bool sync_state = false;

	std::atomic <double> zone_size;

	std::atomic <double> renderFPS;
	std::atomic <double> gameFPS;

	std::atomic <double> renderCurrentFPS;
	std::atomic <double> gameCurrentFPS;

	std::atomic <bool> lockRenderFPS;
	std::atomic <bool> lockGameFPS;

	std::atomic <double> gameSpeed;
	std::atomic <bool> gameRunning;
	std::atomic <bool> _sceneReady;
	std::atomic <bool> _freeingScene;
	std::atomic <GameEvent> _lastGameEvent;
	std::vector <Scene *> scenes;
	Scene* currentScene;

	//for random name generation
	std::random_device rd;
	std::mt19937_64 *generator;
	std::uniform_int_distribution<long long unsigned>* distribution;

	MultithreadManager *_helperManager;
	int _helperCount;
	Barrier * _syncBarrier;
};

#endif


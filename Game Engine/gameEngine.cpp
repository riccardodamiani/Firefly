#include "stdafx.h"
#include "gameEngine.h"
#include "gameObject.h"
#include "graphics.h"
#include "structures.h"
#include "game.h"
#include "input.h"
#include "variables.h"
#include "audio.h"
#include "transform.h"
#include "multithreadManager.h"
#include "barrier.h"
#include "platform.h"
#include "lightObject.h"
#include "scene.h"

#include <chrono>
#include <thread>
#include <string.h>
#include <malloc.h>
#include <shared_mutex>

typedef std::shared_mutex RWLock;
typedef std::unique_lock< RWLock >  WriteLock;
typedef std::shared_lock< RWLock >  ReadLock;

GameEngine::GameEngine(){

	zone_size = 10.0;

	renderFPS = 50;	//fixed frame rate
	gameFPS = 400;
	lockRenderFPS = true;
	lockGameFPS = true;

	gameSpeed = 1;
	gameRunning = true;

	_sceneReady = false;
	_freeingScene = false;

	currentScene = nullptr;
	sync_state = true;		//unlock the draw thread in the first frame

	_syncBarrier = new Barrier(2);
	//int hardware_threads_count = std::thread::hardware_concurrency();
	int hardware_count = GetCoresCount();
	if (hardware_count == 0) {		//failed to get the core count
		_helperCount = 1;
	}
	else {
		_helperCount = hardware_count - 1;
	}

	generator = new std::mt19937_64(rd());
	distribution = new std::uniform_int_distribution<long long unsigned>(1, 0xFFFFFFFFFFFFFFFF);
}


void GameEngine::GameEngine_Start(void) {
	Game::Init();

	_helperManager = new MultithreadManager(_helperCount);

	std::thread t1(&GameEngine::gameThread, this);
	t1.detach();
	mainThread();
}


//close the application
void GameEngine::Quit() {
	exit(EXIT_SUCCESS);
}


//Allocate global variable in the game engine. 
//Returns the pointer to the new variable if it succeeds, nullptr otherwise.
//You schould not free the memory yourself, the game engine will take care of it.
Int* GameEngine::AllocGlobalVariable_Int(EntityName varName, long initVal) {
	std::lock_guard<std::mutex> guard(global_var_mutex);
	auto it = globalVars.find(varName);

	if (it == globalVars.end()) {
		Int* ref = new Int(initVal);
		globalVars[varName] = ref;
		return ref;
	}
	return (Int*)(*it).second;
}
UInt* GameEngine::AllocGlobalVariable_UInt(EntityName varName, unsigned long initVal) {
	std::lock_guard<std::mutex> guard(global_var_mutex);
	auto it = globalVars.find(varName);

	if (it == globalVars.end()) {
		UInt* ref = new UInt(initVal);
		globalVars[varName] = ref;
		return ref;
	}
	return (UInt*)(*it).second;
}
Double* GameEngine::AllocGlobalVariable_Double(EntityName varName, double initVal) {
	std::lock_guard<std::mutex> guard(global_var_mutex);
	auto it = globalVars.find(varName);

	if (it == globalVars.end()) {
		Double* ref = new Double(initVal);
		globalVars[varName] = ref;
		return ref;
	}
	return (Double*)(*it).second;
}
Void_Ptr* GameEngine::AllocGlobalVariable_Ptr(EntityName varName, void* initVal) {
	std::lock_guard<std::mutex> guard(global_var_mutex);
	auto it = globalVars.find(varName);

	if (it == globalVars.end()) {
		Void_Ptr* ref = new Void_Ptr(initVal);
		globalVars[varName] = ref;
		return ref;
	}
	return (Void_Ptr*)(*it).second;
}
Bool* GameEngine::AllocGlobalVariable_Bool(EntityName varName, bool initVal) {
	std::lock_guard<std::mutex> guard(global_var_mutex);
	auto it = globalVars.find(varName);

	if (it == globalVars.end()) {
		Bool* ref = new Bool(initVal);
		globalVars[varName] = ref;
		return ref;
	}
	return (Bool*)(*it).second;
}
Vector2* GameEngine::AllocGlobalVariable_Vector2(EntityName varName, vector2 initVal) {
	std::lock_guard<std::mutex> guard(global_var_mutex);
	auto it = globalVars.find(varName);

	if (it == globalVars.end()) {
		Vector2* ref = new Vector2(initVal);
		globalVars[varName] = ref;
		return ref;
	}
	return (Vector2*)(*it).second;
}


//Return a pointer to the variable. If the variable is not found the function returns nullptr.
Variable* GameEngine::GetGlobalVariable(EntityName varName) {

	std::lock_guard<std::mutex> guard(global_var_mutex);

	auto it = globalVars.find(varName);

	if (it != globalVars.end()) {
		Variable* ptr = (*it).second;
		return ptr;
	}
	return nullptr;
}

void GameEngine::DestroyGlobalVariable(EntityName varName) {

	std::lock_guard<std::mutex> guard(global_var_mutex);

	auto it = globalVars.find(varName);

	if (it != globalVars.end()) {
		delete (*it).second;
		globalVars.erase(it);
	}
}

void GameEngine::FreeAllGlobalVars(void) {
	std::lock_guard<std::mutex> guard(global_var_mutex);

	for (auto it = globalVars.begin(); it != globalVars.end(); it++) {
		delete (*it).second;
		globalVars.erase(it);
	}
}


EntityName GameEngine::GenerateRandomName(void) {
	return (*distribution)(*generator);
}

uint32_t GameEngine::posToZone(vector2 pos) {
	uint32_t zone = 0;
	uint16_t xzone = 0, yzone = 0;
	xzone |= (int)(pos.x / 10);
	yzone |= (int)(pos.y / 10);
	zone = (xzone << 16) | (yzone & 0xffff);
	return zone;
}

//return the number of zones two zones are apart.
//i.e. return 0 if they are the same zone, 1 if the zones are connected, 2 if they are two zones apart and so on
int GameEngine::zoneDistance(uint32_t zone1, uint32_t zone2) {
	int l1 = (int)(zone1 >> 16) - (int)(zone2 >> 16);
	int l2 = (int)(zone1 & 0xffff) - (int)(zone2 & 0xffff);
	int d = std::max(l1, l2);
	return d;
}

//remove a object from the objects vector and add it to the garbage collection
//this function is called from PollRequests() and run on the update thread
//it requires an exclusive mutex to protect the reading of the vector object
void GameEngine::DestroyGameObject_Internal(EntityName name) {
	if (name == 0) {
		return;
	}
	WriteLock w_lock(object_vector_mutex);

	auto [found, index] = FindObject_Internal(name);
	
	if (found) {
		GameObject* obj = _objects[index].obj;
		_PhysicsEngine->RemoveRigidbody(obj->GetRigidbody());

		_garbageCollector.push_back(std::pair <GameObject*, int>(obj, 10));		//the object will be destroyed in 10 frames
		_objects.erase(_objects.begin() + index);

		for (int i = 0; i < _lightObj.size(); i++) {		//delete the light object
			if (obj == _lightObj[i].obj) {
				_lightObj.erase(_lightObj.begin() + i);
			}
		}
	}
}

//create a request to destroy a game object
//requires a mutex to protect from concurrent request creation 
void GameEngine::DestroyGameObject(EntityName name) {

	if (name == 0) {
		return;
	}
	std::lock_guard <std::mutex> guard(request_mutex);
	
	RequestData data;
	data.objectData.name = name;
	data.requestType = GameEngineRequestType::DESTROY_GAME_OBJECT;
	_requests.push_back(data);
}

//it requires an exclusive mutex to protect the reading of the vector object
void GameEngine::RegisterGameObject_Internal(GameObject* obj, EntityName name) {
	
	if (name == 0) {
		name = _GameEngine->GenerateRandomName();
	}
	WriteLock w_lock(object_vector_mutex);

	auto [found, index] = FindObject_Internal(name);
	if (found) {
		return;
	}
	GameObjectData data;
	data.name = name;
	data.obj = obj;
	_objects.insert(_objects.begin() + index, data);
}

void GameEngine::RegisterLightObject_Internal(GameObject* obj, EntityName name) {
	if (name == 0) {
		name = _GameEngine->GenerateRandomName();
	}
	WriteLock w_lock(object_vector_mutex);

	auto [found, index] = FindObject_Internal(name);
	if (found) {
		return;
	}
	GameObjectData data;
	data.name = name;
	data.obj = obj;
	_objects.insert(_objects.begin() + index, data);

	_lightObj.push_back(data);		//insert light object
}

//create a request to register a game object. Return the name the object was registered as
//require a mutex to protect from concurrent request creation
EntityName GameEngine::RegisterGameObject(GameObject* obj, EntityName name) {

	if (obj == nullptr)
		return 0;

	if (name == 0) {
		name = _GameEngine->GenerateRandomName();
	}

	std::lock_guard <std::mutex> guard(request_mutex);
	RequestData data;
	data.objectData.name = name;
	data.objectData.obj = obj;
	data.requestType = GameEngineRequestType::REGISTER_GAME_OBJECT;
	_requests.push_back(data);

	return name;

}

EntityName GameEngine::RegisterLightObject(LightObject* obj, EntityName name) {
	if (obj == nullptr)
		return 0;

	if (name == 0) {
		name = _GameEngine->GenerateRandomName();
	}

	std::lock_guard <std::mutex> guard(request_mutex);
	RequestData data;
	data.objectData.name = name;
	data.objectData.obj = (GameObject *)obj;
	data.requestType = GameEngineRequestType::REGISTER_LIGHT_OBJECT;
	_requests.push_back(data);

	return name;
}

std::vector <LightObject*>* GameEngine::GetLightObjects() {
	std::vector <LightObject*>* vect = new std::vector <LightObject*>();
	std::vector <LightObject*>& ref = *vect;
	for (int i = 0; i < _lightObj.size(); i++) {
		ref.push_back((LightObject*)_lightObj[i].obj);
	}
	return vect;
}

//return a pointer to a gameObject. 
//Never store the pointer returned by this function since the object can be destroyed by other threads.
GameObject* GameEngine::FindGameObject(EntityName name) {

	ReadLock r_lock(object_vector_mutex);
	//std::lock_guard <std::mutex> guard(object_vector_mutex);
	
	auto [found, index] = FindObject_Internal(name);
	if (found) {
		return _objects[index].obj;
	}

	return nullptr;
}

//called only from multithread safe functions so it doesn't need a mutex
std::pair <bool, int> GameEngine::FindObject_Internal(EntityName name) {
	int lower = 0;
	int upper = _objects.size() - 1;
	int mid = lower + (upper - lower + 1) / 2;
	while (lower <= upper) {

		if (name == _objects[mid].name)
		{
			return { true, mid };
		}
		if (name < _objects[mid].name)
			upper = mid - 1;
		else
			lower = mid + 1;

		mid = lower + (upper - lower + 1) / 2;
	}

	return { false, mid };
}

//create requests to destroy every game object. This is called from PollRequests()
//that run in the update thread
void GameEngine::ClearGameObjects_Internal(void) {

	ReadLock r_lock(object_vector_mutex);
	for (int i = 0; i < _objects.size(); i++) {
		DestroyGameObject(_objects[i].name);
	}
}

//this function create a request to clear all game objects from memory
//this function is called from the logic thread when a new scene is loaded
//require a mutex for concurrent request creation
void GameEngine::ClearGameObjects(void) {

	std::lock_guard <std::mutex> guard(request_mutex);
	RequestData data;
	data.requestType = GameEngineRequestType::DESTROY_ALL;
	_requests.push_back(data);
}

//create a scene object. Are required all 4 parameters
void GameEngine::CreateScene(Scene *scene) {
	if (scene == nullptr) {
		return;
	}
	unsigned int id = scene->getID();
	std::lock_guard <std::mutex> guard(scene_mutex);
	for (int i = 0; i < scenes.size(); i++) {
		if (scenes[i]->getID() == id) {		//already present
			return;
		}
	}
	this->scenes.push_back({ scene });
}

//request the game engine to load a scene.
void GameEngine::LoadScene(int sceneId) {

	std::lock_guard <std::mutex> guard(scene_mutex);
	std::lock_guard <std::mutex> guard1(request_mutex);
	RequestData data;
	for (int i = 0; i < scenes.size(); i++) {
		if (scenes[i]->getID() == sceneId) {		//set scene to load
			data.scene = scenes[i];
			data.requestType = GameEngineRequestType::LOAD_SCENE;
			_requests.push_back(data);
			_sceneReady = false;		//lock the logic thread
			return;
		}
	}
}

//call the scene specific gui listener
void GameEngine::_GuiListener(GUI_Element* element, GuiAction action) {
	if (element == nullptr)
		return;
	if (!_sceneReady || currentScene == nullptr)
		return;
	currentScene->gui_listener(element, action);
}

//Clear the game objects memory and call the deconstructor of the previous scene
//This function is called from the game thread (PollRequest()) as a new thread
void GameEngine::ChangeScene_Internal(Scene *newScene) {

	ClearGameObjects();
	//FreeAllGlobalVars();
	_GuiEngine->clearFocus();

	//disable scene lighting
	_graphicsEngine->EnableSceneLighting(false);

	//free current scene
	if(currentScene != nullptr)
		currentScene->onfree();

	currentScene = newScene;
	//currentScene->onload();		//load next scene
	
	_freeingScene = true;
}

//Load the new scene
//This function is called from the game thread (PollRequest()) as a new thread
void GameEngine::LoadScene_Internal() {

	if (currentScene == nullptr) {
		_sceneReady = false;
		_freeingScene = false;
		return;
	}
	
	currentScene->onload();		//load next scene
	currentScene->InitLoadingStateCalc();

	_freeingScene = false;
	_sceneReady = true;

}

//sleeps for the time needed to have a FPS. Returns the time it have sleeped
double GameEngine::limit_fps(double elapsedTime, double maxFPS) {
	if (elapsedTime >= (1.0 / maxFPS)) {
		return 0;
	}
	else {
		std::this_thread::sleep_for(std::chrono::microseconds((long long)(((1.0 / maxFPS) - elapsedTime) * 1000000.0)));
		return ((1.0 / maxFPS) - elapsedTime);
	}
}

//destroy the objects in the garbage collection after few frames from insertion. 
//the delay is needed to prevent the distruction of a object referenced somewhere in the game 
//(expecially due to the FindGameObject() function)
//there is no need to have a mutex in here since the only function that can add elements to the garbage collection
//is DestroyGameObject_Internal which is also called from the update thread.
void GameEngine::ThrowTheGarbage() {
	
	for (int i = 0; i < _garbageCollector.size(); i++) {
		_garbageCollector[i].second--;
		if (_garbageCollector[i].second < 0) {		//time to destroy the object
			delete _garbageCollector[i].first;
			_garbageCollector.erase(_garbageCollector.begin() + i);
			--i;
		}
	}
}


vector2 GameEngine::MousePosition() {
	return _mousePosition;
}

vector2 GameEngine::LastClickPosition() {
	return _lastClickPosition;
}

void GameEngine::updateMouse() {
	std::pair <int, int> click = _InputEngine->getLastClickPosition();
	std::pair <int, int> mouse = _InputEngine->getMousePosition();
	_lastClickPosition = _graphicsEngine->screenToSpace(click.first, click.second);
	_mousePosition = _graphicsEngine->screenToSpace(mouse.first, mouse.second);
}

double GameEngine::GetRenderFPS() {
	return renderCurrentFPS;
}

double GameEngine::GetGameFPS() {
	return gameCurrentFPS;
}

unsigned long GameEngine::GetTaskQueueLen() {
	return _requests.size();
}

void GameEngine::SetGameFPS(double fps){
	gameFPS = fps;
}

//all the calls to sdl libraries must be done from this thread
void GameEngine::mainThread() {
	double elapsedTime = 0;

	while (true) {

		auto startTime = std::chrono::high_resolution_clock::now();
		_InputEngine->beginNewFrame();

		//_syncBarrier->wait();	//syncs with the game thread

		_AudioEngine->PollRequests();

		if (_InputEngine->GetLastEvent() == InputEvent::CLOSE_WINDOW) {
			_lastGameEvent = GameEvent::GAME_QUIT;
		}

		_graphicsEngine->SwapScreenBuffersGraphics();
		_graphicsEngine->Flip();

		auto endTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed = endTime - startTime;
		elapsedTime = elapsed.count();	//elapsed time in seconds

		_graphicsEngine->PollRequests((1.0/renderFPS) - elapsedTime);	//handle some graphics requests

		endTime = std::chrono::high_resolution_clock::now();
		elapsed = endTime - startTime;
		elapsedTime = elapsed.count();	//elapsed time in seconds
		
		if (this->lockRenderFPS) elapsedTime += this->limit_fps(elapsedTime, this->renderFPS);		//limits the fps
		renderCurrentFPS = 1.0 / elapsedTime;
	}
}

void GameEngine::animation_helper_routine(int start_index, int end_index, void* args) {
	UpdateHelperData*data = (UpdateHelperData*)args;
	std::vector <GameObjectData>& vect = *data->objects;
	double elapsedTime = data->elapsedTime;
	
	for (int i = start_index; i < end_index; i++) {
		vect[i].obj->MainAnimationUpdate(elapsedTime);
	}
}

void GameEngine::pre_update_helper_routine(int start_index, int end_index, void* args) {
	UpdateHelperData* data = (UpdateHelperData*)args;
	std::vector <GameObjectData>& vect = *data->objects;
	double elapsedTime = data->elapsedTime;

	for (int i = start_index; i < end_index; i++) {
		vect[i].obj->mainPreUpdate(elapsedTime);
	}
}

void GameEngine::update_helper_routine(int start_index, int end_index, void* args) {
	UpdateHelperData* data = (UpdateHelperData*)args;
	std::vector <GameObjectData>& vect = *data->objects;
	double elapsedTime = data->elapsedTime;

	for (int i = start_index; i < end_index; i++) {
		vect[i].obj->mainUpdate(elapsedTime);
	}
}

void GameEngine::post_update_helper_routine(int start_index, int end_index, void* args) {
	UpdateHelperData* data = (UpdateHelperData*)args;
	std::vector <GameObjectData>& vect = *data->objects;
	double elapsedTime = data->elapsedTime;

	for (int i = start_index; i < end_index; i++) {
		vect[i].obj->mainPostUpdate(elapsedTime);
	}
}

void GameEngine::draw_helper_routine(int start_index, int end_index, void* args) {
	DrawHelperData* data = (DrawHelperData*)args;
	std::vector <GameObjectData>& vect = *data->objects;
	double maxRenderDistance = data->maxRenderRadius;
	vector2 camPos = data->cameraPos;

	for (int i = start_index; i < end_index; i++) {
		double maxScale = std::max(vect[i].obj->transform.scale.x(), vect[i].obj->transform.scale.y()) * 1.5 / 2.0;
		vector2 objPos = vect[i].obj->transform.position;
		double distance = sqrt((camPos.x - objPos.x) * (camPos.x - objPos.x) + (camPos.y - objPos.y) * (camPos.y - objPos.y));
		if (distance - maxScale <= maxRenderDistance)
			vect[i].obj->mainDraw();
	}
}

void GameEngine::physics_helper_routine(int start_index, int end_index, void* args) {

	PhysicsHelperData* d = (PhysicsHelperData*)args;
	_PhysicsEngine->UpdatePhysics(d->timeElapsed, start_index, d->threads);

}

//game thread. From this thread are called all method of the game objects related to the game logic.
//Also it handles the garbage collector of the game engine and calls the 
//function that handle all game engine requests.
void GameEngine::gameThread() {
	double elapsedTime = 0;

	while (true) {
		auto startTime = std::chrono::high_resolution_clock::now();
		elapsedTime *= this->gameSpeed;	//modify game speed

		//_syncBarrier->wait();		//syncs with the render thread

		ThrowTheGarbage();
		updateMouse();

		UpdateHelperData data; data.objects = &_objects; data.elapsedTime = elapsedTime;
		_helperManager->startWork(_objects.size(), animation_helper_routine, &data);

		if (_sceneReady) {
			currentScene->scene_callback(_lastGameEvent, elapsedTime);	//scene callback routine
		}
		else {
			if (_freeingScene) {		//is in the process of freeing the previous scene
				//if all objects in the prevous scene were destroyed
				if (_requests.size() == 0) {
					LoadScene_Internal();
				}

			}
			if (_lastGameEvent == GameEvent::GAME_QUIT) {
				_GameEngine->Quit();
			}
		}

		_helperManager->Wait();	//wait until the end of animation update

		_helperManager->startWork(_objects.size(), pre_update_helper_routine, &data);	//start object pre update (translation update for rigid bodies)
		_helperManager->Wait();

		_helperManager->startWork(_objects.size(), update_helper_routine, &data);	//start object update
		_helperManager->Wait();

		_helperManager->startWork(_objects.size(), post_update_helper_routine, &data);	//start post update (parenting and stuff)
		_GuiEngine->beginNewFrame();	//handle gui events
		_helperManager->Wait();

		//save the current state of the camera for rendering to avoid gliches when a object is parented to the camera
		GameObject* camera = this->FindGameObject(DecodeName("MainCamera"));
		if (camera == nullptr) {
			_graphicsEngine->updateRenderCamera(false, { 0, 0 }, { 0, 0 }, 0);
		}
		else {
			vector2 camScale = camera->transform.scale;
			vector2 camPos = camera->transform.position;
			DrawHelperData d_data;
			d_data.objects = &_objects;
			d_data.maxRenderRadius = sqrt(camScale.x * camScale.x / 4.0 + camScale.y * camScale.y / 4.0);
			d_data.cameraPos = camera->transform.position;

			_helperManager->startWork(_objects.size(), draw_helper_routine, &d_data);		//start draw
			_helperManager->Wait();

			_graphicsEngine->updateRenderCamera(true, camPos, camScale, camera->transform.rotation);

			_graphicsEngine->SwapScreenBuffersPhysics();	//swap buffers

		}

		//update physics
		PhysicsHelperData p_data = { elapsedTime, _helperCount };
		_PhysicsEngine->NewPhysicsFrame(elapsedTime);
		_helperManager->startWork(_helperCount, physics_helper_routine, &p_data);
		_helperManager->Wait();
		_PhysicsEngine->ResolvePhysics(elapsedTime);


		auto endTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed = endTime - startTime;
		elapsedTime = elapsed.count();	//elapsed time in seconds

		PollRequests((1.0/gameFPS) - elapsedTime);		//handle game engine requests

		endTime = std::chrono::high_resolution_clock::now();
		elapsed = endTime - startTime;
		elapsedTime = elapsed.count();	//elapsed time in seconds
		
		if (this->lockGameFPS) elapsedTime += this->limit_fps(elapsedTime, this->gameFPS);		//limits the fps
		gameCurrentFPS = 1.0 / elapsedTime;
	}

}

void GameEngine::PollRequests(double timeLeft) {
	
	int block = 0;
	auto startTime = std::chrono::high_resolution_clock::now();

	request_mutex.lock();
	if (timeLeft <= 0) {
		timeLeft = 0.001;		//bonus of 1ms to handle some requests when it's busy
	}
		
	while (timeLeft > 0 && _requests.size() > 0) {
		RequestData request = _requests[0];
		_requests.erase(_requests.begin());
		
		switch (request.requestType) {
		case GameEngineRequestType::LOAD_SCENE:
		{
			//have to unlock the mutex every time since LoadScene_Internal locks request_mutex
			request_mutex.unlock();
			Scene* scene = request.scene;
			ChangeScene_Internal(scene);
			//std::thread t(&GameEngine::LoadScene_Internal, this, scene);
			//t.detach();
			//LoadScene_Internal(request.sceneData);
			request_mutex.lock();
			break;
		}

		case GameEngineRequestType::REGISTER_GAME_OBJECT:
		{
			RegisterGameObject_Internal(request.objectData.obj, request.objectData.name);
			break;
		}
		case GameEngineRequestType::REGISTER_LIGHT_OBJECT:
		{
			RegisterLightObject_Internal(request.objectData.obj, request.objectData.name);
			break;
		}
		case GameEngineRequestType::DESTROY_GAME_OBJECT:
		{
			DestroyGameObject_Internal(request.objectData.name);
			break;
		}

		case GameEngineRequestType::DESTROY_ALL:
		{
			//have to unlock the mutex every time since ClearGameObjects_Internal locks request_mutex
			request_mutex.unlock();		//have to unlock the mutex every time 
			ClearGameObjects_Internal();
			request_mutex.lock();
			break;
		}
		};

		block++;
		if (block > 10) {		//check the clock every few requests
			request_mutex.unlock();		//leave some time for other threads
			request_mutex.lock();
			auto endTime = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> elapsed = endTime - startTime;
			double elapsedTime = elapsed.count();	//elapsed time in seconds
			timeLeft -= elapsedTime;
			block = 0;
		}
	}

	request_mutex.unlock();

}


#include "scene.h"
#include "audio.h"
#include "graphics.h"

Scene::Scene(unsigned int id) {
	_id = id;
	_loadingPerc = 0;
}

//This function is called every game frame.
//You should re-define this fuction in you derived scene object.
void Scene::scene_callback(GameEvent event, double timeElapsed) {

}

//This function is called when an action happened on a gui element of the 
//current scene. 
//You should re-define this fuction in you derived scene object.
void Scene::gui_listener(GUI_Element* element, GuiAction action) {

}

//This function is called once when the scene is loaded.
//You should re-define this fuction in you derived scene object.
//In here you should create all scene related objects and graphics.
void Scene::onload() {

}

//This function is called once when your scene is freed.
// You should re-define this fuction in you derived scene object.
//In here you should free all scene releted memory and graphics.
void Scene::onfree() {

}

unsigned int Scene::getID() {
	return _id;
}

//return the percentage of scene loading. Not that tis is just an appoximation
float Scene::GetLoadingState() {
	unsigned long current_tasks = GraphicsEngine::getInstance().GetTaskQueueLen() + GameEngine::getInstance().GetTaskQueueLen() + AudioEngine::getInstance().GetTaskQueueLen();
	float current_percentage = (((float)_startingTasks - current_tasks) / _startingTasks) * 100.0;

	//avoid loading going backwards
	if (current_percentage >= _loadingPerc)
		_loadingPerc = current_percentage;

	return _loadingPerc;
}

//initialize the calculation for the loading state of a scene
//this is called by the game engine after onLoad() function
void Scene::InitLoadingStateCalc() {
	_startingTasks = GraphicsEngine::getInstance().GetTaskQueueLen() + GameEngine::getInstance().GetTaskQueueLen() + AudioEngine::getInstance().GetTaskQueueLen();
	_loadingPerc = 0;
}
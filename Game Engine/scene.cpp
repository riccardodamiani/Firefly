#include "stdafx.h"
#include "scene.h"

Scene::Scene(unsigned int id) {
	_id = id;
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
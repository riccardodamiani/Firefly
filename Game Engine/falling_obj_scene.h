#ifndef FALLING_SCENE_H
#define FALLING_SCENE_H

#include "scene.h"
#include "gameEngine.h"

class FallingObjScene : public Scene {
public:
	FallingObjScene(unsigned int id);
	void onload();
	void onfree();
	void scene_callback(GameEvent event, double timeElapsed);
	void gui_listener(GUI_Element* element, GuiAction action);
};

#endif

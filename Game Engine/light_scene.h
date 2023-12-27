#ifndef LIGHT_SCENE_H
#define LIGHT_SCENE_H

#include "scene.h"

class LightScene : public Scene {
public:
	LightScene(unsigned int id);
	void onload();
	void onfree();
	void scene_callback(GameEvent event, double timeElapsed);
	void gui_listener(GUI_Element* element, GuiAction action);
};

#endif

#ifndef FIREFLY_SCENE_H
#define FIREFLY_SCENE_H

#include "scene.h"
#include "gameEngine.h"
#include "audio_source.h"

#include <memory>

class FireflyScene : public Scene {
public:
	FireflyScene(unsigned int id);
	void onload();
	void onfree();
	void scene_callback(GameEvent event, double timeElapsed);
	void gui_listener(GUI_Element* element, GuiAction action);
};

#endif  //FIREFLY_SCENE_H

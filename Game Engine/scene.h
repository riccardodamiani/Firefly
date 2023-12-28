#ifndef SCENE_H
#define SCENE_H

#include "gui.h"
#include "gui_element.h"

class Scene {
public:
	[[deprecated]] Scene();	//always use the Scene(unsigned int) contructor
	Scene(unsigned int id);
	virtual void scene_callback(GameEvent event, double timeElapsed);
	virtual void gui_listener(GUI_Element* element, GuiAction action);
	virtual void onload();
	virtual void onfree();
	unsigned int getID();
	float GetLoadingState();
	void InitLoadingStateCalc();
private:
	unsigned int _id;
	float _loadingPerc;
	long _startingTasks;
};

#endif

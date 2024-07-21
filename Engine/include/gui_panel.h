#ifndef GUI_PANEL_H
#define GUI_PANEL_H

#include "gui_element.h"
#include "entity.h"


//class of gui element: panel
//a panel is a rectangular object. It is interactive (so it interact with the mouse click), but do not return eny event.
//can have a sprite (only one)
class GUI_Panel : public GUI_Element {
public:
	GUI_Panel(EntityName objectName, unsigned int elementCode, EntityName textureName, vector2 pos, vector2 rect, int layer);
	void update(double elapsedTime);

	bool isPanel();
};

#endif


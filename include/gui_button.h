#ifndef GUI_BUTTON_H
#define GUI_BUTTON_H

#include "gui_element.h"
#include "entity.h"
#include "engine_exports.h"

class ENGINE_API GUI_Button : public GUI_Element {
public:
	GUI_Button(EntityName objectName, EntityName textureName, unsigned int elementCode, vector2 pos, vector2 rect, int layer);
	void update(double elapsedTime);
	bool negateStatus();
	bool isButton();
private:

};

#endif


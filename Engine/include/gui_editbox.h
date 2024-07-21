#ifndef GUI_EDITBOX_H
#define GUI_EDITBOX_H

#include "gui_element.h"
#include "variables.h"
#include "entity.h"


#include <mutex>
class Sprite;

class GUI_Editbox : public GUI_Element {
public:
	GUI_Editbox(EntityName objName, unsigned int elementCode, EntityName textureName, std::string hintText, EntityName textFontAlias, EntityName hintFontAlias,
		vector2 pos, vector2 scale, int layer);
	~GUI_Editbox();
	void update(double elapsedTime);
	void setActive(bool active);
	std::string getText();
	void applyAction(GuiAction mouseAction);
	void setText(std::string text);
	void setHintText(std::string hintText);
	void DigitOnly(bool digitOnly);		//true accept only numbers as input
	void insertText(std::string);
	void backspace();
	void cancel();
	void incrementCursorPos();
	void decrementCursorPos();

	bool isEditbox();
private:
	GUI_Text* _text;
	GUI_Text* _hintText;
	Int _cursor;
	Bool _isNumerical;
	std::mutex text_related_mutex;
};

#endif

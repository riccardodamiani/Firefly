#ifndef GUI_TEXT_H
#define GUI_TEXT_H

#include "gui_element.h"
#include "entity.h"
#include "engine_exports.h"
#include "structures.h"

class ENGINE_API GUI_Text : public GUI_Element {
public:
	GUI_Text(EntityName objectName, unsigned int elementCode, EntityName atlasName, vector2 pos, vector2 scale, int layer);
	~GUI_Text();
	void update(double elapsedTime);
	void draw();
	void setText(std::string text); 
	void setTextAtlas(EntityName atlasName);
	void appendText(std::string textToAppend);
	int insertText(std::string text, int charIndex);
	std::string getText();
	int getTextLen();
	void deleteChar(int charIndex);
	vector2 GUI_Text::getCursorPosition(int cursorIndex);
	void ShowCursor(bool show);
	void SetCursorPos(int pos);

	bool isText();
private:
	std::string _text;
	EntityName _atlasName;
	int _cursorPos;
	bool _showCursor;
	double _timeToCursorBlink;
	bool _cursorBlink;
	int _minCharNum;
};

#endif

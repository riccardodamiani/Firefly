#ifndef GUI_DROPLIST_H
#define GUI_DROPLIST_H

//class of gui element: droplist
#include "gui_panel.h"
#include "gui_element.h"
#include "entity.h"

#include <vector>

class GUI_Droplist : public GUI_Element {
public:
	GUI_Droplist(EntityName objectName, unsigned int elementCode, EntityName mainTexture, EntityName background_texture,
		EntityName fontAlias, double textStartOffset, vector2 pos, vector2 scale, int layer);
	~GUI_Droplist();
	void update(double elapsedTime);
	void applyAction(GuiAction mouseAction);
	void addEntry(std::string name);
	void addEntryList(std::vector <std::string>& nameList);
	void selectByName(std::string name);
	void selectById(int id);
	int getSelectedId();
	void closeDroplist();
	void openDroplist();
	std::string getSelectedName();
	void SetActive(bool active);

	bool isDroplist();
private:
	EntityName _fontAlias;
	int _selectedId;
	int _firstElementInView;
	int _hoverElementId;
	int _maxViewableElements;
	int _elementToHL;
	vector2 _mouseClickPosition;
	std::vector <std::string> _elements;
	std::vector <GUI_Text*> _texts;
	GUI_Panel* bkPanel;
	double _textStartOffset;
};

#endif

#pragma once

#include "entity.h"
#include "gui_element.h"
#include "gui_droplist.h"
#include "structures.h"
#include "gui_text.h"
#include "gameEngine.h"

//constructor
GUI_Droplist::GUI_Droplist(EntityName objectName, unsigned int elementCode, EntityName mainTexture, EntityName background_texture,
	EntityName fontAlias, double textStartOffset, vector2 pos, vector2 scale, int layer){
	
	transform.position = pos;
	transform.scale = scale;
	setLayer(layer);

	_texture = new Sprite(layer, mainTexture);
	bkPanel = new GUI_Panel((EntityName)0, -1, background_texture, { 0, 0 }, { 1, 1 }, layer);
	bkPanel->SetVisible(false);

	this->active = true;
	this->visible = true;
	this->_status = false;
	this->_isPressed = false;
	this->_isMouseOn = false;
	this->_fontAlias = fontAlias;
	this->elementCode = elementCode;
	_textStartOffset = textStartOffset;

	_selectedId = -1;
	_firstElementInView = 0;
	_elementToHL = -1;

	RegisterObject(objectName);
}

GUI_Droplist::~GUI_Droplist() {
	for (int i = 0; i < _texts.size(); i++) {
		delete _texts[i];
	}
	delete bkPanel;
	_texts.clear();
	_elements.clear();
	
}

void GUI_Droplist::addEntry(std::string entry) {
	std::lock_guard <std::mutex> guard(update_mutex);
	this->_elements.push_back(entry);
	this->_texts.push_back(new GUI_Text(0, -1, this->_fontAlias, { 0, 0 }, transform.scale, getLayer() + 1));
	this->_texts.back()->setText(entry);
	this->_texts.back()->SetVisible(false);
}

void GUI_Droplist::addEntryList(std::vector <std::string>& nameList) {
	std::lock_guard <std::mutex> guard(update_mutex);
	for (int i = 0; i < nameList.size(); i++) {
		this->addEntry(nameList[i]);
	}
}


void GUI_Droplist::selectByName(std::string name) {

	if (_status)
		closeDroplist();

	std::lock_guard <std::mutex> guard(update_mutex);
	for (int i = 0; i < this->_elements.size(); i++) {
		if (this->_elements[i] == name) {
			if (_selectedId >= 0 && _selectedId < this->_elements.size()) {
				_texts[_selectedId]->SetVisible(false);
			}
			_selectedId = i;
			//set the selected object
			_texts[i]->transform.position = { transform.position.x() + _textStartOffset, transform.position.y() };
			_texts[i]->SetVisible(true);
		}
	}
}


void GUI_Droplist::selectById(int id) {
	
	if(_status)
		closeDroplist();

	std::lock_guard <std::mutex> guard(update_mutex);
	if (id < this->_elements.size() && id >= 0) {
		if (_selectedId >= 0 && _selectedId < this->_elements.size()) {
			_texts[_selectedId]->SetVisible(false);
		}
		_selectedId = id;
		//set the selected object
		_texts[id]->transform.position = { transform.position.x() + _textStartOffset, transform.position.y() };
		_texts[id]->SetVisible(true);
	}
}


int GUI_Droplist::getSelectedId() {
	return this->_selectedId;
}


std::string GUI_Droplist::getSelectedName() {
	std::lock_guard <std::mutex> guard(update_mutex);
	if (this->_selectedId < this->_elements.size() && _selectedId >= 0) {
		return this->_elements[this->_selectedId];
	}
	return "";
}


void GUI_Droplist::SetActive(bool active) {
	
	this->active = active;
	this->_status = false;
	this->_isPressed = false;
	this->_isMouseOn = false;
	this->_firstElementInView = 0;
	if (!active) {
		closeDroplist();
	}
}

void GUI_Droplist::closeDroplist() {
	std::lock_guard <std::mutex> guard(update_mutex);
	this->_status = false;
	for (int i = 0; i < _texts.size(); i++) {
		if (i != _selectedId)
			_texts[i]->SetVisible(false);
	}
	bkPanel->SetVisible(false);
}

void GUI_Droplist::openDroplist() {
	std::lock_guard <std::mutex> guard(update_mutex);
	_status = true;

	vector2 basePos = { transform.position.x(), transform.position.y() - transform.scale.y() };
	double Yincr = transform.scale.y();
	int actualPos = 0;
	for (int i = 0; i < _texts.size(); i++) {
		if (i == _selectedId)	//do nothing with the displayed text
			continue;
		_texts[i]->SetVisible(true);
		vector2 textPos = { basePos.x + _textStartOffset, basePos.y - Yincr * actualPos };
		_texts[i]->transform.position = textPos;
		actualPos++;
	}
	bkPanel->SetVisible(true);
	long panelEntries = (_selectedId < 0 ? _texts.size() : std::max<long>(_texts.size() - 1, 0));
	vector2 panelScale = { transform.scale.x(), panelEntries * transform.scale.y() };
	bkPanel->transform.scale = panelScale;
	vector2 panelPos = { transform.position.x(),  transform.position.y() - (transform.scale.y() / 2.0) - (panelScale.y / 2.0) };
	bkPanel->transform.position = panelPos;
}

void GUI_Droplist::applyAction(GuiAction mouseAction) {
	switch (mouseAction) {
		case GuiAction::LEFT_BUTTON_UP:		//something got clicked
		{
			this->_isPressed = false;
			if (_status) {		//drop list is already open
				_status = false;
				int index = (transform.position.y() + transform.scale.y() / 2.0 - _mouseClickPosition.y) / transform.scale.y();
				if (index == 0) {			//close the droplist
					closeDroplist();
				}
				else {	//clicked on a item
					--index;
					if (_selectedId >= 0 && index >= _selectedId) index++;
					//set the selected object
					{
						std::lock_guard <std::mutex> guard(update_mutex);
						_texts[index]->transform.position = { transform.position.x() + _textStartOffset, transform.position.y() };
						_texts[index]->SetVisible(true);
						_selectedId = index;
					}
					//close the droplist
					closeDroplist();
				}
			}
			else {		//drop list is closed
				_status = true;
				openDroplist();
			}
			break;
		}

		case GuiAction::LEFT_BUTTON_DOWN:
			this->_isPressed = true;
			break;

		case GuiAction::MOUSE_MOVED_OVER:		//here there shouldn't be a break
		case GuiAction::MOUSE_HOVERING:
			this->_isMouseOn = true;
			break;

	}
}

void GUI_Droplist::update(double timeElapsed) {
	vector2 mousePos;
	long panelEntries = (_selectedId < 0 ? _texts.size() : std::max<long>(_texts.size() - 1, 0));

	if (this->_isMouseOn) {
		vector2 thisPos = transform.position;
		vector2 thisRect = transform.scale;
		mousePos = GameEngine::getInstance().MousePosition();
		if (!(mousePos.x >= thisPos.x - thisRect.x / 2.0 &&	//out of the rectangle
			mousePos.x <= thisPos.x + thisRect.x / 2.0 &&
			mousePos.y >= thisPos.y - thisRect.y / 2.0 - ( _status?(panelEntries *transform.scale.y()):0.0 ) &&
			mousePos.y <= thisPos.y + thisRect.y / 2.0)) {
			GUIEngine::getInstance().RegisterGuiAction(GuiAction::MOUSE_MOVED_OUT, this);
			this->_isMouseOn = false;
			return;
		}
	}

	if (this->_isPressed) {		//droplist is pressed
		if (InputEngine::getInstance().wasMouseButtonReleased(SDL_BUTTON_LEFT)) {		//if the button was released

			mousePos = GameEngine::getInstance().MousePosition();
			vector2 thisPos = transform.position;
			vector2 thisRect = transform.scale;

			//if mouse left button was released inside the droplist rectangle
			if (mousePos.x >= thisPos.x - thisRect.x / 2.0 &&
				mousePos.x <= thisPos.x + thisRect.x / 2.0 &&
				mousePos.y >= thisPos.y - thisRect.y / 2.0 - (_status ? (panelEntries * transform.scale.y()) : 0.0) &&
				mousePos.y <= thisPos.y + thisRect.y / 2.0) {
				this->_isPressed = false;
				GUIEngine::getInstance().RegisterGuiAction(GuiAction::LEFT_BUTTON_UP, this);
				_mouseClickPosition = mousePos;		//save the position of the click to find the element pressed
				return;
			}
			else {		//if was released outside
				this->_isPressed = false;
				return;
			}
		}
	}
	else {		//droplist is not pressed
		if (InputEngine::getInstance().wasMouseButtonPressed(SDL_BUTTON_LEFT)) {

			mousePos = GameEngine::getInstance().MousePosition();
			vector2 thisPos = transform.position;
			vector2 thisRect = transform.scale;

			//if mouse left button was clicked inside the droplist rectangle
			if (mousePos.x >= thisPos.x - thisRect.x / 2.0 &&
				mousePos.x <= thisPos.x + thisRect.x / 2.0 &&
				mousePos.y >= thisPos.y - thisRect.y / 2.0 - (_status ? (panelEntries * transform.scale.y()) : 0.0) &&
				mousePos.y <= thisPos.y + thisRect.y / 2.0) {

				GUIEngine::getInstance().RegisterGuiAction(GuiAction::LEFT_BUTTON_DOWN, this);
				return;
			}
			else {		//if was clicked outside
				if (_status) {		//if droplist is open we need to close it
					closeDroplist();
					_status = false;
				}
			}
		}
		else {
			mousePos = GameEngine::getInstance().MousePosition();
			vector2 thisPos = transform.position;
			vector2 thisRect = transform.scale;

			//if mouse cursor is inside the droplist rectangle
			if (mousePos.x >= thisPos.x - thisRect.x / 2.0 &&
				mousePos.x <= thisPos.x + thisRect.x / 2.0 &&
				mousePos.y >= thisPos.y - thisRect.y / 2.0 - (_status ? (panelEntries * transform.scale.y()) : 0.0) &&
				mousePos.y <= thisPos.y + thisRect.y / 2.0) {

				if (this->_isMouseOn) {
					GUIEngine::getInstance().RegisterGuiAction(GuiAction::MOUSE_HOVERING, this);
					return;
				}
				else {
					GUIEngine::getInstance().RegisterGuiAction(GuiAction::MOUSE_MOVED_OVER, this);
					return;
				}
			}
		}
	}
	return;
}

bool GUI_Droplist::isDroplist() {
	return true;
}
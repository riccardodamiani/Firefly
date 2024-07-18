#include "entity.h"
#include "gui_element.h"
#include "gui_button.h"
#include "gui_panel.h"
#include "gui_text.h"
#include "structures.h"
#include "gameEngine.h"
#include "input.h"

GUI_Element::GUI_Element() {}

GUI_Element::~GUI_Element() {
	
}

//draws the element 
void GUI_Element::draw() {

}

void GUI_Element::setActive(bool active) {
	this->active = active;
	this->_isPressed = false;
}


//gets called every frame
void GUI_Element::update() {
	return;
}

 //return the element status
 bool GUI_Element::getStatus() {
	 return this->_status;
 }

 //return the unique code for this element
 unsigned int GUI_Element::GetElementCode() {
	 return elementCode;
 }

 void GUI_Element::setStatus(bool status) {

	 this->_status = status;
 }

void GUI_Element::applyAction(GuiAction mouseAction) {

	switch (mouseAction) {
	case GuiAction::LEFT_BUTTON_UP:
		this->_isPressed = false;
		break;
	case GuiAction::LEFT_BUTTON_DOWN:
		this->_isPressed = true;
		break;

	case GuiAction::MOUSE_MOVED_OVER:		//here there shouldn't be a break
	case GuiAction::MOUSE_HOVERING:
		this->_isMouseOn = true;
		break;

	case GuiAction::MOUSE_MOVED_OUT:
		this->_isMouseOn = false;
		this->_isPressed = false;
		break;
	}
}

 bool GUI_Element::isButton() {
	 return false;
 }

 bool GUI_Element::isPanel() {
	 return false;
 }

 bool GUI_Element::isSlider() {
	 return false;
 }

 bool GUI_Element::isDroplist() {
	 return false;
 }

 bool GUI_Element::isEditbox() {
	 return false;
 }

 bool GUI_Element::isText() {
	 return false;
 }




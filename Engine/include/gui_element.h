#ifndef GUI_ELEMENT_H
#define GUI_ELEMENT_H

#include "variables.h"
#include "gameObject.h"
#include "gui.h"


#include <mutex>

class GUI_Text;

class GUI_Element : public GameObject {
public:
	/*GUI_Element contructor are deprecated. Use Specific element constructor instead*/
	GUI_Element();
	~GUI_Element();
	virtual void setActive(bool active);
	bool getStatus();	//return the element status
	void setStatus(bool status);	//return the element status
	virtual void applyAction(GuiAction mouseAction);
	virtual void draw(void);
	virtual void update();
	unsigned int GetElementCode();

	virtual bool isButton();
	virtual bool isPanel();
	virtual bool isSlider();
	virtual bool isDroplist();
	virtual bool isEditbox();
	virtual bool isText();
protected:
	unsigned int elementCode;
	Bool _status;		//store the element status (i.e. the level of the slider or the status of a checkbox)
	Bool _isPressed;		//true if is currently being pressed by the mouse
	Bool _isMouseOn;		//true if the mouse cursor is in the element rectangle
	std::mutex update_mutex;
};

#endif

#ifndef GUI_SLIDER_H
#define GUI_SLIDER_H

#include "gameEngine.h"
#include "entity.h"
class AnimatedSprite;

class GUI_Slider : public GUI_Element {
public:
	GUI_Slider(EntityName objectName, unsigned int elementCode, std::vector <AnimatedSprite*>&, vector2 pos, vector2 rect,
				double minSliderVal, double maxSliderVal, double minResponseRange, 
		double maxResponseRate, int layer);
	GUI_Slider(EntityName objectName, unsigned int elementCode, AnimatedSprite* spriteAnim, vector2 pos, vector2 rect,
		double minSliderVal, double maxSliderVal, double minResponseRange,
		double maxResponseRate, int layer);
	//set element texture
	void SetValue(double value);
	bool ValueChanged();		//return true if the value changes since last time that was checked
	double getSliderValue();
	void update(double elapsedTime);
	void updateAnimation();
	void applyAction(GuiAction mouseAction);

	bool isSlider();
private:
	Bool wasChanged;
	double _minValue;
	double _maxValue;
	std::atomic <double> _value;
	double _minRange, _maxRange;
	vector2 _lastMousePosition;
};

#endif


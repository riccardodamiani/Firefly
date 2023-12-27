#include "stdafx.h"
#include "lightObject.h"
#include "graphics.h"

LightObject::LightObject(EntityName name, vector2 position, double rotation, LightObject* originalLight) {

	transform.position = position;
	transform.scale = { 1, 1 };
	transform.rotation = rotation;

	_colorRebake = false;
	_isInstance = true;
	_original = nullptr;
	if (originalLight != nullptr) {
		if(originalLight->_createInstance(this))
			_original = originalLight;
	}
	
	_objectName = _GameEngine->RegisterLightObject(this, name);

}

LightObject::LightObject(EntityName name, vector2 position, double rotation, double power, double lightAngle, SDL_Color color, LightType type) {
	
	transform.position = position;
	transform.scale = { 1, 1 };
	transform.rotation = rotation;
	_isInstance = false;

	this->power = power;
	_colorRebake = false;
	this->lightAngle = lightAngle;
	_type = type;
	_color = color;

	_lightTexture = _GameEngine->GenerateRandomName();
	_objectName = _GameEngine->RegisterLightObject(this, name);

	CalculateLight();
}

LightObject::~LightObject() {

	if (_isInstance) {
		if(_original != nullptr)
			_original.load()->_deleteInstance(this);
		return;
	}

	for (int i = 0; i < _instances.size(); i++) {
		_instances[i]->_deleteOriginal();
	}
}

bool LightObject::_createInstance(LightObject* instance) {
	if (_isInstance || instance == nullptr) {
		return false;
	}

	_instances.push_back(instance);

	return true;
}

void LightObject::_deleteInstance(LightObject* instance) {
	for (int i = 0; i < _instances.size(); i++) {
		if (_instances[i] == instance) {
			_instances.erase(_instances.begin() + i);
			return;
		}
	}
}

void LightObject::_deleteOriginal() {
	_original = nullptr;
}

LightObjectData LightObject::GetLightData() {
	LightObjectData data = {};

	//copy everything from the original light except the position and rotation
	if (_isInstance) {
		if (_original == nullptr)
			return data;
		data = _original.load()->GetLightData();
		data.position = transform.position;
		data.rotation = transform.rotation;
		data.colorRebake = false;
		return data;
	}

	data.lightAngle = lightAngle;
	data.position = transform.position;
	data.power = power;
	data.rotation = transform.rotation;
	data.type = _type;
	data.lightRadius = _lightRadius;
	data.lightTextureName = _lightTexture;
	data.colorRebake = _colorRebake;
	data.color = _color;
	data.parab_a = parab_a;
	data.parab_b = parab_b;
	data.parab_c = parab_c;
	return data;
}

void LightObject::SetPower(double power) {

	if (_isInstance) {
		return;
	}

	this->power = power;
	CalculateLight();
}

void LightObject::SetAngle(double angle) {

	if (_isInstance) {
		return;
	}

	this->lightAngle = angle;
	CalculateLight();
}

bool LightObject::RequireColorRebake() {
	return _colorRebake;
}

void LightObject::ResetChanged() {
	_colorRebake = false;
}

void LightObject::SetColor(SDL_Color color) {

	if (_isInstance) {
		return;
	}

	_color = color;
	_colorRebake = true;
}

void LightObject::CalculateLight() {

	if (_isInstance) {
		return;
	}

	if (_type == LightType::POINT_LIGHT) {

		//the radius of the light is equal to the distance from the source where the light intensity is equal to 0.05
		double lightRadius = sqrt(power / (0.05 * (1.0 + 2.0 * lightAngle * (PI / 180.0))));
		_lightRadius = lightRadius;

		//calculate the coefficients of the parabola that aproximate the light decay
		_graphicsEngine->Calculate_Parabola_Coeff_From_Points(0, power, lightRadius, 0, 2 * lightRadius, power, parab_a, parab_b, parab_c);
		//bake light
	}
	_graphicsEngine->BakeLightTexture(GetLightData());
}
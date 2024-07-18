#ifndef LIGHT_OBJECT_H
#define LIGHT_OBJECT_H

#include "gameObject.h"
#include "structures.h"
#include "variables.h"
#include "entity.h"

enum class LightType {
	POINT_LIGHT,
	GLOBAL_LIGHT
};

struct LightObjectData {
	vector2 position;
	double rotation;
	double power;
	double lightAngle;
	double lightRadius;
	LightType type;
	EntityName lightTextureName;
	bool colorRebake;
	SDL_Color color;
	double parab_a, parab_b, parab_c;
};

class LightObject : public GameObject{
public:
	LightObject(EntityName name, vector2 position, double rotation, LightObject* originalLight);
	LightObject(EntityName name, vector2 position, double rotation, double power, double lightAngle, SDL_Color color, LightType type);
	~LightObject();
	LightObjectData GetLightData();
	bool RequireColorRebake();
	void ResetChanged();
	void SetPower(double power);
	void SetAngle(double angle);
	void SetColor(SDL_Color color);
	
	bool _createInstance(LightObject* instance);
	void _deleteInstance(LightObject* instance);
	void _deleteOriginal();
private:
	void CalculateLight();

	bool _isInstance;	//true if the object is an instance of another light object
	std::atomic <LightObject*> _original;	//original light object to instantiate
	std::vector <LightObject*> _instances;	//instancing of this object

	Double power;
	Double lightAngle;
	EntityName _lightTexture;
	Double _lightRadius;
	LightType _type;
	Bool _colorRebake;		//if true tells the graphics engine to redraw the color of the light texture
	SDL_Color _color;
	double parab_a, parab_b, parab_c;
};

#endif

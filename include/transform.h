#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "structures.h"
#include "variables.h"
#include <mutex>
#include <math.h>

#define PI 3.14159265358979323846

typedef struct Transform {
	Vector2 position;
	Vector2 scale;
	Double rotation;
	operator TransformStruct() const {
		TransformStruct ret;
		ret.position = position;
		ret.scale = scale;
		ret.rotation = rotation;
		return ret;
	}
	Transform& operator=(TransformStruct val) {
		position = val.position;
		scale = val.scale;
		rotation = val.rotation;
		return *this;
	}
	void ApplyRotation(double rot, vector2 pivotPoint) {
		vector2 currentPos = position;
		vector2 deltaPos = { currentPos.x - pivotPoint.x , currentPos.y - pivotPoint.y };
		double deltaAngle = atan2(deltaPos.y, deltaPos.x) * (180/PI);
		deltaAngle += rot;
		double distance = sqrt(deltaPos.x * deltaPos.x + deltaPos.y * deltaPos.y);
		vector2 newPos = { pivotPoint.x + distance * cos(PI * deltaAngle / 180.0), pivotPoint.y + distance * sin(PI * deltaAngle / 180.0)};
		position = newPos;
		rotation += rot;
	}
	void ApplyScale(vector2 scaleFactor, vector2 pivotPoint) {
		vector2 currentPos = position;
		vector2 deltaPos = { currentPos.x - pivotPoint.x , currentPos.y - pivotPoint.y };
		deltaPos.x *= scaleFactor.x;
		deltaPos.y *= scaleFactor.y;
		vector2 newPos = { pivotPoint.x + deltaPos.x, pivotPoint.y + deltaPos.y };
		vector2 s = scale;
		s.x *= scaleFactor.x;
		s.y *= scaleFactor.y;
		scale = s;
		position = newPos;
	}
}Transform;




#endif

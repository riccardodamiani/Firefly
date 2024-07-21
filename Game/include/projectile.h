#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "gameObject.h"
#include "structures.h"

class Projectile : public GameObject {
public:
	Projectile();
	void update(double t);
	void OnCollisionEnter(Collision&);
	void OnCollisionExit(Collision&);
};

#endif
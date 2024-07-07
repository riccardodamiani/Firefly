#ifndef TEST_H
#define TEST_H

#include "gameObject.h"
#include "structures.h"

class Test : public GameObject {
public:
	Test(EntityName name, vector2 pos);
	void OnTriggerEnter(Collision&);
	void OnTriggerExit(Collision&);
	void OnTriggerStay(Collision&);
	void update(double);
	void setVelocity(vector2 vel);
	void setAngVel(double vel);
};

#endif
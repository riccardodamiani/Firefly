#include "stdafx.h"
#include "test class.h"
#include <vector>

Test::Test(EntityName name, vector2 pos) {
	std::vector <vector2> mesh = { {-0.5, 0.5}, {0.5, 0.5}, {0.5, -0.5}, {-0.5, -0.5} };

	setTexture(DecodeName("button texture"));
	transform.position = pos;
	transform.scale = { 1, 1 };
	AttachRigidbody(mesh);
	rigidbody->SetBoundingBox(BoundingBoxType::CONVEX);
	rigidbody->SetStatic(true);
	transform.rotation = 45;

	rigidbody->mass = INFINITY;

	//rigidbody->isTrigger = true;
	rigidbody->elasticity = 0.5;
	rigidbody->staticFriction = 0.7;
	rigidbody->dynamicFriction = 0.6;
	group = 1;
	RegisterObject(name);
	
}

void Test::setVelocity(vector2 vel) {
	rigidbody->velocity = vel;
}

void Test::setAngVel(double vel) {
	//rigidbody->angularVelocity = vel;
}

void Test::OnTriggerEnter(Collision&) {
	this->setTexture(DecodeName("poly collided"));
}

void Test::OnTriggerStay(Collision&) {

}

void Test::OnTriggerExit(Collision&) {
	this->setTexture(DecodeName("poly not collided"));
}

void Test::update(double time) {
	vector2 p = transform.position;
	if (p.magnitude() > 30)
		Destroy();
}

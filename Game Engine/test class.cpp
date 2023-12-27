#include "stdafx.h"
#include "test class.h"
#include <vector>

Test::Test(EntityName name, bool circle, vector2 pos) {
	transform.position = pos;
	transform.scale = { 1, 1 };
	transform.rotation = 0;
	this->circle = circle;
	

	std::vector <vector2> mesh = { {-0.5, 0.5}, {0.5, 0.5}, {0.5, -0.5}, {-0.5, -0.5} };
	
	if (circle) {
		AttachRigidbody(mesh);
		this->setTexture(DecodeName("button texture"));
		//rigidbody->SetCircleBoundingBox(0.5);
		rigidbody->SetBoundingBox(BoundingBoxType::CONVEX);
	}
	else {
		//rigidbody->SetConvexBoundingBox(vertex);
		setTexture(DecodeName("button texture"));
		transform.position = pos;
		//transform.scale = { 4, 10 };
		//std::vector <vector2> wallVert = { {-2, 5}, {2, 5}, {2, -5}, {-2, -5} };
		transform.scale = { 1, 1 };
		std::vector <vector2> mesh = { {-0.5, 0.5}, {0.5, 0.5}, {0.5, -0.5}, {-0.5, -0.5} };
		AttachRigidbody(mesh);
		rigidbody->SetBoundingBox(BoundingBoxType::CONVEX);
		rigidbody->SetStatic(true);
		transform.rotation = 45;

		rigidbody->mass = INFINITY;
	}
	//rigidbody->isTrigger = true;
	rigidbody->elasticity = 0.5;
	rigidbody->staticFriction = 0.7;
	rigidbody->dynamicFriction = 0.6;
	RegisterObject(name);
	
}

void Test::setVelocity(vector2 vel) {
	rigidbody->velocity = vel;
}

void Test::setAngVel(double vel) {
	//rigidbody->angularVelocity = vel;
}

void Test::OnTriggerEnter(Collision&) {
	if(circle)
		this->setTexture(DecodeName("circle collided"));
	else
		this->setTexture(DecodeName("poly collided"));
}

void Test::OnTriggerStay(Collision&) {

}

void Test::OnTriggerExit(Collision&) {
	if (circle)
		this->setTexture(DecodeName("circle not collided"));
	else
		this->setTexture(DecodeName("poly not collided"));
}

void Test::update(double time) {
	vector2 p = transform.position;
	if (p.magnitude() > 30)
		Destroy();
}

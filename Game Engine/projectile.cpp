#include "stdafx.h"
#include "projectile.h"

Projectile::Projectile() {
	double x = -7 + (rand() % 1000) / 70.0;
	double y = 6 + (rand() % 1000) / 70.0;

	group = 2;

	transform.position = {x, y};
	transform.scale = { 1, 1 };
	transform.rotation = 45;

	std::vector <vector2> mesh;
	_PhysicsEngine->createRegularPolygon(20, 0.5, mesh);
	
	AttachRigidbody(mesh);
	this->setTexture(DecodeName("sphere"));
	rigidbody->SetBoundingBox(BoundingBoxType::CONVEX);
	rigidbody->elasticity = 0.5;
	rigidbody->mass = 5;
	rigidbody->groupMask = 0x1;
	//rigidbody->velocity = {15, 0};
	
	RegisterObject();
}

void Projectile::update(double t) {
	vector2 p = transform.position;
	if (p.magnitude() > 30)
		Destroy();
}

void Projectile::OnCollisionEnter(Collision&) {
	this->setTexture(DecodeName("red sphere"));
}

void Projectile::OnCollisionExit(Collision&) {
	//this->setTexture(DecodeName("sphere"));
}
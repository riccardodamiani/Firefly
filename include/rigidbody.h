#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include <atomic>
#include "structures.h"
#include "physics.h"

class GameObject;


class Rigidbody {

public:
	Rigidbody(GameObject *parent, std::vector <vector2>& vertexes);
	~Rigidbody();
	void AddForce(vector2 forceVector);
	void AddExplosionForce(vector2 position, double force);
	void SetBoundingBox(BoundingBoxType type);
	bool GetBoundingBox(BoundingBox &b);
	GameObject* getParentObject();
	void getMesh(FMesh &vertexes);
	double getMOI();
	vector2 getForce();
	vector2 getRelativePoint(vector2 point);
	bool isColliding(Rigidbody* body);
	void SetStatic(bool isStatic);
	bool IsStatic();
	bool IsMovable();

	//internal call. Don't use them
	void _updateTransform();
	void _startCollisionFrame(double timeElapsed, vector2 gravity);
	void _endCollisionFrame();
	void _updatePhysics(double timeElapsed, double sleepVelocity);
	void _resolveCollisions(double timeElapsed);
	void _resolveDrag(double timeElapsed);
	void _applyForces(double timeElapsed);
	void _setCollisions(Rigidbody* body, vector2 contactPoint, vector2 collisionNormal,
		vector2 velocity, double impulse, vector2 updatedPosition);
	
	Double mass;
	Double staticFriction;
	Double dynamicFriction;
	Double elasticity;
	Vector2 velocity;
	Double angularVelocity;
	UInt constraints;
	Bool useGravity;
	Bool isTrigger;
	Bool detectCollisions;
	UInt groupMask;
private:
	
	void _findMeshArea();
	
	GameObject* parentObject;
	//vector2 frameVelocity;
	//double frameAngularVelocity;
	BoundingBox* boundingBox;
	FMesh _mesh;
	FMesh _world_mesh;
	vector2 centerOfMass;
	vector2 meshScale;
	vector2 frameForce;
	double meshRot;
	double area, density;
	double momentOfInertia;
	bool isStatic;
	bool transformChanged;
	bool _meshUpdated;
	std::mutex _meshMutex, _collisionMutex, _moiMutex;
	std::vector <ResultCollision> _prevCollision;
};

#endif
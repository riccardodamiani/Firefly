#ifndef PHYSICS_H
#define PHYSICS_H

#include <vector>
#include <mutex>
#include "structures.h"
#include "physics_structs.h"
#include "engine_exports.h"

class ENGINE_API PhysicsEngine {
public:
    static PhysicsEngine& getInstance() {
        static PhysicsEngine instance;
        return instance;
    }

    PhysicsEngine(const PhysicsEngine&) = delete;
    PhysicsEngine& operator=(const PhysicsEngine&) = delete;
	
	void RegisterRigidbody(Rigidbody *);
	void RemoveRigidbody(Rigidbody *);
	void _updateStatic(Rigidbody* r);

	void NewPhysicsFrame(double timeElapsed);
	void UpdatePhysics(double timeElapsed, int thread, int threadCount);
	void ResolvePhysics(double timeElapsed);
	
	double CollisionResponce(Rigidbody* body1, Rigidbody* body2, vector2& r1, vector2& r2, vector2 &collisionNormal,
		vector2 &collisionVelocity);
	
	//developer calls
	void SetGravity(vector2 force);
	long GetBodiesCount();
	void createRegularPolygon(int sidesCount, double radius, std::vector <vector2>& vertexes);
	void SetSleepVelocity(double velocity);
	
private:
	PhysicsEngine();
	~PhysicsEngine();

	void Check_Convex_Convex_Collision(double timeElapsed, Rigidbody *r1, BoundingBox& box1, 
		Rigidbody* r2, BoundingBox& box2, std::vector <CollisionStruct>& frameColl,
		FMesh& mesh1, FMesh& mesh2);
	bool checkPolygonPenetration(FMesh& mesh1, FMesh& mesh2);
	bool checkPolygonPenetration(FMesh& m1, FMesh& m2, vector2& mtv);
	void findVirtualCollisionPoints(FMesh& mesh1, FMesh& mesh2, vector2 velocityAxis,
		std::vector <struct CollisionPoint> &collisions, int round);

	
	void filterCollisionPoints(std::vector <struct CollisionPoint>& collisions);
	void _updatePhysics(double timeElapsed);
	void _resolveCollision(CollisionStruct &c);
	void _resolveFriction(CollisionStruct& c);

	vector2 _gravity;
	double _sleepVelocity;
	std::vector <Rigidbody*> _bodies;
	std::mutex _update_mutex;
	std::mutex _collision_buffer_mutex;
	std::vector <CollisionStruct> frameCollisions;
	int _firstStatic;
};

#endif
#ifndef PHYSICS_STRUCTS_H
#define PHYSICS_STRUCTS_H

#include "structures.h"

#include <vector>
#include <memory>
class Rigidbody;

enum class BoundingBoxType {
	SPHERE,
	CONVEX
};

struct BoundingBox {
	BoundingBoxType type;
	//vector2 centerPoint;
	//vector2 scale;
	//double angle;
	double radius;
};

struct Collision {
	Rigidbody* collider;
	vector2 contact;
	vector2 normal;
	double impulse;
};

struct CollisionPoint {
	vector2 vertex;
	vector2 v_edge1, v_edge2;
	double distance;
	vector2 collisionPoint;
	int body;
};

struct ResultCollision {
	Rigidbody* collider;
	vector2 contactPoint;
	vector2 collisionNormal;
	vector2 collisionVelocity;
	vector2 postCollisionPosition;
	double impulse;
	bool frameCollision;
	bool firstCollision;
};

struct CollisionStruct {
	Rigidbody *A, *B;
	vector2 contactPoint;
	vector2 collisionNormal;
	vector2 collisionVelocity;
	vector2 postPosA, postPosB;
	double impulse;
};

enum RBContraints {
	NO_CONST = 0,
	X_CONST = 1,
	Y_CONST = 2,
	ROT = 4
};

struct PhysicsJob {
	int first_i_index, first_j_index;
	int last_i_index, last_j_index;
};

struct Projection {
	double min, max;
	bool overlap(Projection p, double &overlap) {
		if (min <= p.max && max >= p.min) {
			overlap = p.max - min;
			return true;
		}
		return false;
	}
	bool contains(Projection p) {
		return (min <= p.min && max >= p.max);
	}
};

struct FMesh {
	P_Array v;
	vector2 centerOfMass;
	void translate(vector2 v2) {
		for (int i = 0; i < v.size(); i++) {
			v[i] = { v[i].x + v2.x, v[i].y + v2.y };
		}
		centerOfMass = { centerOfMass.x + v2.x, centerOfMass.y + v2.y };
	}
	Projection project(vector2 axis) {
		double min = axis.dot(v[0]);
		double max = min;
		for (int i = 1; i < v.size(); i++) {
			// NOTE: the axis must be normalized to get accurate projections
			double p = axis.dot(v[i]);
			if (p < min) {
				min = p;
			}
			else if (p > max) {
				max = p;
			}
		}

		return Projection{ min, max };
	}
	std::shared_ptr<std::vector <vector2>> getAxes(void) {
		std::vector <vector2>* a = new std::vector <vector2>;
		auto& axis = *a;

		for (int a = 0; a < v.size(); a++) {
			int b = (a + 1) % v.size();		//second vertex of the side
			//calculate the axis of projection which is the normal of the side
			vector2 axisOfProj = { -(v[a].y - v[b].y), v[a].x - v[b].x };
			axis.push_back(axisOfProj.normalize());
		}
		return std::shared_ptr<std::vector <vector2>>(a);
	}
};

struct Mesh {
	std::vector <vector2> v;
	vector2 centerOfMass;
	void translate(vector2 v2) {
		for (int i = 0; i < v.size(); i++) {
			v[i] = { v[i].x + v2.x, v[i].y + v2.y };
		}
		centerOfMass = { centerOfMass.x + v2.x, centerOfMass.y + v2.y };
	}
	Projection project(vector2 axis) {
		double min = axis.dot(v[0]);
		double max = min;
		for (int i = 1; i < v.size(); i++) {
			// NOTE: the axis must be normalized to get accurate projections
			double p = axis.dot(v[i]);
			if (p < min) {
				min = p;
			}
			else if (p > max) {
				max = p;
			}
		}
		
		return Projection{ min, max };
	}
	std::shared_ptr<std::vector <vector2>> getAxes(void) {
		std::vector <vector2> *a = new std::vector <vector2>;
		auto& axis = *a;

		for (int a = 0; a < v.size(); a++) {
			int b = (a + 1) % v.size();		//second vertex of the side
			//calculate the axis of projection which is the normal of the side
			vector2 axisOfProj = { -(v[a].y - v[b].y), v[a].x - v[b].x };
			axis.push_back(axisOfProj.normalize());
		}
		return std::shared_ptr<std::vector <vector2>> (a);
	}
};

#endif
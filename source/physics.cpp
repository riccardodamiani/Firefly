
#include "physics.h"
#include "rigidbody.h"
#include "gameObject.h"
#include "transform.h"
#include "structures.h"

#include <mutex>
#include <memory>
#include <vector>

PhysicsEngine::PhysicsEngine() {

	_gravity = {};
	_sleepVelocity = {};
	_firstStatic = 0;
}

void PhysicsEngine::RegisterRigidbody(Rigidbody* body) {
	std::lock_guard <std::mutex> guard(_update_mutex);
	if (body->IsStatic()) {
		_bodies.push_back(body);
		return;
	}

	_bodies.insert(_bodies.begin(), body);	//move the object at the start
	++_firstStatic;
}

void PhysicsEngine::RemoveRigidbody(Rigidbody *body) {
	std::lock_guard <std::mutex> guard(_update_mutex);
	for (int i = 0; i < _bodies.size(); i++){
		if (_bodies[i] == body) {
			_bodies.erase(_bodies.begin() + i);
			if (i < _firstStatic)
				--_firstStatic;
			return;
		}
	}
}

void PhysicsEngine::_updateStatic(Rigidbody* r) {
	for (int i = 0; i < _bodies.size(); i++) {
		if (_bodies[i] == r) {
			if (_bodies[i]->IsStatic()) {
				if (i < _firstStatic) {		//is in the non static section
					_bodies.push_back(_bodies[i]);	//move the object at the end
					_bodies.erase(_bodies.begin() + i);
					--_firstStatic;
				}
			}
			else {
				if (i >= _firstStatic) {		//is in the static section
					_bodies.insert(_bodies.begin(), _bodies[i]);	//move the object at the start
					_bodies.erase(_bodies.begin() + i);
					++_firstStatic;
				}
			}
			return;
		}
	}
}

void PhysicsEngine::SetSleepVelocity(double v) {
	_sleepVelocity = v;
}

void PhysicsEngine::NewPhysicsFrame(double timeElapsed) {

	frameCollisions.clear();
	for (int i = 0; i < _bodies.size(); i++) {
		_bodies[i]->_startCollisionFrame(timeElapsed, _gravity);
	}
}

void PhysicsEngine::UpdatePhysics(double timeElapsed, int thread, int threadCount) {
	std::vector <CollisionStruct> localCollisions;
	FMesh* mesh1 = new FMesh(), *mesh2 = new FMesh();

	for (int i = thread; i < _firstStatic; i += threadCount) {
		for (int j = i + 1; j < _bodies.size(); j++) {
			Rigidbody* body1 = _bodies[i];
			Rigidbody* body2 = _bodies[j];
			BoundingBox b1, b2;

			//no bounding box present or the object doesn't want to be detected
			bool check_collision = _bodies[i]->GetBoundingBox(b1) && _bodies[j]->GetBoundingBox(b2)
				&& _bodies[i]->detectCollisions && _bodies[j]->detectCollisions;
			if (!check_collision) {
				continue;
			}

			if (b1.type == BoundingBoxType::CONVEX && b2.type == BoundingBoxType::CONVEX) {
				Check_Convex_Convex_Collision(timeElapsed, body1, b1, body2, b2, localCollisions, *mesh1, *mesh2);
			}
		}
	}

	std::lock_guard <std::mutex> guard(_collision_buffer_mutex);
	for (int i = 0; i < localCollisions.size(); i++) {
		frameCollisions.push_back(localCollisions[i]);
	}

	delete mesh1;
	delete mesh2;
}

void PhysicsEngine::ResolvePhysics(double timeElapsed) {

	for (int i = 0; i < frameCollisions.size(); i++) {
		_resolveCollision(frameCollisions[i]);
	}
	/*for (int i = 0; i < _bodies.size(); i++) {
		_bodies[i]->_applyForces(timeElapsed);
	}*/
	for (int i = 0; i < frameCollisions.size(); i++) {
		_resolveFriction(frameCollisions[i]);
	}
	for (int i = 0; i < _firstStatic; i++) {
		_bodies[i]->_updatePhysics(timeElapsed, _sleepVelocity);
	}

	for (int i = 0; i < _bodies.size(); i++) {
		_bodies[i]->_endCollisionFrame();
	}
}

void PhysicsEngine::_updatePhysics(double timeElapsed) {

	for (int i = 0; i < _firstStatic; i++) {
		_bodies[i]->_updatePhysics(timeElapsed, _sleepVelocity);
	}
}

void PhysicsEngine::_resolveCollision(CollisionStruct& c) {

	vector2 va = c.A->velocity;
	vector2 vb = c.B->velocity;
	double massA = c.A->mass;
	double massB = c.B->mass;
	vector2 normal = c.collisionNormal;
	double impulse = c.impulse;

	c.A->velocity += { impulse / massA * normal.x, impulse / massA * normal.y };
	c.B->velocity -= { impulse / massB * normal.x, impulse / massB * normal.y };

	c.A->getParentObject()->transform.position += c.postPosA;
	c.B->getParentObject()->transform.position += c.postPosB;

	vector2 ra = c.A->getRelativePoint(c.contactPoint);
	vector2 rb = c.B->getRelativePoint(c.contactPoint);

	c.A->angularVelocity += (ra.cross({ normal.x * impulse, normal.y * impulse }) / c.A->getMOI()) * 180 / MATH_PI;
	c.B->angularVelocity -= (rb.cross({ normal.x * impulse, normal.y * impulse }) / c.B->getMOI()) * 180 / MATH_PI;
}

void PhysicsEngine::_resolveFriction(CollisionStruct& c) {

	vector2 v1 = c.A->velocity;
	vector2 v2 = c.B->velocity;
	double massA = c.A->mass;
	double massB = c.B->mass;
	vector2 rv = { v2.x - v1.x, v2.y - v1.y };
	vector2 normal = c.collisionNormal;

	// Solve for the tangent vector
	double t = rv.dot(normal);
	vector2 tangent;
	if (t == 0) return;
	tangent = { rv.x - normal.x * t, rv.y - normal.y * t };
	if (tangent.magnitude() == 0) return;
	//}
	tangent = tangent.normalize();

	// Solve for magnitude to apply along the friction vector
	float jt = -rv.dot(tangent);
	jt = jt / (1 / massA + 1 / massB);

	// Use to approximate mu given friction coefficients of each body
	float mu = sqrt(c.A->staticFriction * c.A->staticFriction + c.B->staticFriction * c.B->staticFriction);

	// Clamp magnitude of friction and create impulse vector
	vector2 frictionImpulse;
	if (fabs(jt) < fabs(c.impulse * mu))		//static friction
		frictionImpulse = { jt * tangent.x, jt * tangent.y };
	else {	//dynamic friction
		float dynamicFric = sqrt(c.A->dynamicFriction * c.A->dynamicFriction + c.B->dynamicFriction * c.B->dynamicFriction);
		frictionImpulse = { -c.impulse * tangent.x * dynamicFric, -c.impulse * tangent.y * dynamicFric };
	}

	// Apply
	c.A->velocity += {(1 / massA)* frictionImpulse.x, (1 / massA)* frictionImpulse.y};
	c.B->velocity -= {(1 / massB)* frictionImpulse.x, (1 / massB)* frictionImpulse.y};

	vector2 va = c.A->velocity;
	vector2 vb = c.B->velocity;
	double Vt_mag = va.dot(tangent);
	if (fabs(Vt_mag) < 0.05 && fabs(c.A->getForce().dot(tangent) < 0.05)) {
		vector2 vt = { tangent.x * Vt_mag, tangent.y * Vt_mag };
		c.A->velocity -= vt;
	}
	Vt_mag = vb.dot(tangent);
	if (fabs(Vt_mag) < 0.05 && fabs(c.B->getForce().dot(tangent) < 0.05)) {
		vector2 vt = { tangent.x * Vt_mag, tangent.y * Vt_mag };
		c.B->velocity -= vt;
	}
}

void PhysicsEngine::createRegularPolygon(int sidesCount, double radius, std::vector <vector2>& vertexes) {
	for (int i = 0; i < sidesCount; i++) {
		vertexes.push_back({ radius * cos(2 * MATH_PI * i / sidesCount), radius * sin(2 * MATH_PI * i / sidesCount) });
	}
}

void PhysicsEngine::SetGravity(vector2 force) {
	_gravity = force;
}

long PhysicsEngine::GetBodiesCount() {
	return _bodies.size();
}

bool PhysicsEngine::checkPolygonPenetration(FMesh& m1, FMesh& m2, vector2 &mtv) {

	double overlap = INFINITY;
	vector2 smallest = {};

	auto axes1(m1.getAxes());
	auto axes2(m2.getAxes());

	// loop over the axes1
	for (int i = 0; i < axes1->size(); i++) {
		vector2 axis = (*axes1)[i];
		// project both shapes onto the axis
		Projection p1 = m1.project(axis);
		Projection p2 = m2.project(axis);
		// do the projections overlap?
		double o;
		if (!p1.overlap(p2, o)) {
			return false;
		}

		if (p1.contains(p2) || p2.contains(p1)) {
			// get the overlap plus the distance from the minimum end points
			double mins = fabs(p1.min - p2.min);
			double maxs = fabs(p1.max - p2.max);
			// NOTE: depending on which is smaller you may need to
			// negate the separating axis!!
			if (mins < maxs) {
				o += mins;
			}
			else {
				o += maxs;
			}
		}

		// check for minimum
		if (o < overlap) {
			// then set this one as the smallest
			overlap = o;
			smallest = axis;
		}
	
	}
	// loop over the axes2
	for (int i = 0; i < axes2->size(); i++) {
		vector2 axis = (*axes2)[i];
		// project both shapes onto the axis
		Projection p1 = m1.project(axis);
		Projection p2 = m2.project(axis);
		// do the projections overlap?
		double o;
		if (!p1.overlap(p2, o)) {
			return false;
		}

		if (p1.contains(p2) || p2.contains(p1)) {
			// get the overlap plus the distance from the minimum end points
			double mins = fabs(p1.min - p2.min);
			double maxs = fabs(p1.max - p2.max);
			// NOTE: depending on which is smaller you may need to
			// negate the separating axis!!
			if (mins < maxs) {
				o += mins;
			}
			else {
				o += maxs;
			}
		}

		// check for minimum
		if (o < overlap) {
			// then set this one as the smallest
			overlap = o;
			smallest = axis;
		}
	}

	mtv = { smallest.x * overlap, smallest.y * overlap };
	return true;
}

bool PhysicsEngine::checkPolygonPenetration(FMesh &mesh1, FMesh& mesh2) {

	for (int a = 0; a < mesh1.v.size(); a++) {
		int b = (a + 1) % mesh1.v.size();		//second vertex of the side
		//calculate the axis of projection which is the normal of the side
		vector2 axisOfProj = { -(mesh1.v[a].y - mesh1.v[b].y), mesh1.v[a].x - mesh1.v[b].x };
		double min_r1 = INFINITY;
		double max_r1 = -INFINITY;

		//calculate the minimum and maximum value of projection for the first shape
		for (int p = 0; p < mesh1.v.size(); p++) {
			//do a dot product between the vertex and the vector of the axis of projection
			double q = mesh1.v[p].x * axisOfProj.x + mesh1.v[p].y * axisOfProj.y;
			min_r1 = std::min(min_r1, q);
			max_r1 = std::max(max_r1, q);
		}

		//calculate the minimum and maximum value of projection for the second shape
		double min_r2 = INFINITY;
		double max_r2 = -INFINITY;
		for (int p = 0; p < mesh2.v.size(); p++) {
			//do a dot product between the vertex and the vector of the axis of projection
			double q = mesh2.v[p].x * axisOfProj.x + mesh2.v[p].y * axisOfProj.y;
			min_r2 = std::min(min_r2, q);
			max_r2 = std::max(max_r2, q);
		}

		if (!(max_r2 >= min_r1 && max_r1 >= min_r2)) {	//they don't overlap
			return false;
		}
	}

	return true;
}

void PhysicsEngine::findVirtualCollisionPoints(FMesh& mesh1, FMesh& mesh2, vector2 velocityAxis,
	std::vector <struct CollisionPoint>& collisions, int round) {

	vector2 axisOfProj = { -velocityAxis.y, velocityAxis.x };
	for (int i = 0; i < mesh2.v.size(); i++) {
		double q_v = mesh2.v[i].x * axisOfProj.x + mesh2.v[i].y * axisOfProj.y;
		for (int v1 = 0; v1 < mesh1.v.size(); v1++) {
			int v2 = (v1 + 1) % mesh1.v.size();
			double q1 = mesh1.v[v1].x * axisOfProj.x + mesh1.v[v1].y * axisOfProj.y;
			double q2 = mesh1.v[v2].x * axisOfProj.x + mesh1.v[v2].y * axisOfProj.y;

			bool contact = ((q1 <= q_v) & (q_v <= q2)) | ((q2 <= q_v) & (q_v <= q1));
			if (!contact) continue;
			if (q1 == q_v && q1 == q2) continue;	//all three point aligned

			//calculate the normalized vector of the edge
			vector2 d2_v = { mesh1.v[v2].x - mesh1.v[v1].x, mesh1.v[v2].y - mesh1.v[v1].y };
			double d2_m = sqrt(d2_v.x * d2_v.x + d2_v.y * d2_v.y);
			vector2 d2 = { d2_v.x / d2_m, d2_v.y / d2_m };

			//calculate the distance of the collision point along the edge from v1 vertex
			double h = (velocityAxis.x * mesh1.v[v1].y + velocityAxis.y * mesh2.v[i].x
				- velocityAxis.y * mesh1.v[v1].x - velocityAxis.x * mesh2.v[i].y) / (velocityAxis.y * d2.x - velocityAxis.x * d2.y);

			//calculate the collision point on the edge
			vector2 collisionPoint = { mesh1.v[v1].x + d2.x * h, mesh1.v[v1].y + d2.y * h };

			//calculate the distance between the vertex and the collision point
			double d;
			if (velocityAxis.x == 0) {
				d = fabs(collisionPoint.y - mesh2.v[i].y);
			}else
				d = fabs((mesh1.v[v1].x - mesh2.v[i].x + d2.x * h) / velocityAxis.x);

			collisions.push_back({ mesh2.v[i], mesh1.v[v1] , mesh1.v[v2], d, collisionPoint, round});
		}
	}
}

void PhysicsEngine::Check_Convex_Convex_Collision(double timeElapsed,
	Rigidbody* body1, BoundingBox& box1, Rigidbody* body2, BoundingBox& box2,
	std::vector <CollisionStruct>& frameCollisions, FMesh &mesh1, FMesh &mesh2) {

	BoundingBox* convex1 = &box1;
	BoundingBox* convex2 = &box2;
	vector2 pos1 = body1->getParentObject()->transform.position;
	vector2 pos2 = body2->getParentObject()->transform.position;

	if (sqrt((pos1.x - pos2.x) *
		(pos1.x - pos2.x) +
		(pos1.y - pos2.y) *
		(pos1.y - pos2.y))
			> box1.radius + box2.radius)
		return;

	body1->getMesh(mesh1);
	body2->getMesh(mesh2);

	vector2 velocity1 = body1->velocity;
	vector2 velocity2 = body2->velocity;

	vector2 mtv;
	if (!checkPolygonPenetration(mesh1, mesh2, mtv))
		return;

	//if we get down here the polygons are intersecting

	vector2 contactsPoint;
	//if either of the two rigidbody is a trigger there is no collision so we can stop here
	if (body1->isTrigger | body2->isTrigger) {
		if (body1->isTrigger) {
			body1->_setCollisions(body2, {}, {}, {}, 0, {});
		}
		if (body2->isTrigger) {
			body2->_setCollisions(body1, {}, {}, {}, 0, {});
		}
		return;
	}

	//calculate the vector of the relative velocity
	vector2 relVelocity = { velocity1.x - velocity2.x, velocity1.y - velocity2.y };
	double magnitude = sqrt(relVelocity.x * relVelocity.x + relVelocity.y * relVelocity.y);
	vector2 velocityAxis = { relVelocity.x / magnitude, relVelocity.y / magnitude };
	vector2 deltaP1 = {}, deltaP2 = {};
	//bool isStatis1 = (body1->mass == INFINITY) | body1->IsStatic();
	bool isStatis2 = !body2->IsMovable();//(body2->mass == INFINITY) | body2->IsStatic();

	if (isStatis2) {
		mesh1.translate(mtv);
		deltaP1 = mtv;
	}
	else {
		if (velocity1.magnitude() > velocity2.magnitude()) {
			mesh1.translate(mtv);
			deltaP1 = mtv;
		}
		else {
			mesh2.translate({ -mtv.x, -mtv.y });
			deltaP2 = mtv.invert();
		}
	}

	std::vector <struct CollisionPoint> tempCollisions;
	findVirtualCollisionPoints(mesh1, mesh2, velocityAxis, tempCollisions, 1);
	findVirtualCollisionPoints(mesh2, mesh1, { -velocityAxis.x, -velocityAxis.y }, tempCollisions, -1);

	if (tempCollisions.size() == 0)
		return;

	//find the first contact
	filterCollisionPoints(tempCollisions);

	//find the real collision point
	vector2 vertexPoint = {};
	vector2 edgePoint = {};
	vector2 collisionPoint;

	for (int i = 0; i < tempCollisions.size(); i++) {
		vertexPoint = { vertexPoint.x + tempCollisions[i].vertex.x, vertexPoint.y + tempCollisions[i].vertex.y };
		edgePoint = { edgePoint.x + tempCollisions[i].collisionPoint.x, edgePoint.y + tempCollisions[i].collisionPoint.y };
	}
	collisionPoint = { (vertexPoint.x + edgePoint.x) / (2 * tempCollisions.size()),
						(vertexPoint.y + edgePoint.y) / (2 * tempCollisions.size()) };

	//calculate stuff
	vector2 r1 = { collisionPoint.x - mesh1.centerOfMass.x, collisionPoint.y - mesh1.centerOfMass.y };
	vector2 r2 = { collisionPoint.x - mesh2.centerOfMass.x, collisionPoint.y - mesh2.centerOfMass.y };

	double av1 = body1->angularVelocity * MATH_PI / 180.0;
	double av2 = body2->angularVelocity * MATH_PI / 180.0;

	vector2 cp1_v = { velocity1.x - av1 * r1.y, velocity1.y + av1 * r1.x };
	vector2 cp2_v = { velocity2.x - av2 * r2.y, velocity2.y + av2 * r2.x };
	vector2 vr = { cp1_v.x - cp2_v.x,  cp1_v.y - cp2_v.y };
	if (vr.magnitude() < 0.05) return;

	//factor to adjust the collision normal based on the body order
	double collisionBodyAdj = static_cast<double>(tempCollisions[0].body);

	vector2 edgeV = { tempCollisions[0].v_edge2.x - tempCollisions[0].v_edge1.x,
		tempCollisions[0].v_edge2.y - tempCollisions[0].v_edge1.y };
	double mag_v = sqrt(edgeV.x * edgeV.x + edgeV.y * edgeV.y);
	vector2 collisionNormal = edgeV.normal();
	collisionNormal = { collisionBodyAdj * collisionNormal.x / mag_v, collisionBodyAdj * collisionNormal.y / mag_v };

	double impulse = CollisionResponce(body1, body2, r1, r2, collisionNormal, vr);

	body1->_setCollisions(body2, collisionPoint,  collisionNormal.invert(), vr, impulse, deltaP1);
	body2->_setCollisions(body1, collisionPoint, collisionNormal, vr, impulse, deltaP2);

	frameCollisions.push_back({body1, body2, collisionPoint, collisionNormal, vr, deltaP1, deltaP2, impulse });

}

void PhysicsEngine::filterCollisionPoints(std::vector <struct CollisionPoint>& collisions) {
	//find first contact
	double dmin = INFINITY;
	int minIndex;

	for (int i = 0; i < collisions.size(); i++) {
		if (collisions[i].distance < dmin) {
			dmin = collisions[i].distance;
			minIndex = i;
		}
	}

	//erase all useless collisions
	for (int i = 0; i < collisions.size(); i++) {
		if (collisions[i].distance > dmin) {
			collisions.erase(collisions.begin() + i);
			i--;
		}
	}

	//remove double collisions
	for (int i = 0; i < collisions.size(); i++) {
		if (collisions[i].body != collisions[0].body) {
			collisions.erase(collisions.begin() + i);
			i--;
		}
	}
}

double PhysicsEngine::CollisionResponce(Rigidbody* body1, Rigidbody* body2, 
	vector2& r1, vector2 &r2, vector2& normal, vector2& vr) {

	double e = body1->elasticity * body2->elasticity;
	double m1 = body1->mass;
	double m2 = body2->mass;

	double I1 = body1->getMOI();
	double I2 = body2->getMOI();

	double z = r1.cross(normal);
	vector2 a1 = { -z * r1.y / I1, z * r1.x / I1 };
	//vector2 a1 = IJ1.dot(temp);
	z = r2.cross(normal);
	//temp = { z * r2.y, z * r2.x };
	vector2 a2 = { -z * r2.y / I2, z * r2.x / I2 };

	double jr = -(1+e) * vr.dot(normal);
	jr = jr / ((1 / m1) + (1 / m2) + normal.dot({ a1.x + a2.x, a1.y + a2.y }));

	return jr;
}

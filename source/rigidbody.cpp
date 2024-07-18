#include "rigidbody.h"
#include "gameObject.h"
#include "physics.h"

Rigidbody::Rigidbody(GameObject* parent, std::vector <vector2> &vertexes) {

	boundingBox = nullptr;
	parentObject = parent;
	mass = 1.0;
	staticFriction = 0.5;
	dynamicFriction = 0.3;
	elasticity = 1.0;
	velocity = { 0, 0 };
	angularVelocity = 0;
	useGravity = true;
	isTrigger = false;
	constraints = RBContraints::NO_CONST;

	detectCollisions = true;
	_mesh.v = vertexes;
	_world_mesh.v = vertexes;

	centerOfMass = parent->transform.position;
	
	meshScale = parent->transform.scale;
	meshRot = parent->transform.rotation;
	
	_findMeshArea();
	density = mass / area;
	transformChanged = true;
	isStatic = false;

	groupMask = 0xffffffff;

	PhysicsEngine::getInstance().RegisterRigidbody(this);
}

Rigidbody::~Rigidbody() {
	if (boundingBox != nullptr)
		delete boundingBox;
	PhysicsEngine::getInstance().RemoveRigidbody(this);
}

GameObject* Rigidbody::getParentObject() {
	return parentObject;
}

void Rigidbody::_findMeshArea() {
	area = 0;
	for (int i = 0; i < _mesh.v.size(); i++) {
		vector2 v0 = _mesh.v[i];
		vector2 v1 = _mesh.v[(i+1) % _mesh.v.size()];
		area += 0.5 * fabs(v0.x * v1.y - v1.x * v0.y);
	}
}

double Rigidbody::getMOI() {

	std::lock_guard <std::mutex> guard(_moiMutex);

	density = mass / area;
	momentOfInertia = 0;
	for (int i = 0; i < _mesh.v.size(); i++) {
		vector2 v0 = _mesh.v[i];
		vector2 v1 = _mesh.v[(i + 1) % _mesh.v.size()];
		double a = fabs(v0.x * v1.y - v1.x * v0.y);
		double t1 = v0.y * v0.y + v0.y * v1.y + v1.y * v1.y;
		double t2 = v0.x * v0.x + v0.x * v1.x + v1.x * v1.x;
		momentOfInertia += a * (t1 + t2);
	}
	momentOfInertia *= density / 12.0;
	return momentOfInertia;
}

vector2 Rigidbody::getForce() {
	return frameForce;
}

vector2 Rigidbody::getRelativePoint(vector2 point) {
	return {point.x - centerOfMass.x, point.y - centerOfMass.y};
}

//add force at the center of the bounding box
void Rigidbody::AddForce(vector2 forceVector) {
	
}

void Rigidbody::AddExplosionForce(vector2 position, double force) {

}

void Rigidbody::SetBoundingBox(BoundingBoxType type) {
	if (boundingBox != nullptr)
		delete boundingBox;

	boundingBox = new BoundingBox();

	boundingBox->type = type;
	double radius = 0;
	for (int i = 0; i < _mesh.v.size(); i++) {
		radius = std::max(radius, sqrt(_mesh.v[i].x * _mesh.v[i].x
			+ _mesh.v[i].y * _mesh.v[i].y));
	}
	boundingBox->radius = radius;

}

bool Rigidbody::GetBoundingBox(BoundingBox& b) {
	if (boundingBox == nullptr)
		return false;

	b = *boundingBox;
	return true;
}


void Rigidbody::_setCollisions(Rigidbody* body, vector2 contactPoint, 
	vector2 collisionNormal, vector2 velocity, double impulse, vector2 updatedPosition) {

	std::lock_guard <std::mutex> guard(_collisionMutex);

	for (int i = 0; i < _prevCollision.size(); i++) {
		if (_prevCollision[i].collider == body) {
			_prevCollision[i].frameCollision = true;
			_prevCollision[i].contactPoint = contactPoint;
			_prevCollision[i].collisionNormal = collisionNormal;
			_prevCollision[i].collisionVelocity = velocity;
			_prevCollision[i].firstCollision = false;
			_prevCollision[i].postCollisionPosition = updatedPosition;
			_prevCollision[i].impulse = impulse;
			return;
		}
	}
	_prevCollision.push_back({ body, contactPoint, collisionNormal, velocity, updatedPosition, impulse, true, true });

}

 void Rigidbody::_updateTransform() {

	vector2 scale = parentObject->transform.scale;
	centerOfMass = parentObject->transform.position;

	//nothing changed
	bool scaleChanged = !(scale.x == meshScale.x && scale.y == meshScale.y);
	bool rotChanged = !(parentObject->transform.rotation == meshRot);
	transformChanged = scaleChanged | rotChanged;
	if (!transformChanged){
		return;
	}

	//translate, rotate and scale the polygon
	double rot = (parentObject->transform.rotation - meshRot) * (PI / 180.0);
	for (int i = 0; i < _mesh.v.size(); i++) {
		_mesh.v[i] = { (_mesh.v[i].x / meshScale.x) * scale.x, (_mesh.v[i].y / meshScale.y) * scale.y };
		_mesh.v[i] = {
			_mesh.v[i].x * cos(rot) - _mesh.v[i].y * sin(rot),
			_mesh.v[i].x * sin(rot) + _mesh.v[i].y * cos(rot)
		};
	}

	//calculates the radius of the bb
	if (scaleChanged && (boundingBox != nullptr) ) {
		double radius = 0;
		for (int i = 0; i < _mesh.v.size(); i++) {
			radius = std::max(radius, sqrt(_mesh.v[i].x * _mesh.v[i].x
				+ _mesh.v[i].y * _mesh.v[i].y));
		}
		boundingBox->radius = radius;
	}

	meshScale = scale;
	meshRot = parentObject->transform.rotation;

	//update density and area
	vector2 realScale = { scale.x / meshScale.x, scale.y / meshScale.y };
	area *= realScale.x * realScale.y;
	density = mass / area;

	return;
}

 //return the mesh vertexes with all transforms applied
 void Rigidbody::getMesh(FMesh& m) {

	 std::lock_guard <std::mutex> guard(_meshMutex);

	 if (!_meshUpdated) {
		 //update the mesh in world coordinates for the current frame
		 for (int i = 0; i < _mesh.v.size(); i++) {
			 _world_mesh.v[i] = { _mesh.v[i].x + centerOfMass.x, _mesh.v[i].y + centerOfMass.y };
		 }
		 _meshUpdated = true;
	 }

	 m = _world_mesh;
	 m.centerOfMass = centerOfMass;
 }

 bool Rigidbody::isColliding(Rigidbody* body) {
	 for (int i = 0; i < _prevCollision.size(); i++) {
		 if (_prevCollision[i].collider == body)
			 return true;
	 }
	 return false;
 }

//internal call. Don't use it
void Rigidbody::_resolveCollisions(double timeElapsed) {

	for (int i = 0; i < _prevCollision.size(); i++) {
		vector2 v = velocity;
		vector2 normal = _prevCollision[i].collisionNormal;
		double impulse = _prevCollision[i].impulse;

		velocity = { v.x - impulse / mass * normal.x, v.y - impulse / mass * normal.y };

		parentObject->transform.position += _prevCollision[i].postCollisionPosition;

		vector2 r = getRelativePoint(_prevCollision[i].contactPoint);
		angularVelocity -= (r.cross({ normal.x * impulse, normal.y * impulse }) / momentOfInertia) * 180 / PI;
	}
}

void Rigidbody::_updatePhysics(double timeElapsed, double sleepVelocity) {

	velocity += {frameForce.x* timeElapsed / mass, frameForce.y* timeElapsed / mass};

	//apply velocity contraints
	unsigned long c = constraints;
	if (c & RBContraints::ROT) angularVelocity = 0;
	if (c & RBContraints::X_CONST) velocity = { 0, velocity.y() };
	if (c & RBContraints::Y_CONST) velocity = { velocity.x(), 0 };

	//move the object
	this->parentObject->transform.position += {this->velocity.x() * timeElapsed, this->velocity.y() * timeElapsed};
	this->parentObject->transform.rotation += this->angularVelocity * timeElapsed;
}

void Rigidbody::_resolveDrag(double timeElapsed) {

	for (int i = 0; i < _prevCollision.size(); i++) {
		ResultCollision& c = _prevCollision[i];

		vector2 v1 = velocity;
		vector2 v2 = c.collider->velocity;
		vector2 rv = { v2.x - v1.x, v2.y - v1.y };
		vector2 normal = c.collisionNormal;

		// Solve for the tangent vector
		double t = rv.dot(normal);
		vector2 tangent;
		if (t != 0) {
			tangent = { rv.x - normal.x * t, rv.y - normal.y * t };
			if (tangent.magnitude() == 0) continue;
		}
		else {
			double t = frameForce.dot(normal);
			if (t != 0) {
				tangent = { frameForce.x - normal.x * t, frameForce.y - normal.y * t };
				if (tangent.magnitude() == 0) continue;
			}
			else continue;
		}
		tangent = tangent.normalize();

		// Solve for magnitude to apply along the friction vector
		float jt = -rv.dot(tangent);
		jt = jt / (1 / mass + 1 / c.collider->mass);

		// Use to approximate mu given friction coefficients of each body
		float mu = sqrt(staticFriction * staticFriction + c.collider->staticFriction * c.collider->staticFriction);

		// Clamp magnitude of friction and create impulse vector
		vector2 frictionImpulse;
		if (fabs(jt) < fabs(c.impulse * mu))		//static friction
			frictionImpulse = { jt * tangent.x, jt * tangent.y };
		else{	//dynamic friction
			frictionImpulse = { -c.impulse * tangent.x * mu, -c.impulse * tangent.y * mu };
		}

		// Apply
		velocity += {(1 / mass)* frictionImpulse.x, (1 / mass)* frictionImpulse.y};

		//if the object is almost still in the direction of the tangent
		//and there is no force applied, remove the tangent component
		vector2 v = velocity;
		double Vt_mag = v.dot(tangent);
		if (fabs(Vt_mag) < 0.05 && fabs(frameForce.dot(tangent) < 0.05)) {
			vector2 vt = {tangent.x * Vt_mag, tangent.y * Vt_mag };
			velocity -= vt;
		}
		//B->velocity += (1 / B->mass) * frictionImpulse
	}
}

//internal call. Don't use it
void Rigidbody::_startCollisionFrame(double timeElapsed, vector2 gravity) {

	if (useGravity) {
		if(mass != INFINITY)
			frameForce = { frameForce.x + gravity.x * mass, frameForce.y + gravity.y * mass};
	}

	for (int i = 0; i < _prevCollision.size(); i++) {
		_prevCollision[i].firstCollision = false;
		_prevCollision[i].frameCollision = false;
	}
	_meshUpdated = false;
}

//internal call. Don't use it
void Rigidbody::_endCollisionFrame() {

	frameForce = {};

	for (int i = 0; i < _prevCollision.size(); i++) {

		if (_prevCollision[i].frameCollision == false) {		//no collision detected this frame
			Collision c = {};
			c.collider = _prevCollision[i].collider;

			if (groupMask & c.collider->getParentObject()->group) {
				if (isTrigger) {
					parentObject->OnTriggerExit(c);
				}
				else {
					parentObject->OnCollisionExit(c);
				}
			}

			_prevCollision.erase(_prevCollision.begin() + i);
			i--;
			
			continue;
		}

		if (_prevCollision[i].firstCollision) {		//first collision

			Collision c = {};
			c.collider = _prevCollision[i].collider;
			c.contact = _prevCollision[i].contactPoint;
			c.normal = _prevCollision[i].collisionNormal;
			c.impulse = _prevCollision[i].impulse;

			if (groupMask & c.collider->getParentObject()->group) {
				if (isTrigger) {
					parentObject->OnTriggerEnter(c);
				}
				else {
					parentObject->OnCollisionEnter(c);
				}
			}
		}
		else {	//continuous collision

			Collision c = {};
			c.collider = _prevCollision[i].collider;
			c.contact = _prevCollision[i].contactPoint;
			c.normal = _prevCollision[i].collisionNormal;
			c.impulse = _prevCollision[i].impulse;

			if (groupMask & c.collider->getParentObject()->group) {
				if (isTrigger) {
					parentObject->OnTriggerStay(c);
				}
				else {
					parentObject->OnCollisionStay(c);
				}
			}
		}

	}
}

void Rigidbody::_applyForces(double timeElapsed) {
	velocity += {frameForce.x* timeElapsed / mass, frameForce.y* timeElapsed / mass};
}

bool Rigidbody::IsMovable() {
	return (!isStatic) && !(constraints & (RBContraints::X_CONST | RBContraints::Y_CONST))
		&& mass != INFINITY;
}

void Rigidbody::SetStatic(bool isS) {
	isStatic = isS;
	PhysicsEngine::getInstance()._updateStatic(this);
}

bool Rigidbody::IsStatic() {
	return isStatic;
}
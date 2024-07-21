#include "firefly.h"

#include "physics.h"
#include "gameObject.h"
#include "rigidbody.h"

Firefly::Firefly(double max_acceleration, double max_velocity) :
    _max_acceleration(max_acceleration),
    _max_velocity(max_velocity),
    _time_dir_change(static_cast<double>(rand()) / (RAND_MAX * 2))
 {
    time_to_dir_change = 0;
    accel = {0, 0};

    //initialize the firefly position randomly
	double x = -12 + (rand() % 1000) / 40.0;
	double y = -6 + (rand() % 1000) / 80.0;
	transform.position = {x, y};

    //add a rigidbody to the game object
	std::vector <vector2> mesh;
	PhysicsEngine::getInstance().createRegularPolygon(20, 0.5, mesh);
	AttachRigidbody(mesh);

    //register the game object
	RegisterObject();
}

//update function called every physics cycle. Implements a simple firefly-like behaviour
void Firefly::update(double t) {
    time_to_dir_change += t;

    //once in a while update firefly velocity
    if(time_to_dir_change >= _time_dir_change){
        //update firefly velocity
        double angle = static_cast<double>(rand()) / RAND_MAX * 2 * 3.14159265f;  // Random angle in radians
        double acceleration = static_cast<double>(rand()) / RAND_MAX * _max_acceleration;  // Random acceleration
        accel = {acceleration * cos(angle), acceleration * sin(angle)};
        vector2 v = rigidbody->velocity;
        v.x += accel.x;
        v.y += accel.y;

        //if the firefly when too far from the 0, 0 position, push it torward 0, 0
        vector2 pos = transform.position;
        if (sqrt(pos.x * pos.x + pos.y * pos.y) > 10) {
            v.x -= pos.x;
            v.y -= pos.y;
        }

        // Cap the velocity to the maximum speed
        float speed = sqrt(v.x * v.x +  v.y *  v.y);
        if (speed > _max_velocity) {
            v.x = ( v.x / speed) * _max_velocity;
            v.y = ( v.y / speed) * _max_velocity;
        }

        //apply the new velocity
        rigidbody->velocity = v;
        time_to_dir_change = 0;
    }
    
}

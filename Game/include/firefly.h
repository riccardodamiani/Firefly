#ifndef FIREFLY_H
#define FIREFLY_H

#include "gameObject.h"
#include "structures.h"

class Firefly : public GameObject {
public:
	Firefly(double max_acceleration, double max_velocity);
	void update(double t);
private:
    vector2 accel;
    double time_to_dir_change;
    const double _max_acceleration, _max_velocity, _time_dir_change;
};

#endif  //FIREFLY_H
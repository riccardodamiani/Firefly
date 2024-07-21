#ifndef CAMERA_H
#define CAMERA_H

#include "gameObject.h"
#include "structures.h"


#include <mutex>
#include <atomic>



class Camera : public GameObject {
	struct rot_shake_struct {
		double blendOutTime;
		vector2 oscillation;
	};
	struct trans_shake_struct {
		double blendOutTime;
		vector2 x_oscillation;
		vector2 y_oscillation;
	};
	struct shake_struct {
		double blendOutTime;
		vector2 x_oscillation;
		vector2 y_oscillation;
		vector2 rot_oscillation;
	};
public:
	Camera(vector2 cameraSize, vector2 position, double rotation);
	void update(double elapsedTime);
	void shake(double shakeTime, double blendOutTime, vector2 x_oscillation, vector2 y_oscillation, vector2 rot_oscillation);

private:
	static void shake_anim_func(double time, TransformStruct& val, void* args);
	static vector2 trans_anim_func(double time, void* args);
	std::mutex camera_mutex;
};

#endif

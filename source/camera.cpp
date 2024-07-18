#include "stdafx.h"
#include "camera.h"
#include "gameEngine.h"
#include "gameObject.h"
#include "animation.h"

Camera::Camera(vector2 cameraSize, vector2 position, double rotation) {
	transform.position = position;
	transform.scale = cameraSize;
	transform.rotation = rotation;
	RegisterObject(DecodeName("MainCamera"));
}

void Camera::update(double elapsedTime) {

	//safe to do since the only since the only way the camera animations can be modified 
	//is using shake() which is also protected
	if (_animations.size() > 0) {
		std::lock_guard<std::mutex> guard(_anim_mutex);
		for (int i = 0; i < _animations.size(); i++) {
			if (!_animations[i]->isPlaying()) {
				delete _animations[i];
				_animations.erase(_animations.begin() + i);
			}
		}
	}
}

void Camera::shake(double shakeTime, double blendOutTime, vector2 x_oscillation, vector2 y_oscillation, vector2 rot_oscillation) {

	//std::lock_guard<std::mutex> guard(camera_mutex);
	/*struct rot_shake_struct rot_shake;
	struct trans_shake_struct trans_shake;
	trans_shake = {blendOutTime, x_oscillation , y_oscillation };
	rot_shake = { blendOutTime, rot_oscillation };*/
	shake_struct shake = { blendOutTime, x_oscillation, y_oscillation, rot_oscillation };
	
	std::lock_guard<std::mutex> guard(_anim_mutex);
	/*_animations.push_back(new Animation(&transform.position, trans_anim_func, &trans_shake, sizeof(trans_shake), shakeTime, Animation::PlayMode::SINGLE_RESET, true));
	_animations.push_back(new Animation(&transform.rotation, rot_anim_func, &rot_shake, sizeof(rot_shake), shakeTime, Animation::PlayMode::SINGLE_RESET, true));*/
	_animations.push_back(new Animation(&transform, shake_anim_func, &shake, sizeof(shake_struct), shakeTime, Animation::PlayMode::SINGLE_RESET, true));
}

void Camera::shake_anim_func(double time, TransformStruct &val, void *args) {
	struct shake_struct* s = (struct shake_struct*)args;

	vector2 trans_delta = { s->x_oscillation.x * sin(s->x_oscillation.y * time), s->y_oscillation.x * sin(s->y_oscillation.y * time) };
	double rot_delta = s->rot_oscillation.x * sin(s->rot_oscillation.y * time);

	//blend out
	if (1.0 - time < s->blendOutTime && s->blendOutTime > 0) {
		double damping = (1.0 - time) / s->blendOutTime;
		rot_delta *= damping;
		trans_delta.x *= damping;
		trans_delta.y *= damping;
	}
	val.position = trans_delta;
	val.rotation = rot_delta;
	return;
}


vector2 Camera::trans_anim_func(double time, void* args) {
	struct trans_shake_struct* s = (struct trans_shake_struct*)args;

	vector2 delta = { s->x_oscillation.x * sin(s->x_oscillation.y * time), s->y_oscillation.x * sin(s->y_oscillation.y * time) };
	//blend out
	if (1.0 - time < s->blendOutTime && s->blendOutTime > 0) {
		double damping = (1.0 - time) / s->blendOutTime;
		delta.x *= damping;
		delta.y *= damping;
	}
	return delta;
}

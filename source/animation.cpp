#include "animation.h"
#include "gameEngine.h"
#include "structures.h"
#include "variables.h"
#include <vector>
#include <atomic>
#include <mutex>

Animation::Animation(Double* variable_to_animate, void (*anim_func)(double time, double& anim_val, void* params), void* params, size_t params_size, double animation_time, PlayMode mode, bool play) {
	var_to_animate_double = variable_to_animate;
	var_to_animate_v2 = nullptr;
	var_to_animate_transform = nullptr;
	prev_val_double = 0;
	prev_val_v2 = { 0, 0 };
	prev_val_transform = { {0, 0}, {0, 0}, 0 };
	_isPlaying = play;
	current_animation_time = 0;
	total_animation_time = animation_time;
	this->_mode = mode;

	if (params_size <= 1024 && params != nullptr) {
		anim_func_params = malloc(params_size);
		if(anim_func_params != nullptr)
			memcpy(anim_func_params, params, params_size);
	}
	else anim_func_params = nullptr;

	anim_func_transform = nullptr;
	anim_func_v2 = nullptr;
	anim_func_double = anim_func;
}


Animation::Animation(Vector2* variable_to_animate, void (*anim_func)(double time, vector2& anim_val, void* params), void* params, size_t params_size, double animation_time, PlayMode mode, bool play) {
	var_to_animate_double = nullptr;
	var_to_animate_transform = nullptr;
	var_to_animate_v2 = variable_to_animate;
	prev_val_double = 0;
	prev_val_v2 = { 0, 0 };
	prev_val_transform = { {0, 0}, {0, 0}, 0 };
	_isPlaying = play;
	current_animation_time = 0;
	total_animation_time = animation_time;
	this->_mode = mode;

	if (params_size <= 1024 && params != nullptr) {
		anim_func_params = malloc(params_size);
		if (anim_func_params != nullptr)
			memcpy(anim_func_params, params, params_size);
	}
	else anim_func_params = nullptr;

	anim_func_double = nullptr;
	anim_func_transform = nullptr;
	anim_func_v2 = anim_func;
}

Animation::Animation(Transform* variable_to_animate, void (*anim_func)(double time, TransformStruct& anim_val, void* params), void* params, size_t params_size, double animation_time, PlayMode mode, bool play) {
	var_to_animate_double = nullptr;
	var_to_animate_v2 = nullptr;
	var_to_animate_transform = variable_to_animate;
	prev_val_double = 0;
	prev_val_v2 = { 0, 0 };
	prev_val_transform = { {0, 0}, {0, 0}, 0 };
	_isPlaying = play;
	current_animation_time = 0;
	total_animation_time = animation_time;
	this->_mode = mode;

	if (params_size <= 1024 && params != nullptr) {
		anim_func_params = malloc(params_size);
		if (anim_func_params != nullptr)
			memcpy(anim_func_params, params, params_size);
	}
	else anim_func_params = nullptr;

	anim_func_double = nullptr;
	anim_func_v2 = nullptr;
	anim_func_transform = anim_func;
}

Animation::~Animation() {
	if (anim_func_params != nullptr)
		free(anim_func_params);
}

void Animation::endAnimation() {

	if (_mode == PlayMode::LOOP) {
		std::lock_guard <std::mutex> guard(var_update_mutex);
		current_animation_time = 0;
		_isPlaying = true;
	}
	else if(_mode == PlayMode::SINGLE){
		{
			std::lock_guard <std::mutex> guard(var_update_mutex);
			current_animation_time = total_animation_time;
			_isPlaying = false;
		}
		if (anim_func_double != nullptr) {
			update_double(0);
		}
		else if (anim_func_v2 != nullptr) {
			update_v2(0);
		}
	}
	else {	//SINGLE_RESET
		reset();
	}
}

void Animation::update(double elapsedTime) {

	if (!_isPlaying)
		return;

	if (current_animation_time >= total_animation_time) {
		endAnimation();
		return;
	}

	if(anim_func_double != nullptr){
		update_double(elapsedTime);
	}
	else if (anim_func_v2 != nullptr) {
		update_v2(elapsedTime);
	}
	else if (anim_func_transform != nullptr) {
		update_transform(elapsedTime);
	}
}

void Animation::update_double(double elapsedTime) {
	std::lock_guard <std::mutex> guard(var_update_mutex);

	double t = current_animation_time / total_animation_time;		//create a time variable from 0 to 1
	double newVal = 0;
	anim_func_double(t, newVal, anim_func_params);
	double val = (*var_to_animate_double);
	val -= prev_val_double;
	val += newVal;
	(*var_to_animate_double) = val;
	prev_val_double = newVal;

	double a = current_animation_time + elapsedTime;
	current_animation_time.store(a);
}

void Animation::update_v2(double elapsedTime) {
	std::lock_guard <std::mutex> guard(var_update_mutex);

	double t = current_animation_time / total_animation_time;		//create a time variable from 0 to 1
	vector2 newVal = {0, 0};
	anim_func_v2(t, newVal, anim_func_params);
	vector2 val = (*var_to_animate_v2);
	val.x -= prev_val_v2.x;
	val.y -= prev_val_v2.y;
	val.x += newVal.x;
	val.y += newVal.y;
	(*var_to_animate_v2) = val;
	prev_val_v2 = newVal;

	double a = current_animation_time + elapsedTime;
	current_animation_time.store(a);
}


void Animation::update_transform(double elapsedTime) {
	std::lock_guard <std::mutex> guard(var_update_mutex);

	double t = current_animation_time / total_animation_time;		//create a time variable from 0 to 1
	TransformStruct newVal = { {0, 0}, {0, 0}, 0 };
	anim_func_transform(t, newVal, anim_func_params);
	TransformStruct val = (*var_to_animate_transform);

	val.position.x -= prev_val_transform.position.x;
	val.position.y -= prev_val_transform.position.y;
	val.scale.x -= prev_val_transform.scale.x;
	val.scale.y -= prev_val_transform.scale.y;
	val.rotation -= prev_val_transform.rotation;

	val.position.x += newVal.position.x;
	val.position.y += newVal.position.y;
	val.scale.x += newVal.scale.x;
	val.scale.y += newVal.scale.y;
	val.rotation += newVal.rotation;

	(*var_to_animate_transform) = val;
	prev_val_transform = newVal;

	double a = current_animation_time + elapsedTime;
	current_animation_time.store(a);
}


void Animation::setTime(double time) {
	std::lock_guard <std::mutex> guard(var_update_mutex);
	current_animation_time = time;
}

bool Animation::isPlaying(void) {
	return _isPlaying;
}

void Animation::play(bool play) {
	std::lock_guard <std::mutex> guard(var_update_mutex);
	_isPlaying = play;
}

void Animation::reset() {

	std::lock_guard <std::mutex> guard(var_update_mutex);
	current_animation_time = 0;
	_isPlaying = false;

	if (anim_func_double != nullptr) {
		double val = (*var_to_animate_double);
		val -= prev_val_double;
		(*var_to_animate_double) = val;
		prev_val_double = 0;
	}
	else if (anim_func_v2 != nullptr) {
		vector2 val = (*var_to_animate_v2);
		val.x -= prev_val_v2.x;
		val.y -= prev_val_v2.y;
		(*var_to_animate_v2) = val;
		prev_val_v2 = {0, 0};
	}
	else if (anim_func_transform != nullptr) {
		TransformStruct val = (*var_to_animate_transform);
		val.position.x -= prev_val_transform.position.x;
		val.position.y -= prev_val_transform.position.y;
		val.scale.x -= prev_val_transform.scale.x;
		val.scale.y -= prev_val_transform.scale.y;
		val.rotation -= prev_val_transform.rotation;
		(*var_to_animate_transform) = val;
		prev_val_transform = { {0, 0}, {0, 0}, 0 };
	}
}

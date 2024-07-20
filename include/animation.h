#ifndef ANIMATION_H
#define ANIMATION_H

#include <vector>
#include <mutex>
#include <atomic>
#include "structures.h"
#include "variables.h"
#include "transform.h"
#include "engine_exports.h"

class ENGINE_API Animation {
public:
	enum class PlayMode {
		SINGLE,			//play the animation once. The variable is left with the final value
		SINGLE_RESET,		//play the animation once. Resets the transformation at the end
		LOOP			//loops the animation indefinitely
	};

	Animation(
		Double* variable_to_animate,		//pointer to the variable to animate
		void (*anim_func)(double time, double &anim_val, void *params),	//function that describe the animation
		void* params,									//pointer to the parameters to give to the function
		size_t params_size,								//size of the parameters
		double animation_time,							//total time of the animation
		PlayMode mode,									//mode of the animation
		bool play										//play immidiately
	);

	Animation(
		Vector2* variable_to_animate,		//pointer to the variable to animate
		void (*anim_func)(double time, vector2& anim_val, void *params),	//function that describe the animation
		void* params,									//pointer to the parameters to give to the function
		size_t params_size,								//size of the parameters
		double animation_time,							//total time of the animation
		PlayMode mode,									//mode of the animation
		bool play										//play immidiately
	);

	Animation(
		Transform* variable_to_animate,		//pointer to the atomic variable to animate
		void (*anim_func)(double time, TransformStruct & anim_val, void* params),	//function that describe the animation
		void* params,									//pointer to the parameters to give to the function
		size_t params_size,								//size of the parameters
		double animation_time,							//total time of the animation
		PlayMode mode,									//mode of the animation
		bool play										//play immidiately
	);

	~Animation();

	void update(double elapsedTime);
	void update_double(double elapsedTime);
	void update_v2(double elapsedTime);
	void update_transform(double elapsedTime);
	void play(bool play);
	void reset();
	void setTime(double time);
	bool isPlaying();
private:
	void endAnimation();

	void (*anim_func_v2)(double time, vector2& anim_val, void* params);
	void (*anim_func_double)(double time, double& anim_val, void* params);
	void (*anim_func_transform)(double time, TransformStruct& anim_val, void* params);
	void* anim_func_params;

	Double* var_to_animate_double;
	Vector2* var_to_animate_v2;
	Transform* var_to_animate_transform;

	double prev_val_double;
	vector2 prev_val_v2;
	TransformStruct prev_val_transform;
	
	std::atomic <bool> _isPlaying;
	std::atomic <double> current_animation_time;
	double total_animation_time;
	PlayMode _mode;

	std::mutex var_update_mutex;
};

#endif

#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "structures.h"
#include "variables.h"
#include "gameEngine.h"
#include "rigidbody.h"
#include "entity.h"

#include <vector>
#include <atomic>
#include <mutex>
class Sprite;
class AnimatedSprite;
class Animation;
class Transform;

class GameObject {
public:
	
	GameObject();
	~GameObject();

	virtual void SetActive(bool active);
	virtual void SetVisible(bool visible);
	bool IsVisible();
	bool IsActive();
	void Destroy(void);

	void mainDraw();
	virtual void draw();
	void MainAnimationUpdate(double timeElapsed);
	void mainUpdate(double timeElapsed);
	void mainPreUpdate(double timeElapsed);
	void mainPostUpdate(double timeElapsed);
	virtual void update(double timeElapsed);

	EntityName getObjectName();
	void setTexture(EntityName textureName);

	void AnimateSprite(bool animate);
	void PlaySpriteAnimation(unsigned int id);
	void SetActiveSpriteAnimation(unsigned int id);
	void ResumeSpriteAnimation();
	void PauseSpriteAnimation();
	bool SpriteAnimation_IsPlaying();
	void StopSpriteAnimation();
	void SpriteAnimationNextStep();

	void NewAnimation(Animation *);

	void SetConstraintParent(EntityName objectName, bool translation, bool scale, bool rotation);
	void SetConstraintParent(GameObject *object, bool translation, bool scale, bool rotation);
	void SetConstraintParent(EntityName objectName, bool translation_x, bool translation_y, bool scale_x, bool scale_y, bool rotation);
	void SetConstraintParent(GameObject* object, bool translation_x, bool translation_y, bool scale_x, bool scale_y, bool rotation);
	void ClearConstraintParent();
	void updateConstraintParenting();

	void SetChild(EntityName objectName);
	void SetChild(GameObject* object);
	void SetParent(EntityName objectName);
	void SetParent(GameObject* object);
	bool RemoveChild(EntityName objectName);
	void ClearChild();
	[[nodiscard]] std::vector <EntityName>* GetChild();

	void AttachRigidbody(std::vector <vector2>& vertexes);
	Rigidbody* GetRigidbody();

	virtual void OnCollisionEnter(Collision &);
	virtual void OnCollisionStay(Collision&);
	virtual void OnCollisionExit(Collision&);
	virtual void OnTriggerEnter(Collision&);
	virtual void OnTriggerStay(Collision&);
	virtual void OnTriggerExit(Collision&);

	void setLayer(uint16_t layer);
	uint16_t getLayer();

	Transform transform;
	UInt group;
protected:
	void RegisterObject(EntityName name = 0);
	
	Bool active;
	Bool visible;
	std::vector <AnimatedSprite*> _spriteAnimations;		//sprite animations can be set only from the constructor
	UInt spriteAnimationID;
	std::vector <Animation *> _animations;
	EntityName _objectName;
	std::atomic <Sprite*> _texture;
	
	Rigidbody* rigidbody;

	std::mutex _u_mutex;		//mutex to lock when modifing object variables
	std::mutex _anim_mutex;		//mutex to lock when adding or deleting animations
	std::mutex graphics_anim_mutex;
	std::mutex children_mutex;
	std::mutex constraint_mutex;


	struct constraintParent {
		EntityName parent;
		bool translX, translY, scaleX, scaleY, rotation;
		vector2 lastPos, lastScale;
		double lastRot;
	}_constraintParent;
private:
	Bool animated;
	std::vector <EntityName> _children;
	UInt _layer;		//stores the layer of the element (higher layers have priority over lower layers. The lowest layer is 0)
};

#endif
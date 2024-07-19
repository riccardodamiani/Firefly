#include "entity.h"
#include "gameObject.h"
#include "gameEngine.h"
#include "AnimatedSprite.h"
#include "animation.h"
#include "sprite.h"
#include "variables.h"
#include "transform.h"
#include "physics.h"
#include "rigidbody.h"

#include <mutex>
#include <vector>

GameObject::GameObject(){
	_constraintParent = { 0, false, false, false, false, false, {0, 0}, {0, 0}, 0 };
	animated = false;
	active = true;
	visible = true;
	spriteAnimationID = 0;
	animated = false;
	_texture = nullptr;
	rigidbody = nullptr;
	group = 0x1;
	_layer = 0;

	transform.position = {0, 0};
	transform.scale = { 1, 1 };
	transform.rotation = 0;
}


GameObject::~GameObject() {
	for (int i = 0; i < _animations.size(); i++) {
		delete _animations[i];
	}
	for (int i = 0; i < _spriteAnimations.size(); i++) {
		delete _spriteAnimations[i];
	}
	if (rigidbody != nullptr) {
		delete rigidbody;
	}
}

void GameObject::Destroy(void) {

	visible = false;
	active = false;
	GameEngine::getInstance().DestroyGameObject(this->_objectName);

	GameObject* child;
	std::lock_guard <std::mutex> guard(children_mutex);
	for (int i = 0; i < _children.size(); i++) {
		child = GameEngine::getInstance().FindGameObject(_children[i]);
		if (child != nullptr) {
			child->Destroy();
		}
	}
}

 
/*
* This function register the game object in the game engine with the specified name which must be unique in the entire game. 
* If you don't care about the name leave it 0.
* This function must be called one and only one time in the object contructor otherwise the object will not be displayed nor updated.
*/
void GameObject::RegisterObject(EntityName name) {
	_objectName = GameEngine::getInstance().RegisterGameObject(this, name);
}

//return the Entity Name
EntityName GameObject::getObjectName() {
	return _objectName;
}

void GameObject::SetActive(bool new_active) {
	this->active = new_active;
	GameObject* child;

	std::lock_guard <std::mutex> guard(children_mutex);
	for (int i = 0; i < _children.size(); i++) {
		child = GameEngine::getInstance().FindGameObject(_children[i]);
		if (child != nullptr) {
			child->SetActive(new_active);
		}
	}
}

void GameObject::SetVisible(bool new_visible) {
	this->visible = new_visible;
	GameObject* child;

	std::lock_guard <std::mutex> guard(children_mutex);
	for (int i = 0; i < _children.size(); i++) {
		child = GameEngine::getInstance().FindGameObject(_children[i]);
		if (child != nullptr) {
			child->SetVisible(new_visible);
		}
	}
}

bool GameObject::IsVisible() {
	return visible;
}

bool GameObject::IsActive() {
	return active;
}

//return true if the child was found and removed
bool GameObject::RemoveChild(EntityName objectName) {
	std::lock_guard <std::mutex> guard(children_mutex);
	for (int i = 0; i < _children.size(); i++) {
		if (objectName == _children[i]) {
			_children.erase(_children.begin() + i);
			return true;
		}
	}
	return false;
}

void GameObject::ClearChild() {
	std::lock_guard <std::mutex> guard(children_mutex);
	_children.clear();
}

/*
* Return a pointer to a vector containing the children's names.
* You should delete the vector once you have done.
*/
[[nodiscard]] std::vector <EntityName>* GameObject::GetChild() {
	std::lock_guard <std::mutex> guard(children_mutex);

	return new std::vector <EntityName>(_children);
}


void GameObject::SetChild(EntityName objectName) {
	if (objectName == this->_objectName || objectName == 0)
		return;
	std::lock_guard <std::mutex> guard(children_mutex);
	_children.push_back(objectName);
}

void GameObject::SetChild(GameObject* object) {
	if (object == this || object == nullptr)
		return;
	std::lock_guard <std::mutex> guard(children_mutex);
	
	EntityName objectName = object->getObjectName();
	if (objectName == 0)
		return;
	_children.push_back(objectName);
}

void GameObject::SetParent(EntityName objectName) {
	if (objectName == this->_objectName || objectName == 0)
		return;
	
	GameObject * parent = GameEngine::getInstance().FindGameObject(objectName);
	if (parent == nullptr)
		return;
	parent->SetChild(this->_objectName);
}

void GameObject::SetParent(GameObject* object) {
	if (object == this || object == nullptr)
		return;

	object->SetChild(this->_objectName);
}

//called by the game engine to get the texture to be printed on screen
//this function only reads atomic values so it doesn't need a mutex
void GameObject::mainDraw() {

	if (!visible)
		return;

	if (animated) {	//animations are enabled
		if (spriteAnimationID < _spriteAnimations.size()) {
			AnimatedSprite* as = _spriteAnimations[spriteAnimationID];
			if (as != nullptr)
				as->draw(transform.position, transform.scale, transform.rotation);
			return;
		}
	}
	else {	//animations are disabled->draw the main texture
		Sprite* s = _texture;
		if (s != nullptr) {
			s->draw(transform.position, transform.scale, transform.rotation);
		}
	}

	draw();
	
}

void GameObject::draw() {}

//play a animation by its id. 
//All other animations are stopped
void GameObject::PlaySpriteAnimation(unsigned int id) {
	std::lock_guard<std::mutex> guard(graphics_anim_mutex);

	if (spriteAnimationID != id) {
		for (int i = 0; i < _spriteAnimations.size(); i++) {
			_spriteAnimations[i]->stopAnimation();
		}
	}

	if (id < _spriteAnimations.size()) {
		_spriteAnimations[id]->playAnimation(true);
		spriteAnimationID = id;
	}
}

bool GameObject::SpriteAnimation_IsPlaying() {
	std::lock_guard<std::mutex> guard(graphics_anim_mutex);
	if (spriteAnimationID < _spriteAnimations.size()) {
		return _spriteAnimations[spriteAnimationID]->isPlaying();
	}
}

void GameObject::AnimateSprite(bool animate) {
	this->animated = animate;
}

//set a sprite animation as the current animation for the object without playing it
//stops all other animations
void GameObject::SetActiveSpriteAnimation(unsigned int id) {
	std::lock_guard<std::mutex> guard(graphics_anim_mutex);
	if (id < _spriteAnimations.size()) {
		spriteAnimationID = id;
	}
}

void GameObject::SpriteAnimationNextStep() {
	std::lock_guard<std::mutex> guard(graphics_anim_mutex);
	if (spriteAnimationID < _spriteAnimations.size()) {
		_spriteAnimations[spriteAnimationID]->nextStep();
	}
}

//resume the current sprite animation
void GameObject::ResumeSpriteAnimation() {
	std::lock_guard<std::mutex> guard(graphics_anim_mutex);
	if (spriteAnimationID < _spriteAnimations.size()) {
		_spriteAnimations[spriteAnimationID]->playAnimation(true);
	}
}

//pause the current sprite animation
void GameObject::PauseSpriteAnimation() {
	std::lock_guard<std::mutex> guard(graphics_anim_mutex);
	if (spriteAnimationID < _spriteAnimations.size()) {
		_spriteAnimations[spriteAnimationID]->playAnimation(false);
	}

}

//reset the current sprite animation at its first frame
void GameObject::StopSpriteAnimation() {
	std::lock_guard<std::mutex> guard(graphics_anim_mutex);
	if (spriteAnimationID < _spriteAnimations.size()) {
		_spriteAnimations[spriteAnimationID]->stopAnimation();
	}

}

void GameObject::setTexture(EntityName textureName) {
	std::lock_guard <std::mutex> guard(_u_mutex);
		
	Sprite* tempT;

	tempT = _texture;
	_texture = new Sprite(_layer, textureName);
	if (tempT != nullptr) {
		delete tempT;
	}
}


void GameObject::SetConstraintParent(EntityName objectName, bool translation, bool rotation, bool scale) {
	GameObject* obj;
	if ((obj = GameEngine::getInstance().FindGameObject(objectName)) == nullptr || obj == this) return;
	_constraintParent = { objectName, translation, translation, scale, scale, rotation,
		obj->transform.position, obj->transform.scale, obj->transform.rotation };

}

void GameObject::SetConstraintParent(GameObject* object, bool translation, bool scale, bool rotation) {
	if (object == nullptr || object->getObjectName() == 0) return;
	_constraintParent = { object->getObjectName(), translation, translation, scale, scale, rotation,
		object->transform.position, object->transform.scale, object->transform.rotation };

}

void GameObject::SetConstraintParent(EntityName objectName, bool translation_x, bool translation_y, bool scale_x, bool scale_y, bool rotation) {
	GameObject* obj;
	if ((obj = GameEngine::getInstance().FindGameObject(objectName)) == nullptr || obj == this) return;
	_constraintParent = { objectName, translation_x, translation_y, scale_x, scale_y, rotation,
		obj->transform.position, obj->transform.scale, obj->transform.rotation };

}

void GameObject::SetConstraintParent(GameObject* object, bool translation_x, bool translation_y, bool scale_x, bool scale_y, bool rotation) {
	if (object == nullptr || object->getObjectName() == 0) return;
	_constraintParent = { object->getObjectName(), translation_x, translation_y, scale_x, scale_y, rotation,
		object->transform.position, object->transform.scale, object->transform.rotation };

}

void GameObject::ClearConstraintParent() {
	_constraintParent.parent = 0;
	
}

void GameObject::MainAnimationUpdate(double timeElapsed) {
	if (active && visible) {
		//play object animations
		std::lock_guard<std::mutex> guard(_anim_mutex);		//avoid cuncurrent modification of animations
		std::lock_guard<std::mutex> guard1(graphics_anim_mutex);

		if (_animations.size() > 0) {
			for (int i = 0; i < _animations.size(); i++) {
				if (_animations[i]->isPlaying()) {
					_animations[i]->update(timeElapsed);
				}
			}
		}
		if (animated && spriteAnimationID < _spriteAnimations.size()) {
			AnimatedSprite* as = _spriteAnimations[spriteAnimationID];
			if (as != nullptr && as->isPlaying())
				as->update(timeElapsed);
			return;
		}
	}
}

void GameObject::mainPreUpdate(double timeElapsed) {
	if (rigidbody != nullptr) {
		rigidbody->_updateTransform();
	}
	
}

void GameObject::mainPostUpdate(double timeElapsed) {
	if (active && visible) {
		updateConstraintParenting();
	}
}

void GameObject::updateConstraintParenting() {
	if (_constraintParent.parent == 0)
		return;

	GameObject* p = GameEngine::getInstance().FindGameObject(_constraintParent.parent);

	if (p == nullptr) {	
		_constraintParent.parent = 0;
		return;
	}
	if (p->_constraintParent.parent != 0) {		//the parent has a parent
		p->updateConstraintParenting();
	}

	std::lock_guard <std::mutex> guard(constraint_mutex);		//avoid multiple updating of the contraints
	//update transform relative to parent
	if (_constraintParent.translX || _constraintParent.translY) {
		vector2 parentPos = p->transform.position;
		vector2 rel = { parentPos.x - _constraintParent.lastPos.x, parentPos.y - _constraintParent.lastPos.y };
		_constraintParent.lastPos = parentPos;

		if(!_constraintParent.translX) rel.x = 0;
		if (!_constraintParent.translY) rel.y = 0;
		transform.position += rel;
	}
	if (_constraintParent.scaleX || _constraintParent.scaleY) {
		vector2 parentScale = p->transform.scale;
		vector2 rel = { parentScale.x / _constraintParent.lastScale.x, parentScale.y / _constraintParent.lastScale.y };
		_constraintParent.lastScale = parentScale;

		if (!_constraintParent.scaleX) rel.x = 1;
		if (!_constraintParent.scaleY) rel.y = 1;
		transform.ApplyScale(rel, p->transform.position);
	}
	if (_constraintParent.rotation) {
		double parentRot = p->transform.rotation;
		double rel = parentRot - _constraintParent.lastRot;
		_constraintParent.lastRot = parentRot;

		transform.ApplyRotation(rel, p->transform.position);
	}
	
}

void GameObject::NewAnimation(Animation *animation) {
	std::lock_guard<std::mutex> guard(_anim_mutex);
	_animations.push_back(animation);
}

//called from game engine to update physics
void GameObject::mainUpdate(double timeElapsed) {

	if (active && visible) {

		//call virtual update
		this->update(timeElapsed);

	}

}

//update the physics of the object. 
//To avoid concurrency problems when updating a variable use std::lock_guard<std::mutex> guard(u_mutex) or u_mutex.lock() and u_mutex.unlock();
void GameObject::update(double timeElapsed) {

}


void GameObject::AttachRigidbody(std::vector <vector2>& vertexes) {
	rigidbody = new Rigidbody(this, vertexes);
}

Rigidbody* GameObject::GetRigidbody() {
	return rigidbody;
}


void GameObject::setLayer(uint16_t layer) {
	if (layer == _layer)
		return;
	if (_texture) {
		_texture.load()->setLayer(layer);
	}
	for (auto it = _spriteAnimations.begin(); it != _spriteAnimations.end(); it++) {
		(*it)->setLayer(layer);
	}
	_layer = layer;
}

uint16_t GameObject::getLayer() {
	return _layer;
}

void GameObject::OnCollisionEnter(Collision& c) {}
void GameObject::OnCollisionStay(Collision& c) {}
void GameObject::OnCollisionExit(Collision& c) {}
void GameObject::OnTriggerEnter(Collision& c) {}
void GameObject::OnTriggerStay(Collision& c) {}
void GameObject::OnTriggerExit(Collision& c) {}
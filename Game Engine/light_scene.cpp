#include "stdafx.h"
#include "light_scene.h"

#include "gui_text.h"
#include "gui_button.h"
#include "camera.h"
#include "animation.h"
#include "audio.h"

LightScene::LightScene(unsigned int id) : Scene(id) {

}



void cw_rot_func(double time, TransformStruct& val, void* ptr) {
	val.rotation = -time * 360;
}
void ccw_rot_func(double time, TransformStruct& val, void* ptr) {
	val.rotation = time * 360;
}

void LightScene::onload() {

	_GameEngine->AllocGlobalVariable_Double(DecodeName("timer"), 1.0);
	_GameEngine->AllocGlobalVariable_Double(DecodeName("timer1"), 5.0);

	_graphicsEngine->EnableSceneLighting(true, 3);
	GameObject* cam = new Camera({ 32, 18 }, { 0, 0 }, 0);		//create a new camera in the scene

	GameObject* renderFps = new GUI_Text(DecodeName("render fps text"), 1, DecodeName("redFont"), { 0, 0.6 }, { 8, 1.2 }, 1);
	renderFps->SetConstraintParent(cam, true, true, true);

	GameObject* gameFps = new GUI_Text(DecodeName("game fps text"), 2, DecodeName("redFont"), { 0, -0.6 }, { 8, 1.2 }, 1);
	gameFps->SetConstraintParent(cam, true, true, true);

	GameObject* button = new GUI_Button(DecodeName("button"), DecodeName("button texture"), 3, { 0, -3 }, { 2, 0.5 }, 2);
	button->SetConstraintParent(cam, true, true, true, true, true);

	GameObject* org = new LightObject(DecodeName("mainLight"), { 0, 0 }, 0, 1, 360, { 200, 200, 200, 250 }, LightType::POINT_LIGHT);
	class Point : public GameObject {
	public:
		Point(vector2 position) {
			transform.position = position;
			RegisterObject();
		}
	};

	for (int j = 0; j < 30; j++) {
		double x = ((rand() % 1000) / 1000.0) * 40 - 20;
		double y = ((rand() % 1000) / 1000.0) * 40 - 20;
		GameObject* ref = new Point({x, y});
		Animation* a = new Animation(&ref->transform, cw_rot_func, nullptr, 0, 60, Animation::PlayMode::LOOP, true);
		ref->NewAnimation(a);
		for (int i = 0; i < 100; i++) {
			double y = ((rand() % 1000) / 1000.0) * 6.28;
			double x = ((rand() % 1000) / 1000.0) * 40;
			GameObject* obj = new LightObject(0, { x*std::cos(y), x*std::sin(y) }, 0, (LightObject*)org);
			obj->SetConstraintParent(ref, false, false, true);
		}
	}
	
}


void LightScene::onfree() {
	_GameEngine->DestroyGlobalVariable(DecodeName("timer"));
	_GameEngine->DestroyGlobalVariable(DecodeName("timer1"));
}


void LightScene::scene_callback(GameEvent event, double timeElapsed) {
	if (event == GameEvent::GAME_QUIT) {
		//here you can add code that need to be excecuted when the user close the window
		_GameEngine->Quit();
	}

	GameObject* camera = _GameEngine->FindGameObject(DecodeName("MainCamera"));
	if (camera) {
		if (_InputEngine->didMouseWheelMove()) {
			int wY = _InputEngine->getLastWheelMoviment().second;
			if (wY > 0) {
				camera->transform.scale *= {0.9, 0.9};
			}
			else {
				camera->transform.scale *= {1.1, 1.1};
			}
		}
		if (_InputEngine->isKeyHeld(SDL_SCANCODE_LEFT)) {
			camera->transform.position += {-0.05, 0};
		}
		if (_InputEngine->isKeyHeld(SDL_SCANCODE_RIGHT)) {
			camera->transform.position += {0.05, 0};
		}
		if (_InputEngine->isKeyHeld(SDL_SCANCODE_UP)) {
			camera->transform.position += {0, 0.05};
		}
		if (_InputEngine->isKeyHeld(SDL_SCANCODE_DOWN)) {
			camera->transform.position += {0, -0.05};
		}
	}

	Double* timer = (Double*)_GameEngine->GetGlobalVariable(DecodeName("timer"));
	Double* timer1 = (Double*)_GameEngine->GetGlobalVariable(DecodeName("timer1"));
	if (timer) {
		(*timer) -= timeElapsed;
		if (*timer < 0) {

			GUI_Text* text = (GUI_Text*)_GameEngine->FindGameObject(DecodeName("render fps text"));
			if (text != nullptr) {
				text->setText("Render  FPS:  " + std::to_string(_GameEngine->GetRenderFPS()));
			}
			text = (GUI_Text*)_GameEngine->FindGameObject(DecodeName("game fps text"));
			if (text != nullptr) {
				text->setText("Game  FPS:  " + std::to_string(_GameEngine->GetGameFPS()));
			}
			*timer = 1.0;
		}
	}
	if (timer1) {
		(*timer1) -= timeElapsed;
		if (*timer1 < 0) {
			LightObject* light = (LightObject*)_GameEngine->FindGameObject(DecodeName("mainLight"));
			if (light) {
				light->SetColor({ (unsigned char)(rand() % 255), 
					(unsigned char)(rand() % 255),
					(unsigned char)(rand() % 255), 255 });
			}
			*timer1 = 5.0;
		}
	}
}


void LightScene::gui_listener(GUI_Element* element, GuiAction action) {
	switch (element->GetElementCode()) {
	case 3:		//button
	{
		if (action == GuiAction::MOUSE_MOVED_OVER) {
			((GameObject*)element)->setTexture(DecodeName("button texture 1"));
			_AudioEngine->PlayUIEffect(DecodeName("tic"), true, 30, element->transform.position);
		}
		else if (action == GuiAction::MOUSE_MOVED_OUT) {
			((GameObject*)element)->setTexture(DecodeName("button texture"));
			_AudioEngine->PlayUIEffect(DecodeName("tic"), true, 30, element->transform.position);
		}
		else if (action == GuiAction::LEFT_BUTTON_UP) {
			_GameEngine->LoadScene(0);
		}
		break;
	}
	}
}

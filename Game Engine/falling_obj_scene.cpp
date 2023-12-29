#include "stdafx.h"
#include "falling_obj_scene.h"
#include "graphics.h"
#include "camera.h"
#include "physics.h"
#include "gui_text.h"
#include "gui_button.h"
#include "gui_slider.h"
#include "gui_panel.h"
#include "test class.h"
#include "projectile.h"
#include "AnimatedSprite.h"
#include "audio.h"

#include <memory>


FallingObjScene::FallingObjScene(unsigned int id) : Scene(id) {

}


void texture_filter(CustomFilterData& data) {
	float dist = sqrt(data.x * data.x + data.y * data.y) / 2;
	data.pixelColor.r = data.pixelColor.g = data.pixelColor.b = data.pixelColor.a = 100;
	data.pixelColor.r += dist;
}

void circle_filter(CustomFilterData& data) {
	if (sqrt(data.x * data.x + data.y * data.y) > data.textureWidth / 2)
		return;
	if (fabs(data.x) < 3) {
		data.pixelColor.r = 200;
		data.pixelColor.g = data.pixelColor.b = 0;
		data.pixelColor.a = 255;
		return;
	}
	data.pixelColor.r = data.pixelColor.g = data.pixelColor.b = 100;
	data.pixelColor.a = 255;
}

void FallingObjScene::onload() {

	//_graphicsEngine->LoadTextureGroup("test group");

	auto bg_panel = new GUI_Panel(DecodeName("loading_panel"), 8, DecodeName("loading_bg"), { 0, 0 }, { 32, 18 }, 4);
	auto slideAnim = new AnimatedSprite("slider", 1.0, 155, false, false, 5);
	GUI_Slider* slide = new GUI_Slider(DecodeName("loading_slider"), 9, slideAnim, { -0.4, 0.5 }, { 4, 0.4 }, 0, 100.0, 0.1, 0.9, 5);
	slide->SetActive(false);

	GameObject* cam = new Camera({ 32, 18 }, { 0, 0 }, 0);		//create a new camera in the scene

	_GameEngine->SetGameFPS(600);

	GameObject* renderFps = new GUI_Text(DecodeName("render fps text"), 1, DecodeName("redFont"), { -6.8, 4 }, { 2.0, 0.3 }, 2);
	renderFps->SetConstraintParent(cam, true, true, true);

	GameObject* gameFps = new GUI_Text(DecodeName("game fps text"), 2, DecodeName("redFont"), { -6.8, 3.7 }, { 2.0, 0.3 }, 2);
	gameFps->SetConstraintParent(cam, true, true, true);

	GameObject* bodiesCount = new GUI_Text(DecodeName("bodies count text"), 4, DecodeName("redFont"), { -6.8, 3.4 }, { 2.0, 0.3 }, 2);
	bodiesCount->SetConstraintParent(cam, true, true, true);

	_GameEngine->AllocGlobalVariable_Double(DecodeName("timer"), 0.0);
	_GameEngine->AllocGlobalVariable_Double(DecodeName("loading_timer"), 0.5);

	SDL_Color c1 = { 150, 50, 50, 150 };
	SDL_Color c2 = { 200, 200, 200, 150 };
	_graphicsEngine->CreateCustomTexture(500, 300, texture_filter, DecodeName("button texture"), nullptr);
	_graphicsEngine->CreateRectangleTexture(c2, 500, 300, true, DecodeName("button texture 1"));

	GameObject* button = new GUI_Button(DecodeName("button"), DecodeName("button texture"), 3, { -1, -3 }, { 2, 0.5 }, 2);
	button->SetConstraintParent(cam, true, true, true, true, true);

	button = new GUI_Button(DecodeName("audio test button"), DecodeName("button texture"), 5, { 1, -3 }, { 2, 0.5 }, 2);

	_graphicsEngine->CreateRectangleTexture(c1, 500, 300, true, DecodeName("poly collided"));
	_graphicsEngine->CreateRectangleTexture(c2, 500, 300, true, DecodeName("poly not collided"));
	_graphicsEngine->CreateCustomTexture(400, 400, circle_filter, DecodeName("circle not collided"), nullptr);
	_graphicsEngine->CreateRectangleTexture(c1, 50, 300, true, DecodeName("wall material"));
	_graphicsEngine->CreateCircleTexture(c2, 200, true, DecodeName("sphere"));
	_graphicsEngine->CreateCircleTexture(c1, 200, true, DecodeName("red sphere"));

	for (int j = 0; j < 4; j++) {
		for (int i = 0; i < 6; i++) {
			new Test(DecodeName(""), false, { (double)(-8.0 + i * 3.4 + (j % 2) * 1.6), (double)(3 - j) * 1.8 });
		}
	}

	_PhysicsEngine->SetGravity({ 0, -9.81 });
	_PhysicsEngine->SetSleepVelocity(0.05);
	load_sound = false;
	loading_scene = true;
}

void FallingObjScene::onfree() {
	_GameEngine->DestroyGlobalVariable(DecodeName("timer"));
	_GameEngine->DestroyGlobalVariable(DecodeName("loading_timer"));
	_audioTest->Stop();
	_audioTest = nullptr;
}

void FallingObjScene::scene_callback(GameEvent event, double timeElapsed) {
	if (event == GameEvent::GAME_QUIT) {
		//here you can add code that need to be excecuted when the user close the window
		_GameEngine->Quit();
	}

	//display loading screen
	if (loading_scene) {
		float scene_loading = GetLoadingState();
		if (scene_loading < 100) {
			((GUI_Slider*)_GameEngine->FindGameObject(DecodeName("loading_slider")))->SetValue(scene_loading);
		}
		else {
			Double* timer = (Double*)_GameEngine->GetGlobalVariable(DecodeName("loading_timer"));
			if (*timer > 0) {
				*timer -= timeElapsed;
			}
			else {
				loading_scene = false;
				_GameEngine->FindGameObject(DecodeName("loading_slider"))->Destroy();
				_GameEngine->FindGameObject(DecodeName("loading_panel"))->Destroy();
			}
		}
	}
	

	GameObject* camera = _GameEngine->FindGameObject(DecodeName("MainCamera"));
	if (camera) {
		if (!load_sound) {
			load_sound = true;
			_audioTest = std::shared_ptr <UIAudioObj>(new UIAudioObj("lavender", true, 100, { -20, 10 }));
			_audioTest->Play();
		}
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
			text = (GUI_Text*)_GameEngine->FindGameObject(DecodeName("bodies count text"));
			if (text != nullptr) {
				text->setText("Bodies:  " + std::to_string(_PhysicsEngine->GetBodiesCount()));
			}
			*timer = 0.5;
			for (int i = 0; i < 20; i++) {
				new Projectile();
			}
		}
	}
}

void FallingObjScene::gui_listener(GUI_Element* element, GuiAction action) {

	switch (element->GetElementCode()) {
	case 3:		//button
	{
		if (action == GuiAction::MOUSE_MOVED_OVER) {
			((GameObject*)element)->setTexture(DecodeName("button texture 1"));
			_AudioEngine->PlayUIEffect(DecodeName("tic"), nullptr, true, 30, element->transform.position);
		}
		else if (action == GuiAction::MOUSE_MOVED_OUT) {
			((GameObject*)element)->setTexture(DecodeName("button texture"));
			_AudioEngine->PlayUIEffect(DecodeName("tic"), nullptr, true, 30, element->transform.position);
		}
		else if (action == GuiAction::LEFT_BUTTON_UP) {
			_GameEngine->LoadScene(1);
		}
		break;
	}
	case 5:	//audio test button
	{
		if (action == GuiAction::MOUSE_MOVED_OVER) {
			((GameObject*)element)->setTexture(DecodeName("button texture 1"));
			_AudioEngine->PlayUIEffect(DecodeName("tic"), nullptr, true, 30, element->transform.position);
		}
		else if (action == GuiAction::MOUSE_MOVED_OUT) {
			((GameObject*)element)->setTexture(DecodeName("button texture"));
			_AudioEngine->PlayUIEffect(DecodeName("tic"), nullptr, true, 30, element->transform.position);
		}
		else if (action == GuiAction::LEFT_BUTTON_UP) {
			if (_audioTest->GetStatus() == AUDIO_STATUS_PLAYING) {
				_audioTest->Pause();
			}
			else if (_audioTest->GetStatus() == AUDIO_STATUS_PAUSE) {
				_audioTest->Resume();
			}
		}
		break;
	}
	case 4:		//slider
	{
		if (action == GuiAction::MOUSE_MOVED_OVER) {
			_AudioEngine->PlayUIEffect(DecodeName("tic"));
		}
		else if (action == GuiAction::MOUSE_MOVED_OUT) {
			_AudioEngine->PlayUIEffect(DecodeName("tic"));
		}
		else if (action == GuiAction::LEFT_BUTTON_DOWN) {
			GameObject* light2 = _GameEngine->FindGameObject(DecodeName("light2"));
			if (light2) {
				light2->transform.rotation = ((GUI_Slider*)element)->getSliderValue();
			}
		}
		break;
	}
	}
}
#include "falling_obj_scene.h"
#include "graphics.h"
#include "camera.h"
#include "physics.h"
#include "gui_text.h"
#include "gui_button.h"
#include "gui_slider.h"
#include "gui_panel.h"
#include "gui_droplist.h"
#include "test_class.h"
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

	GraphicsEngine::getInstance().SetLightingQuality(LightingQuality::HIGH_QUALITY);
	auto a = new LightObject(DecodeName("globallight1"), { -7, 7 }, -75, 35, 180, { 255, 0, 0, 0 }, LightType::POINT_LIGHT);
	a = new LightObject(DecodeName("globallight2"), { 7, 7 }, -105, 35, 180, { 0, 0, 255, 0 }, LightType::POINT_LIGHT);
	GraphicsEngine::getInstance().EnableSceneLighting(true, 1);

	GameObject* cam = new Camera({ 32, 18 }, { 0, 0 }, 0);		//create a new camera in the scene

	GameEngine::getInstance().SetGameFPS(600);

	GameObject* renderFps = new GUI_Text(DecodeName("render fps text"), 1, DecodeName("redFont"), { -6.8, 4 }, { 2.0, 0.3 }, 2);
	renderFps->SetConstraintParent(cam, true, true, true);

	GameObject* gameFps = new GUI_Text(DecodeName("game fps text"), 2, DecodeName("redFont"), { -6.8, 3.7 }, { 2.0, 0.3 }, 2);
	gameFps->SetConstraintParent(cam, true, true, true);

	GameObject* bodiesCount = new GUI_Text(DecodeName("bodies count text"), 4, DecodeName("redFont"), { -6.8, 3.4 }, { 2.0, 0.3 }, 2);
	bodiesCount->SetConstraintParent(cam, true, true, true);

	GameEngine::getInstance().AllocGlobalVariable_Double(DecodeName("timer"), 0.0);
	GameEngine::getInstance().AllocGlobalVariable_Double(DecodeName("light_timer"), 3.0);
	GameEngine::getInstance().AllocGlobalVariable_Double(DecodeName("loading_timer"), 0.5);

	RGBA_Color c1 = { 150, 50, 50, 150 };
	RGBA_Color c2 = { 200, 200, 200, 150 };
	GraphicsEngine::getInstance().CreateCustomTexture(500, 300, texture_filter, DecodeName("button texture"), nullptr);
	GraphicsEngine::getInstance().CreateRectangleTexture(c2, 500, 300, true, DecodeName("button texture 1"));

	GameObject* button = new GUI_Button(DecodeName("button"), DecodeName("button texture"), 3, { -1.2, -3 }, { 2, 0.5 }, 2);
	button->SetConstraintParent(cam, true, true, true, true, true);

	button = new GUI_Button(DecodeName("audio test button"), DecodeName("button texture"), 5, { 1.2, -3 }, { 2, 0.5 }, 2);

	_audioTest = new AudioSource(EntityName("lavenderSound"), "lavender", 0, 30, true);
	((GameObject*)_audioTest)->transform.position = { 0, 0 };

	GUI_Droplist* droplist = new GUI_Droplist(DecodeName("droplist"), 12, DecodeName("droplist_icon"), DecodeName("droplist_bg"), DecodeName("redFont"), 
		0.2, {0, -4}, {2.5, 0.5}, 2);
	droplist->addEntry("rosso");
	droplist->addEntry("verde");
	droplist->addEntry("blu");
	droplist->addEntry("giallo");

	GraphicsEngine::getInstance().CreateRectangleTexture(c1, 500, 300, true, DecodeName("poly collided"));
	GraphicsEngine::getInstance().CreateRectangleTexture(c2, 500, 300, true, DecodeName("poly not collided"));
	GraphicsEngine::getInstance().CreateCustomTexture(400, 400, circle_filter, DecodeName("circle not collided"), nullptr);
	GraphicsEngine::getInstance().CreateRectangleTexture(c1, 50, 300, true, DecodeName("wall material"));
	GraphicsEngine::getInstance().CreateCircleTexture(c2, 200, true, DecodeName("sphere"));
	GraphicsEngine::getInstance().CreateCircleTexture(c1, 200, true, DecodeName("red sphere"));

	for (int j = 0; j < 4; j++) {
		for (int i = 0; i < 6; i++) {
			new Test(DecodeName(""), { (double)(-8.0 + i * 3.4 + (j % 2) * 1.6), (double)(3 - j) * 1.8 });
		}
	}

	PhysicsEngine::getInstance().SetGravity({ 0, -9.81 });
	PhysicsEngine::getInstance().SetSleepVelocity(0.05);
	loading_scene = true;
}

void FallingObjScene::onfree() {
	GameEngine::getInstance().DestroyGlobalVariable(DecodeName("timer"));
	GameEngine::getInstance().DestroyGlobalVariable(DecodeName("loading_timer"));
	_audioTest->Stop();
	_audioTest = nullptr;
}

void FallingObjScene::scene_callback(GameEvent event, double timeElapsed) {
	if (event == GameEvent::GAME_QUIT) {
		//here you can add code that need to be excecuted when the user close the window
		GameEngine::getInstance().Quit();
	}

	//display loading screen
	if (loading_scene) {
		float scene_loading = GetLoadingState();
		if (scene_loading < 100) {
			((GUI_Slider*)GameEngine::getInstance().FindGameObject(DecodeName("loading_slider")))->SetValue(scene_loading);
		}
		else {
			Double* timer = (Double*)GameEngine::getInstance().GetGlobalVariable(DecodeName("loading_timer"));
			if (*timer > 0) {
				*timer -= timeElapsed;
			}
			else {
				loading_scene = false;
				_audioTest->Play();
				GameEngine::getInstance().FindGameObject(DecodeName("loading_slider"))->Destroy();
				GameEngine::getInstance().FindGameObject(DecodeName("loading_panel"))->Destroy();
			}
		}
	}
	

	GameObject* camera = GameEngine::getInstance().FindGameObject(DecodeName("MainCamera"));
	if (camera && !loading_scene) {
		if (InputEngine::getInstance().didMouseWheelMove()) {
			int wY = InputEngine::getInstance().getLastWheelMoviment().second;
			if (wY > 0) {
				camera->transform.scale *= {0.9, 0.9};
			}
			else {
				camera->transform.scale *= {1.1, 1.1};
			}
		}
		if (InputEngine::getInstance().isKeyHeld(Scancode::SCANCODE_LEFT)) {
			camera->transform.position += {-0.05, 0};
		}
		if (InputEngine::getInstance().isKeyHeld(Scancode::SCANCODE_RIGHT)) {
			camera->transform.position += {0.05, 0};
		}
		if (InputEngine::getInstance().isKeyHeld(Scancode::SCANCODE_UP)) {
			camera->transform.position += {0, 0.05};
		}
		if (InputEngine::getInstance().isKeyHeld(Scancode::SCANCODE_DOWN)) {
			camera->transform.position += {0, -0.05};
		}
	}

	Double* timer = (Double*)GameEngine::getInstance().GetGlobalVariable(DecodeName("light_timer"));
	if (timer) {
		(*timer) -= timeElapsed;
		if (*timer < 0) {
			*timer = 3;
			LightObject* globallight = (LightObject*)GameEngine::getInstance().FindGameObject(DecodeName("globallight1"));
			if (globallight) {
				std::random_device dev;
				std::mt19937 rng(dev());
				std::uniform_real_distribution<double> dist6(10, 40);
				std::uniform_int_distribution<long> color_dist(1, 255);
				globallight->SetColor({ (uint8_t)color_dist(rng), (uint8_t)color_dist(rng), (uint8_t)color_dist(rng), 0 });
				globallight->SetPower(dist6(rng));
			}
			globallight = (LightObject*)GameEngine::getInstance().FindGameObject(DecodeName("globallight2"));
			if (globallight) {
				std::random_device dev;
				std::mt19937 rng(dev());
				std::uniform_real_distribution<double> dist6(10, 40);
				std::uniform_int_distribution<long> color_dist(1, 255);
				globallight->SetColor({ (uint8_t)color_dist(rng), (uint8_t)color_dist(rng), (uint8_t)color_dist(rng), 0 });
				globallight->SetPower(dist6(rng));
			}
		}
	}

	timer = (Double*)GameEngine::getInstance().GetGlobalVariable(DecodeName("timer"));
	if (timer) {
		(*timer) -= timeElapsed;
		if (*timer < 0) {
			
			GUI_Text* text = (GUI_Text*)GameEngine::getInstance().FindGameObject(DecodeName("render fps text"));
			if (text != nullptr) {
				text->setText("Render  FPS:  " + std::to_string(GameEngine::getInstance().GetRenderFPS()));
			}
			text = (GUI_Text*)GameEngine::getInstance().FindGameObject(DecodeName("game fps text"));
			if (text != nullptr) {
				text->setText("Game  FPS:  " + std::to_string(GameEngine::getInstance().GetGameFPS()));
			}
			text = (GUI_Text*)GameEngine::getInstance().FindGameObject(DecodeName("bodies count text"));
			if (text != nullptr) {
				text->setText("Bodies:  " + std::to_string(PhysicsEngine::getInstance().GetBodiesCount()));
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
			AudioEngine::getInstance().PlayTrack(DecodeName("tic"), 0, 20, element->transform.position, true, 0);
		}
		else if (action == GuiAction::MOUSE_MOVED_OUT) {
			((GameObject*)element)->setTexture(DecodeName("button texture"));
			AudioEngine::getInstance().PlayTrack(DecodeName("tic"), 0, 20, element->transform.position, true, 0);
		}
		else if (action == GuiAction::LEFT_BUTTON_UP) {
			GameEngine::getInstance().LoadScene(1);
		}
		break;
	}
	case 5:	//audio test button
	{
		if (action == GuiAction::MOUSE_MOVED_OVER) {
			((GameObject*)element)->setTexture(DecodeName("button texture 1"));
			AudioEngine::getInstance().PlayTrack(DecodeName("tic"), 0, 20, element->transform.position, true, 0);
		}
		else if (action == GuiAction::MOUSE_MOVED_OUT) {
			((GameObject*)element)->setTexture(DecodeName("button texture"));
			AudioEngine::getInstance().PlayTrack(DecodeName("tic"), 0, 20, element->transform.position, true, 0);
		}
		else if (action == GuiAction::LEFT_BUTTON_UP) {
			if (_audioTest->GetStatus() == AUDIO_STATUS_PLAYING) {
				_audioTest->Pause();
			}
			else if (_audioTest->GetStatus() == AUDIO_STATUS_PAUSE) {
				_audioTest->Resume();
			}
			else if (_audioTest->GetStatus() == AUDIO_STATUS_FINISHED) {
				_audioTest->Play();
			}
		}
		break;
	}
	}
}
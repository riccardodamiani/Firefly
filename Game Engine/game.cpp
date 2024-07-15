#include "stdafx.h"

#include <chrono>
#include <thread>

#include "game.h"
#include "gameEngine.h"
#include "graphics.h"
#include "input.h"
#include "gui.h"
#include "gui_element.h"
#include "gui_slider.h"
#include "gui_panel.h"
#include "gui_text.h"
#include "gui_button.h"
#include "gui_droplist.h"
#include "gui_editbox.h"
#include "structures.h"
#include "audio.h"
#include "variables.h"
#include "camera.h"
#include "animation.h"
#include "lightObject.h"

#include "test class.h"
#include "projectile.h"
#include "falling_obj_scene.h"
#include "light_scene.h"

//class Game
//holds the information about the game

Game::Game() {

}

Game::~Game() {

}

//This function is called before the game starts.
//In here you should load fonts, important textures,
//create scenes and load the first scene of your game
void Game::Init() {
	std::vector <uint16_t> channels = {100};
	_AudioEngine->ConfigEngine(channels, 128, 128);

	//_graphicsEngine->LoadFontAtlas(DecodeName("blackFont"), { 50, 50, 50 }, { 0, 0, 0, 0 }, "TrulyMadlyDpad-Regular", 72);
	_graphicsEngine->LoadFontAtlas(DecodeName("redFont"), { 200, 70, 70, 255 }, { 0, 0, 0, 0 }, "TrulyMadlyDpad-Regular", 72);
	_graphicsEngine->LoadFontAtlas(DecodeName("whiteFont"), { 240, 240, 240, 255 }, { 0, 0, 0, 0 }, "TrulyMadlyDpad-Regular", 72);
	_graphicsEngine->LoadFontAtlas(DecodeName("whiteFont-hint"), { 150, 150, 150, 255 }, { 0, 0, 0, 0 }, "TrulyMadlyDpad-Regular", 72);
	SDL_Color bk = {50, 50, 50, 100};
	_graphicsEngine->LoadTextureGroup("default");
	_graphicsEngine->SetBackgroundColor(bk);
	_GameEngine->CreateScene(new FallingObjScene(0));
	_GameEngine->CreateScene(new LightScene(1));
	_GameEngine->LoadScene(0);
}

/*
void rot_func(double time, TransformStruct& val, void* ptr) {
	val.rotation = -time * 360;
}
*/
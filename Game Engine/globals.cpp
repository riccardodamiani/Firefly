#include "stdafx.h"
#include "globals.h"
#include "graphics.h"

#include "platform.h"
#include "gameEngine.h"
#include "audio.h"
#include "gui.h"
#include "input.h"
#include "physics.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>



namespace {
	Graphics* graphicsE;
	GameEngine* GE;
	Audio* AE;
	GUI* guiE;
	Input* iE;
	PhysicsEngine* pE;
	bool initialise_all()
	{
		SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");
		SDL_Init(SDL_INIT_EVERYTHING);
		IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
		TTF_Init();

		graphicsE = new Graphics(MODE_WINDOW_MAX_SIZE, 0, 0, 10);
		GE = new GameEngine();
		guiE = new GUI();
		AE = new Audio(100, 5, 5);
		iE = new Input(*guiE);
		pE = new PhysicsEngine();
		return true;
	}
	bool a = initialise_all();
}

Graphics* const _graphicsEngine = graphicsE;
GameEngine* const _GameEngine = GE;
Audio* const _AudioEngine = AE;
GUI* const _GuiEngine = guiE;
Input* const _InputEngine = iE;
PhysicsEngine* const _PhysicsEngine = pE;

#include "game.h"
#include "gameEngine.h"
#include "graphics.h"
#include "structures.h"
#include "audio.h"

#include "game_definitions.h"


//This function is called before the game starts.
//In here you should load fonts, important textures,
//create scenes and load the first scene of your game
void InitGame() {
	std::vector <uint16_t> channels = {100};
	AudioEngine::getInstance().ConfigEngine(channels, 128, 128);

	//GraphicsEngine::getInstance().LoadFontAtlas(DecodeName("blackFont"), { 50, 50, 50 }, { 0, 0, 0, 0 }, "TrulyMadlyDpad-Regular", 72);
	GraphicsEngine::getInstance().LoadFontAtlas(DecodeName("redFont"), { 200, 70, 70, 255 }, { 0, 0, 0, 0 }, "TrulyMadlyDpad-Regular", 72);
	GraphicsEngine::getInstance().LoadFontAtlas(DecodeName("whiteFont"), { 240, 240, 240, 255 }, { 0, 0, 0, 0 }, "TrulyMadlyDpad-Regular", 72);
	GraphicsEngine::getInstance().LoadFontAtlas(DecodeName("whiteFont-hint"), { 150, 150, 150, 255 }, { 0, 0, 0, 0 }, "TrulyMadlyDpad-Regular", 72);
	SDL_Color bk = {50, 50, 50, 100};
	GraphicsEngine::getInstance().LoadTextureGroup("default");
	GraphicsEngine::getInstance().SetBackgroundColor(bk);
	GameEngine::getInstance().CreateScene(new FallingObjScene(0));
	GameEngine::getInstance().CreateScene(new LightScene(1));
	GameEngine::getInstance().LoadScene(0);
}

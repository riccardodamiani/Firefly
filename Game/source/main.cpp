#include <chrono>
#include <thread>

#include "gameEngine.h"
#include "graphics.h"
#include "structures.h"
#include "variables.h"

#include "falling_obj_scene.h"
#include "light_scene.h"
#include "game_options.h"

#undef main		//must be here to avoid complainings from the linker

//This function is called before the game starts.
//In here you should load fonts, important textures,
//create scenes and load the first scene of your game
void InitGame() {
    GraphicsEngine& graphics_Engine = GraphicsEngine::getInstance();
    GameEngine& game_Engine = GameEngine::getInstance();

    //load some fonts
	graphics_Engine.LoadFontAtlas(DecodeName("blackFont"), { 50, 50, 50 }, { 0, 0, 0, 0 }, "TrulyMadlyDpad-Regular", 72);
	graphics_Engine.LoadFontAtlas(DecodeName("redFont"), { 200, 70, 70, 255 }, { 0, 0, 0, 0 }, "TrulyMadlyDpad-Regular", 72);
	graphics_Engine.LoadFontAtlas(DecodeName("whiteFont"), { 240, 240, 240, 255 }, { 0, 0, 0, 0 }, "TrulyMadlyDpad-Regular", 72);
	graphics_Engine.LoadFontAtlas(DecodeName("whiteFont-hint"), { 150, 150, 150, 255 }, { 0, 0, 0, 0 }, "TrulyMadlyDpad-Regular", 72);

    //set background color
	RGBA_Color bk = {50, 50, 50, 100};
    graphics_Engine.SetBackgroundColor(bk);

    //load basic texture group
	graphics_Engine.LoadTextureGroup("default");
	
    //create the scenes
	game_Engine.CreateScene(new FallingObjScene(0));
	game_Engine.CreateScene(new LightScene(1));
	game_Engine.LoadScene(0);
}


int main(){
    GraphicsOptions g_options;
    AudioOptions a_options;
    std::vector<unsigned short> channel_to_allocate;
    channel_to_allocate.push_back(100);

    g_options.activeLayers = 10;
    g_options.mode = WindowMode::MODE_WINDOW_MAX_SIZE;
    a_options.defaultMusicVol = 128;
    a_options.defaultTrackVol = 128;
    a_options.groupChannels = channel_to_allocate;

	GameEngine::getInstance().GameEngine_Start(InitGame, g_options, a_options);
    return 0;
}


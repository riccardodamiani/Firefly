#include <chrono>
#include <thread>

#include "gameEngine.h"
#include "graphics.h"
#include "structures.h"
#include "variables.h"

#include "firefly_scene.h"
#include "game_options.h"

#undef main		//must be here to avoid complainings from the linker

//This function is called before the game starts.
//In here you should load fonts and important textures,
//create all the scenes and load the first scene of your game
void InitGame() {
    GraphicsEngine& graphics_Engine = GraphicsEngine::getInstance();
    GameEngine& game_Engine = GameEngine::getInstance();

    //set the window title
    graphics_Engine.SetWindowTitle("Firefly Demo");

    //load some fonts
    graphics_Engine.LoadFontAtlas(DecodeName("blueFont"), { 50, 115, 219, 250 }, { 0, 0, 0, 0 }, "Branda", 1000);

    //set background color
	RGBA_Color bg = {50, 50, 50, 100};
    graphics_Engine.SetBackgroundColor(bg);
	
    //create and load the demo scene
	game_Engine.CreateScene(new FireflyScene(0));
	game_Engine.LoadScene(0);
}


int main(){
    GraphicsOptions g_options;
    AudioOptions a_options;

    //define channel groups
    std::vector<unsigned short> channel_groups;
    channel_groups.push_back(10);

    //set graphics options
    g_options.activeLayers = 10;
    g_options.mode = WindowMode::MODE_WINDOW_MAX_SIZE;

    //set audio options
    a_options.defaultMusicVol = 128;
    a_options.defaultTrackVol = 128;
    a_options.groupChannels = channel_groups;

    //start the game
	GameEngine::getInstance().GameEngine_Start(InitGame, g_options, a_options);
    return 0;
}


// Defines the entry point for the console application.
//

#include "stdafx.h"
#include "globals.h"
#include "gameEngine.h"
#include <SDL.h>
#undef main		//must be here to avoid complainings from the linker

int main(){
	_GameEngine->GameEngine_Start();
    return 0;
}


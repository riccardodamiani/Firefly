#include "firefly_scene.h"
#include "firefly.h"

#include "camera.h"
#include "gameObject.h"
#include "lightObject.h"
#include "entity.h"
#include "transform.h"
#include "gui_text.h"
#include "graphics.h"

#include <iostream>

FireflyScene::FireflyScene(unsigned int id) : Scene(id){
    
}

//on load function. Is called only once when the scene is loaded
void FireflyScene::onload(){
    //creates a camera
    GameObject* cam = new Camera({ 16, 9 }, { 0, 0 }, 0);		//create a new camera in the scene

    //enable scene lighting
    GraphicsEngine::getInstance().EnableSceneLighting(true, 1);

    //create the demo text
    auto text1 = new GUI_Text(DecodeName("firefly engine text"), 0, DecodeName("blueFont"), { 0, 1.5 }, { 12, 2.2 }, 1);
    text1->setText("Firefly Engine");
    auto text2 = new GUI_Text(DecodeName("demo text"), 0, DecodeName("blueFont"), { 0, -1.5 }, { 12, 2.2 }, 1);
    text2->setText("Demo");

    //create a global light
    new LightObject(DecodeName("globalLight"), { 0, 0 }, 0, 0.1, 360, { 200, 200, 200, 200 }, LightType::GLOBAL_LIGHT);

    //create the main firefly light (see light instancing for more details)
    LightObject *mainLight = new LightObject(DecodeName("mainLight"), {0, 0}, 0, 1, 360, {200, 200, 200, 200}, LightType::POINT_LIGHT);
    mainLight->SetVisible(false);

    //create the fireflies
    for (int i = 0; i < 80; i++) {
        //create the firefly object
        GameObject* firefly = new Firefly(0.15, 1);

        //create a light instance
        GameObject *light = new LightObject(0, firefly->transform.position, 0, mainLight);

        //create a constraint between the firefly and it's light
        light->SetConstraintParent(firefly, true, true, false);
    }
    
}

void FireflyScene::onfree(){

}

void FireflyScene::scene_callback(GameEvent event, double timeElapsed){
    if (event == GameEvent::GAME_QUIT) {
		//here you can add code that need to be excecuted when the user close the window
		GameEngine::getInstance().Quit();
	}
}

void FireflyScene::gui_listener(GUI_Element* element, GuiAction action){

}
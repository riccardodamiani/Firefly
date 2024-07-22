# Firefly Engine
Firefly is a tiny cross-platform game engine based on SDL. It provides:
* a multithreading capable game engine
* a 2D, tile based graphics engine
* audio engine with 2D spatial sound
* a scene template to easily create and load scenes
* a gui engine with the most common elements: text, editbox, button, panel, slider and droplist
* a static lighting engine capable of in-game light texture baking 
* a game object class to easily add objects in the game world
* a physics engine capable of collision and friction simulation
  
This engine was made for my sole entertainment. If your objective is to create a game there are probably better solutions out there. However if you want to try something different, check it out.  
Note the it's most definitely very buggy in the current moment, especially for Linux.  
Also note that there is no documentation for the engine yet.  

## Requirements
* SDL, SDL image, SDL mixer, SDL ttf
* CMake
* C++ compiler (Visual Studio, g++)

## Build for Windows
* Clone the repository
* Inside the folder create a third-party folder as follow:  
third-party/  
├── SDL  
├── SDL-image  
├── SDL-mixer  
├── SDL-ttf  
```
mkdir third-party
cd third-party
mkdir SDL
mkdir SDL-image
mkdir SDL-mixer
mkdir SDL-ttf
cd ..
```
* Download the SDL development packages and unpack them inside the relative folders in third-party:
    * [SDL](https://github.com/libsdl-org/SDL/releases/download/release-2.30.5/SDL2-devel-2.30.5-VC.zip)
    * [SDL-image](https://github.com/libsdl-org/SDL_image/releases/download/release-2.8.2/SDL2_image-devel-2.8.2-VC.zip)
    * [SDL-mixer](https://github.com/libsdl-org/SDL_mixer/releases/download/release-2.8.0/SDL2_mixer-devel-2.8.0-VC.zip)
    * [SDL-ttf](https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.22.0/SDL2_ttf-devel-2.22.0-VC.zip)
* Run CMake
```
mkdir build
cmake ..
```
* Open the visual studio project in the build/ folder
* Compile the project

## Run the game on Windows
To run the game you need to copy all the .dll files from the following folders and put them in the same folder of the game executable:
* third-party/SDL/lib/x64/
* third-party/SDL-image/lib/x64/
* third-party/SDL-image/lib/x64/optional/
* third-party/SDL-mixer/lib/x64/
* third-party/SDL-mixer/lib/x64/optional/
* third-party/SDL-ttf/lib/x64/  

## Build for Linux
* Clone the repository
* Install all the SDL libraries
```
sudo apt update
sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev 
```
* Build the game  
```
mkdir build
cd build
cmake ..
make
```

## Run the game on Linux
If you didn't already installed the SDL dev packages than you need to install the basic packages:  
```
sudo apt update
sudo apt install libsdl2-2.0-0 libsdl2-image-2.0-0 libsdl2-mixer-2.0-0 libsdl2-ttf-2.0-0
```

## Run the demo project
The demo project in this repo uses a ttf font called Branda that can be downloaded [here](https://www.fontspace.com/).  
In the folder containing the executable create a new folder named 'Fonts' and inside put the file Branda.ttf.
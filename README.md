# Firefly Engine
It's a tiny game engine based on SDL. 

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

## Run on Windows
To run the game you need to copy all the .dll files from the following folders and put them in the same folder of the game executable:
* third-party/SDL/lib/x64/
* third-party/SDL-image/lib/x64/
* third-party/SDL-image/lib/x64/optional/
* third-party/SDL-mixer/lib/x64/
* third-party/SDL-mixer/lib/x64/optional/
* third-party/SDL-ttf/lib/x64/  

Also, you need to create the data folders for the game. So the final look of the game folder should be:  
Game folder/  
├── game.exe  
├── SDL.dll  
├── SDL_image.dll  
├── ...dll  
├── Graphics/  
├── Sound/  
├── ├── Music/  
├── ├── Tracks/  
├── Fonts/  

## Run the demo project
The demo project uses a ttf font called Branda that can be downloaded [here](https://www.fontspace.com/get/family/974o8).  
Put the ttf file in the Fonts/ folder of the game folder named as Branda.ttf
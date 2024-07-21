#include "entity.h"
#include "graphics.h"
#include "structures.h"
#include "gameEngine.h"
#include "camera.h"
#include "lightObject.h"
#include "game_options.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <filesystem>
#include <fstream>
#include <cmath>
#include <iostream>
#include <vector>

namespace fs = std::filesystem;


#define MAX_LAYER 50

GraphicsEngine::GraphicsEngine() {
	
}

GraphicsEngine::~GraphicsEngine() {
	SDL_DestroyWindow(this->_window);
}

void GraphicsEngine::Init(GraphicsOptions &options){
	_updateQueue = _TextureQueues[0];
	_waitingQueue = _TextureQueues[1];
	_renderQueue = _TextureQueues[2];
	_updateCamera = &_CameraTransforms[0];
	_waitingCamera = &_CameraTransforms[1];
	_renderCamera = &_CameraTransforms[2];
	_renderCamera->present = false;
	_updateCamera->present = false;
	_lightingOverlay = nullptr;
	enableRenderDepth = false;

	enableSceneLighting = false;
	max_lighting_layer = 10;
	
	ResolveWindowMode((int)options.mode, options.width, options.height);		//set the dimension of the screen

	_window = SDL_CreateWindow("", 0, 0, this->windowWidth, this->windowHeight, 0);
	_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);

	SetLightingQuality_Internal(LightingQuality::LOW_QUALITY);
	SDL_SetWindowPosition(_window, 0, (options.mode == WindowMode::MODE_FULLSCREEN) ? 0 : 25);
	SDL_SetRenderDrawBlendMode(this->_renderer, SDL_BLENDMODE_BLEND);
	SetActiveLayers(options.activeLayers);
}

//Enable the pseudo-3d rendering. Objects of the same layer are printed based on their
//y position. Higher objects are considered farther away so are printed first.
//Enabling this option can have a impact on framerate
void GraphicsEngine::EnableRenderingDepth(bool enable) {
	enableRenderDepth = enable;
}

//only called from the main thread so it doesn't need a mutex
std::pair <bool, int> GraphicsEngine::FindTexture(EntityName texName) {
	int lower = 0;
	int upper = _textures.size() - 1;
	int mid = lower + (upper - lower + 1) / 2;
	while (lower <= upper) {

		if (texName == _textures[mid].textureName)
		{
			return { true, mid };
		}
		if (texName < _textures[mid].textureName)
			upper = mid - 1;
		else
			lower = mid + 1;

		mid = lower + (upper - lower + 1) / 2;
	}

	return { false, mid };
}

void GraphicsEngine::EnableSceneLighting(bool enable, unsigned int maxLayer) {
	enableSceneLighting = enable;
	max_lighting_layer = maxLayer;
}

void GraphicsEngine::SetMaxLightingLayer(unsigned int layer) {
	max_lighting_layer = layer;
}

//calculate a parabola equation coefficient from three points.
//This parabola is used to aproximate light decay due to distance instead of the inverse square law since
//the last one goes to 0 at infinity which is bad for calculation
void GraphicsEngine::Calculate_Parabola_Coeff_From_Points(double x1, double y1, double x2, double y2, double x3, double y3, double& A, double& B, double& C) {
	double denom = (x1 - x2) * (x1 - x3) * (x2 - x3);
	A = (x3 * (y2 - y1) + x2 * (y1 - y3) + x1 * (y3 - y2)) / denom;
	B = (x3 * x3 * (y1 - y2) + x2 * x2 * (y3 - y1) + x1 * x1 * (y2 - y3)) / denom;
	C = (x2 * x3 * (x2 - x3) * y1 + x3 * x1 * (x3 - x1) * y2 + x1 * x2 * (x1 - x2) * y3) / denom;
}

void GraphicsEngine::SetLightingQuality(LightingQuality quality) {

	std::lock_guard <std::mutex> guard(request_mutex);

	LightingQuality* l = new LightingQuality();
	*l = quality;
	void* data = l;
	std::pair < GraphicRequestType, void*> request(GraphicRequestType::SET_LIGHTING_QUALITY, data);
	this->_requests.push_back(request);
}

//set the mode of the screen (fullscreen of window) and the size of the screen
void GraphicsEngine::ResolveWindowMode(int mode, int width, int height) {

	std::lock_guard<std::mutex> guard(window_resize_mutex);
	SDL_DisplayMode DM = { SDL_PIXELFORMAT_BGRA8888 , 1920, 1080, 50, 0 };

	if ((WindowMode)mode == WindowMode::MODE_FULLSCREEN) {		//fullscreen without window bar
		SDL_GetCurrentDisplayMode(0, &DM);
		this->windowWidth = DM.w;
		this->windowHeight = DM.h;
		this->_windowMode = mode;
	}
	else if ((WindowMode)mode == WindowMode::MODE_WINDOW_MAX_SIZE) {	//fullscreen with window bar
		SDL_GetCurrentDisplayMode(0, &DM);
		this->windowWidth = DM.w;
		this->windowHeight = DM.h - 25;
		this->_windowMode = mode;
	}
	else if ((WindowMode)mode == WindowMode::MODE_WINDOW) {		//window
		this->windowWidth = width;
		this->windowHeight = height;
		this->_windowMode = mode;
	}

}

//resize the current window
void GraphicsEngine::SetWindow(int mode, int width, int height) {

	std::lock_guard <std::mutex> guard(request_mutex);
	WindowUpdate* data = new WindowUpdate();
	data->mode = mode;
	data->windowWidth = width;
	data->windowHeight = height;
	std::pair < GraphicRequestType, void*> request(GraphicRequestType::RESIZE_WINDOW, data);
	this->_requests.push_back(request);
	
}
void GraphicsEngine::SetWindow_Internal(int mode, int width, int height) {
	this->ResolveWindowMode(mode, width, height);

	std::lock_guard<std::mutex> guard(window_resize_mutex);
	SDL_SetWindowSize(this->_window, this->windowWidth, this->windowHeight);
	SDL_SetWindowPosition(this->_window, 0, (mode == (int)WindowMode::MODE_FULLSCREEN) ? 0 : 25);
}

int GraphicsEngine::GetWindowMode() {
	return this->_windowMode;
}

//this function could return slight a wrong queue size
unsigned long GraphicsEngine::GetTaskQueueLen() {
	return _requests.size();
}


//handles the requests regarding graphics for a maximum amount of time
//after which the mainThread needs to start the next frame. Runs on the main thread
void GraphicsEngine::PollRequests(double tick_tock_motherfucker) {
	int block = 0;
	auto startTime = std::chrono::high_resolution_clock::now();

	request_mutex.lock();
	while (tick_tock_motherfucker > 0 && _requests.size() > 0) {
		
		std::pair <GraphicRequestType, void*> request = _requests[0];
		_requests.erase(_requests.begin());

		switch (request.first) {
		case GraphicRequestType::SET_WINDOW_TITLE:		//set window title. Require sdl calls
		{
			WindowUpdate* data = (WindowUpdate*)request.second;
			SetWindowTitle_Internal(data->title);
			delete data;
			break;
		}
		case GraphicRequestType::RESIZE_WINDOW:		//resize game window. Require sdl calls
		{
			WindowUpdate* data = (WindowUpdate*)request.second;
			SetWindow_Internal(data->mode, data->windowWidth, data->windowHeight);
			delete data;
			break;
		}
		case GraphicRequestType::SET_LIGHTING_QUALITY:
		{
			LightingQuality* data = (LightingQuality*)request.second;
			SetLightingQuality_Internal(*data);
			delete data;
			break;
		}
		case GraphicRequestType::CREATE_LIGHT_TEXTURE:
		{
			LightObjectData* data = (LightObjectData*)request.second;
			BakeLightTexture_Internal(*data);
			delete data;
			break;
		}
		case GraphicRequestType::KILL_LIGHT_BAKING:
		{
			KillLightBaking_Internal();
			break;
		}
		case GraphicRequestType::CREATE_FONT_ATLAS:		//create a font. Require sdl calls
		{
			FontStruct* data = (FontStruct*)request.second;
			LoadFontAtlas_Internal(data->atlasName, data->color, data->backgroundColor, data->fontName, data->resolution);
			delete data;
			break;
		}
		case GraphicRequestType::CREATE_FONT_CHAR:		//create a letter of a font
		{
			fontCharCreation* data = (fontCharCreation*)request.second;
			LoadFontChar_Internal(data);
			delete data;
			break;
		}

		case GraphicRequestType::CREATE_TEXTURE:	//Create a texture. Require sdl calls
		{
			TextureCreation* data = (TextureCreation*)request.second;
			
			if (data->processType == 0) {		//custom texture
				CreateCustomTexture_Internal(data->width_or_radius, data->height, data->custom_filter, data->name, data->args);
			}
			else if (data->processType == 1) {	//circle texture
				CreateCircleTexture_Internal(data->color, data->width_or_radius, data->fill, data->name);
			}
			else {	//rectangle texture
				CreateRectangleTexture_Internal(data->color, data->width_or_radius, data->height, data->fill, data->name);
			}
			delete data;
			break;
		}

		case GraphicRequestType::LOAD_FROM_FILE:		//load one texture from a file. Require sdl calls
		{
			request_mutex.unlock();
			LoadFileStruct* data = (LoadFileStruct*)request.second;
			LoadTextureFromFile_Internal(data->pathName, data->filename, data->groupName);
			delete data;
			request_mutex.lock();
			break;
		}

		case GraphicRequestType::DESTROY_TEXTURE:		//destroy a texture. Require sdl calls
		{
			TextureToDestroy* data = (TextureToDestroy*)request.second;
			DestroyTexture_Internal(data->name);
			delete data;
			break;
		}

		case GraphicRequestType::FREE_TEXTURE_GROUP:	//Create a request to free a texture group. Doesn't require sdl
		{
			request_mutex.unlock();
			DestroyTextureGroup* data = (DestroyTextureGroup*)request.second;
			UnloadTextureGroup_Internal(data->textureGroup);
			delete data;
			request_mutex.lock();
			break;
		}

		case GraphicRequestType::FREE_ALL:		//Create a request to free all textures. Doesn't require sdl
		{
			request_mutex.unlock();
			UnloadAllGraphics_Internal();
			request_mutex.lock();
			break;
		}
			
		}

		if (block >= 10) {		//check the clock every few requests
			request_mutex.unlock();		//leave some time to other threads
			auto endTime = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> elapsed = endTime - startTime;
			double elapsedTime = elapsed.count();	//elapsed time in seconds
			tick_tock_motherfucker -= elapsedTime;
			block = 0;
			request_mutex.lock();
		}
		block++;
	}

	request_mutex.unlock();
}

//add a texture to the stack
//is called only from PollRequests which rns on the main thread
void GraphicsEngine::PushTexture(TextureData* texture) {

	//std::lock_guard<std::mutex> guard(update_image_vect_mutex);
	if (texture->textureName == 0) {
		texture->textureName = GameEngine::getInstance().GenerateRandomName();
	}
	auto [found, index] = FindTexture(texture->textureName);
	if (found) {
		return;
	}
	_textures.insert(_textures.begin()+index, *texture);

}

//only runs on the main thread. Doesn't need a mutex
SDL_Surface* GraphicsEngine::CreateSurface(int width, int height) {
	/* Create a 32-bit surface with the bytes of each pixel in R,G,B,A order,
	   as expected by OpenGL for textures */
	SDL_Surface* surface;
	Uint32 rmask, gmask, bmask, amask;

	/* SDL interprets each pixel as a 32-bit number, so our masks must depend
	   on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif

	surface = SDL_CreateRGBSurface(0, width, height, 32, rmask, gmask, bmask, amask);

	return surface;
}

//create a curstomized texture appling the filter to each pixel of the image.
//filter is a function witch receive 3 parameters: x and y of the pixel and a pointer to a already created color structure. 
//x and y are calculated from the center of the texture
void GraphicsEngine::CreateCustomTexture(int width, int height, void (*filter)(CustomFilterData& data), EntityName name, void* args) {
	
	if (filter == nullptr)
		return;

	if (name == 0) {
		name = GameEngine::getInstance().GenerateRandomName();
	}
	TextureCreation* data = new TextureCreation();
	data->processType = 0;
	data->width_or_radius = width;
	data->height = height;
	data->custom_filter = filter;
	data->name = name;
	data->args = args;
	std::pair < GraphicRequestType, void*> request(GraphicRequestType::CREATE_TEXTURE, data);
	this->_requests.push_back(request);
}


//create a texture of a circle
void GraphicsEngine::CreateCircleTexture(RGBA_Color& color, int radius, bool fill, EntityName name) {
	if (name == 0) {
		name = GameEngine::getInstance().GenerateRandomName();
	}
	TextureCreation* data = new TextureCreation();
	data->processType = 1;
	data->width_or_radius = radius;
	data->color = color;
	data->fill = fill;
	data->name = name;
	std::pair < GraphicRequestType, void*> request(GraphicRequestType::CREATE_TEXTURE, data);
	this->_requests.push_back(request);
}

//create a texture of a solid rectangle and returns the id of the texture
void GraphicsEngine::CreateRectangleTexture(RGBA_Color& color, int width, int height, bool fill, EntityName name) {

	if (name == 0) {
		name = GameEngine::getInstance().GenerateRandomName();
	}
	TextureCreation* data = new TextureCreation();
	data->processType = 2;
	data->width_or_radius = width;
	data->height = height;
	data->color = color;
	data->fill = fill;
	data->name = name;
	std::pair < GraphicRequestType, void*> request(GraphicRequestType::CREATE_TEXTURE, data);
	this->_requests.push_back(request);

}

void GraphicsEngine::CreateRectangleTexture_Internal(RGBA_Color& color, int width, int height, bool fill, EntityName name) {
	SDL_Surface* surface = this->CreateSurface(width, height);
	DrawRectangleInSurface(surface, &color, width, height, fill);

	SDL_Texture* texture = SDL_CreateTextureFromSurface(this->_renderer, surface);
	SDL_FreeSurface(surface);
	if (texture != NULL) {
		TextureData t = { name, texture, 0};
		return this->PushTexture(&t);
	}
	
}


void GraphicsEngine::CreateCircleTexture_Internal(RGBA_Color& color, int radius, bool fill, EntityName name) {
	SDL_Surface* surface = this->CreateSurface(radius * 2 + 1, radius * 2 + 1);
	//there is no need to lock the surface since it will never leave this scope

	DrawCircleInSurface(surface, &color, radius, fill);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(this->_renderer, surface);
	SDL_FreeSurface(surface);

	if (texture != NULL) {
		TextureData t = { name, texture, 0};
		this->PushTexture(&t);
	}
	
}


void GraphicsEngine::CreateCustomTexture_Internal(int width, int height, void (*filter)(CustomFilterData& data), EntityName name, void* args) {
	
	SDL_Surface* surface = this->CreateSurface(width, height);
	DrawCustomSurface(surface, width, height, filter, args);

	SDL_Texture* texture = SDL_CreateTextureFromSurface(this->_renderer, surface);
	SDL_FreeSurface(surface);
	if (texture != NULL) {
		TextureData t = { name, texture, 0};
		this->PushTexture(&t);
	}
	
}


//draw a custom texture with a pattern defined by the function filter
//filter is a function witch receive 3 parameters: x and y of the pixel and a pointer to a already created color structure. 
//x and y are calculated from the center of the texture
void GraphicsEngine::DrawCustomSurface(SDL_Surface* surface, int width, int height, void (*filter)(CustomFilterData& data), void* args) {
	if (filter == nullptr)
		return;

	SDL_LockSurface(surface);
	memset(surface->pixels, 0, surface->pitch * surface->h);

	CustomFilterData data;
	data.args = args;
	data.textureWidth = width;
	data.textureHeight = height;

	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			int x = i - width / 2;	//the point passed to the filter function is recalculated from the center of the texture
			int y = j - height / 2;
			data.pixelColor = { 0, 0, 0, 0 };
			data.x = x;
			data.y = y;
			filter(data);
			drawPointInSurface(surface, data.pixelColor, i, j);
		}
	}

	SDL_UnlockSurface(surface);
}


//draw a light surface
//this function is run in parallel in a different thread that the main one
void GraphicsEngine::DrawLightSurface(LightTextureBakeData *lightBakeData, int width, int height, 
	void (*filter)(CustomFilterData& data)) {
	if (filter == nullptr || lightBakeData->surface == nullptr) {
		lightBakeData->done = true;
		return;
	}
		
	memset(lightBakeData->surface->pixels, 0, lightBakeData->surface->pitch * lightBakeData->surface->h);

	CustomFilterData data;
	data.args = &lightBakeData->lightObject;
	data.textureWidth = width;
	data.textureHeight = height;

	for (int i = 0; i < width; i++) {
		if (lightBakeData->abort == true) {	//a way to abort the baking
			lightBakeData->done = true;
			return;
		}
		for (int j = 0; j < height; j++) {
			int x = i - width / 2;	//the point passed to the filter function is recalculated from the center of the texture
			int y = j - height / 2;
			data.pixelColor = { 0, 0, 0, 0 };
			data.x = x;
			data.y = y;
			filter(data);
			drawPointInSurface(lightBakeData->surface, data.pixelColor, i, j);
		}
	}
	lightBakeData->done = true;
	return;
}



void GraphicsEngine::DrawRectangleInSurface(SDL_Surface* surface, RGBA_Color *color, int width, int height, bool fill)
{
	SDL_LockSurface(surface);
	memset(surface->pixels, 0, surface->pitch * surface->h);

	if (fill) {
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				drawPointInSurface(surface, *color, i, j);
			}
		}
	}
	else {
		//vertical lines
		for (int y = 0; y < height; y++) {
			drawPointInSurface(surface, *color, 0, y);
			drawPointInSurface(surface, *color, width-1, y);
		}
		//horizontal lines
		for (int x = 0; x < width; x++) {
			drawPointInSurface(surface, *color, x, 0);
			drawPointInSurface(surface, *color, x, height-1);
		}
	}

	SDL_UnlockSurface(surface);
}

void GraphicsEngine::DrawCircleInSurface(SDL_Surface* surface, RGBA_Color* color, int radius, bool fill) {

	SDL_LockSurface(surface);
	memset(surface->pixels, 0, surface->pitch * surface->h);

	int32_t centerX = surface->w / 2;
	int32_t centerY = surface->h / 2;
	const int32_t diameter = (radius * 2);

	int32_t x = (radius - 1);
	int32_t y = 0;
	int32_t tx = 1;
	int32_t ty = 1;
	int32_t error = (tx - diameter);

	while (x >= y)
	{
		//  Each of the following renders an octant of the circle
		drawPointInSurface(surface, *color, centerX + x, centerY - y);
		drawPointInSurface(surface, *color, centerX + x, centerY + y);
		drawPointInSurface(surface, *color, centerX - x, centerY - y);
		drawPointInSurface(surface, *color, centerX - x, centerY + y);
		drawPointInSurface(surface, *color, centerX + y, centerY - x);
		drawPointInSurface(surface, *color, centerX + y, centerY + x);
		drawPointInSurface(surface, *color, centerX - y, centerY - x);
		drawPointInSurface(surface, *color, centerX - y, centerY + x);

		if (error <= 0)
		{
			++y;
			error += ty;
			ty += 2;
		}

		if (error > 0)
		{
			--x;
			tx += 2;
			error += (tx - diameter);
		}
	}

	if (fill) {
		int xc = surface->w / 2;
		int yc = surface->h / 2;
		for (int i = xc + radius; i > xc - radius; i--) {
			for (int j = 0; j <= radius; j++) {
				if (sqrt((double)(xc - i) * (xc - i) + (yc - (j + yc)) * (yc - (j + yc))) <= (double)radius) {
					drawPointInSurface(surface, *color, i, yc + j);
					drawPointInSurface(surface, *color, i, yc - j);
				}
			}
		}
	}

	SDL_UnlockSurface(surface);
}

void GraphicsEngine::drawPointInSurface(SDL_Surface* surface, RGBA_Color& color, int x, int y) {
	//no need to lock the surface since this function is only used when the surface is already locked
	Uint8* target_pixel = (Uint8*)surface->pixels + y * surface->pitch + x * 4;
	*target_pixel = color.r;
	*(target_pixel + 1) = color.g;
	*(target_pixel + 2) = color.b;
	*(target_pixel + 3) = color.a;

}

//request to set the window title
void GraphicsEngine::SetWindowTitle(std::string title) {
	std::lock_guard <std::mutex> guard(request_mutex);
	WindowUpdate* data = new WindowUpdate();
	data->title = title;
	std::pair < GraphicRequestType, void*> request(GraphicRequestType::SET_WINDOW_TITLE, data);
	this->_requests.push_back(request);
}

void GraphicsEngine::SetWindowTitle_Internal(std::string title) {
	SDL_SetWindowTitle(this->_window, title.c_str());
}

void GraphicsEngine::LoadTextureFromFile(std::string& pathName, std::string& filename, EntityName groupName) {
	std::lock_guard <std::mutex> guard(request_mutex);
	LoadFileStruct* data = new LoadFileStruct();
	data->pathName = pathName;
	data->filename = filename;
	data->groupName = groupName;
	std::pair < GraphicRequestType, void*> request(GraphicRequestType::LOAD_FROM_FILE, data);
	this->_requests.push_back(request);
}

void GraphicsEngine::LoadTextureFromFile_Internal(std::string& pathName, std::string& filename, EntityName groupName) {
	//load the image
	SDL_Surface* sourceSurface = IMG_Load(pathName.c_str());

	if (sourceSurface == NULL) {
		std::cout << "Unable to load texture from file " << pathName << std::endl;
		return;
	}
	
	SDL_Texture* texture = SDL_CreateTextureFromSurface(this->_renderer, sourceSurface);
	if (texture != nullptr) {
		EntityName name = DecodeName(filename.c_str());
		TextureData data = { name, texture, groupName };
		PushTexture(&data);		//add the texture to the stack
	}
	else {
		std::cout << "Unable to create a texture from surface file: " << filename << std::endl;
	}
	SDL_FreeSurface(sourceSurface);
}


void GraphicsEngine::LoadTextureGroup(const char *groupName) {
	if (groupName == nullptr)
		return;
	std::string str_textureGroup = groupName;
	EntityName groupCode = DecodeName(groupName);
#ifdef _WIN32
	std::string folder = "Graphics\\" + str_textureGroup;
#else
	std::string folder = "Graphics/" + groupName;
#endif
	LoadFromDir(folder, groupCode);
}


void GraphicsEngine::UnloadTextureGroup(EntityName groupName) {
	std::lock_guard <std::mutex> guard(request_mutex);
	DestroyTextureGroup* data = new DestroyTextureGroup();
	data->textureGroup = groupName;
	std::pair < GraphicRequestType, void*> request(GraphicRequestType::FREE_TEXTURE_GROUP, data);
	this->_requests.push_back(request);
	
}

void GraphicsEngine::UnloadTextureGroup_Internal(EntityName groupName) {
	std::lock_guard <std::mutex> guard(request_mutex);
	for (int i = 0; i < _textures.size(); i++) {
		TextureData tData = this->_textures[i];
		if (tData.filter == groupName) {
			TextureToDestroy* data = new TextureToDestroy();
			data->name = _textures[i].textureName;
			std::pair < GraphicRequestType, void*> request(GraphicRequestType::DESTROY_TEXTURE, data);
			this->_requests.push_back(request);
		}
	}
}


void GraphicsEngine::UnloadAllGraphics() {
	std::lock_guard <std::mutex> guard(request_mutex);
	std::pair < GraphicRequestType, void*> request(GraphicRequestType::FREE_ALL, nullptr);
	this->_requests.push_back(request);
}

void GraphicsEngine::UnloadAllGraphics_Internal() {
	std::lock_guard <std::mutex> guard(request_mutex);
	for (int i = 0; i < _textures.size(); i++) {
		TextureToDestroy* data = new TextureToDestroy();
		data->name = _textures[i].textureName;
		std::pair < GraphicRequestType, void*> request(GraphicRequestType::DESTROY_TEXTURE, data);
		this->_requests.push_back(request);
	}
}


void GraphicsEngine::BakeLightTexture(LightObjectData lightData) {

	std::lock_guard <std::mutex> guard(request_mutex);
	LightObjectData* data = new LightObjectData();
	*data = lightData;

	std::pair < GraphicRequestType, void*> request(GraphicRequestType::CREATE_LIGHT_TEXTURE, data);
	this->_requests.push_back(request);
}


void GraphicsEngine::CompleteLightBaking() {
	for (auto it = _lightBakingTasks.begin(); it != _lightBakingTasks.end();) {
		if ((*it)->done) {
			CompleteBakingLightTexture_Internal(*it);
			it = _lightBakingTasks.erase(it);
		}
		else {
			++it;
		}
	}
}

//starts the new thread that draw the surface of the light texture
void GraphicsEngine::BakeLightTexture_Internal(LightObjectData& lightData) {

	if (lightData.type == LightType::POINT_LIGHT) {

		//GameEngine::getInstance().FindGameObject(DecodeName("MainCamera"));
		
		SDL_Surface* surface = this->CreateSurface(_lightingOverlaySize.x, _lightingOverlaySize.x);
		//DrawCustomSurface(surface, _lightingOverlaySize.x, _lightingOverlaySize.x, PointLightFilter, &lightData);
		if (surface == nullptr)
			return;

		LightTextureBakeData* task = new LightTextureBakeData();
		task->done = false;
		task->lightObject = lightData;
		task->surface = surface;
		task->abort = false;
		
		SDL_LockSurface(surface);	//lock the surface before starting the new thread
		_lightBakingTasks.push_back(task);
		auto thr = std::thread([this, task]() {DrawLightSurface(task, _lightingOverlaySize.x, _lightingOverlaySize.x, PointLightFilter); });
		thr.detach();
	}
	else if(lightData.type == LightType::GLOBAL_LIGHT) {
		lightData.color.a = 255.0 * (1.0 - pow(1.7, -lightData.power));
		

		SDL_Surface* surface = this->CreateSurface(_lightingOverlaySize.x, _lightingOverlaySize.y);
		SDL_Rect sur_rect = {0, 0, _lightingOverlaySize.x, _lightingOverlaySize.y};
		SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
		uint32_t* color = reinterpret_cast<uint32_t*>(&lightData.color);
		SDL_FillRect(surface, &sur_rect, *color);

		if (surface == nullptr)
			return;

		LightTextureBakeData* task = new LightTextureBakeData();
		task->done = true;
		task->lightObject = lightData;
		task->surface = surface;
		task->abort = false;

		SDL_LockSurface(surface);
		_lightBakingTasks.push_back(task);
	}
}

//transforms the light object surface into a texture and pushes it into the texture list
void GraphicsEngine::CompleteBakingLightTexture_Internal(LightTextureBakeData* lightData) {
	if (lightData->surface == nullptr)
		return;

	//unlock the surface before going on with the tasks
	SDL_UnlockSurface(lightData->surface);
	SDL_Texture* temp_texture = SDL_CreateTextureFromSurface(this->_renderer, lightData->surface);

	SDL_Texture* texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, _lightingOverlaySize.x, _lightingOverlaySize.x);
	//SDL_LockTexture(texture, NULL, NULL, NULL);
	SDL_SetRenderTarget(_renderer, texture);
	SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_NONE);
	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);
	SDL_SetTextureBlendMode(temp_texture, SDL_BLENDMODE_NONE);
	SDL_RenderCopy(_renderer, temp_texture, NULL, NULL);
	SDL_SetRenderTarget(_renderer, NULL);
	//SDL_UnlockTexture(texture);

	SDL_FreeSurface(lightData->surface);
	SDL_DestroyTexture(temp_texture);

	if (texture != NULL) {
		DestroyTexture_Internal(lightData->lightObject.lightTextureName);
		TextureData t = { lightData->lightObject.lightTextureName, texture, 0 };
		this->PushTexture(&t);
	}
}

//create a task in the queue to stop alla light baking
void GraphicsEngine::KillLightBaking() {
	std::lock_guard <std::mutex> guard(request_mutex);

	std::pair < GraphicRequestType, void*> request(GraphicRequestType::KILL_LIGHT_BAKING, nullptr);
	this->_requests.push_back(request);
}

void GraphicsEngine::KillLightBaking_Internal() {
	for (auto it = _lightBakingTasks.begin(); it != _lightBakingTasks.end(); it++) {
		(*it)->abort = true;
		while (!(*it)->done);	//waits the thread to finish
		SDL_UnlockSurface((*it)->surface);
		SDL_FreeSurface((*it)->surface);
	}
	_lightBakingTasks.clear();
}

void GraphicsEngine::DestroyTexture(EntityName name) {
	std::lock_guard <std::mutex> guard(request_mutex);
	TextureToDestroy* data = new TextureToDestroy();
	data->name = name;
	std::pair < GraphicRequestType, void*> request(GraphicRequestType::DESTROY_TEXTURE, data);
	this->_requests.push_back(request);
}

//destroy a texture by name
//it's called from PollRequests that runs on the main thread
void GraphicsEngine::DestroyTexture_Internal(EntityName name) {

	auto [found, texIndex] = FindTexture(name);
	if (found) {
		SDL_DestroyTexture(_textures[texIndex].texture);
		_textures.erase(_textures.begin() + texIndex);
	}
}

void GraphicsEngine::SetLightingQuality_Internal(LightingQuality quality) {
	if (_lightingOverlay != nullptr)
		SDL_DestroyTexture(_lightingOverlay);

	int width = 1280;
	int height = 720;
	if (quality == LightingQuality::LOW_QUALITY) {
		width = 1280;
		height = 720;
	}else if (quality == LightingQuality::MEDIUM_QUALITY) {
		width = 1920;
		height = 1080;
	}
	else if (quality == LightingQuality::HIGH_QUALITY) {
		width = 2560;
		height = 1440;
	}

	/*RGBA_Color color = { 0, 0, 0, 255 };
	SDL_Surface* surface = this->CreateSurface(width, height);
	DrawRectangleInSurface(surface, &color, width, height, true);*/

	SDL_Texture* texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, height);//SDL_CreateTextureFromSurface(this->_renderer, surface);
	_lightingOverlaySize = {(double)width, (double)height};

	//SDL_FreeSurface(surface);
	_lightingOverlay = texture;
}


void GraphicsEngine::LoadFromDir(std::string directory, EntityName groupName) {

	try {
		for (auto& dirEntry : fs::recursive_directory_iterator(directory)) {
			//std::cout << dirEntry << std::endl;
			std::string filename = dirEntry.path().string();
			this->LoadFromDir(filename, groupName);		//try to load files that are inside a folder

			//remove what's before and after the filename
			std::string name;
			size_t index = filename.find_last_of(".");
			if (index != std::string::npos) {
				name = filename.substr(0, index);
			}
			else {
				name = filename;
			}
#ifdef _WIN32
			index = name.find_last_of('\\') + 1;
#else
			index = name.find_last_of('/') + 1;
#endif
			if (index != std::string::npos) {
				name = name.substr(index, name.size());
			}
			LoadTextureFromFile(filename, name, groupName);
		}
	}
	catch (std::filesystem::filesystem_error const& ex) {
		
	}
}


//save the state of a texture that needs to be printed on screen
void GraphicsEngine::BlitSurface(EntityName textureName, int screenLayer, vector2 pos, vector2 scale, double rot, TextureFlip flip) {
	if (textureName == 0)
		return;
	
	if (screenLayer >= 100)
		return;

	TextureObj obj;
	obj.flip = flip;
	obj.pos = pos;
	obj.rot = rot;
	obj.scale = scale;
	obj.screenLayer = screenLayer;
	obj.textureName = textureName;

	std::lock_guard<std::mutex> guard(update_queue_mutex);
	this->_updateQueue[screenLayer].push_back(obj);

}

void GraphicsEngine::BlitTextSurface(EntityName fontAtlas, std::string text, int layer, vector2 pos, vector2 scale, double rot, TextureFlip flip, int cursorPos) {
	//update_queue_mutex.lock();

	if (_fontsRef.find(fontAtlas) == _fontsRef.end()) {
		//update_queue_mutex.unlock();
		return;
	}

	std::vector <TextLetterStruct>& ref = _fontsRef[fontAtlas];
	if (ref.size() < 256)		//prevent reading non existing letters
		return;
	//update_queue_mutex.unlock();

	double availableY = scale.y * 0.8;
	EntityName cursorName = ref[(unsigned char)'|'].name;
	vector2 cSize = ref[(unsigned char)'|'].size;
	vector2 cursorSize = { (cSize.x/ cSize.y) * availableY * 0.7, availableY * 1.2 };

	vector2 delta = { - scale.x / 2, 0 };
	int i = 0;
	for (i; i < text.size(); i++) {
		EntityName name = ref[(unsigned char)text.at(i)].name;
		vector2 letteSize = ref[(unsigned char)text.at(i)].size;
		if ((char)text.at(i) == '\n') {
			delta.y -= scale.y;
			delta.x = - scale.x / 2;
			continue;
		}

		//print the cursor
		if (cursorPos == i) {
			vector2 cursorPos = { pos.x + delta.x * std::cos(rot * (MATH_PI / 180.0)), pos.y + delta.y + delta.x * std::sin(rot * (MATH_PI / 180.0)) };
			GraphicsEngine::getInstance().BlitSurface(cursorName, layer, cursorPos, cursorSize, rot, TextureFlip::FLIP_NONE);
		}

		//print the letter
		double letterScale = letteSize.x / letteSize.y;
		vector2 size = { letterScale * availableY, availableY };
		delta.x += size.x / 2;
		vector2 letterPos = { pos.x + delta.x * std::cos(rot * (MATH_PI / 180.0)), pos.y + delta.y + delta.x * std::sin(rot * (MATH_PI / 180.0)) };
		GraphicsEngine::getInstance().BlitSurface(name, layer, letterPos, size, rot, TextureFlip::FLIP_NONE);

		delta.x += size.x / 2;
	}
	//print the cursor
	if (cursorPos == i) {
		vector2 cursorPos = { pos.x + delta.x * std::cos(rot * (MATH_PI / 180.0)), pos.y + delta.y + delta.x * std::sin(rot * (MATH_PI / 180.0)) };
		GraphicsEngine::getInstance().BlitSurface(cursorName, layer, cursorPos, cursorSize, rot, TextureFlip::FLIP_NONE);
	}
}


void GraphicsEngine::SetBackgroundColor(RGBA_Color& color) {
	this->_backgroundColor = color;
}

bool GraphicsEngine::compareY(TextureObj &i1, TextureObj &i2) {
	return (i1.pos.y - i1.scale.y/2.0 > i2.pos.y - i2.scale.y/2.0);
}

vector2 GraphicsEngine::screenToSpace(int x_coord, int y_coord) {
	vector2 cPos = _cameraPos;
	vector2 s_s_scale = spaceToScreenScale;
	vector2 spacePos = { cPos.x + (double)(x_coord - this->windowWidth/2) / s_s_scale.x,
						 cPos.y - (double)(y_coord - this->windowHeight / 2) / s_s_scale.y };

	return spacePos;
}

void GraphicsEngine::updateRenderCamera(bool present, vector2 pos, vector2 scale, double rotation) {
	_updateCamera->present = present;
	_updateCamera->pos = pos;
	_updateCamera->rot = rotation;
	_updateCamera->scale = scale;
}

//swap the screen buffers. It's called from the main thread
/*void GraphicsEngine::SwapScreenBuffers() {
	//swap the two texture queues

	//Very last protection to avoid swapping the buffers while the main thread is rendering
	//This mutex should avoid crashing when the main thread renders much quicker that the draw thread
	//is updating the graphics data. In normal conditions this should not happen
	std::lock_guard <std::mutex> swap_buffer_guard(swap_buffer_mutex);

	//update_queue_mutex.lock();
	std::vector <textureObject>* temp = _renderQueue;
	_renderQueue = _updateQueue;
	_updateQueue = temp;
	CameraTransform* ctemp = _renderCamera;
	_renderCamera = _updateCamera;
	_updateCamera = ctemp;
	//update_queue_mutex.unlock();

	for (int i = 0; i < _activeLayers; i++) {	//clear the ex render buffer
		_updateQueue[i].clear();
	}
}*/

void GraphicsEngine::SwapScreenBuffersPhysics() {
	std::lock_guard <std::mutex> swap_buffer_guard(swap_buffer_mutex);

	std::vector <textureObject>* temp = _waitingQueue;
	_waitingQueue = _updateQueue;
	_updateQueue = temp;
	CameraTransform* ctemp = _waitingCamera;
	_waitingCamera = _updateCamera;
	_updateCamera = ctemp;

	for (int i = 0; i < _activeLayers; i++) {	//clear the ex render buffer
		_updateQueue[i].clear();
	}
}


void GraphicsEngine::SwapScreenBuffersGraphics() {
	std::lock_guard <std::mutex> swap_buffer_guard(swap_buffer_mutex);

	std::vector <textureObject>* temp = _waitingQueue;
	_waitingQueue = _renderQueue;
	_renderQueue = temp;
	CameraTransform* ctemp = _waitingCamera;
	_waitingCamera = _renderCamera;
	_renderCamera = ctemp;


}

//draws all the textures on the window
//It's called from the main thread
void GraphicsEngine::Flip() {

	//Very last protection to avoid swapping the buffers while the main thread is rendering
	//This mutex should avoid crashing when the main thread renders much quicker that the draw thread
	//is updating the graphics data. In normal conditions this should not happen
	std::lock_guard <std::mutex> swap_buffer_guard(swap_buffer_mutex);

	TextureObj*texture;

	if(_renderCamera->present == false){
		spaceToScreenScale = { 0, 0 };
		_cameraPos = { 0, 0 };
		SDL_SetRenderDrawBlendMode(this->_renderer, SDL_BLENDMODE_NONE);
		SDL_SetRenderDrawColor(this->_renderer, 0, 0, 0, 1);		//clear the screen black
		SDL_RenderClear(this->_renderer);
		SDL_RenderPresent(this->_renderer);
		return;
	}
	vector2 cameraWindow = _renderCamera->scale;
	vector2 cameraPos = _renderCamera->pos;
	double sdl2_cameraRotation = _renderCamera->rot;		//camera rotation angle

	vector2 cameraToScreenScale = { (double)this->windowWidth / cameraWindow.x, (double)this->windowHeight / cameraWindow.y};
	spaceToScreenScale = cameraToScreenScale;
	_cameraPos = cameraPos;

	SDL_SetRenderDrawBlendMode(this->_renderer, SDL_BLENDMODE_NONE);
	SDL_SetRenderDrawColor(this->_renderer, this->_backgroundColor.r, this->_backgroundColor.g, this->_backgroundColor.b, this->_backgroundColor.a);
	SDL_RenderClear(this->_renderer);
	SDL_SetRenderDrawBlendMode(this->_renderer, SDL_BLENDMODE_BLEND);

	for (int j = 0; j < this->_activeLayers; j++) {
		if (enableSceneLighting && max_lighting_layer == j-1) {
			DrawLighting(cameraPos, cameraWindow, sdl2_cameraRotation);
		}

		if(enableRenderDepth) std::sort(this->_renderQueue[j].begin(), this->_renderQueue[j].end(), compareY);
		for (int i = 0; i < this->_renderQueue[j].size(); i++) {
			texture = &this->_renderQueue[j][i];

			double camera_obj_dist = sqrt((texture->pos.x - cameraPos.x) * (texture->pos.x - cameraPos.x)
			+ (cameraPos.y - texture->pos.y) * (cameraPos.y - texture->pos.y));
			//get the angle between the camera and the object
			double angle = std::atan2<double>(texture->pos.y - cameraPos.y, texture->pos.x - cameraPos.x) * (180.0/MATH_PI);
			angle -= sdl2_cameraRotation;	//the object rotate in opposite direction from the camera prospective

			vector2 textureScreenPos = { this->windowWidth / 2.0 + cameraToScreenScale.x * camera_obj_dist * std::cos(angle * (MATH_PI / 180.0)),
										this->windowHeight / 2.0 - cameraToScreenScale.y * camera_obj_dist * std::sin(angle * (MATH_PI / 180.0)) };	//y is inverted on screen

			//size of the rectangle of the texture (in pixel)
			vector2 textureScreenRect = {texture->scale.x * cameraToScreenScale.x, texture->scale.y * cameraToScreenScale.y};
			//destination rectangle on screen
			SDL_Rect destRect = { textureScreenPos.x - textureScreenRect.x/2, textureScreenPos.y - textureScreenRect.y/2, textureScreenRect.x, textureScreenRect.y };

			auto [found, tex_index] = FindTexture(texture->textureName);
			if (!found) {
				continue;
			}
			TextureData data = _textures[tex_index];
			SDL_Texture* tex = data.texture;
			double sdl2_textureRot = texture->rot;
			if (tex != nullptr) {
				//rotation should be positive-> counter clockwise
				SDL_RenderCopyEx(this->_renderer, tex, NULL, &destRect, -(sdl2_textureRot - sdl2_cameraRotation), NULL, (SDL_RendererFlip)texture->flip);
			}
		}
	}
	
	SDL_RenderPresent(this->_renderer);
}

void GraphicsEngine::DrawLighting(vector2 cameraPos, vector2 cameraScale, double cameraRot) {

	std::vector <LightObject*>* lights = GameEngine::getInstance().GetLightObjects();
	std::vector < LightObject*>& ref = *lights;
	if (ref.size() == 0)
		return;
	
	//blends the colors and subtract the alphas. Used for light rendering
	SDL_BlendMode subMode = SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_SRC_ALPHA,
		SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
		SDL_BLENDOPERATION_ADD,
		SDL_BLENDFACTOR_ONE,
		SDL_BLENDFACTOR_ONE,
		SDL_BLENDOPERATION_REV_SUBTRACT);

	SDL_SetRenderTarget(_renderer, _lightingOverlay);
	SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);
	SDL_RenderClear(_renderer);

	vector2 cameraToScreenScale = { _lightingOverlaySize.x / cameraScale.x, _lightingOverlaySize.y / cameraScale.y };
	for (int i = 0; i < ref.size(); i++) {

		if (!ref[i]->IsVisible())
			continue;

		LightObjectData lightData;
		lightData = ref[i]->GetLightData();

		if (lightData.type == LightType::POINT_LIGHT) {
			double camera_obj_dist = sqrt((lightData.position.x - cameraPos.x) * (lightData.position.x - cameraPos.x)
				+ (cameraPos.y - lightData.position.y) * (cameraPos.y - lightData.position.y));

			//prints only light in view
			if (camera_obj_dist - lightData.lightRadius > sqrt(cameraScale.x * cameraScale.x/4 + cameraScale.y * cameraScale.y/4)) {
				continue;
			}

			if (lightData.colorRebake > 0) {		//needs to redraw the light color
				BakeLightColor(lightData);
				ref[i]->ResetChanged();
				SDL_SetRenderTarget(_renderer, _lightingOverlay);
			}

			//get the angle between the camera and the object
			double angle = std::atan2<double>(lightData.position.y - cameraPos.y, lightData.position.x - cameraPos.x) * (180.0 / MATH_PI);
			angle -= cameraRot;	//the object rotate in opposite direction from the camera prospective

			vector2 textureScreenPos = { _lightingOverlaySize.x / 2.0 + cameraToScreenScale.x * camera_obj_dist * std::cos(angle * (MATH_PI / 180.0)),
										_lightingOverlaySize.y / 2.0 - cameraToScreenScale.y * camera_obj_dist * std::sin(angle * (MATH_PI / 180.0)) };	//y is inverted on screen

			//size of the rectangle of the texture (in pixel)
			vector2 textureScreenRect = { 2 * lightData.lightRadius * cameraToScreenScale.x, 2 * lightData.lightRadius * cameraToScreenScale.y };

			//destination rectangle on screen
			SDL_Rect destRect = { textureScreenPos.x - textureScreenRect.x / 2, textureScreenPos.y - textureScreenRect.y / 2, textureScreenRect.x, textureScreenRect.y };

			auto [found, tex_index] = FindTexture(lightData.lightTextureName);
			if (!found) {
				continue;
			}
			TextureData data = _textures[tex_index];
			SDL_Texture* tex = data.texture;

			SDL_SetTextureBlendMode(tex, subMode);
			SDL_RenderCopyEx(this->_renderer, tex, NULL, &destRect, -(lightData.rotation - cameraRot), NULL, SDL_RendererFlip::SDL_FLIP_NONE);
		}
		else if (lightData.type == LightType::GLOBAL_LIGHT) {
			if (lightData.colorRebake > 0) {		//needs to redraw the light color
				BakeLightColor(lightData);
				ref[i]->ResetChanged();
				SDL_SetRenderTarget(_renderer, _lightingOverlay);
			}
			auto [found, tex_index] = FindTexture(lightData.lightTextureName);
			if (!found) {
				continue;
			}
			TextureData data = _textures[tex_index];
			SDL_Texture* tex = data.texture;

			SDL_SetTextureBlendMode(tex, subMode);
			SDL_RenderCopyEx(this->_renderer, tex, NULL, NULL, 0, NULL, SDL_RendererFlip::SDL_FLIP_NONE);
		}
		
	}
	SDL_SetRenderTarget(_renderer, NULL);
	SDL_SetRenderDrawBlendMode(this->_renderer, SDL_BLENDMODE_BLEND);
	SDL_SetTextureBlendMode(_lightingOverlay, SDL_BLENDMODE_BLEND);
	SDL_RenderCopyEx(_renderer, _lightingOverlay, NULL, NULL, 0, NULL, SDL_RendererFlip::SDL_FLIP_NONE);

	delete lights;
}

//Redraw only the color of a light texture without touching the alpha channel
//This is called directly from the flip() function since the color change is
//done using the SDL_RenderFillRect which is really fast
void GraphicsEngine::BakeLightColor(LightObjectData& data) {
	SDL_BlendMode colorMode = SDL_ComposeCustomBlendMode(SDL_BLENDFACTOR_ONE,
		SDL_BLENDFACTOR_ZERO,
		SDL_BLENDOPERATION_ADD,
		SDL_BLENDFACTOR_ZERO,
		SDL_BLENDFACTOR_ONE,
		SDL_BLENDOPERATION_ADD);

	auto [found, tex_index] = FindTexture(data.lightTextureName);
	if (!found) {
		return;
	}
	TextureData texData = _textures[tex_index];
	SDL_Texture* texture = texData.texture;

	SDL_SetRenderTarget(_renderer, texture);
	SDL_SetRenderDrawBlendMode(_renderer, colorMode);
	SDL_SetRenderDrawColor(_renderer, data.color.r, data.color.g, data.color.b, 0);
	SDL_RenderFillRect(_renderer, NULL);

	SDL_SetRenderTarget(_renderer, NULL);
}

//Custom algorithm to calculate a light texture
//To calculate the light intensity in each pixel it's uses a quadratic function that
//aproximate the inverse square law function. This is done because the 1/distance^2 
// function goes to 0 at distance -> infinity so even at large distances from the light source 
// there is some light left which is bad for performance.
//To calculate the luminosity of the pixel from the light intensity a algorithm similar to HDR is used.
//For low light intensity (0.0-2.0) the increase in pixel luminosity is linear, 
//but the higher the intensity is and the less the luminosity increase.
//Ideally you get maximum luminosity for intensity -> infinity
void GraphicsEngine::PointLightFilter(CustomFilterData &data) {

	LightObjectData* lightdata = (LightObjectData*)data.args;
	double scale = data.textureWidth / (lightdata->lightRadius * 2.0);
	double distance = sqrt(data.x * data.x + data.y * data.y) / scale;
	
	double angle = atan2(data.y, data.x)*(180.0/MATH_PI);

	data.pixelColor.r = lightdata->color.r;
	data.pixelColor.g = lightdata->color.g;
	data.pixelColor.b = lightdata->color.b;
	if (distance > lightdata->lightRadius) {
		data.pixelColor.a = 0;
		return;
	}

	//calculate the distance from the edge of the light cone. p = 1 means the pixel is
	//on the edge, p = 0 means you are in the center of the light cone
	double p = fabs(angle) / (lightdata->lightAngle / 2.0);
	
	if (lightdata->lightAngle < 360) {	//light source is a cone
		if (p >= 1) {		//out of the light cone
			data.pixelColor.a = 0;
			return;
		}
		
		double intensity = lightdata->parab_a * distance * distance + lightdata->parab_b * distance + lightdata->parab_c;	//parabola y = ax^2 + bx + c, where x is the distance
		data.pixelColor.a = 255.0 *(1.0 - pow(2.0, -log(1+intensity*intensity))) * (1 - p*p);	//hdr-ish algorithm + light decay in the edge of the light cone
		return;
	}
	
	double intensity = lightdata->parab_a * distance * distance + lightdata->parab_b * distance + lightdata->parab_c;	//parabola y = ax^2 + bx + c, where x is the distance
	data.pixelColor.a = 255.0 * (1.0 - pow(2.0, -log(1 + intensity * intensity)));	//hdr-ish algorithm
}

vector2 GraphicsEngine::Internal_GetTextureSize(SDL_Texture* texture) {
	int w, h;

	SDL_QueryTexture(texture, NULL, NULL, &w, &h);
	return {(double)w,(double)h};
}


//scale a surface to the width and height specified
SDL_Surface* GraphicsEngine::ScaleSurface(SDL_Surface *Surface, int Width, int Height)
{
	int bytesNum = Surface->format->BitsPerPixel/8;

	if (!Surface || !Width || !Height)
		return 0;

	SDL_Surface *_ret = SDL_CreateRGBSurface(Surface->flags, Width, Height, Surface->format->BitsPerPixel,
		Surface->format->Rmask, Surface->format->Gmask, Surface->format->Bmask, Surface->format->Amask);

	double    _stretch_factor_x = (static_cast<double>(Width) / static_cast<double>(Surface->w)),
		_stretch_factor_y = (static_cast<double>(Height) / static_cast<double>(Surface->h));

	for (Sint32 y = 0; y < Surface->h; y++)
		for (Sint32 x = 0; x < Surface->w; x++)
			for (Sint32 o_y = 0; o_y < _stretch_factor_y; ++o_y)
				for (Sint32 o_x = 0; o_x < _stretch_factor_x; ++o_x)
					drawPixel(_ret, static_cast<Sint32>(_stretch_factor_x * x) + o_x,
						static_cast<Sint32>(_stretch_factor_y * y) + o_y, readPixel(Surface, x, y, bytesNum), bytesNum);

	return _ret;
}

void GraphicsEngine::drawPixel(SDL_Surface *surface, int x, int y, uint32_t pixel, int bytesNum){

	Uint8 *target_pixel = (Uint8 *)surface->pixels + y * surface->w * bytesNum + x * bytesNum;
	Uint8 *pixel8 = (Uint8*)&pixel;
	for (int i = 0; i < bytesNum; i++) {
		target_pixel[i] = pixel8[i];
	}
}

uint32_t GraphicsEngine::readPixel(SDL_Surface *surface, int x, int y, int bytesNum){

	Uint8 *pixels = (Uint8 *)surface->pixels;
	Uint32 *pixel32 = 0;
	pixel32 = (Uint32 *) &pixels[(y * surface->w * bytesNum) + x * bytesNum];
	return *pixel32;
}

//return the width of the window
std::pair <int, int> GraphicsEngine::GetWindowSize() {
	return { this->windowWidth, this->windowHeight };
}

//draw a line between point 1 and 2
void GraphicsEngine::drawLine(int x1, int y1, int x2, int y2,int width, uint8_t R, uint8_t G, uint8_t B, uint8_t A) {
	SDL_SetRenderDrawColor(this->_renderer, R, G, B, A);
	SDL_RenderDrawLine(this->_renderer, x1, y1, x2, y2);
}

void GraphicsEngine::setRendererScale(double xScale, double yScale) {
	SDL_RenderSetScale(this->_renderer, xScale, yScale);
}

void GraphicsEngine::drawCircle(vector2 center, int32_t radius, bool fill){	
	int32_t centerX = center.x;
	int32_t centerY = center.y;
	const int32_t diameter = (radius * 2);

	int32_t x = (radius - 1);
	int32_t y = 0;
	int32_t tx = 1;
	int32_t ty = 1;
	int32_t error = (tx - diameter);

	while (x >= y)
	{
		//  Each of the following renders an octant of the circle
		SDL_RenderDrawPoint(this->_renderer, centerX + x, centerY - y);
		SDL_RenderDrawPoint(this->_renderer, centerX + x, centerY + y);
		SDL_RenderDrawPoint(this->_renderer, centerX - x, centerY - y);
		SDL_RenderDrawPoint(this->_renderer, centerX - x, centerY + y);
		SDL_RenderDrawPoint(this->_renderer, centerX + y, centerY - x);
		SDL_RenderDrawPoint(this->_renderer, centerX + y, centerY + x);
		SDL_RenderDrawPoint(this->_renderer, centerX - y, centerY - x);
		SDL_RenderDrawPoint(this->_renderer, centerX - y, centerY + x);

		if (error <= 0)
		{
			++y;
			error += ty;
			ty += 2;
		}

		if (error > 0)
		{
			--x;
			tx += 2;
			error += (tx - diameter);
		}
	}

	if (fill) {
		int xc = center.x;
		int yc = center.y;
		for (int i = xc + radius; i > xc - radius; i--) {
			for (int j = 0; j <= radius; j++) {
				if (sqrt((double)(xc - i) * (xc - i) + (yc - (j + yc)) * (yc - (j + yc))) <= (double)radius) {
					SDL_RenderDrawPoint(this->_renderer, i, yc + j);
					SDL_RenderDrawPoint(this->_renderer, i, yc - j);
				}
			}
		}
	}
	
}

void GraphicsEngine::drawEllipse(vector2 center, int32_t a, int32_t b){
	int32_t centerX = center.x;
	int32_t centerY = center.y;

	if (a <= 0 || b <= 0)
		return;

	int32_t x, y;

	for (x = 0; x <= a; x++) {
		y = (int)sqrt((1.0 - (pow(x, 2) / pow(a, 2)))*pow(b, 2));
		SDL_RenderDrawPoint(this->_renderer, centerX + x, centerY + y);
		SDL_RenderDrawPoint(this->_renderer, centerX + x, centerY - y);
		SDL_RenderDrawPoint(this->_renderer, centerX - x, centerY + y);
		SDL_RenderDrawPoint(this->_renderer, centerX - x, centerY - y);
	}
}

void GraphicsEngine::CreateTextSurface(EntityName name, std::string text, TTF_Font* font, RGBA_Color textColor, RGBA_Color backgroundColor, vector2& size) {

	//Unfortunately TTF_RenderText_Shaded doesn't seems to work with opengles2 as driver
	//so i have to manually create the text and the background

	//create blended text texture
	auto surfaceMessage = TTF_RenderText_Blended(font, text.c_str(), { textColor.r, textColor.g, textColor.b, textColor.a });
	SDL_Texture* text_texture = SDL_CreateTextureFromSurface(this->_renderer, surfaceMessage); //now you can convert it into a texture

	//create background texture
	SDL_Texture *bgTexture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, surfaceMessage->w, surfaceMessage->h);

	//set the background color
	SDL_SetTextureBlendMode(bgTexture, SDL_BLENDMODE_NONE);
	SDL_SetRenderTarget(_renderer, bgTexture);
	SDL_SetRenderDrawColor(_renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
	SDL_RenderClear(_renderer);

	//copy the text over the background
	SDL_SetTextureBlendMode(bgTexture, SDL_BLENDMODE_BLEND);
	SDL_SetTextureBlendMode(text_texture, SDL_BLENDMODE_BLEND);		//probably not needed
	SDL_RenderCopy(_renderer, text_texture, NULL, NULL);
	SDL_SetRenderTarget(_renderer, NULL);

	//clear everything
	SDL_FreeSurface(surfaceMessage);
	SDL_DestroyTexture(text_texture);

	if (bgTexture != nullptr) {
		size = Internal_GetTextureSize(bgTexture);
		TextureData data = { name, bgTexture, 0};
		PushTexture(&data);
	}
}


//set the number of layers that will be rendered (counted from 0). Higher rendered layers means worse performance. Max: 100 layers
//This function should be called only inside the contructor of a scene
void GraphicsEngine::SetActiveLayers(int layers) {
	if (layers > 100 || layers < 1)
		return;
	this->_activeLayers = layers;
}

vector2 GraphicsEngine::GetTextSize(EntityName atlasName, std::string text, int count, double Y_TextScale) {
	font_mutex.lock();

	if (_fontsRef.find(atlasName) == _fontsRef.end()) {
		font_mutex.unlock();
		return {0, 0};
	}

	std::vector <TextLetterStruct>& ref = _fontsRef[atlasName];
	
	count = std::min(count, (int)text.size());
	vector2 delta = { 0, 0 };
	double lastSizeY = 0;
	double availableY = Y_TextScale * 0.8;

	for (int i = 0; i < count; i++) {
		vector2 pixelSize = ref[(unsigned char)text.at(i)].size;
		if ((char)text.at(i) == '\n') {
			delta.y += Y_TextScale;
			delta.x = 0;
			continue;
		}
		double letterScale = pixelSize.x / pixelSize.y;
		delta.x += letterScale * availableY;
	}

	font_mutex.unlock();
	return delta;

}

void GraphicsEngine::LoadFontAtlas(EntityName atlasName, RGBA_Color color, RGBA_Color backgroundColor, std::string fontName, long resolution) {
	if (atlasName == 0) {
		return;
	}
	FontStruct* data = new FontStruct();
	data->backgroundColor = backgroundColor;
	data->color = color;
	data->fontName = fontName;
	data->resolution = resolution;
	data->atlasName = atlasName;
	std::pair < GraphicRequestType, void*> request(GraphicRequestType::CREATE_FONT_ATLAS, data);
	this->_requests.push_back(request);
}

void GraphicsEngine::LoadFontChar_Internal(fontCharCreation* fontCharData) {

	//clear everything when done
	if (fontCharData->deleteFontObj) {
		TTF_CloseFont(fontCharData->fontInfo->font);
		delete fontCharData->fontInfo;
	}
	
	//create a single char texture
	std::vector <TextLetterStruct>& vec = _fontsRef[fontCharData->fontInfo->atlasName];

	std::string character = "";
	character.append(1, (char)fontCharData->char_code);
	vector2 size = {0, 0};
	EntityName name = DecodeName(character.c_str()) * fontCharData->fontInfo->atlasName;
	if (this->isCharPrint((char)fontCharData->char_code)) {

		this->CreateTextSurface(name, character,
			fontCharData->fontInfo->font,
			fontCharData->fontInfo->color,
			fontCharData->fontInfo->bgColor,
			size);
	}
	
	vec.push_back({ name, size });
}

void GraphicsEngine::LoadFontAtlas_Internal(EntityName atlasName, RGBA_Color color, RGBA_Color backgroundColor, std::string fontName, long resolution) {
	char character[2] = "";
	std::string fontCompleteName = "Fonts\\" + fontName + ".ttf";

	if (_fontsRef.find(atlasName) != _fontsRef.end())	//if already exist
		return;

	TTF_Font* font = TTF_OpenFont(fontCompleteName.c_str(), resolution); //this opens a font style and sets a size

	if (font == NULL) {
		std::cout << "Unable to load text font " << fontName << std::endl;
		return;
	}

	std::vector <TextLetterStruct>& vec = _fontsRef[atlasName];
	fontAtlasInfo* fontInfo = new fontAtlasInfo();
	fontInfo->atlasName = atlasName;
	fontInfo->bgColor = backgroundColor;
	fontInfo->color = color;
	fontInfo->font = font;

	//queue all letters in the request list
	for (int i = 0; i < 256; i++) {
		EntityName name = 0;
		vector2 size = {0, 0};
		
		fontCharCreation *fontCharInfo = new fontCharCreation();
		fontCharInfo->fontInfo = fontInfo;
		fontCharInfo->char_code = (char)i;
		fontCharInfo->deleteFontObj = false;
		std::pair < GraphicRequestType, void*> request(GraphicRequestType::CREATE_FONT_CHAR, fontCharInfo);
		this->_requests.push_back(request);
	}

	//delete the font object once finished loading
	fontCharCreation* fontCharInfo = new fontCharCreation();
	fontCharInfo->fontInfo = fontInfo;
	fontCharInfo->deleteFontObj = true;

}

bool GraphicsEngine::isCharPrint(char c) {
	int h = (unsigned char)c;
	if (h >= 32 && h <= 254)
		return true;
	else return false;
}
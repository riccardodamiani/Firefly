#pragma once
#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <map>
#include <string>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <mutex>
#include <atomic>
#include <vector>

#include "structures.h"
#include "gameEngine.h"
#include "lightObject.h"
#include "entity.h"
#include "game_options.h"
#include "graphics_structs.h"

class GraphicsEngine {
	//some internal structures
	typedef struct textureObject {
		EntityName textureName;
		int screenLayer;
		vector2 pos;
		vector2 scale;
		double rot;
		TextureFlip flip;
	} TextureObj;


	typedef struct textureData {
		EntityName textureName;
		SDL_Texture* texture;
		EntityName filter;
	}TextureData;

	typedef struct textLetterStruct {
		EntityName name;
		vector2 size;
	}TextLetterStruct;

	enum class GraphicRequestType {
		SET_WINDOW_TITLE,
		RESIZE_WINDOW,
		SET_LIGHTING_QUALITY,
		CREATE_LIGHT_TEXTURE,
		KILL_LIGHT_BAKING,
		CREATE_TEXTURE,
		CREATE_FONT_ATLAS,
		CREATE_FONT_CHAR,
		LOAD_FROM_FILE,
		DESTROY_TEXTURE,
		FREE_TEXTURE_GROUP,
		FREE_ALL
	};

	struct LightTextureBakeData {
		LightObjectData lightObject;
		SDL_Surface* surface;
		std::atomic <bool> done;
		std::atomic <bool> abort;
	};

	typedef struct textureCreation {
		int processType;
		RGBA_Color color;
		bool fill;
		int width_or_radius, height;
		EntityName name;
		void (*custom_filter)(CustomFilterData& data);
		void* args;
	}TextureCreation;

	struct fontAtlasInfo {
		EntityName atlasName;
		RGBA_Color color, bgColor;
		TTF_Font* font;		//contains the font object
	};

	struct fontCharCreation {
		fontAtlasInfo* fontInfo;
		uint8_t char_code;
		bool deleteFontObj;	//to clear the font object when all letters are created
	};

	typedef struct windowUpdate {
		int mode;
		int windowWidth;
		int windowHeight;
		std::string title;
	}WindowUpdate;

	typedef struct fontStruct {
		EntityName atlasName;
		std::string fontName;
		RGBA_Color color, backgroundColor;
		long resolution = 72;
	}FontStruct;

	typedef struct loadFile {
		std::string pathName;
		std::string filename;
		EntityName groupName;
	}LoadFileStruct;

	typedef struct textureToDestroy {
		EntityName name;
	}TextureToDestroy;

	typedef struct destroyTextureGroup {
		EntityName textureGroup;
	}DestroyTextureGroup;

	typedef struct cameraTransform {
		bool present;
		vector2 pos;
		vector2 scale;
		double rot;
	}CameraTransform;

	struct LightRenderData {
		LightObjectData data;
		double scale;
	};

	typedef std::vector <TextureObj> buffer_vector[32][50];

public:
    static GraphicsEngine& getInstance() {
        static GraphicsEngine instance;
        return instance;
    }

    GraphicsEngine(const GraphicsEngine&) = delete;
    GraphicsEngine& operator=(const GraphicsEngine&) = delete;

	void Init(GraphicsOptions &options);

	//requests that can be processed immediately
	void Flip();		//renders everything to the screen
	void SwapScreenBuffersPhysics();
	void SwapScreenBuffersGraphics();
	void updateRenderCamera(bool present, vector2 pos, vector2 scale, double rotation);
	void BlitSurface(EntityName textureName, int screenLayer, vector2 pos, vector2 rect, double rot, TextureFlip flip);
	void BlitTextSurface(EntityName atlasName, std::string text, int layer, vector2 pos, vector2 rect, double rot, TextureFlip flip, int cursorPos);
	void EnableRenderingDepth(bool enable);

	int GetWindowMode();
	std::pair <int, int> GetWindowSize();		//return the width of the window
	void SetBackgroundColor(RGBA_Color& color);
	
	vector2 GetTextSize(EntityName atlasName, std::string text, int count, double Y_TextScale);
	vector2 screenToSpace(int x_coord, int y_coord);

	//method for creating requests to the graphics engine
	void SetWindowTitle(std::string title);
	void SetWindow(int mode, int width, int height);		//set window size and mode (public)
	void CreateRectangleTexture(RGBA_Color& color, int width, int height, bool fill, EntityName name);
	void CreateCircleTexture(RGBA_Color& color, int radius, bool fill, EntityName name);
	void CreateCustomTexture(int width, int height, void (*filter)(CustomFilterData &data), EntityName name, void* args);
	void LoadFontAtlas(EntityName atlasName, RGBA_Color color, RGBA_Color backgroundColor, std::string fontName = "OpenSans", long resolution = 72);
	void BakeLightTexture(LightObjectData lightData);
	void LoadTextureGroup(const char* groupName);
	void DestroyTexture(EntityName name);
	void UnloadTextureGroup(EntityName groupName);
	void UnloadAllGraphics();

	//lighting
	void EnableSceneLighting(bool enable, unsigned int maxLayer = 10);
	void SetMaxLightingLayer(unsigned int layer);
	void SetLightingQuality(LightingQuality quality);
	void Calculate_Parabola_Coeff_From_Points(double x1, double y1, double x2, double y2, double x3, double y3, double &A, double &B, double &C);

	void PollRequests(double timeLeft);		//poll requests from the request queue

	void CompleteLightBaking();

	unsigned long GetTaskQueueLen();
private:
	GraphicsEngine();
	~GraphicsEngine();

	//internal methods for handling specific requests that need SDL calls
	void SetWindowTitle_Internal(std::string title);
	void SetWindow_Internal(int mode, int width, int height);
	void CreateRectangleTexture_Internal(RGBA_Color& color, int width, int height, bool fill, EntityName name);
	void CreateCircleTexture_Internal(RGBA_Color& color, int radius, bool fill, EntityName name);
	void CreateCustomTexture_Internal(int width, int height, void (*filter)(CustomFilterData& data), EntityName name, void* args);
	void LoadFontAtlas_Internal(EntityName atlasName, RGBA_Color color, RGBA_Color backgroundColor, std::string fontName, long resolution);
	void LoadFontChar_Internal(fontCharCreation* fontCharData);
	void LoadTextureFromFile(std::string &pathName, std::string &filename, EntityName groupName);
	void LoadTextureFromFile_Internal(std::string& pathName, std::string &filename, EntityName groupName);
	void UnloadTextureGroup_Internal(EntityName groupName);
	void DestroyTexture_Internal(EntityName name);
	void UnloadAllGraphics_Internal();

	//light baking
	void BakeLightTexture_Internal(LightObjectData& lightData);
	void CompleteBakingLightTexture_Internal(LightTextureBakeData* lightData);
	void KillLightBaking();
	void KillLightBaking_Internal();

	static void PointLightFilter(CustomFilterData &data);
	void SetLightingQuality_Internal(LightingQuality quality);
	void DrawLighting(vector2 cameraPos, vector2 cameraScale, double cameraRot);
	void BakeLightColor(LightObjectData& data);

	void LoadFromDir(std::string directory, EntityName groupName);	//load images from a directory

	void SetActiveLayers(int layer);
	vector2 Internal_GetTextureSize(SDL_Texture *texture);		//sdl call to find out texture size
	void ResolveWindowMode(int mode, int width, int height);		//set window size and screen mode

	static bool compareY(TextureObj& i1, TextureObj& i2);	//used for pseudo-3d rendering

	//int createTexture(int width, int height, int filter);
	SDL_Surface* CreateSurface(int width, int height);
	void DrawRectangleInSurface(SDL_Surface* surface, RGBA_Color* color, int width, int height, bool fill);
	void DrawCircleInSurface(SDL_Surface* surface, RGBA_Color* color, int radius, bool fill);
	void DrawCustomSurface(SDL_Surface* surface, int width, int height, void (*filter)(CustomFilterData& data), void* args);
	void DrawLightSurface(LightTextureBakeData*, int width, int height, void (*filter)(CustomFilterData& data));

	void drawLine(int x1, int y1, int x2, int y2, int width, Uint8 R, Uint8 G, Uint8 B, Uint8 A);		//draw a line between point 1 and 2
	void setRendererScale(double xScale, double yScale);
	void drawCircle(vector2 center, int32_t radius, bool fill);
	void drawEllipse(vector2 center, int32_t a, int32_t b);

	void drawPixel(SDL_Surface* surface, int x, int y, Uint32 pixel, int bytes);	//write a pixel in a surface
	Uint32 readPixel(SDL_Surface* surface, int x, int y, int bytes);		//read the color of a pixel froma asurface
	void drawPointInSurface(SDL_Surface* surface, RGBA_Color& color, int x, int y);

	SDL_Surface* ScaleSurface(SDL_Surface* Surface, int Width, int Height);		//scale a surface and return it

	std::pair <bool, int> FindTexture(EntityName texName);
	void PushTexture(TextureData* texture);

	void CreateTextSurface(EntityName name, std::string text, TTF_Font *font, RGBA_Color color, RGBA_Color backgroundColor, vector2 &size);
	bool isCharPrint(char c);
	
	//window stuff
	SDL_Window* _window;
	SDL_Renderer* _renderer;
	RGBA_Color _backgroundColor;
	std::atomic <int> windowWidth;
	std::atomic <int> windowHeight;
	std::atomic <int> _windowMode;

	//enable pseudo-3d rendering
	std::atomic <bool> enableRenderDepth;

	//fonts list
	std::map <EntityName, std::vector <TextLetterStruct>> _fontsRef;

	//texture vector
	std::vector <TextureData> _textures;
	
	//requests vector
	std::vector <std::pair <GraphicRequestType, void *>> _requests;

	//buffers and camera stuff for rendering
	std::vector <TextureObj> _TextureQueues[3][50];
	std::vector <TextureObj>* _renderQueue;
	std::vector <TextureObj>* _waitingQueue;
	std::vector <TextureObj>* _updateQueue;
	CameraTransform* _renderCamera, *_updateCamera, *_waitingCamera;
	CameraTransform _CameraTransforms[3];
	std::atomic <vector2> spaceToScreenScale;
	std::atomic <vector2> _cameraPos;

	//vector for parallel light baking
	std::vector<LightTextureBakeData *> _lightBakingTasks;
	
	//mutexes
	std::mutex font_mutex;
	std::mutex update_queue_mutex;
	std::mutex swap_buffer_mutex;
	std::mutex buffer_mutexes[32];
	std::mutex request_mutex;
	std::mutex window_resize_mutex;

	std::atomic <int> _activeLayers;

	//lighting
	std::atomic <bool> enableSceneLighting;
	std::atomic <unsigned int> max_lighting_layer;
	LightingQuality _lightingQuality;
	SDL_Texture* _lightingOverlay;
	vector2 _lightingOverlaySize;
};

#endif
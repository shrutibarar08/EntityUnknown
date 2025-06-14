#pragma once
#include <d3d11.h>
#include <unordered_map>

#include "RenderManager/Bitmap/IBitmap.h"
#include "RenderManager/Camera/CameraController.h"
#include "RenderManager/Sprite/WorldSpaceSprite/WorldSpaceSprite.h"

class Render2DQueue
{
public:
	Render2DQueue(CameraController* controller, ID3D11Device* device);

	static bool UpdateBuffers(ID3D11DeviceContext* deviceContext);
	static void UpdateScreenSize(int width, int height);
	static void Clean();

	//~ Background screen Sprites
	static bool AddBackgroundSprite(IBitmap* sprite);
	static bool RemoveBackgroundSprite(const IBitmap* sprite);
	static bool RemoveBackgroundSprite(uint64_t spriteID);
	static bool UpdateBackgroundSprite(ID3D11DeviceContext* deviceContext);
	static void RenderBackgroundSprites(ID3D11DeviceContext* deviceContext);

	//~ Screen Sprites (on screen rendering - front ones only)
	static bool AddScreenSprite(IBitmap* sprite);
	static bool RemoveScreenSprite(const IBitmap* sprite);
	static bool RemoveScreenSprite(uint64_t spriteID);
	static bool UpdateScreenSprite(ID3D11DeviceContext* deviceContext);
	static void RenderScreenSprites(ID3D11DeviceContext* deviceContext);

	//~ ZBuffer Sprites
	static bool AddSpaceSprite(WorldSpaceSprite* sprite);
	static bool RemoveSpaceSprite(const WorldSpaceSprite* sprite);
	static bool RemoveSpaceSprite(uint64_t spriteId);
	static bool UpdateSpaceSprite(ID3D11DeviceContext* deviceContext);
	static void RenderSpaceSprites(ID3D11DeviceContext* deviceContext);

	//~ Light Related
	static bool AddLight(ILightAnyType* light);
	static bool RemoveLight(ILightAnyType* light);

private:
	static void SortWorldSpaceSprites();

private:
	inline static ID3D11Device* m_Device = nullptr;
	inline static bool m_Initialized{ false };
	inline static CameraController* m_CameraController = nullptr;
	inline static std::unordered_map<uint64_t, IBitmap*> m_ScreenSprites = {};
	inline static std::unordered_map<uint64_t, IBitmap*> m_BackgroundSprites = {};
	inline static int m_ScreenHeight{ 720 };
	inline static int m_ScreenWidth{ 1280 };

	//~ Light Images
	inline static std::unordered_map<uint64_t, ILightAnyType*> m_Lights = {};

	//~ Render Sprites which needed z buffer
	inline static std::unordered_map<uint64_t, WorldSpaceSprite*> m_WorldSpaceSprites{};
	inline static std::vector<WorldSpaceSprite*> m_SortedSpaceSprites;
};

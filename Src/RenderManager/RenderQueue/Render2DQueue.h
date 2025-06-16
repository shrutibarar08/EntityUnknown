#pragma once
#include <d3d11.h>
#include <unordered_map>

#include "PhysicsManager/PhysicsSystem.h"
#include "RenderManager/Camera/CameraController.h"
#include "RenderManager/Sprite/BackgroundSprite/BackgroundSprite.h"
#include "RenderManager/Sprite/ScreenSprite/ScreenSprite.h"
#include "RenderManager/Sprite/WorldSpaceSprite/WorldSpaceSprite.h"

class Render2DQueue
{
public:
	Render2DQueue(CameraController* controller, ID3D11Device* device, PhysicsSystem* physics);

	static bool UpdateBuffers(ID3D11DeviceContext* deviceContext);
	static void UpdateScreenSize(int width, int height);
	static void Clean();

	//~ Background screen Sprites
	static bool AddBackgroundSprite(BackgroundSprite* sprite);
	static bool RemoveBackgroundSprite(const BackgroundSprite* sprite);
	static bool RemoveBackgroundSprite(uint64_t spriteID);
	static bool UpdateBackgroundSprite(ID3D11DeviceContext* deviceContext);
	static void RenderBackgroundSprites(ID3D11DeviceContext* deviceContext);

	//~ Screen Sprites (on screen rendering - front ones only)
	static bool AddScreenSprite(ScreenSprite* sprite);
	static bool RemoveScreenSprite(const ScreenSprite* sprite);
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
	static bool AddLight(DirectionalLight* light);
	static bool RemoveLight(DirectionalLight* light);

private:
	static void SortBackgroundSprites();
	static void SortScreenSprites();
	static void SortWorldSpaceSprites();

private:
	inline static PhysicsSystem* m_PhysicsSystem{ nullptr };
	inline static bool m_Initialized{ false };
	inline static ID3D11Device* m_Device = nullptr;
	inline static CameraController* m_CameraController = nullptr;
	inline static int m_ScreenHeight{ 720 };
	inline static int m_ScreenWidth{ 1280 };

	//~ Renders Sprite at the back!
	inline static std::unordered_map<uint64_t, BackgroundSprite*> m_BackgroundSprites = {};
	inline static std::vector<BackgroundSprite*> m_SortedBackgroundSprites;

	//~ Render Sprites Directly on Screen!;
	inline static std::unordered_map<uint64_t, ScreenSprite*> m_ScreenSprites = {};
	inline static std::vector<ScreenSprite*> m_SortedScreenSprites;

	//~ Light Images
	inline static std::unordered_map<uint64_t, DirectionalLight*> m_Lights = {};

	//~ Render Sprites which needed z buffer
	inline static std::unordered_map<uint64_t, WorldSpaceSprite*> m_WorldSpaceSprites{};
	inline static std::vector<WorldSpaceSprite*> m_SortedSpaceSprites;
};

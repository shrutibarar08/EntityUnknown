#pragma once
#include <d3d11.h>
#include <unordered_map>

#include "RenderManager/Bitmap/IBitmap.h"
#include "RenderManager/Camera/CameraController.h"

class Render2DQueue
{
public:
	Render2DQueue(CameraController* controller, ID3D11Device* device);
	static bool AddBitmap(IBitmap* bitmap);
	static bool AddBackgroundBitmap(IBitmap* bitmap);
	static bool RemoveBitmap(const IBitmap* bitmap);
	static bool RemoveBackgroundBitmap(const IBitmap* bitmap);
	static bool RemoveBitmap(uint64_t bitmapId);
	static bool RemoveBackgroundBitmap(uint64_t bitmapId);
	static bool UpdateBuffers(ID3D11DeviceContext* context);
	static void RenderAll(ID3D11DeviceContext* context);
	static void RenderBackgroundAll(ID3D11DeviceContext* context);
	static void Clean();

	static void UpdateScreenSize(int width, int height);


private:
	inline static ID3D11Device* m_Device = nullptr;
	inline static bool m_Initialized{ false };
	inline static CameraController* m_CameraController = nullptr;
	inline static std::unordered_map<uint64_t, IBitmap*> m_BitmapsToRender = {};
	inline static std::unordered_map<uint64_t, IBitmap*> m_BitmapsToRenderInBackGround = {};
	inline static int m_ScreenHeight{ 720 };
	inline static int m_ScreenWidth{ 1280 };
};

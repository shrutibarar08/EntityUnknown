#pragma once

#include "RenderManager/Components/ModelBuffer.h"

struct ModelVertex
{
	DirectX::XMFLOAT3 Position;     // World/Model space position
	DirectX::XMFLOAT2 TexCoord;     // UV mapping
	DirectX::XMFLOAT3 Normal;       // Surface normal
	DirectX::XMFLOAT3 Tangent;      // Tangent for normal mapping
	DirectX::XMFLOAT3 BiNormal;     // Binormal for TBN matrix (optional, can be computed)
};
using DEFAULT_VERTEX = ModelVertex;
using MeshBuffer = StaticModelBufferSource<DEFAULT_VERTEX, uint32_t>;

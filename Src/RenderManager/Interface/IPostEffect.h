#pragma once

#include <d3d11.h>
#include <unordered_map>
#include <memory>

#include "SystemManager/PrimaryID.h"

class EURenderTarget;

class IPostEffect: public PrimaryID
{
public:
    virtual ~IPostEffect() = default;

    virtual bool OnResize(UINT width, UINT height) = 0;

    // Apply the effect: sample from 'src' and write to 'dst'.
    virtual bool Apply(ID3D11Device* device,
        ID3D11DeviceContext* ctx,
        EURenderTarget& src,
        EURenderTarget& dst) = 0;

    virtual bool IsEnabled() const { return true; }
};

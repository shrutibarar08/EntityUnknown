#pragma once
#include <cstdint>

#include "RenderManager/Sprite/ISprite.h"

enum class SpriteAnimMode: uint8_t
{
    EqualTimePerFrame,
    CustomTimePerFrame
};

class SpriteAnim
{
public:
    SpriteAnim(ISprite* targetSprite);
    ~SpriteAnim() = default;

    SpriteAnim(const SpriteAnim&) = default;
    SpriteAnim(SpriteAnim&&) = default;
    SpriteAnim& operator=(const SpriteAnim&) = default;
    SpriteAnim& operator=(SpriteAnim&&) = default;

    void Build(ID3D11Device* device, ID3D11DeviceContext* deviceContext);

    void SetMode(SpriteAnimMode mode);
    void SetLooping(bool loop);
    void SetTotalDuration(float seconds);
    bool IsFinished() const;
    void Play();
    void Stop();

    void AddFrame(const std::string& texturePath, float renderTime = 0.0f);
    void Update(float deltaTime);

private:
    void FinalizeFrame();

private:
    std::vector<std::pair<std::string, float>> m_FramesMetadata{};

    struct Frame
    {
        TEXTURE_RESOURCE TextureResource;
        float RenderTime;
    };

    ISprite* m_TargetSprite{ nullptr };
    std::vector<Frame> m_Frames;
    SpriteAnimMode m_AnimMode{ SpriteAnimMode::EqualTimePerFrame };

    //~ Meta Information
    float m_TotalDuration{ 1.0f };
    float m_Timer{ 0.0f };
    float m_CurrentFrame{ 0.0 };
    bool m_bPlaying{ true };
    bool m_Looping{ true };
    bool m_bFinished{ false };
};

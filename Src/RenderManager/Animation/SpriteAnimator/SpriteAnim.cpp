#include "SpriteAnim.h"

SpriteAnim::SpriteAnim(ISprite* targetSprite)
	: m_TargetSprite(targetSprite)
{}

void SpriteAnim::Build(ID3D11Device* device)
{
	for (auto& [texturePath, startTime]: m_FramesMetadata)
	{
		TEXTURE_RESOURCE resource = TextureLoader::GetTexture(device, texturePath);
		m_Frames.push_back({resource, startTime});
	}
	FinalizeFrame();
}

void SpriteAnim::SetMode(SpriteAnimMode mode)
{
	m_AnimMode = mode;
}

void SpriteAnim::SetLooping(bool loop)
{
	m_Looping = loop;
}

void SpriteAnim::SetTotalDuration(float seconds)
{
	m_TotalDuration = seconds;
}

bool SpriteAnim::IsFinished() const
{
	return m_bFinished;
}

void SpriteAnim::Play()
{
	m_bPlaying = true;
	m_bFinished = false;
	m_CurrentFrame = 0.0f;
	m_Timer = 0.0f;
}

void SpriteAnim::Stop()
{
	m_bPlaying = false;
	m_bFinished = false;
	m_CurrentFrame = 0.0f;
	m_Timer = 0.0f;
}

void SpriteAnim::AddFrame(const std::string& texturePath, float renderTime)
{
	m_FramesMetadata.emplace_back(std::make_pair(texturePath, renderTime));
}

void SpriteAnim::FinalizeFrame()
{
    if (m_Frames.empty())
        return;

    if (m_AnimMode == SpriteAnimMode::EqualTimePerFrame)
    {
        float perFrameTime = m_TotalDuration / static_cast<float>(m_Frames.size());
        for (auto& frame : m_Frames)
            frame.RenderTime = perFrameTime;
    }
    else // CustomTimePerFrame
    {
        float totalExplicitTime = 0.0f;
        int unsetCount = 0;

        for (const auto& frame : m_Frames)
        {
            if (frame.RenderTime > 0.0f)
                totalExplicitTime += frame.RenderTime;
            else
                ++unsetCount;
        }

        // If user gave all custom times, no need to balance
        if (unsetCount == 0)
        {
            m_TotalDuration = totalExplicitTime;
            return;
        }

        // Fill unset ones with distributed time from m_TotalDuration if available
        float remaining = m_TotalDuration - totalExplicitTime;
        float remainingTime = remaining > 0.0f ? remaining : 0.0f;
        float timePerUnset = (unsetCount > 0) ? (remainingTime / unsetCount) : 0.0f;

        for (auto& frame : m_Frames)
        {
            if (frame.RenderTime <= 0.0f)
                frame.RenderTime = timePerUnset;
        }

        // Recalculate final total duration
        m_TotalDuration = 0.0f;
        for (const auto& frame : m_Frames)
            m_TotalDuration += frame.RenderTime;
    }
}

void SpriteAnim::Update(float deltaTime)
{
    if (!m_bPlaying || m_Frames.empty())
        return;

    m_Timer += deltaTime;

    // Handle non-looping completion
    if (!m_Looping && m_Timer >= m_TotalDuration)
    {
        m_Timer = m_TotalDuration;
        m_CurrentFrame = m_Frames.size() - 1;
        m_bFinished = true;
        m_bPlaying = false;

        m_TargetSprite->UpdateTextureResource(m_Frames[m_CurrentFrame].TextureResource);
        return;
    }

    // Handle looping
    if (m_Looping && m_Timer >= m_TotalDuration)
    {
        m_Timer = fmod(m_Timer, m_TotalDuration);
        m_bFinished = false;
    }

    // Find the current frame based on accumulated time
    float accumulated = 0.0f;
    for (size_t i = 0; i < m_Frames.size(); ++i)
    {
        accumulated += m_Frames[i].RenderTime;

        // Always include last frame, in case accumulated slightly < total due to float error
        if (m_Timer < accumulated || i == m_Frames.size() - 1)
        {
            // Only update texture if frame changed
            if (m_CurrentFrame != i)
            {
                m_CurrentFrame = i;
                m_TargetSprite->UpdateTextureResource(m_Frames[m_CurrentFrame].TextureResource);
            }
            return;
        }
    }
}

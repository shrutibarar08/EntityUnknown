#include "SpriteAnim.h"

#include <ranges>

#include "Imgui/imgui.h"

SpriteAnim::SpriteAnim(ISprite* targetSprite)
	: m_TargetSprite(targetSprite)
{}

void SpriteAnim::Build(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
    if (m_bBuild) return;
    m_Frames.clear();
	for (auto& [texturePath, startTime]: m_FramesMetadata)
	{
		TEXTURE_RESOURCE resource = TextureLoader::GetTexture(device, deviceContext, texturePath);
		m_Frames.push_back({resource, startTime});
	}
	FinalizeFrame();

    m_bBuild = true;
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
    m_bBuild = false;
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

        m_TargetSprite->GetShaderResource()->UpdateTextureResource(m_Frames[m_CurrentFrame].TextureResource);
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
                m_TargetSprite->GetShaderResource()->UpdateTextureResource(m_Frames[m_CurrentFrame].TextureResource);
            }
            return;
        }
    }
}

void SpriteAnim::LoadFromSweetData(const SweetLoader& sweetData)
{
    m_FramesMetadata.clear();
    m_Frames.clear();

    // Load mode
    std::string modeStr = sweetData["Mode"].GetValue();
    if (modeStr == "EqualTimePerFrame")
        m_AnimMode = SpriteAnimMode::EqualTimePerFrame;
    else if (modeStr == "CustomTimePerFrame")
        m_AnimMode = SpriteAnimMode::CustomTimePerFrame;

    m_TotalDuration = sweetData["TotalDuration"].AsFloat();
    m_Looping = sweetData["Looping"].AsBool();

    for (const auto& frame : sweetData["Frames"] | std::views::values)
    {
        std::string texturePath = frame["TexturePath"].GetValue();
        float duration = frame["RenderTime"].AsFloat();

        m_FramesMetadata.emplace_back(texturePath, duration);
    }

    m_bBuild = false;
}

SweetLoader SpriteAnim::GetSweetData()
{
    SweetLoader loader;
    loader.GetOrCreate("Mode") = (m_AnimMode == SpriteAnimMode::EqualTimePerFrame) ? "EqualTimePerFrame" : "CustomTimePerFrame";
    loader.GetOrCreate("TotalDuration") = std::to_string(m_TotalDuration);
    loader.GetOrCreate("Looping") = m_Looping ? "true" : "false";

    auto& framesNode = loader.GetOrCreate("Frames");
    for (size_t i = 0; i < m_FramesMetadata.size(); ++i)
    {
        const auto& [path, time] = m_FramesMetadata[i];
        framesNode.GetOrCreate(std::to_string(i)).GetOrCreate("TexturePath") = path;
        framesNode.GetOrCreate(std::to_string(i)).GetOrCreate("RenderTime") = std::to_string(time);
    }

    return loader;
}

void SpriteAnim::ControlUI()
{
    // === Animation Mode ===
    const char* modeItems[] = { "EqualTimePerFrame", "CustomTimePerFrame" };
    int currentMode = static_cast<int>(m_AnimMode);
    if (ImGui::Combo("Anim Mode", &currentMode, modeItems, IM_ARRAYSIZE(modeItems)))
    {
        m_AnimMode = static_cast<SpriteAnimMode>(currentMode);
    }

    // === Looping Toggle ===
    ImGui::Checkbox("Looping", &m_Looping);

    // === Total Duration ===
    if (m_AnimMode == SpriteAnimMode::EqualTimePerFrame)
    {
        ImGui::DragFloat("Total Duration", &m_TotalDuration, 0.1f, 0.01f, 100.0f);
    }

    // === Frame List ===
    if (ImGui::TreeNode("Frames"))
    {
        for (size_t i = 0; i < m_FramesMetadata.size(); ++i)
        {
            auto& [path, time] = m_FramesMetadata[i];
            ImGui::PushID(static_cast<int>(i));

            if (ImGui::InputText("Texture", path.data(), path.capacity() + 1))
            {
                // Optional: force rebuild required
                m_bBuild = false;
            }

            ImGui::SameLine();
            if (ImGui::Button("..."))
            {
                std::string file = IRender::OpenFileDialog();
                if (!file.empty())
                {
                    path = file;
                    m_bBuild = false;
                }
            }

            if (m_AnimMode == SpriteAnimMode::CustomTimePerFrame)
            {
                ImGui::DragFloat("Render Time", &time, 0.01f, 0.01f, 10.0f);
            }

            ImGui::SameLine();
            if (ImGui::Button("Remove"))
            {
                m_FramesMetadata.erase(m_FramesMetadata.begin() + i);
                m_bBuild = false;
                ImGui::PopID();
                break;
            }

            ImGui::Separator();
            ImGui::PopID();
        }

        if (ImGui::Button("Add Frame"))
        {
            m_FramesMetadata.emplace_back("", 0.1f);
            m_bBuild = false;
        }

        ImGui::TreePop();
    }
}

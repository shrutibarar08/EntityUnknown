#include "EngineToolPanelPolicy.h"
#include <algorithm>
#include <cstring>

#if defined(_WIN32)
#include <windows.h>
#endif

#include "Editor/Core/EditorContext.h"

using std::string;
namespace fs = std::filesystem;

string EngineToolPanelPolicy::ToLower(string s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char)std::tolower(c); });
    return s;
}

std::wstring EngineToolPanelPolicy::Utf8ToWide(const std::string& sUtf8)
{
#if defined(_WIN32)
    if (sUtf8.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, sUtf8.c_str(), (int)sUtf8.size(), nullptr, 0);
    std::wstring ws; ws.resize(n);
    MultiByteToWideChar(CP_UTF8, 0, sUtf8.c_str(), (int)sUtf8.size(), ws.data(), n);
    return ws;
#else
    return std::wstring(sUtf8.begin(), sUtf8.end());
#endif
}

std::string EngineToolPanelPolicy::WideToUtf8(const std::wstring& ws)
{
#if defined(_WIN32)
    if (ws.empty()) return {};
    int n = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), nullptr, 0, nullptr, nullptr);
    std::string s; s.resize(n);
    WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), s.data(), n, nullptr, nullptr);
    return s;
#else
    return std::string(ws.begin(), ws.end());
#endif
}

std::string EngineToolPanelPolicy::NameFromPath(const std::string& relUtf8)
{
#if defined(_WIN32)
    std::u8string u8(reinterpret_cast<const char8_t*>(relUtf8.c_str()));
    fs::path p(u8);
#else
    fs::path p(relUtf8);
#endif
    auto stem = p.stem().string();
    return stem.empty() ? "NewPostFX" : stem;
}

bool EngineToolPanelPolicy::PassesFilter(const EU_POST_EFFECT_SHARED_VIEW& v) const
{
    if (m_filter.empty()) return true;
    const string hay = ToLower(v.EffectName + "|" + WideToUtf8(v.BlobDesc.FilePath));
    return hay.find(ToLower(m_filter)) != string::npos;
}

void EngineToolPanelPolicy::SetStatus(Status s, const std::string& msg)
{
    m_status = s;
    m_statusText = msg;
    m_statusTimer = 3.0f;
}

std::string EngineToolPanelPolicy::TryExtractUtf8PathFromPayload(const ImGuiPayload* p) const
{
    if (!p || !p->Data || p->DataSize <= 0) return {};
    // try header-aware
    if ((size_t)p->DataSize >= sizeof(PayloadHeader))
    {
        const auto* hdr = reinterpret_cast<const PayloadHeader*>(p->Data);
        if (hdr->magic == 0xA5517BAD && hdr->version == 1)
        {
            const char* pathz = reinterpret_cast<const char*>(p->Data) + sizeof(PayloadHeader);
            const char* end = reinterpret_cast<const char*>(p->Data) + p->DataSize;
            if (const void* zero = memchr(pathz, 0, (size_t)(end - pathz)))
                return std::string(pathz);
        }
    }
    // fallback: raw cstring
    const char* s = (const char*)p->Data;
    if (const void* z = memchr(s, 0, (size_t)p->DataSize))
        return std::string(s);
    return {};
}

// ---------------- Public ----------------
bool EngineToolPanelPolicy::Init(LevelEditorContext* /*ctx*/)
{
    return true;
}

void EngineToolPanelPolicy::DrawTools(LevelEditorContext* ctx)
{
    // ROW 1: header (search only), its own line, non-overlapping with tabs
    DrawHeaderRow();

    ImGui::Separator();

    if (ImGui::BeginTabBar("##pcp_tabs", ImGuiTabBarFlags_Reorderable))
    {
        if (ImGui::BeginTabItem("View"))
        {
            DrawViewTab(ctx);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Create"))
        {
            DrawCreateTab(ctx);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    DrawRenamePopup();
    DrawToast();
}

// ---------------- UI ----------------
void EngineToolPanelPolicy::DrawHeaderRow()
{
    // keep everything on one row, and inside current panel width
    const float avail = ImGui::GetContentRegionAvail().x;
    const float minFilter = 180.f;
    const float filterW = std::max(minFilter, avail);

    ImGui::SetNextItemWidth(filterW);
    ImGui::InputTextWithHint("##fx_filter", "Search by name or path…", &m_filter);
}

void EngineToolPanelPolicy::DrawViewTab(LevelEditorContext* ctx)
{
    // Table stretches evenly; no spilling
    const ImGuiTableFlags flags =
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
        ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchSame;

    if (ImGui::BeginTable("##pcp_view_table", 5, flags))
    {
        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 90.f);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Path", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Valid", ImGuiTableColumnFlags_WidthFixed, 60.f);
        ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 180.f);
        ImGui::TableHeadersRow();

        const auto& list = PostEffectPool::Get().GetAllEffects();
        int row = 0;
        for (const auto& v : list)
        {
            if (!PassesFilter(v)) continue;

            ImGui::TableNextRow();
            ImGui::PushID(row++);

            // ID
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%llu", (unsigned long long)v.EffectID);

            // Name (wrapped)
            ImGui::TableSetColumnIndex(1);
            {
                float wrapTo = ImGui::GetCursorPosX() + ImGui::GetColumnWidth();
                ImGui::PushTextWrapPos(wrapTo);
                ImGui::TextUnformatted(v.EffectName.c_str());
                ImGui::PopTextWrapPos();
            }

            // Path (wrapped)
            ImGui::TableSetColumnIndex(2);
            {
                float wrapTo = ImGui::GetCursorPosX() + ImGui::GetColumnWidth();
                ImGui::PushTextWrapPos(wrapTo);
                ImGui::TextUnformatted(WideToUtf8(v.BlobDesc.FilePath).c_str());
                ImGui::PopTextWrapPos();
            }

            // Valid
            ImGui::TableSetColumnIndex(3);
            ImGui::TextUnformatted(v.IsValid() ? "Yes" : "No");

            // Actions: Rename | Apply
            ImGui::TableSetColumnIndex(4);
            if (ImGui::SmallButton("Rename"))
            {
                m_pendingRenameID = v.EffectID;
                m_renameBuffer = v.EffectName;
                m_showRenamePopup = true;
                m_focusRenameOnce = true;
                ImGui::OpenPopup("Rename Post Effect");
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Apply"))
            {
                ApplyByID(ctx, v.EffectID);
            }

            ImGui::PopID();
        }

        ImGui::EndTable();
    }
}

void EngineToolPanelPolicy::DrawCreateTab(LevelEditorContext* ctx)
{
    // Two-column compact form (label column fixed)
    const float avail = ImGui::GetContentRegionAvail().x;
    const float labelW = 110.f;
    const float fieldW = std::max(220.f, avail - labelW - ImGui::GetStyle().ItemSpacing.x);

    // Name
    ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Effect Name"); ImGui::SameLine(labelW);
    ImGui::SetNextItemWidth(fieldW);
    ImGui::InputText("##fx_name", &m_newDesc.EffectName);

    // Path (editable) + DnD on the same widget line (no big drop bar)
    ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("HLSL Path"); ImGui::SameLine(labelW);
    {
        std::string disp = m_newDescPathUtf8;
        ImGui::SetNextItemWidth(fieldW);
        if (ImGui::InputText("##fx_path", &disp))
        {
            m_newDescPathUtf8 = disp;
#if defined(_WIN32)
            std::u8string u8(reinterpret_cast<const char8_t*>(disp.c_str()));
            m_newDesc.BlobDesc.FilePath = std::filesystem::path(u8);
#else
            m_newDesc.BlobDesc.FilePath = std::filesystem::path(disp);
#endif
        }
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload(nullptr))
            {
                if (auto rel = TryExtractUtf8PathFromPayload(p); !rel.empty())
                {
                    m_newDescPathUtf8 = rel;
#if defined(_WIN32)
                    std::u8string u8(reinterpret_cast<const char8_t*>(rel.c_str()));
                    m_newDesc.BlobDesc.FilePath = std::filesystem::path(u8);
#else
                    m_newDesc.BlobDesc.FilePath = std::filesystem::path(rel);
#endif
                    if (m_newDesc.EffectName == "NewPostFX" || m_newDesc.EffectName.empty())
                        m_newDesc.EffectName = NameFromPath(rel);
                }
            }
            ImGui::EndDragDropTarget();
        }
    }

    // Entry/Target on one line
    ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Entry/Target"); ImGui::SameLine(labelW);
    float half = (fieldW - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
    ImGui::SetNextItemWidth(half);
    ImGui::InputText("##fx_entry", &m_newDesc.BlobDesc.EntryPoint);
    ImGui::SameLine(0, ImGui::GetStyle().ItemSpacing.x);
    ImGui::SetNextItemWidth(half);
    ImGui::InputText("##fx_target", &m_newDesc.BlobDesc.Target);

    // Actions (horizontal)
    const bool valid =
        !m_newDesc.EffectName.empty() &&
        !m_newDesc.BlobDesc.EntryPoint.empty() &&
        !m_newDesc.BlobDesc.Target.empty() &&
        !m_newDesc.BlobDesc.FilePath.empty();

    ImGui::Dummy(ImVec2(0, 4));
    ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Actions"); ImGui::SameLine(labelW);

    if (valid && ImGui::Button("Create"))
    {
        if (PostEffectPool::Get().IsExits(m_newDesc))
        {
            SetStatus(Status::Warn, "Effect already exists.");
        }
        else
        {
            const ID id = PostEffectPool::Get().Add(m_newDesc);
            SetStatus(id ? Status::Ok : Status::Error,
                id ? ("Created: " + m_newDesc.EffectName + " (ID " + std::to_string((uint64_t)id) + ")")
                : "Create failed.");
            if (id) m_newDesc.EffectName = "NewPostFX"; // keep path for rapid repeat
        }
    }
    ImGui::SameLine();

    if (valid && ImGui::Button("Apply"))
    {
        ApplyByDesc(ctx, m_newDesc);
    }
    ImGui::SameLine();

    if (ImGui::Button("Clear"))
    {
        m_newDesc.EffectName = "NewPostFX";
        m_newDesc.BlobDesc.EntryPoint = "main";
        m_newDesc.BlobDesc.Target = "ps_5_0";
        m_newDescPathUtf8 = "Assets/Shader/Post/MyEffect.hlsl";
#if defined(_WIN32)
        std::u8string u8(reinterpret_cast<const char8_t*>(m_newDescPathUtf8.c_str()));
        m_newDesc.BlobDesc.FilePath = std::filesystem::path(u8);
#else
        m_newDesc.BlobDesc.FilePath = std::filesystem::path(m_newDescPathUtf8);
#endif
    }

    if (!valid)
        ImGui::SameLine(), ImGui::TextDisabled("Fill fields to enable Create/Apply.");
}

void EngineToolPanelPolicy::DrawRenamePopup()
{
    if (m_showRenamePopup)
        ImGui::OpenPopup("Rename Post Effect");

    if (ImGui::BeginPopupModal("Rename Post Effect", /*p_open=*/nullptr,
        ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (m_focusRenameOnce)
        {
            ImGui::SetKeyboardFocusHere();
            m_focusRenameOnce = false;
        }

        ImGui::InputText("New Name", &m_renameBuffer);

        if (ImGui::Button("OK"))
        {
            bool ok = false;
            if (!m_renameBuffer.empty() && m_pendingRenameID != 0)
                ok = PostEffectPool::Get().Rename(m_pendingRenameID, m_renameBuffer);

            if (ok)
            {
                PostEffectPool::Get().GetAllEffects();
                SetStatus(Status::Ok, "Renamed to '" + m_renameBuffer + "'.");
            }
            else
            {
                SetStatus(Status::Error, "Rename failed (duplicate name or invalid ID).");
            }

            m_pendingRenameID = 0;
            m_renameBuffer.clear();
            m_showRenamePopup = false;
            m_focusRenameOnce = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel"))
        {
            m_pendingRenameID = 0;
            m_renameBuffer.clear();
            m_showRenamePopup = false;
            m_focusRenameOnce = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
    else
    {
        if (m_pendingRenameID == 0)
            m_showRenamePopup = false;
    }
}

void EngineToolPanelPolicy::DrawToast()
{
    if (m_status == Status::None) return;

    m_statusTimer -= ImGui::GetIO().DeltaTime;
    if (m_statusTimer <= 0.f)
    {
        m_status = Status::None;
        m_statusText.clear();
        return;
    }

    ImGui::SetNextWindowBgAlpha(0.9f);
    ImGui::Begin("##pcp_toast", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove);
    const char* tag = "";
    switch (m_status)
    {
    case Status::Info:  tag = "Info";  break;
    case Status::Ok:    tag = "OK";    break;
    case Status::Warn:  tag = "Warn";  break;
    case Status::Error: tag = "Error"; break;
    default: break;
    }
    ImGui::Text("[%s] %s", tag, m_statusText.c_str());
    ImGui::End();
}

// ---------------- Actions ----------------
void EngineToolPanelPolicy::ApplyByID(LevelEditorContext* ctx, ID effectId)
{
    if (!ctx) { SetStatus(Status::Error, "No context."); return; }
    auto* cs = ctx->GetCommandStack();
    if (!cs) { SetStatus(Status::Error, "No CommandStack."); return; }

    cs->Execute(std::make_unique<CmdApplyPostEffect>(effectId), ctx);
    SetStatus(Status::Ok, "Applied Post Effect ID " + std::to_string((uint64_t)effectId));
}

void EngineToolPanelPolicy::ApplyByDesc(LevelEditorContext* ctx, const EU_POST_EFFECT_INIT_DESC& d)
{
    if (!ctx) { SetStatus(Status::Error, "No context."); return; }
    auto* cs = ctx->GetCommandStack();
    if (!cs) { SetStatus(Status::Error, "No CommandStack."); return; }

    cs->Execute(std::make_unique<CmdApplyPostEffect>(d), ctx);
    SetStatus(Status::Ok, "Applied '" + d.EffectName + "'");
}

// ---------------- Drop Zone ----------------
void EngineToolPanelPolicy::DropZone(const char* id, const ImVec2& size, const char* hint,
    const std::function<void(const std::string&)>& onDrop)
{
    ImVec2 cur = ImGui::GetCursorPos();
    ImGui::InvisibleButton(id, size);
    const bool hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 p0 = ImGui::GetItemRectMin();
    ImVec2 p1 = ImGui::GetItemRectMax();
    dl->AddRect(p0, p1, hovered ? IM_COL32(200, 200, 255, 200) : IM_COL32(140, 140, 140, 140), 6.f, 0, 1.5f);

    ImVec2 center = { (p0.x + p1.x) * 0.5f, (p0.y + p1.y) * 0.5f };
    ImVec2 tsize = ImGui::CalcTextSize(hint);
    ImGui::SetCursorScreenPos({ center.x - tsize.x * 0.5f, center.y - tsize.y * 0.5f });
    ImGui::TextUnformatted(hint);
    ImGui::SetCursorPos(cur);

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload(nullptr)) // accept any; parse internally
        {
            if (auto rel = TryExtractUtf8PathFromPayload(p); !rel.empty())
                onDrop(rel);
        }
        ImGui::EndDragDropTarget();
    }
}

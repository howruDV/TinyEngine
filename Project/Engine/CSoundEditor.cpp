#include "pch.h"
#include "CSoundEditor.h"

CSoundEditor::CSoundEditor()
    : CEditor(EDITOR_TYPE::SOUND)
    , m_Sound(nullptr)
    , m_PlayPosition(0.f)
    , m_Volume(0.5f)
{
}

CSoundEditor::~CSoundEditor()
{
}

void CSoundEditor::render(bool* open)
{
    // =====================================
    // DockSpace
    // =====================================
    ImGuiWindowClass window_class;
    window_class.ClassId = ImGui::GetMainViewport()->ID;
    window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoDockingSplitOther;
    window_class.DockingAllowUnclassed = true;
    ImGui::SetNextWindowClass(&window_class);

    if (!ImGui::Begin(ToString(GetName()).c_str(), open))
    {
        ImGui::End();
        return;
    }

    ImGuiID dockSpace = ImGui::GetID("Sound Editor DockSpace");
    ImGui::DockSpace(dockSpace);

    // =====================================
    // Material Editor Render
    // =====================================
    render();

    ImGui::End();
}

void CSoundEditor::render()
{
    if (nullptr == m_Sound.Get())
        return;

    ImGui_SetWindowClass(GetEditorType());
    DrawDetails();
}

void CSoundEditor::DrawDetails()
{
    ImGui::Begin("Details##SoundEditor");

    ImGui_InputText("Sound Name", ToString(m_Sound->GetName()));

    if (ImGui::SliderFloat(ImGui_LabelPrefix("Volume").c_str(), &m_Volume, 0.f, 1.f))
        m_Sound->SetVolume(m_Volume);

    ImGui::Text("Progress");
    ImGui::SameLine();
    ImGui::ProgressBar((float)m_Sound->GetPosition() / (float)m_Sound->GetLength());

    if (m_Sound->IsPlaying())
    {
        if (ImGui::Button("Pause", ImVec2(120, 0)))
        {
            m_Sound->Pause();
        }
    }
    else
    {
        if (ImGui::Button("Play", ImVec2(120, 0)))
        {
            m_Sound->Play(1, m_Volume, false);
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Stop", ImVec2(120, 0)))
    {
        m_Sound->Stop();
    }

    ImGui::End();
}

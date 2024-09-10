#include "pch.h"
#include "CElfilisD_Damage.h"
#include "CElfilisFSM.h"
#include "CBossLevelFlowMgr.h"

#include "CCameraController.h"

CElfilisD_Damage::CElfilisD_Damage()
    : m_PrevDrag(0.f)
{
}

CElfilisD_Damage::~CElfilisD_Damage()
{
}

void CElfilisD_Damage::tick()
{
    switch (m_Step)
    {
    case StateStep::Start: {
        Start();
    }
    break;
    case StateStep::Progress: {
        Process();
    }
    break;
    }
}

void CElfilisD_Damage::Enter_Step()
{
    switch (m_Step)
    {
    case StateStep::Start: {
        GetOwner()->Animator()->Play(ANIMPREFIX("Damage"), false, false, 2.5f, 0.f);
        GetOwner()->Transform()->SetWorldPos(Vec3());
        GetOwner()->Transform()->SetWorldRotation(Vec3());

        ELFFSM->SetStateLock(true);

        CBossMgr::GetBossFlowMgr()->ChangeFlow(BossLevelFlow::DemoPlay);

        // Sound
        wstring Damage = L"sound\\wav\\CharaBossChimera2\\0018_Damage.wav";
        GamePlayStatic::Play2DSound(Damage, 1, SOUND_ELFILIS);

        // Camera : Fixed View
        CAMERACTRL->SetMainTarget(BOSS);
        CAMERACTRL->FixedView(true, Vec3(-132.45f, 83.80f, -140.07f));

        CAMERACTRL->SetOffset(Vec3(0.f, 0.f, 0.f));
        CAMERACTRL->SetMinSpeed(200.f);
        CAMERACTRL->SetMaxSpeed(500.f);
        CAMERACTRL->SetThresholdDistance(500.f);
        CAMERACTRL->SetRotationSpeed(150.f);
        CAMERACTRL->SetZoomMinSpeed(100.f);
        CAMERACTRL->SetZoomMaxSpeed(500.f);
        CAMERACTRL->SetZoomThreshold(500.f);
        CAMERACTRL->SetTargetOffset(Vec3(0.f, 75.f, 0.f));
    }
    break;
    case StateStep::Progress: {
        // BackStep
        m_ForceDir = GetOwner()->Transform()->GetWorldDir(DIR_TYPE::FRONT) * -1.f;
        m_ForceDir.y = 0.f;
        m_ForceDir.Normalize();
        m_TargetPos = m_ForceDir * 300.f;

        // Sound
        wstring ResistBGM = L"sound\\stream\\K15_ChimeraSoulCoreStart\\K15_ChimeraSoulCoreStart.wav";
        GamePlayStatic::Play2DSound(ResistBGM, 1, SOUND_BGM);
        CBossMgr::GetBossFlowMgr()->ChangeBGMSize(0.f, 1.f);
    }
    break;
    }
}

void CElfilisD_Damage::Exit_Step()
{
    switch (m_Step)
    {
    case StateStep::Start:
        break;
    case StateStep::Progress: {
        GetOwner()->Rigidbody()->SetVelocity(Vec3());
        GetOwner()->Rigidbody()->SetAngularVelocity(Vec3());
    }
    break;
    }
}

void CElfilisD_Damage::Start()
{
    ChangeStep(StateStep::Progress);
}

void CElfilisD_Damage::Process()
{
    // Move
    Vec3 NewPos = GetOwner()->Transform()->GetWorldPos();
    float t = NewPos.Length() / m_TargetPos.Length();
    float SinGraph = clamp(sinf(t * XM_PI * 0.99f) * 1.3f, 0.f, 1.f);
    float NewSpeed = 25.f * SinGraph;
    GetOwner()->Rigidbody()->SetVelocity(m_ForceDir * NewSpeed);

    if (t >= 1.f)
    {
        ELFFSM->SetStateLock(false);
        ELFFSM->ChangeStateGroup(ElfilisStateGroup::DEMO, L"DEMO_APPEAR2_JUMP");
    }
}
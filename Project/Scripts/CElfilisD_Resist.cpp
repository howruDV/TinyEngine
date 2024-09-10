#include "pch.h"
#include "CElfilisD_Resist.h"
#include "CElfilisFSM.h"
#include "CCameraController.h"
#include "CBossLevelFlowMgr.h"

CElfilisD_Resist::CElfilisD_Resist()
    : m_AccTime(0.f)
    , m_bFrmEnter(false)
{
}

CElfilisD_Resist::~CElfilisD_Resist()
{
}

void CElfilisD_Resist::tick()
{
    switch (m_Step)
    {
    case StateStep::ReadyStart: {
        ReadyStart();
    }
    break;
    case StateStep::Ready: {
        Ready();
    }
    break;
    case StateStep::Start: {
        Start();
    }
    break;
    case StateStep::Progress: {
        Progress();
    }
    break;
    }
}

void CElfilisD_Resist::Enter()
{
    ELFFSM->SetResist(true);

    m_Step = StateStep::ReadyStart;
    Enter_Step();
}

void CElfilisD_Resist::Exit()
{
    ELFFSM->SetResist(false);
    Exit_Step();
}

void CElfilisD_Resist::Enter_Step()
{
    switch (m_Step)
    {
    case StateStep::ReadyStart: {
        GetOwner()->Animator()->Play(ANIMPREFIX("LastDamageStart"), false, false, 1.5f, 0.f);
        GetOwner()->Transform()->SetWorldPos(ELFFSM->GetMapFloorOffset());
        GetOwner()->Transform()->SetWorldRotation(Vec3());

        ELFFSM->SetStateLock(true);

        CBossMgr::GetBossFlowMgr()->ChangeFlow(BossLevelFlow::DemoPlay);

        // Sound
        wstring Damage = L"sound\\wav\\CharaBossChimera2\\0060_ResistDamage.wav";
        GamePlayStatic::Play2DSound(Damage, 1, SOUND_ELFILIS);

        // Camera : ���Ǹ��� Ÿ�� (������ġ, ���� �� �׻� ����)
        CAMERACTRL->SetMainTarget(BOSS);
        CAMERACTRL->SetOffset(Vec3(0.f, 0.f, 0.f));
        CAMERACTRL->SetTargetOffset(Vec3(0.f, 50.f, 0.f));
        CAMERACTRL->SetSubTargetOffset(Vec3(0.f, 0.f, 0.f));
        CAMERACTRL->SetLookDir(Vec3(0.2f, -0.3f, -1.f).Normalize());
        CAMERACTRL->SetLookDist(400.f);
        CAMERACTRL->Normal(true);
    }
    break;
    case StateStep::Ready: {
        GetOwner()->Animator()->Play(ANIMPREFIX("LastDamageWait"), false, false, 1.5f);
        m_AccTime = 0.f;
        m_bFrmEnter = true;

        // Sound
        wstring Resist = L"sound\\wav\\CharaBossChimera2\\0062_Resist.wav";
        GamePlayStatic::Play2DSound(Resist, 1, SOUND_ELFILIS);

        // Camera : ����
        CAMERACTRL->SetLookDir(Vec3(-0.8f, -0.1f, -0.9f).Normalize());
        CAMERACTRL->SetLookDist(150.f);
    }
    break;
    case StateStep::Start: {
        ELFFSM->GetUnit()->AddResistHP();
        // GetOwner()->Animator()->Play(ANIMPREFIX("ResistStart"), false, false, 1.5f);
        CBossMgr::GetBossFlowMgr()->ChangeFlow(BossLevelFlow::Fight);
        m_AccTime = 0.f;

        // Camera : ��Ÿ��
        CAMERACTRL->LoadInitSetting(true);
        CAMERACTRL->SetElfilisTwoTarget();
    }
    break;
    case StateStep::Progress: {
        GetOwner()->Animator()->Play(ANIMPREFIX("ResistLoop"), true, true, 1.5f, 0.3f);
        m_AccTime += DT;
    }
    break;
    }
}

void CElfilisD_Resist::Exit_Step()
{
    switch (m_Step)
    {
    case StateStep::ReadyStart:
        break;
    case StateStep::Ready:
        break;
    case StateStep::Start:
        break;
    case StateStep::Progress: {
        GetOwner()->Rigidbody()->SetVelocity(Vec3());
        GetOwner()->Rigidbody()->SetAngularVelocity(Vec3());
    }
    break;
    }
}

void CElfilisD_Resist::ReadyStart()
{
    // Sound
    if (CHECK_ANIMFRM(GetOwner(), 60) && m_bFrmEnter)
    {
        m_bFrmEnter = false;

        wstring DropHalberd = L"sound\\wav\\CharaBossChimera2\\0061_ResistDropHalberd.wav";
        GamePlayStatic::Play2DSound(DropHalberd, 1, SOUND_ELFILIS * 0.7f);
    }

    if (GetOwner()->Animator()->IsFinish())
    {
        ChangeStep(StateStep::Ready);
    }
}

void CElfilisD_Resist::Ready()
{
    m_AccTime += DT;

    if (m_AccTime > 1.75f)
    {
        ChangeStep(StateStep::Start);
    }
}

void CElfilisD_Resist::Start()
{
    if (GetOwner()->Animator()->IsFinish())
    {
        ChangeStep(StateStep::Progress);
    }
}

void CElfilisD_Resist::Progress()
{
    m_AccTime += DT;
    RotateToPlayer(DT * 0.3f);

    // Move
    float Speed = 0.5f;
    float RotSpeed = 0.5f;
    Vec3 Force = GetOwner()->Transform()->GetWorldDir(DIR_TYPE::FRONT) * -1.f;
    Force.y = 0.f;
    Force.Normalize();
    Force = (GetOwner()->Rigidbody()->GetVelocity() + Force * RotSpeed * DT).Normalize() * Speed;

    if (m_AccTime <= 1.f)
    {
        float t = m_AccTime / 1.f;
        float Delta = sinf(XM_PI / 2.f * t);
        GetOwner()->Rigidbody()->SetVelocity(Force * Delta);
    }
    else
    {
        GetOwner()->Rigidbody()->SetVelocity(Force);
    }

    // Change State
    bool bSuccess = (BOSSUNIT->GetCurInfo().HP <= 0.f);

    if (bSuccess)
    {
        ELFFSM->SetStateLock(false);
        ELFFSM->ChangeStateGroup(ElfilisStateGroup::DEMO, L"DEMO_RESIST_FAIL");
    }
    else
    {
        if (m_AccTime > 10.f)
        {
            ELFFSM->SetStateLock(false);
            ELFFSM->ChangeStateGroup(ElfilisStateGroup::DEMO, L"DEMO_RESIST_SUCCESS");
        }
    }
}
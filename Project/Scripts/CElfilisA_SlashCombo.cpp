#include "pch.h"
#include "CElfilisA_SlashCombo.h"
#include "CElfilisFSM.h"
#include "CMomentaryObjScript.h"
#include "CCameraController.h"

CElfilisA_SlashCombo::CElfilisA_SlashCombo()
    : m_ComboLevel(0)
    , m_SpawnDist(0.f)
    , m_bCamShake(false)
{
    m_StabRockPref = CAssetMgr::GetInst()->Load<CPrefab>(L"prefab\\ElfilisSlashStone.pref", L"prefab\\ElfilisSlashStone.pref");
}

CElfilisA_SlashCombo::~CElfilisA_SlashCombo()
{
}

void CElfilisA_SlashCombo::tick()
{
    switch (m_Step)
    {
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
    case StateStep::Wait: {
        Wait();
    }
    break;
    }
}

void CElfilisA_SlashCombo::Enter()
{
    m_ComboLevel = 0;
    m_Step = StateStep::Ready;
    Enter_Step();
}

void CElfilisA_SlashCombo::Exit()
{
    Exit_Step();

    GetOwner()->Rigidbody()->SetVelocity(Vec3());
    GetOwner()->Rigidbody()->SetAngularVelocity(Vec3());
    ELFFSM->OffWeaponTrigger();
}

void CElfilisA_SlashCombo::Enter_Step()
{
    switch (m_Step)
    {
    case StateStep::Ready: {
        if (m_ComboLevel == 0)
        {
            GetOwner()->Animator()->Play(ANIMPREFIX("SlashReady"), false);
        }
        else if (m_ComboLevel == 1)
        {
            GetOwner()->Animator()->Play(ANIMPREFIX("SlashChainReady"), false);
        }
        else if (m_ComboLevel == 2)
        {
            GetOwner()->Animator()->Play(ANIMPREFIX("SlashChainStabReady"), false);
        }

        ELFFSM->OnWeaponTrigger();
        m_StartPos = GetOwner()->Transform()->GetWorldPos();

        // sound
        wstring MoveAir = L"sound\\wav\\CharaBossChimera2\\0024_MoveAir.wav";
        GamePlayStatic::Play2DSound(MoveAir, 1, SOUND_ELFILIS);
    }
    break;
    case StateStep::Start: {
        GetOwner()->Animator()->Play(ANIMPREFIX("SlashStart"), false);
        GetOwner()->Rigidbody()->SetVelocity(Vec3::Zero);
        GetOwner()->Rigidbody()->AddForce(Vec3(0.f, -1200.f, 1.f), ForceMode::Impulse);
        //@Effect Halberd Ʈ���� ����Ʈ

        // sound
        wstring Wind = L"sound\\wav\\CharaBossChimera2\\0045_SlashWind.wav";
        GamePlayStatic::Play2DSound(Wind, 1, SOUND_ELFILIS);
    }
    break;
    case StateStep::Progress: {
        GetOwner()->Animator()->Play(ANIMPREFIX("Slash"));
        m_SpawnDist = 0.f;
        m_PrevPos = GetOwner()->Transform()->GetWorldPos();
        m_PrevPos.y = 0.f;
        m_bCamShake = false;

        // Slash
        Vec3 Dir = PLAYER->Transform()->GetWorldPos() - GetOwner()->Transform()->GetWorldPos();
        Dir.y = 0.f;
        Dir.Normalize();
        GetOwner()->Rigidbody()->SetVelocity(Vec3());
        GetOwner()->Rigidbody()->AddForce(Dir * 150.f, ForceMode::VelocityChange);

        //@Effect ������ ����Ʈ

        // sound
        wstring Crash = L"sound\\wav\\CharaBossChimera2\\0046_SlashCrash.wav";
        wstring Fire;

        int RandIdx = GetRandomInt(0, 2);
        if (RandIdx == 0)
            Fire = L"sound\\wav\\CharaBossChimera2\\0047_SlashFire.wav";
        else if (RandIdx == 1)
            Fire = L"sound\\wav\\CharaBossChimera2\\0048_SlashFire.wav";
        else
            Fire = L"sound\\wav\\CharaBossChimera2\\0049_SlashFire.wav";

        GamePlayStatic::Play2DSound(Crash, 1, SOUND_ELFILIS * 0.5f);
        GamePlayStatic::Play2DSound(Fire, 1, SOUND_ELFILIS * 0.3f);
    }
    break;
    case StateStep::Wait: {
        GetOwner()->Animator()->Play(ANIMPREFIX("SlashEnd"), false);

        // Up
        GetOwner()->Rigidbody()->SetVelocity(Vec3::Zero);
        GetOwner()->Rigidbody()->AddForce(Vec3(0.f, 1000.f, 1.f), ForceMode::Impulse);

        // sound
        wstring MapOver = L"sound\\wav\\CharaBossChimera2\\0050_SlashMapOver.wav";
        GamePlayStatic::Play2DSound(MapOver, 1, SOUND_ELFILIS * 0.2f);
    }
    break;
    }
}

void CElfilisA_SlashCombo::Exit_Step()
{
    switch (m_Step)
    {
    case StateStep::Ready: {
        if (m_ComboLevel == 2)
        {
            ELFFSM->AddComboLevel();
        }
    }
    break;
    case StateStep::Start:
        break;
    case StateStep::Progress:
        break;
    case StateStep::Wait:
        break;
    }
}

void CElfilisA_SlashCombo::Ready()
{
    RotateToPlayer();

    if (GetOwner()->Animator()->IsFinish())
    {
        if (m_ComboLevel < 2)
        {
            ChangeStep(StateStep::Start);
        }
        else if (m_ComboLevel == 2)
        {
            ELFFSM->ChangeStateGroup(ElfilisStateGroup::AirToGround, L"AIR_TOGROUND_STAB");
        }
    }
}

void CElfilisA_SlashCombo::Start()
{
    RotateToPlayer();

    if (GetOwner()->Transform()->GetWorldPos().y <= 100.f)
    {
        ChangeStep(StateStep::Progress);
    }
}

void CElfilisA_SlashCombo::Progress()
{
    Vec3 CurPos = GetOwner()->Transform()->GetWorldPos() - ELFFSM->GetMapFloorOffset();
    CurPos.y = 0.f;

    // Camera Shake
    Vec3 PlayerPos = PLAYER->Transform()->GetWorldPos();
    PlayerPos.y = 0.f;

    if (!m_bCamShake && CurPos.Cross(PlayerPos).Dot(m_PrevPos.Cross(PlayerPos)) < 0.f)
    {
        m_bCamShake = true;
        CAMERACTRL->Shake(0.5f, 20.f, 40.f);
    }

    // Spawn Slash Rock
    if (CurPos.Length() <= ELFFSM->GetMapSizeRadius())
    {
        float SpawnBetween = 5.f;
        m_SpawnDist += (CurPos - m_PrevPos).Length();

        if (m_SpawnDist >= SpawnBetween && m_StabRockPref != nullptr)
        {
            m_SpawnDist -= SpawnBetween;
            CGameObject* pRock = m_StabRockPref->Instantiate();

            Vec3 NewPos = GetOwner()->Transform()->GetWorldPos();
            NewPos.y = 0.f;

            Vec3 NewDir = Vec3(GetRandomfloat(-100.f, 100.f), 0.f, GetRandomfloat(-100.f, 100.f)).Normalize();
            if (NewDir.Length() <= 0.f)
            {
                NewDir = Vec3(1.f, 0.f, 1.f);
            }

            pRock->Transform()->SetWorldPos(NewPos);
            pRock->Transform()->SetDirection(NewDir);

            CMomentaryObjScript* pScript = pRock->GetScript<CMomentaryObjScript>();
            if (pScript)
            {
                pScript->SetPlayTime(5.f);
            }

            GamePlayStatic::SpawnGameObject(pRock, LAYER_DYNAMIC);
        }
    }

    m_PrevPos = CurPos;

    // Change Step
    float ProgressDist = (m_StartPos - GetOwner()->Transform()->GetWorldPos()).Length();
    if (ProgressDist >= ELFFSM->GetAirPos().z * 2.f)
    {
        ChangeStep(StateStep::Wait);
    }
}

void CElfilisA_SlashCombo::Wait()
{
    if (GetOwner()->Transform()->GetWorldPos().y >= ELFFSM->GetAirPos().y)
    {
        GetOwner()->Rigidbody()->SetVelocity(Vec3());
        m_ComboLevel++;

        ChangeStep(StateStep::Ready);
    }
}
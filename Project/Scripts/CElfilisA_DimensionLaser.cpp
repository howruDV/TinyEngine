#include "pch.h"
#include "CElfilisA_DimensionLaser.h"
#include "CElfilisFSM.h"
#include "CElfilisDimensionLaser.h"
#include <Engine\CAssetMgr.h>
#include <Engine\CPrefab.h>

#include "CCameraController.h"

CElfilisA_DimensionLaser::CElfilisA_DimensionLaser()
    : m_Dimension{nullptr,}
    , m_DimensionStart(nullptr)
    , m_DimensionScript{nullptr,}
    , m_ProgressStep(0)
    , m_AccTime(0.f)
{
    m_DimensionStartPref = CAssetMgr::GetInst()->Load<CPrefab>(L"prefab\\ElfilisDimensionLaserStart.pref");
    m_DimensionPref = CAssetMgr::GetInst()->Load<CPrefab>(L"prefab\\ElfilisDimensionLaser.pref");
}

CElfilisA_DimensionLaser::~CElfilisA_DimensionLaser()
{
}

void CElfilisA_DimensionLaser::tick()
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
    case StateStep::End: {
        End();
    }
    break;
    }
}

void CElfilisA_DimensionLaser::Enter()
{
    m_Step = StateStep::Ready;
    Enter_Step();
}

void CElfilisA_DimensionLaser::Exit()
{
    Exit_Step();

    for (int i = 0; i < 5; ++i)
    {
        if (m_Dimension[i])
        {
            GamePlayStatic::DestroyGameObject(m_Dimension[i]);
            m_Dimension[i] = nullptr;
            m_DimensionScript[i] = nullptr;
        }
    }

    if (m_DimensionStart)
    {
        GamePlayStatic::DestroyGameObject(m_DimensionStart->GetOwner());
        m_DimensionStart = nullptr;
    }
}

void CElfilisA_DimensionLaser::Enter_Step()
{
    switch (m_Step)
    {
    case StateStep::Ready: {
        GetOwner()->Animator()->Play(ANIMPREFIX("DimensionLaserReady"), false);
        //@Effect 차징 파티클

        // Camera : 땅 뷰
        CAMERACTRL->SetElfilisGround();

        // sound
        wstring Sound = L"sound\\wav\\CharaBossChimeraSoul\\0012_DimensionLaserCharging.wav";
        GamePlayStatic::Play2DSound(Sound, 1, SOUND_ELFILIS);
    }
    break;
    case StateStep::Start: {
        GetOwner()->Animator()->Play(ANIMPREFIX("DimensionLaserStart"), false, false, 2.5f, 0.3f);

        // sound
        wstring Sound = L"sound\\wav\\CharaBossChimera2\\0031_DimensionLaserChargeFinish.wav";
        GamePlayStatic::Play2DSound(Sound, 1, SOUND_ELFILIS);
    }
    break;
    case StateStep::Progress: {
        GetOwner()->Animator()->Play(ANIMPREFIX("DimensionLaser"));
        m_ProgressStep = 0;
        m_AccTime = 0.f;

        // Laser Spawn
        if (m_DimensionStartPref != nullptr)
        {
            Vec3 NewRot = GetOwner()->Transform()->GetWorldRotation();
            NewRot.x += XMConvertToRadians(50.f);

            CGameObject* DimensionStart = m_DimensionStartPref->Instantiate();
            m_DimensionStart = DimensionStart->GetScript<CElfilisDimensionLaser>();

            DimensionStart->Transform()->SetWorldPos(GetOwner()->Transform()->GetWorldPos());
            DimensionStart->Transform()->SetWorldRotation(NewRot);
            m_DimensionStart->SetLaserStartLcok(true);
            m_DimensionStart->PlaySpawn();
            GamePlayStatic::SpawnGameObject(DimensionStart, LAYER_MONSTERATK_TRIGGER);
        }
    }
    break;
    case StateStep::End: {
        GetOwner()->Animator()->Play(ANIMPREFIX("DimensionLaserEnd"), false);
    }
    break;
    }
}

void CElfilisA_DimensionLaser::Exit_Step()
{
    switch (m_Step)
    {
    case StateStep::Ready:
        break;
    case StateStep::Start:
        break;
    case StateStep::Progress:
        break;
    case StateStep::End: {
        // Camera : 투 타겟
        CAMERACTRL->SetElfilisTwoTarget();
    }
    break;
    }
}

void CElfilisA_DimensionLaser::Ready()
{
    if (GetOwner()->Animator()->IsFinish())
    {
        ChangeStep(StateStep::Start);
    }
}

void CElfilisA_DimensionLaser::Start()
{
    if (GetOwner()->Animator()->IsFinish())
    {
        ChangeStep(StateStep::Progress);
    }
}

void CElfilisA_DimensionLaser::Progress()
{
    m_AccTime += DT;

    switch (m_ProgressStep)
    {
    case 0: {
        // 중심 게이트 생성이 끝났다면 : 레이저 발사 & idx 0 게이트 오픈
        if (m_DimensionStart->IsSpawnFinish())
        {
            m_DimensionStart->PlayStartLaser();
            m_DimensionStart->SetLaserRepeat(true);
            SpawnDimension(0);

            m_ProgressStep = 1;
        }
    }
    break;
    case 1: {
        // 게이트 순차 생성 : 이전 게이트에서 레이저 발사가 시작됐다면 다음 게이트 오픈
        for (int i = 1; i < 5; i++)
        {
            if (!m_Dimension[i] && m_Dimension[i - 1] && m_DimensionScript[i - 1]->IsLaserStart())
            {
                SpawnDimension(i);
            }
        }

        // 게이트 생성 종료 : 마지막 게이트까지 레이저 발사가 진행됐다면 중앙 게이트 레이저 종료
        if (m_Dimension[4] && m_DimensionScript[4]->IsLaserEnd())
        {
            m_DimensionStart->PlayEndLaser();
            m_ProgressStep = 2;

            Vec3 CenterPos = m_DimensionScript[4]->Transform()->GetWorldPos();
            Vec3 RightDir = m_DimensionScript[4]->Transform()->GetWorldDir(DIR_TYPE::RIGHT);

            for (int i = 0; i < 5; ++i)
            {
                ELFFSM->SpawnDropStar(CenterPos + RightDir * (-400.f + 150.f * i));
            }
        }
    }
    break;
    case 2: {
        // Change State
        // 중앙 게이트가 닫혔다면 : 게이트들 레벨에서 빼내고 스텝 변환
        if (m_DimensionStart->IsDisappearFinish())
        {
            GamePlayStatic::DestroyGameObject(m_DimensionStart->GetOwner());
            for (int i = 0; i < 5; ++i)
            {
                if (m_Dimension[i])
                {
                    GamePlayStatic::DestroyGameObject(m_Dimension[i]);
                    m_Dimension[i] = nullptr;
                    m_DimensionScript[i] = nullptr;
                }
            }

            ChangeStep(StateStep::End);
        }
    }
    break;
    }
}

void CElfilisA_DimensionLaser::End()
{
    if (GetOwner()->Animator()->IsFinish())
    {
        ELFFSM->ChangeStateGroup(ElfilisStateGroup::AirToGround, L"AIR_TOGROUND_TELEPORT");
    }
}

void CElfilisA_DimensionLaser::SpawnDimension(int _Idx)
{
    m_Dimension[_Idx] = m_DimensionPref->Instantiate();
    CElfilisDimensionLaser* Script = m_Dimension[_Idx]->GetScript<CElfilisDimensionLaser>();
    m_DimensionScript[_Idx] = Script;

    Vec3 InitDir = GetOwner()->Transform()->GetWorldDir(DIR_TYPE::FRONT);
    InitDir.y = 0.f;
    InitDir.Normalize();

    Vec3 InitPos = Vec3();
    InitPos += InitDir * -150.f * (2.f - _Idx);

    m_DimensionScript[_Idx]->PlaySpawn();
    m_DimensionScript[_Idx]->Transform()->Slerp(InitDir, 1.f);
    m_DimensionScript[_Idx]->Transform()->SetWorldPos(InitPos);
    GamePlayStatic::SpawnGameObject(m_Dimension[_Idx], LAYER_MONSTERATK_TRIGGER);

    // sound
    wstring Sound = L"sound\\wav\\CharaBossChimeraSoul\\0013_DimensionLaser_LaserStrat.wav";
    GamePlayStatic::Play2DSound(Sound, 1, SOUND_ELFILIS * 0.6f, true, false);
}

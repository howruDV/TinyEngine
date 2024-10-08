#include "pch.h"
#include "CMorphoDemo_Appear.h"
#include "CMorphoFSM.h"
#include "CChangeAlphaScript.h"
#include "CBossLevelFlowMgr.h"
#include "CCameraController.h"

CMorphoDemo_Appear::CMorphoDemo_Appear()
    : m_StartPos(Vec3(0.f, 0.f, -100.f))
    , m_DownSpeed(80.f)
    , m_AccTime(0.f)
    , m_BossName(nullptr)
    , m_bFrmEnter(false)
{
    m_BossNamePref = CAssetMgr::GetInst()->Load<CPrefab>(L"prefab\\BossName_Morpho.pref", L"prefab\\BossName_Morpho.pref");
}

CMorphoDemo_Appear::~CMorphoDemo_Appear()
{
}

void CMorphoDemo_Appear::tick()
{
    switch (m_Step)
    {
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

void CMorphoDemo_Appear::Exit()
{
    Exit_Step();

    if (m_BossName)
    {
        GamePlayStatic::DestroyGameObject(m_BossName);
    }

    CBossMgr::GetBossFlowMgr()->ChangeFlow(BossLevelFlow::Fight);
}

void CMorphoDemo_Appear::Enter_Step()
{
    switch (m_Step)
    {
    case StateStep::Start: {
        GetOwner()->Animator()->Play(ANIMPREFIX("DemoMorphoKnightAppearCut4"), false, false, 1.5f, 0.f);
        GetOwner()->Animator()->SetClipFrameIndex(400);
        GetOwner()->Transform()->SetWorldPos(m_StartPos);
        GetOwner()->Transform()->SetWorldRotation(Vec3());

        m_AccTime = 0.f;
        m_bFrmEnter = true;

        // sound
        wstring Sound = L"sound\\wav\\DemoBossMorphoKnight\\0009_Appear.wav";
        GamePlayStatic::Play2DSound(Sound, 1, SOUND_MORPHO);

        // Camera : 점점 몰포 가까이
        CGameObject* Target = BOSS->GetChildObject(L"CameraTarget");

        CAMERACTRL->SetMainTarget(Target);
        CAMERACTRL->SetOffset(Vec3(0.f, 0.f, 0.f));
        CAMERACTRL->SetTargetOffset(Vec3(0.f, 0.f, 0.f));
        CAMERACTRL->SetLookDir(Vec3(0.f, 0.f, -1.f));
        CAMERACTRL->SetLookDist(60.f);

        CAMERACTRL->ResetCamera();
    }
    break;
    case StateStep::Progress: {
        GetOwner()->Animator()->Play(ANIMPREFIX("DemoMorphoKnightAppearCut5"), false, false, 1.5f);
    }
    break;
    case StateStep::End: {
        GetOwner()->Animator()->Play(ANIMPREFIX("DemoBirthEnd"), false, false, 1.5f);
        m_AccTime = 0.f;

        // Particle On
        MRPFSM->EnableTeleportParticle(true);

        // sound
        wstring Sound = L"sound\\wav\\DemoBossMorphoKnight\\0010_Fly.wav";
        GamePlayStatic::Play2DSound(Sound, 1, SOUND_MORPHO * 0.7f);

        // Camera : 투타겟
        CAMERACTRL->LoadInitSetting(true);
        CAMERACTRL->SetMorphoTwoTarget();
    }
    break;
    }
}

void CMorphoDemo_Appear::Exit_Step()
{
    switch (m_Step)
    {
    case StateStep::Start: {
        // Particle Off
        MRPFSM->GetParticleButterflyPink()->ParticleSystem()->EnableModule(PARTICLE_MODULE::SPAWN, false);
        MRPFSM->GetParticleButterflyYellowPink()->ParticleSystem()->EnableModule(PARTICLE_MODULE::SPAWN, false);
        MRPFSM->GetParticleCircleDust()->ParticleSystem()->EnableModule(PARTICLE_MODULE::SPAWN, false);
    }
    break;
    case StateStep::Progress: {
        if (m_BossName)
        {
            CChangeAlphaScript* Script = m_BossName->GetScript<CChangeAlphaScript>();
            Script->FadeOutDestroy(0.3f);
            m_BossName = nullptr;
        }
    }
    break;
    case StateStep::End: {
        // Particle Off
        MRPFSM->EnableTeleportParticle(false);
    }
    break;
    }
}

void CMorphoDemo_Appear::Start()
{
    if (m_bFrmEnter && CHECK_ANIMFRM(GetOwner(), 603))
    {
        // spawn BossName
        if (m_BossNamePref != nullptr)
        {
            m_BossName = m_BossNamePref->Instantiate();
            CChangeAlphaScript* Script = m_BossName->GetScript<CChangeAlphaScript>();

            m_BossName->Transform()->SetWorldPos(Vec3(0.f, 155.f, -230.f));
            m_BossName->Transform()->SetWorldRotation(Vec3());
            m_BossName->Transform()->SetWorldScale(Vec3(5.f));
            Script->FadeIn(0.5f);

            GamePlayStatic::SpawnGameObject(m_BossName, LAYER_STATIC);
        }

        // Particle On
        MRPFSM->GetParticleButterflyPink()->ParticleSystem()->EnableModule(PARTICLE_MODULE::SPAWN, true);
        MRPFSM->GetParticleButterflyYellowPink()->ParticleSystem()->EnableModule(PARTICLE_MODULE::SPAWN, true);
        MRPFSM->GetParticleCircleDust()->ParticleSystem()->EnableModule(PARTICLE_MODULE::SPAWN, true);

        // sound
        wstring Sound = L"sound\\wav\\DemoBossBasic\\0000_AppearName.wav";
        GamePlayStatic::Play2DSound(Sound, 1, SOUND_MORPHO);

        // Camera : 뒤로 이동(고정)
        CAMERACTRL->SetLookDir(Vec3(0.f, 0.35f, -0.937f));
        CAMERACTRL->SetLookDist(100.f);

        CAMERACTRL->SetZoomMinSpeed(300.f);
        CAMERACTRL->SetZoomMaxSpeed(500.f);
        CAMERACTRL->SetZoomThreshold(500.f);
        CAMERACTRL->SetRotationSpeed(150.f);

        m_bFrmEnter = false;
    }

    if (GetOwner()->Animator()->IsFinish())
    {
        ChangeStep(StateStep::Progress);
    }
}

void CMorphoDemo_Appear::Progress()
{
    if (GetOwner()->Animator()->IsFinish())
    {
        ChangeStep(StateStep::End);
    }
}

void CMorphoDemo_Appear::End()
{
    if (GetOwner()->Animator()->IsFinish())
    {
        MRPFSM->ChangeStateGroup(MorphoStateGroup::AtkGroundWait);
    }
}
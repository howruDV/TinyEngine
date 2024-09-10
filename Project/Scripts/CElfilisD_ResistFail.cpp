#include "pch.h"
#include "CElfilisD_ResistFail.h"
#include "CElfilisFSM.h"
#include "CCameraController.h"
#include "CBossLevelFlowMgr.h"
#include "CStageClear.h"

CElfilisD_ResistFail::CElfilisD_ResistFail()
    : m_AccTime(0.f)
    , m_bFrmEnter(true)
{
}

CElfilisD_ResistFail::~CElfilisD_ResistFail()
{
}

void CElfilisD_ResistFail::Exit()
{
    Exit_Step();
    ELFFSM->SetResist(false);
}

void CElfilisD_ResistFail::tick()
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
    }
}

void CElfilisD_ResistFail::Enter_Step()
{
    switch (m_Step)
    {
    case StateStep::Start: {
        GetOwner()->Animator()->Play(ANIMPREFIX("ResistFail"), false);
        m_AccTime = 0.f;

        ELFFSM->SetResist(true);
        ELFFSM->SetStateLock(true);

        // Sound
        // Sound
        wstring Resist = L"sound\\wav\\CharaBossChimeraSoul\\0002_Death.wav";
        GamePlayStatic::Play2DSound(Resist, 1, SOUND_ELFILIS);
        CBossMgr::GetBossFlowMgr()->ChangeBGMSize(0.f, 1.f);

        // TimeScale & Camera ��� ����
        CTimeMgr::GetInst()->SetTimeScale(0.5f, 0.f);
        CAMERACTRL->Shake(0.3f, 30.f, 30.f);
        CAMERACTRL->SetLock(true, 0.5f);
    }
    break;
    case StateStep::Progress: {
        m_AccTime = 0.f;
        m_bFrmEnter = true;

        ELFFSM->DestroySumon();

        // Flow Mgr
        CBossMgr::GetBossFlowMgr()->ChangeFlow(BossLevelFlow::Death);

        // Sound
        wstring Resist = L"sound\\wav\\CharaBossChimera2\\0069_Death.wav";
        GamePlayStatic::Play2DSound(Resist, 1, SOUND_ELFILIS);

        // CAMERA : ���Ǹ��� Ÿ��, �ó׸���
        CAMERACTRL->SetMainTarget(BOSS);
        CAMERACTRL->SetOffset(Vec3(0.f, 0.f, 0.f));
        CAMERACTRL->SetTargetOffset(Vec3(0.f, 0.f, 0.f));
        CAMERACTRL->SetSubTargetOffset(Vec3(0.f, 0.f, 0.f));

        Vec3 BossDirFront = BOSS->Transform()->GetWorldDir(DIR_TYPE::FRONT);
        Vec3 BossDirRight = BOSS->Transform()->GetWorldDir(DIR_TYPE::RIGHT);
        Vec3 CamDir = BossDirFront + BossDirRight;
        CamDir.Normalize();
        CamDir.y = tanf(40.f * XM_PI / 180.f);
        CamDir.Normalize();

        CAMERACTRL->SetLookDir(-CamDir);
        CAMERACTRL->SetLookDist(400.f);
        CAMERACTRL->Normal(true);
    }
    break;
    }
}

void CElfilisD_ResistFail::Exit_Step()
{
    switch (m_Step)
    {
    case StateStep::Start:
        break;
    case StateStep::Progress: {
        ELFFSM->ResetEmissive();

        // Camera : Ŀ��Ÿ��
        CAMERACTRL->SetMainTarget(PLAYER);
        CAMERACTRL->LoadInitSetting(true);
        CAMERACTRL->ResetCamera();
    }
    break;
    }
}

void CElfilisD_ResistFail::Start()
{
    m_AccTime += DT;

    if (m_AccTime > 0.5f)
    {
        ChangeStep(StateStep::Progress);
    }
}

void CElfilisD_ResistFail::Progress()
{
    m_AccTime += DT;

    // emissive
    float ProgressTime = 2.f;
    float t1 = m_AccTime / ProgressTime;
    float t2 = sinf(t1 * XM_PI / 2.f) * 2.f;
    Vec3 Color = Vec3(t2, t2, t1);

    ELFFSM->AddEmissive(Color);

    if (m_AccTime > 3.f)
    {
        if (m_bFrmEnter)
        {
            //@EFFECT ������ ��ƼŬ

            // Sound
            wstring EndBGM = L"sound\\stream\\K15_ChimeraSoulAfter1\\K15_ChimeraSoulAfter1.marker.dspadpcm.wav";
            GamePlayStatic::PlayBGM(EndBGM, SOUND_BGM);

            m_bFrmEnter = false;
        }

        ELFFSM->SetStateLock(false);
        CBossMgr::GetBossFlowMgr()->ChangeFlow(BossLevelFlow::Clear);
    }
}
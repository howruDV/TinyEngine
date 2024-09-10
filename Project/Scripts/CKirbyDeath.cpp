#include "pch.h"
#include "CKirbyDeath.h"

#include "CCameraController.h"

#include "CLevelFlowMgr.h"

CKirbyDeath::CKirbyDeath()
{
}

CKirbyDeath::~CKirbyDeath()
{
}

void CKirbyDeath::tick()
{
    m_Acc += DT_ENGINE;

    if (m_Acc != 0.f)
    {
        PLAYER->Animator()->SetPlay(false);
    }

    // ���ߴ� �ð��� �������
    if (m_Acc > m_Duration)
    {
        // �ִϸ��̼� ���
        PLAYER->Animator()->SetPlay(true);
    }

    // ���ߴ� �ð��� �������
    if (m_Acc > m_Duration && m_bSoundPlay == false)
    {
        // sound
        CSoundMgr::GetInst()->SetSoundLock(false);
        GamePlayStatic::Play2DSound(L"sound\\stream\\K15_Dead1\\K15_Dead1.marker.dspadpcm.wav", 1, KIRBY_EFFECTSOUND * 4.f);
        CSoundMgr::GetInst()->SetSoundLock(true);
        m_bSoundPlay = true;
    }

    if (m_Acc > m_FaceDuraion)
    {
        CPlayerMgr::SetPlayerFace(FaceType::Close);

        // Layer Off
        CAMERACTRL->TurnOffMonsterLayer();

        // Ŀ��� UI�� ������ ����� ���������� Fade Out
        if (m_bFadeEffect == false)
        {
            CLevelFlowMgr* FlowMgr = CLevelMgr::GetInst()->GetCurrentLevel()->FindObjectByName(L"Manager")->GetScript<CLevelFlowMgr>();
            FlowMgr->OnDimensionFade(1.f, 0.f, 2.5f);

            m_bFadeEffect = true;
        }
    }

    if (m_Acc > m_DeathDuraion)
    {
        CTimeMgr::GetInst()->SetTimeScale(1.f);

        // Level Restart
        CLevelFlowMgr* FlowMgr = CLevelMgr::GetInst()->GetCurrentLevel()->FindObjectByName(L"Manager")->GetScript<CLevelFlowMgr>();
        FlowMgr->LevelRestart();
    }
}

void CKirbyDeath::Enter()
{
    // Effect Lock
    CAMERACTRL->SetEffectLock(true);

    PLAYERFSM->SetInvincible(true);
    PLAYERFSM->SetEmissive(false, 0.f);

    m_Acc = 0.f;
    m_Duration = 1.f;
    m_FaceDuraion = 3.f;
    m_DeathDuraion = 6.f;
    m_bFadeEffect = false;
    m_bSoundPlay = false;

    // FlowMgr���� ƽ�� �ʱ� ������ ùƽ���� ȿ���� ������ ���� ������ ����Ǿ��ִ� coef���� �������� Fade�� �ȴ�. �׷��� coef�� �̸� 1�� �ʱ�ȭ
    CLevelFlowMgr* FlowMgr = CLevelMgr::GetInst()->GetCurrentLevel()->FindObjectByName(L"Manager")->GetScript<CLevelFlowMgr>();
    FlowMgr->OnDimensionFade(1.f);
    FlowMgr->OffDimensionFade();

    GamePlayStatic::StopAllSound();
    CSoundMgr::GetInst()->SetSoundLock(true);

    // m_Duration ��ŭ �ð��� ���߱�
    CTimeMgr::GetInst()->SetTimeScale(0.f, m_Duration);

    // Ŀ�� ǥ��
    CPlayerMgr::SetPlayerFace(FaceType::Frown);

    // CameraSetting
    CCameraController* CamCtrl = CAMERACTRL;

    CamCtrl->Normal(false);
    CamCtrl->SetLock(true, m_FaceDuraion);
    CamCtrl->LockSetup(true);

    CamCtrl->SetOffset(Vec3(0.f, 10.f, 0.f));
    CamCtrl->SetTargetOffset(Vec3(0.f, 0.f, 0.f));
    CamCtrl->RotationLookDirRightAxis(50.f);

    CamCtrl->SetLookDist(100.f);
    CamCtrl->SetZoomMinSpeed(50.f);
    CamCtrl->SetZoomMaxSpeed(100.f);
    CamCtrl->SetZoomThreshold(500.f);

    // Camera Shake
    CamCtrl->Shake(0.5f, 50.f, 50.f);

    Vec3 CamPos = CamCtrl->GetOwner()->Transform()->GetWorldPos();

    // Ŀ�� ���� ����
    Vec3 PlayerPos = PLAYER->Transform()->GetWorldPos();
    Vec3 Dir = CamPos - PlayerPos;
    Dir.y = 0.f;
    Dir.Normalize();

    ForceDirInfo DirInfo = {ForceDirType::STAGEEVENT, Dir, true};
    PLAYERCTRL->ForceDir(DirInfo);
    PLAYERFSM->SetStateLock(true);
    GetOwner()->Animator()->Play(ANIMPREFIX("Death"), false);

    PLAYERCTRL->LockInput();
    PLAYERCTRL->LockMove();
    PLAYERCTRL->LockJump();

    // UI ����
    {
        CLevelFlowMgr* FlowMgr = CLevelMgr::GetInst()->GetCurrentLevel()->FindObjectByName(L"Manager")->GetScript<CLevelFlowMgr>();
        FlowMgr->TurnOffBossHP();
        FlowMgr->TurnOffPlayerHP();
        FlowMgr->ActiveOffDropUI();
    }
}

void CKirbyDeath::Exit()
{
    PLAYERCTRL->UnlockDirection();
    PLAYERCTRL->UnlockMove();
    PLAYERCTRL->UnlockJump();
}
#include "pch.h"
#include "CKirbyChangeAbility.h"

#include "CCameraController.h"

#include "CLevelFlowMgr.h"

CKirbyChangeAbility::CKirbyChangeAbility()
{
}

CKirbyChangeAbility::~CKirbyChangeAbility()
{
}

void CKirbyChangeAbility::tick()
{
    if (!m_bFrameEnter && CHECK_ANIMFRM(GetOwner(), 0))
    {
        PLAYER->Animator()->SetPlay(false);
        m_bFrameEnter = true;
    }
    else
    {
        PLAYER->Animator()->SetPlay(true);
    }

    if (!m_StarEffect && CHECK_ANIMFRM(GetOwner(), 19))
    {
        GamePlayStatic::Play2DSound(L"sound\\wav\\HeroBasic\\Skrr.wav", 1, KIRBY_EFFECTSOUND * 1.5f);

        CGameObject* pStarEffect =
            CAssetMgr::GetInst()
                ->Load<CPrefab>(L"prefab\\Effect_KirbyChangeAbilityStarSpawn.pref", L"prefab\\Effect_KirbyChangeAbilityStarSpawn.pref")
                ->Instantiate();

        GamePlayStatic::SpawnGameObject(pStarEffect, pStarEffect->GetLayerIdx());

        m_StarEffect = true;
    }

    // �ִϸ��̼� ����� �ʿ��� �۾�
    PLAYERFSM->GetCurAbility()->ChangeAbility();

    if (PLAYER->Animator()->IsFinish())
    {
        switch (PLAYERFSM->GetCurAbilityIdx())
        {
        case AbilityCopyType::FIRE:
        case AbilityCopyType::NORMAL:
        case AbilityCopyType::CUTTER:
        case AbilityCopyType::SWORD: {
            if (PLAYER->Animator()->IsFinish())
            {
                PLAYERFSM->SetStateLock(false);
                ChangeState(L"IDLE");
            }
        }
        break;
        case AbilityCopyType::SLEEP: {
            if (PLAYER->Animator()->IsFinish())
            {
                PLAYERFSM->SetStateLock(false);
                ChangeState(L"IDLE_START");
            }
        }
        break;
        }
    }
}

void CKirbyChangeAbility::Enter()
{
    CGameObject* Smoke = CAssetMgr::GetInst()->Load<CPrefab>(L"prefab\\LandingSmokeSpawner.pref")->Instantiate();
    Smoke->Transform()->SetWorldPos(PLAYER->Transform()->GetWorldPos());
    GamePlayStatic::SpawnGameObject(Smoke, Smoke->GetLayerIdx());

    m_StarEffect = false;

    PLAYERFSM->ChangeNextAbility();

    m_bFrameEnter = false;

    // ���� ���� �� Ŀ�� �̹̽ú� ȿ���� �����ʵ��� �Ѵ�.
    PLAYERFSM->SetSkrr(true);
    PLAYERFSM->SetStateLock(true);
    PLAYERFSM->SetInvincible(true);

    // ���� �Ⱦ��ֱ�
    PLAYERFSM->GetCurAbility()->ChangeAbilityEnter();

    // Ŀ�� ���� �ִϸ��̼� ���
    PLAYER->Animator()->Play(ANIMPREFIX("CopyFirst"), false, false, 1.f);

    // Ŀ�� ���͸��� �ٽ� ����
    CPlayerMgr::ClearBodyMtrl();
    CPlayerMgr::ClearMouthMtrl();
    CPlayerMgr::SetPlayerMtrl(PLAYERMESH(BodyNormal));
    CPlayerMgr::SetPlayerMtrl(PLAYERMESH(MouthSmileOpen));
    CPlayerMgr::SetPlayerFace(FaceType::Normal);

    // MoveController Lock
    PLAYERCTRL->LockInput();
    PLAYERCTRL->LockJump();
    PLAYERCTRL->LockMove();

    // Ŀ�� ������ ��� ������Ʈ�� ���ߵ��� Ÿ�� �������� ����
    CTimeMgr::GetInst()->SetTimeScale(0.f);
    PLAYERCTRL->Animator()->SetAnimatorUpdateMode(AnimatorUpdateMode::UnscaledTime);

    // ��� �� ȿ��
    CGameObject* Manager = CLevelMgr::GetInst()->GetCurrentLevel()->FindObjectByName(L"Manager");
    CLevelFlowMgr* FLowMgrScript = Manager->GetScript<CLevelFlowMgr>();

    if (FLowMgrScript != nullptr)
    {
        FLowMgrScript->OnDimensionFade(0.3f);
    }
}

void CKirbyChangeAbility::Exit()
{
    CTimeMgr::GetInst()->SetTimeScale(1.f);

    // MoveController Lock
    PLAYERCTRL->UnlockInput();
    PLAYERCTRL->UnlockJump();
    PLAYERCTRL->UnlockMove();

    PLAYERFSM->GetCurAbility()->ChangeAbilityExit();

    // Ŀ�� ǥ�� ����
    CPlayerMgr::ClearMouthMtrl();
    CPlayerMgr::SetPlayerMtrl(PLAYERMESH(MouthNormal));

    // CameraSetting
    CCameraController* CamCtrl = CAMERACTRL;
    CamCtrl->LoadSetting();
    CamCtrl->SetCameraSetup((CameraSetup)PLAYERFSM->GetLastCameraSetup());

    // ��� �� ȿ�� ����
    CGameObject* Manager = CLevelMgr::GetInst()->GetCurrentLevel()->FindObjectByName(L"Manager");
    CLevelFlowMgr* FLowMgrScript = Manager->GetScript<CLevelFlowMgr>();

    if (FLowMgrScript != nullptr)
    {
        FLowMgrScript->OffDimensionFade();
    }

    // Ÿ�� ������ ����
    CTimeMgr::GetInst()->SetTimeScale(1.f);
    PLAYERCTRL->Animator()->SetAnimatorUpdateMode(AnimatorUpdateMode::Normal);

    // Emissive�� �ٽ� �޵��� ����
    PLAYERFSM->SetSkrr(false);
    PLAYERFSM->SetInvincible(false);
}

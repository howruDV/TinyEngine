#include "pch.h"
#include "CKirbyChangeAbilityEnd.h"

#include "CCameraController.h"

CKirbyChangeAbilityEnd::CKirbyChangeAbilityEnd()
{
}

CKirbyChangeAbilityEnd::~CKirbyChangeAbilityEnd()
{
}

void CKirbyChangeAbilityEnd::tick()
{
    // State Change

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

void CKirbyChangeAbilityEnd::Enter()
{
    // �ִϸ��̼� ���
    PLAYER->Animator()->Play(ANIMPREFIX("EvolutionCopyEnd"), false, false, 1.5f);
    CPlayerMgr::ClearMouthMtrl();
    CPlayerMgr::SetPlayerMtrl(PLAYERMESH(MouthSmileOpen));

    // MoveController Lock
    PLAYERCTRL->LockInput();
    PLAYERCTRL->LockJump();
    PLAYERCTRL->LockMove();

    PLAYERFSM->SetStateLock(true);
    PLAYERFSM->SetInvincible(true);
}

void CKirbyChangeAbilityEnd::Exit()
{
    // Ŀ�� ǥ�� ����
    CPlayerMgr::ClearMouthMtrl();
    CPlayerMgr::SetPlayerMtrl(PLAYERMESH(MouthNormal));

    // CameraSetting
    CCameraController* CamCtrl = CAMERACTRL;
    CamCtrl->LoadSetting();

    // MoveController Lock
    PLAYERCTRL->UnlockInput();
    PLAYERCTRL->UnlockJump();
    PLAYERCTRL->UnlockMove();

    PLAYERFSM->SetInvincible(false);

    //@TODO
    // ��� �� ȿ�� ����
    // Ŀ�� �ǿ��� ������Ʈ �ٽ� DT�޵��� ����
}

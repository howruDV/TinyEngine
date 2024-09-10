#include "pch.h"
#include "CKirbyChangeAbilityWait.h"

CKirbyChangeAbilityWait::CKirbyChangeAbilityWait()
{
}

CKirbyChangeAbilityWait::~CKirbyChangeAbilityWait()
{
}

void CKirbyChangeAbilityWait::tick()
{
    if (KEY_TAP(KEY_ATK))
    {
        PLAYERFSM->SetStateLock(false);
        ChangeState(L"CHANGE_ABILITY_END");
    }
}

void CKirbyChangeAbilityWait::Enter()
{
    PLAYERFSM->SetStateLock(true);
    PLAYERFSM->SetInvincible(true);

    // �ִϸ��̼� ���
    PLAYER->Animator()->Play(ANIMPREFIX("EvolutionCopyWait"), true, false, 1.5f);
    CPlayerMgr::ClearMouthMtrl();
    CPlayerMgr::SetPlayerMtrl(PLAYERMESH(MouthSmileOpen));
    CPlayerMgr::SetPlayerFace(FaceType::Frown);
}

void CKirbyChangeAbilityWait::Exit()
{
    CPlayerMgr::ClearMouthMtrl();
    CPlayerMgr::SetPlayerMtrl(PLAYERMESH(MouthNormal));
    CPlayerMgr::SetPlayerFace(FaceType::Normal);
    PLAYERFSM->SetInvincible(false);
}

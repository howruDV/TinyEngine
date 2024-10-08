#include "pch.h"
#include "CKirbyAttackCharge1.h"

#include "CKirbyFSM.h"
#include "CPlayerMgr.h"
#include "CKirbyObject.h"
#include "CKirbyAbility.h"

CKirbyAttackCharge1::CKirbyAttackCharge1()
{
}

CKirbyAttackCharge1::~CKirbyAttackCharge1()
{
}

void CKirbyAttackCharge1::tick()
{
    PLAY_CURSTATE(AttackCharge1)

    // State Change
    if (PLAYERFSM->GetCurObjectIdx() != ObjectCopyType::NONE)
    {
    }
    else
    {
        switch (PLAYERFSM->GetCurAbilityIdx())
        {
        case AbilityCopyType::NORMAL: 
        break;
        case AbilityCopyType::FIRE:
        {
            if (KEY_RELEASED(KEY_ATK) || KEY_NONE(KEY_ATK))
            {
                ChangeState(L"ATTACK_CHARGE1_END");
            }
            else if (PLAYERCTRL->GetInput().Length() != 0.f)
            {
                ChangeState(L"ATTACK_CHARGE1_RUN");
            }
        }
            break;
        case AbilityCopyType::CUTTER:
            break;
        case AbilityCopyType::SWORD: {
            if (GetOwner()->Animator()->IsFinish())
            {
                ChangeState(L"ATTACK_CHARGE1_END");
            }
        }
            break;
        }
    }
}

void CKirbyAttackCharge1::Enter()
{
    PLAY_CURSTATE(AttackCharge1Enter)
}

void CKirbyAttackCharge1::Exit()
{
    PLAY_CURSTATE(AttackCharge1Exit)
}

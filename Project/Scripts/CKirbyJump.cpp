#include "pch.h"
#include "CKirbyJump.h"

CKirbyJump::CKirbyJump()
{
}

CKirbyJump::~CKirbyJump()
{
}

void CKirbyJump::tick()
{
    PLAY_CURSTATE(Jump)

    // State Change
    if (PLAYERFSM->GetCurObjectIdx() != ObjectCopyType::NONE)
    {
    }
    else
    {
        switch (PLAYERFSM->GetCurAbilityIdx())
        {
        case AbilityCopyType::NORMAL: 
        {
            if (KEY_TAP(KEY_ATK) || KEY_PRESSED(KEY_ATK))
            {
                ChangeState(L"ATTACK_CHARGE1_START");
            }
            else if (GetOwner()->CharacterController()->IsGrounded())
            {
                ChangeState(L"LANDING");
            }
            else if (GetOwner()->Animator()->IsFinish())
            {
                ChangeState(L"JUMP_END");
            }
        }
        break;
        case AbilityCopyType::FIRE:
            break;
        case AbilityCopyType::RANGER:
            break;
        case AbilityCopyType::SWORD:
            break;
        }
    }
}

void CKirbyJump::Enter()
{
    PLAY_CURSTATE(JumpEnter)
}

void CKirbyJump::Exit()
{
    PLAY_CURSTATE(JumpExit)
}
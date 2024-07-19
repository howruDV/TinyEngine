#include "pch.h"
#include "CKirbyAttackCharge2Slash.h"
#define PLAYTIME 1.5f

CKirbyAttackCharge2Slash::CKirbyAttackCharge2Slash()
    : m_PrevSpeed(0.f)
    , m_PlayTime(PLAYTIME)
{
}

CKirbyAttackCharge2Slash::~CKirbyAttackCharge2Slash()
{
}

void CKirbyAttackCharge2Slash::tick()
{
    m_PlayTime -= DT;

    if (m_PlayTime < 0.f)
    {
        ChangeState(L"ATTACK_CHARGE2_SLASH_END");
    }
}

void CKirbyAttackCharge2Slash::Enter()
{
    PLAYER->Animator()->Play(ANIMPREFIX("SuperSpinSlashLoop"), true, false, 1.5f);

    m_PrevSpeed = PLAYERCTRL->GetSpeed();
    PLAYERCTRL->SetSpeed(13.f);
    PLAYERCTRL->LockDirection();
    PLAYERCTRL->LockJump();

    m_PlayTime = PLAYTIME;
}

void CKirbyAttackCharge2Slash::Exit()
{
    PLAYERCTRL->SetSpeed(m_PrevSpeed);
    PLAYERCTRL->UnlockDirection();
    PLAYERCTRL->UnlockJump();
}

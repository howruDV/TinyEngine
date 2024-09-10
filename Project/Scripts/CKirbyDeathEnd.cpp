#include "pch.h"
#include "CKirbyDeathEnd.h"

CKirbyDeathEnd::CKirbyDeathEnd()
{
}

CKirbyDeathEnd::~CKirbyDeathEnd()
{
}

void CKirbyDeathEnd::tick()
{
}

void CKirbyDeathEnd::Enter()
{
    PLAYERFSM->SetStateLock(true);
    GetOwner()->Animator()->Play(ANIMPREFIX("DeathWait"));

    PLAYERCTRL->LockDirection();
    PLAYERCTRL->LockMove();
    PLAYERCTRL->LockJump();
}

void CKirbyDeathEnd::Exit()
{
    PLAYERCTRL->UnlockDirection();
    PLAYERCTRL->UnlockMove();
    PLAYERCTRL->UnlockJump();
}
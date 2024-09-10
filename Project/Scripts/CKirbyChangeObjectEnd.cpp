#include "pch.h"
#include "CKirbyChangeObjectEnd.h"

#include "CLevelFlowMgr.h"

CKirbyChangeObjectEnd::CKirbyChangeObjectEnd()
{
}

CKirbyChangeObjectEnd::~CKirbyChangeObjectEnd()
{
}

void CKirbyChangeObjectEnd::tick()
{
    PLAYERFSM->GetCurObject()->ChangeObjectEnd();

    // State Change
    switch (PLAYERFSM->GetCurObjectIdx())
    {
    case ObjectCopyType::CONE:
    case ObjectCopyType::VENDING_MACHINE:
    case ObjectCopyType::LIGHT: {
        if (PLAYER->Animator()->IsFinish())
        {
            PLAYERFSM->SetStateLock(false);
            ChangeState(L"IDLE");
        }
    }
    break;
    }
}

void CKirbyChangeObjectEnd::Enter()
{
    PLAYERFSM->SetStateLock(true);
    PLAYERFSM->GetCurObject()->ChangeObjectEndEnter();

    // ���� ���� �� Ŀ�� �̹̽ú� ȿ���� �����ʵ��� �Ѵ�.
    PLAYERFSM->SetSkrr(true);
    PLAYERFSM->SetInvincible(true);
}

void CKirbyChangeObjectEnd::Exit()
{
    PLAYERFSM->GetCurObject()->ChangeObjectEndExit();

    // Emissive�� �ٽ� �޵��� ����
    PLAYERFSM->SetSkrr(false);
    PLAYERFSM->SetInvincible(false);
}
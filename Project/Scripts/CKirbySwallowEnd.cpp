#include "pch.h"
#include "CKirbySwallowEnd.h"

#include "CCameraController.h"
#include "CLevelFlowMgr.h"

CKirbySwallowEnd::CKirbySwallowEnd()
{
}

CKirbySwallowEnd::~CKirbySwallowEnd()
{
}

void CKirbySwallowEnd::tick()
{
    if (PLAYER->Animator()->IsFinish())
    {
        PLAYERFSM->SetStateLock(false);
        ChangeState(L"CHANGE_ABILITY");
    }

}

void CKirbySwallowEnd::Enter()
{
    // ���� ���� �� Ŀ�� �̹̽ú� ȿ���� �����ʵ��� �Ѵ�.
    PLAYERFSM->SetSkrr(true);
    PLAYERFSM->SetStateLock(true);
    PLAYERFSM->SetInvincible(true);

    // Ŀ�� SwallowEnd �ִϸ��̼� ���
    PLAYER->Animator()->Play(ANIMPREFIX("SwallowEnd"), false, false, 2.f);

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

void CKirbySwallowEnd::Exit()
{
    PLAYERFSM->SetSkrr(false);
    PLAYERFSM->SetInvincible(false);

    // MoveController Lock
    PLAYERCTRL->UnlockInput();
    PLAYERCTRL->UnlockJump();
    PLAYERCTRL->UnlockMove();

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

}

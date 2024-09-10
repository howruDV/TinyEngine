#include "pch.h"
#include "CKirbySwallowStart.h"

#include "CCameraController.h"
#include "CLevelFlowMgr.h"

CKirbySwallowStart::CKirbySwallowStart()
{
}

CKirbySwallowStart::~CKirbySwallowStart()
{
}

void CKirbySwallowStart::tick()
{
    if (PLAYER->Animator()->IsFinish())
    {
        PLAYERFSM->SetStateLock(false);
        ChangeState(L"SWALLOW_END");
    }
}

void CKirbySwallowStart::Enter()
{
    // ���� ���� �� Ŀ�� �̹̽ú� ȿ���� �����ʵ��� �Ѵ�.
    PLAYERFSM->SetSkrr(true);
    PLAYERFSM->SetStateLock(true);
    PLAYERFSM->SetInvincible(true);

    // Ŀ�� SwallowStart �ִϸ��̼� ���
    PLAYER->Animator()->Play(ANIMPREFIX("SwallowStart"), false, false, 2.f);

    // MoveController Lock
    PLAYERCTRL->LockInput();
    PLAYERCTRL->LockJump();
    PLAYERCTRL->LockMove();

    // Ŀ�� ������ ��� ������Ʈ�� ���ߵ��� Ÿ�� �������� ����
    CTimeMgr::GetInst()->SetTimeScale(0.f);
    PLAYERCTRL->Animator()->SetAnimatorUpdateMode(AnimatorUpdateMode::UnscaledTime);

    // CameraCtrl
    CCameraController* CamCtrl = CAMERACTRL;
    CamCtrl->SaveSetting();

    Vec3 CamPos = CamCtrl->GetOwner()->Transform()->GetWorldPos();

    // Ŀ�� ���� ����
    Vec3 PlayerPos = PLAYER->Transform()->GetWorldPos();
    Vec3 Dir = CamPos - PlayerPos;
    Dir.y = 0.f;
    Dir.Normalize();

    ForceDirInfo DirInfo = {ForceDirType::STAGEEVENT, Dir, false};
    PLAYERCTRL->ForceDir(DirInfo);

    // CameraSetting
    Vec3 DirToKirby = -Dir;
    DirToKirby.y = -0.577f;
    DirToKirby.Normalize();

    CamCtrl->SetOffset(Vec3(0.f, 15.f, 0));
    CamCtrl->SetTargetOffset(Vec3(0.f, 0.f, 0.f));
    CamCtrl->SetLookDir(DirToKirby);
    CamCtrl->SetLookDist(250.f);

    CamCtrl->SetZoomMinSpeed(100.f);
    CamCtrl->SetZoomMaxSpeed(500.f);
    CamCtrl->SetZoomThreshold(100.f);

    // Normal Setup���� ����
    PLAYERFSM->SaveLastCameraSetup((UINT)CamCtrl->GetCameraSetup());
    CamCtrl->SetCameraSetup(CameraSetup::NORMAL);

    // ��� �� ȿ��
    CGameObject* Manager = CLevelMgr::GetInst()->GetCurrentLevel()->FindObjectByName(L"Manager");
    CLevelFlowMgr* FLowMgrScript = Manager->GetScript<CLevelFlowMgr>();

    if (FLowMgrScript != nullptr)
    {
        FLowMgrScript->OnDimensionFade(0.3f);
    }

}

void CKirbySwallowStart::Exit()
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

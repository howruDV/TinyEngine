#include "pch.h"
#include "CKirbyChangeObject.h"

#include "CCameraController.h"
#include "CLevelFlowMgr.h"

CKirbyChangeObject::CKirbyChangeObject()
{
}

CKirbyChangeObject::~CKirbyChangeObject()
{
}

void CKirbyChangeObject::tick()
{
    PLAYERFSM->GetNextObject()->ChangeObject();

    // State Change
    switch (PLAYERFSM->GetNextObjectIdx())
    {
    case ObjectCopyType::CONE:
    case ObjectCopyType::VENDING_MACHINE:
    case ObjectCopyType::LIGHT: {
        if (PLAYER->Animator()->IsFinish())
        {
            PLAYERFSM->SetStateLock(false);
            ChangeState(L"CHANGE_OBJECT_END");
        }
    }
    break;
    }
}

void CKirbyChangeObject::Enter()
{
    GamePlayStatic::Play2DSound(L"sound\\wav\\HeroBasic\\0000.wav", 1, KIRBY_EFFECTSOUND);

    PLAYERFSM->SetStateLock(true);
    PLAYERFSM->GetNextObject()->ChangeObjectEnter();

    // ���� ���� �� Ŀ�� �̹̽ú� ȿ���� �����ʵ��� �Ѵ�.
    PLAYERFSM->SetSkrr(true);
    PLAYERFSM->SetInvincible(true);

    // CameraSetting
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

    // Normal Setup���� ����
    m_SaveSetup = (UINT)CamCtrl->GetCameraSetup();
    CamCtrl->SetCameraSetup(CameraSetup::NORMAL);

    // ��� �� ȿ��
    CGameObject* Manager = CLevelMgr::GetInst()->GetCurrentLevel()->FindObjectByName(L"Manager");
    CLevelFlowMgr* FLowMgrScript = Manager->GetScript<CLevelFlowMgr>();

    if (FLowMgrScript != nullptr)
    {
        FLowMgrScript->OnDimensionFade(0.3f);
    }

    // Ŀ�� ������ ��� ������Ʈ�� ���ߵ��� Ÿ�� �������� ����
    CTimeMgr::GetInst()->SetTimeScale(0.f);
    PLAYERCTRL->Animator()->SetAnimatorUpdateMode(AnimatorUpdateMode::UnscaledTime);
}

void CKirbyChangeObject::Exit()
{
    PLAYERFSM->GetNextObject()->ChangeObjectExit();

    // CameraSetting
    CCameraController* CamCtrl = CAMERACTRL;
    CamCtrl->LoadSetting();

    CamCtrl->SetCameraSetup((CameraSetup)m_SaveSetup);

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

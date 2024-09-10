#include "pch.h"
#include "CKirbyDamage.h"

CKirbyDamage::CKirbyDamage()
    : m_Acc(0.f)
    , m_Duration(0.05f)
{
}

CKirbyDamage::~CKirbyDamage()
{
}

void CKirbyDamage::tick()
{
    m_Acc += DT;

    if (m_Duration < m_Acc)
    {
        PLAYERFSM->SetStateLock(false);
        ChangeState(L"BACKJUMP");
    }
}

void CKirbyDamage::Enter()
{
    PLAYERFSM->SetStateLock(true);

    // �ִϸ��̼� ���
    PLAYER->Animator()->Play(ANIMPREFIX("Damage"), true, false, 1.f);
    CPlayerMgr::SetPlayerFace(FaceType::Frown);
    if (PLAYERFSM->GetCurObjectIdx() == ObjectCopyType::NONE)
    {
        CPlayerMgr::ClearMouthMtrl();
        CPlayerMgr::SetPlayerMtrl(PLAYERMESH(MouthAngryClose));
    }

    // ����, ����, �̵� Lock
    PLAYERCTRL->LockMove();
    PLAYERCTRL->LockDirection();
    PLAYERCTRL->LockJump();

    PLAYERCTRL->SetVelocity({0.f, 0.f, 0.f});

    // Acc �ʱ�ȭ
    m_Acc = 0.f;

    // ����, �̹̽ú� ȿ��
    PLAYERFSM->SetInvincible(true, 3.f);
    PLAYERFSM->SetEmissive(true, 3.f);

    PLAYERFSM->SetKnockBackDir(PLAYERUNIT->GetHitDir());
}

void CKirbyDamage::Exit()
{
    // ����, ����, �̵� Unlock
    PLAYERCTRL->UnlockMove();
    PLAYERCTRL->UnlockDirection();
    PLAYERCTRL->UnlockJump();
}
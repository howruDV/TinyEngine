#include "pch.h"
#include "CKirbyBodyCollider.h"
#include "CPlayerMgr.h"
#include "CKirbyFSM.h"
#include "CKirbyVacuumCollider.h"
#include "CKirbyUnitScript.h"

CKirbyBodyCollider::CKirbyBodyCollider()
    : CScript(KIRBYBODYCOLLIDER)
{
}

CKirbyBodyCollider::~CKirbyBodyCollider()
{
}

void CKirbyBodyCollider::OnCollisionEnter(CCollider* _OtherCollider)
{
}

void CKirbyBodyCollider::OnCollisionStay(CCollider* _OtherCollider)
{
}

void CKirbyBodyCollider::OnCollisionExit(CCollider* _OtherCollider)
{
}

void CKirbyBodyCollider::OnTriggerEnter(CCollider* _OtherCollider)
{
    // 흡입을 시작한 상태(타겟을 결정해 빨아들이기 시작한 상태)에서 충돌한 경우
    if (PLAYERFSM->IsDrawing())
    {
        PLAYERFSM->DrawingCollisionEnter(_OtherCollider->GetOwner());
    }
    else if (_OtherCollider->GetOwner()->GetLayerIdx() == 5)
    {
        UnitHit HitInfo = {DAMAGE_TYPE::NORMAL, 10.f, 0.f, 0.f};
        PLAYERUNIT->GetDamage(HitInfo);

        // 무적상태일 경우
        if (PLAYERFSM->IsInvincible())
        {
            
        }
        // 무적 상태가 아닌경우
        else
        {
            // 무적상태로 바꿈
            PLAYERFSM->SetInvincible();

            // 날라가는 방향 설정
            Vec3 MonsterPos = _OtherCollider->Transform()->GetWorldPos();
            Vec3 KirbyPos = Transform()->GetWorldPos();

            Vec3 KnockBackDir = KirbyPos - MonsterPos;
            KnockBackDir.y = 0.f;
            KnockBackDir.Normalize();

            PLAYERFSM->SetKnockBackDir(KnockBackDir);

            // 상태 변경
            PLAYERFSM->ChangeState(L"DAMAGE");
        }



    }

    
}

void CKirbyBodyCollider::OnTriggerStay(CCollider* _OtherCollider)
{
}

void CKirbyBodyCollider::OnTriggerExit(CCollider* _OtherCollider)
{
}

void CKirbyBodyCollider::SaveToLevelFile(FILE* _File)
{
}

void CKirbyBodyCollider::LoadFromLevelFile(FILE* _File)
{
}

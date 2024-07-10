#pragma once
#include "CFSMScript.h"

enum class LastJumpType
{
    LOW,
    HIGH,
};

enum class DodgeType
{
    NONE,
    FRONT,
    RIGHT,
    BACK,
    LEFT,
};
    
class CKirbyAbility;
class CKirbyObject;
class CKirbyVacuumCollider;

class CKirbyFSM : public CFSMScript
{
private:
    CKirbyAbility*          m_arrAbility[(UINT)AbilityCopyType::END];
    AbilityCopyType         m_CurAbility;

    CKirbyObject*           m_arrObject[(UINT)ObjectCopyType::END];
    ObjectCopyType          m_CurObject;
    
    CGameObject*            m_StuffedCopyObj;
    CKirbyVacuumCollider*   m_VacuumCollider;

    // 상태 관리를 위한 값들
    const float             m_HoveringLimitTime;
    float                   m_HoveringAccTime;
    bool                    m_bHovering;

    float                   m_ChargeAccTime;
    LastJumpType            m_LastJump;
    DodgeType               m_DodgeType;
    bool                    m_bStuffed;

    Vec3                    m_KnockbackDir;
    float                   m_InvincibleAcc;
    float                   m_InvincibleDuration;
    float                   m_EmissiveCoef;
    bool                    m_bInvincible;

public:
    void begin() override;
    void tick() override;

    void ChangeAbilityCopy(AbilityCopyType _Type);
    void ChangeObjectCopy(ObjectCopyType _Type);
    void StartStuffed(CGameObject* _Target);
    void DrawingCollisionEnter(CGameObject* _CollisionObject);

public:
    void SetLastJump(LastJumpType _Type) { m_LastJump = _Type; }
    void SetHovering(bool _bHovering);
    void SetDodgeType(DodgeType _Type) { m_DodgeType = _Type; }
    void SetKnockBackDir(Vec3 _Dir) { m_KnockbackDir = _Dir; }
    void SetInvincible() { m_bInvincible = true;  m_InvincibleAcc = 0.f;}
    void ClearChargeAccTime() { m_ChargeAccTime = 0.f; }
    void ClearHoveringAccTime() { m_HoveringAccTime = 0.f; }
    void ClearStuff();

    CKirbyAbility* GetCurAbility() const { return m_arrAbility[(UINT)m_CurAbility]; }
    CKirbyObject* GetCurObject() const { return m_arrObject[(UINT)m_CurObject]; }
    AbilityCopyType GetCurAbilityIdx() const { return m_CurAbility; }
    ObjectCopyType GetCurObjectIdx() const { return m_CurObject; }
    CKirbyVacuumCollider* GetVacuumCol() const { return m_VacuumCollider; }
    LastJumpType GetLastJump() const { return m_LastJump; }
    CGameObject* GetStuffedCopyObj() { return m_StuffedCopyObj; }
    DodgeType GetDodgeType() const { return m_DodgeType; }
    float GetChargeAccTime() const { return m_ChargeAccTime; }
    float GetHoveringAccTime() const { return m_HoveringAccTime; }
    float GetHoveringLimitTime() const { return m_HoveringLimitTime; }
    bool IsStuffed() const { return m_bStuffed; }
    bool IsHovering() const { return m_bHovering; }
    bool IsDrawing() const;
    bool IsInvincible() const { return m_bInvincible; }
    Vec3 GetKnockBackDir() const { return m_KnockbackDir; }

    

public:
    virtual void SaveToLevelFile(FILE* _File) override;
    virtual void LoadFromLevelFile(FILE* _File) override;

public:
    CLONE(CKirbyFSM)
    CKirbyFSM();
    CKirbyFSM(const CKirbyFSM& _Origin);
    virtual ~CKirbyFSM();

    friend class CKirbyRun;
};

#pragma once
#include "CUnitScript.h"

class CBossWeaponHitbox : public CScript
{
private:
    CGameObject* m_Owner;
    CGameObject* m_Target;

    // damage
    float m_RandMin;
    float m_RandMax;
    int m_DamageTypeIdx;

    // repeat
    float m_AccTime;
    float m_RepeatTime;
    bool m_bRepeatDamage;
    bool m_bOnTrigger;

public:
    virtual void begin() override;
    virtual void tick() override;

private:
    virtual void OnTriggerEnter(CCollider* _OtherCollider);
    virtual void OnTriggerExit(CCollider* _OtherCollider);

    float GetRandDamage();
    void AddDamage();

public:
    virtual UINT SaveToLevelFile(FILE* _File) override;
    virtual UINT LoadFromLevelFile(FILE* _File) override;

public:
    CLONE(CBossWeaponHitbox)
    CBossWeaponHitbox();
    CBossWeaponHitbox(const CBossWeaponHitbox& _Origin);
    virtual ~CBossWeaponHitbox();
};
#pragma once

#include <Engine/CScript.h>

#define DOT_TERM 1.f

enum class DAMAGE_TYPE
{
    NORMAL,
    DOT,

    END,
};

// Unit 정보 구조체는 자식 생성자에서 넣어주기
struct UnitInfo
{
    float HP;
    float Speed;
    float JumpPower;
    float MAXHP;
};

struct UnitHit
{
    DAMAGE_TYPE Type;
    float       Damage;
    float       Duration;
    float       Acc;
};

class CUnitScript : public CScript
{
private:
    UnitInfo       m_CurInfo;
    UnitInfo       m_PrevInfo;
    std::list<UnitHit> m_HitHistory;

public:
    virtual void tick() override;
    virtual void AttackReward() {}              // Atk: Unit의 Attack에 따른 보상 처리 (흡혈효과 등)
    virtual void GetDamage(UnitHit _Damage);    // Hit: Unit의 Hit에 따른 패널티 처리 (HP감소 등)

private:
    void DamageProc();

public:
    virtual void SaveToLevelFile(FILE* _File) override;
    virtual void LoadFromLevelFile(FILE* _File) override;

    CUnitScript* Clone() = 0;
public: 
    CUnitScript(SCRIPT_TYPE _Type);
    virtual ~CUnitScript();
};

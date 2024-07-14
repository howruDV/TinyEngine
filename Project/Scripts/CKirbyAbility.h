#pragma once

// ====================================================================================
// USAGE: Kirby의 Ability 종속적인 States를 모아 정의해둔 States Group Class
// ====================================================================================
// - Ability 종속적인 기능(해당 class에 포함된 함수)은 State에서 call하는 방식으로 구현
// - Ability와 관계없이 공유할 수 있는 기능은 State에서 직접 구현
// - Ability와 Object: 둘은 서로 관여하는 기능이 다르므로 서로 다른 parent class를 상속

class CKirbyAbility
{
protected:
    float m_Charge1Time;

protected:
    // 사용 메쉬: hat, weapon
    void ChangeState(const wstring& _strStateName);

public:
    // 필수: Change, Idle, Attack, Jump Attack
    // 선택: ChargeAttack1, ChargeAttack2

    virtual void Idle(){};
    virtual void IdleEnter();
    virtual void IdleExit(){};

    virtual void Run(){};
    virtual void RunEnter(){};
    virtual void RunExit(){};

    virtual void RunStart(){};
    virtual void RunStartEnter(){};
    virtual void RunStartExit(){};

    virtual void Jump(){};
    virtual void JumpEnter();
    virtual void JumpExit(){};

    virtual void JumpStart(){};
    virtual void JumpStartEnter();
    virtual void JumpStartExit(){};

    virtual void JumpFall(){};
    virtual void JumpFallEnter();
    virtual void JumpFallExit(){};

    virtual void Landing(){};
    virtual void LandingEnter();
    virtual void LandingExit();

    virtual void LandingEnd(){};
    virtual void LandingEndEnter();
    virtual void LandingEndExit();

    //virtual void Hovering();

    //virtual void Evasiveness();

    virtual void Attack(){};
    virtual void AttackEnter(){};
    virtual void AttackExit(){};

    virtual void AttackEnd(){};
    virtual void AttackEndEnter(){};
    virtual void AttackEndExit(){};

    virtual void AttackCharge1(){};
    virtual void AttackCharge1Enter(){};
    virtual void AttackCharge1Exit(){};

    virtual void AttackCharge1Start(){};
    virtual void AttackCharge1StartEnter(){};
    virtual void AttackCharge1StartExit(){};

    virtual void AttackCharge1End(){};
    virtual void AttackCharge1EndEnter(){};
    virtual void AttackCharge1EndExit(){};

    virtual void AttackCharge1Run(){};
    virtual void AttackCharge1RunEnter(){};
    virtual void AttackCharge1RunExit(){};

    virtual void AttackCharge2(){};
    virtual void AttackCharge2Enter(){};
    virtual void AttackCharge2Exit(){};

    virtual void AttackCharge2Start(){};
    virtual void AttackCharge2StartEnter(){};
    virtual void AttackCharge2StartExit(){};

    virtual void AttackCharge2Run(){};
    virtual void AttackCharge2RunEnter(){};
    virtual void AttackCharge2RunExit(){};

    virtual void JumpAttack() = 0;
    virtual void JumpAttackEnter() = 0;
    virtual void JumpAttackExit() = 0;

    //virtual void Throw();

    //virtual void Change();

    // 변신
    virtual void ChangeAbility(){};
    virtual void ChangeAbilityEnter(){};
    virtual void ChangeAbilityExit(){};

public:
    float GetCharge1Time() const { return m_Charge1Time; }

public:
    virtual CKirbyAbility* Clone() = 0;

public:
    CKirbyAbility();
    virtual ~CKirbyAbility();
};

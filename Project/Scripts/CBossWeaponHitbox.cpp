#include "pch.h"
#include "CBossWeaponHitbox.h"
#include "CPlayerMgr.h"
#include "CBossMgr.h"
#include "CKirbyUnitScript.h"

CBossWeaponHitbox::CBossWeaponHitbox()
    : CScript(BOSSWEAPONHITBOX)
    , m_Owner(nullptr)
    , m_Target(nullptr)
    , m_RandMin(5.f)
    , m_RandMax(5.f)
    , m_DamageTypeIdx(0)
    , m_AccTime(0.f)
    , m_RepeatTime(2.f)
    , m_bRepeatDamage(false)
    , m_bOnTrigger(false)
{
    AddScriptParam(SCRIPT_PARAM::FLOAT, &m_RandMin, "Damage (random min)");
    AddScriptParam(SCRIPT_PARAM::FLOAT, &m_RandMax, "Damage (random max)");
    AddScriptParam(SCRIPT_PARAM::INT, &m_DamageTypeIdx, "Damage Type");

    AddScriptParam(SCRIPT_PARAM::BOOL, &m_bRepeatDamage, "Damage Repeat (stay trigger)");
    AddScriptParam(SCRIPT_PARAM::FLOAT, &m_RepeatTime, "Repeat Time");
}

CBossWeaponHitbox::CBossWeaponHitbox(const CBossWeaponHitbox& _Origin)
    : CScript(_Origin)
    , m_Owner(nullptr)
    , m_Target(nullptr)
    , m_RandMin(_Origin.m_RandMin)
    , m_RandMax(_Origin.m_RandMax)
    , m_DamageTypeIdx(_Origin.m_DamageTypeIdx)
    , m_AccTime(0.f)
    , m_RepeatTime(_Origin.m_RepeatTime)
    , m_bRepeatDamage(_Origin.m_bRepeatDamage)
    , m_bOnTrigger(false)
{
    AddScriptParam(SCRIPT_PARAM::FLOAT, &m_RandMin, "Damage (random min)");
    AddScriptParam(SCRIPT_PARAM::FLOAT, &m_RandMax, "Damage (random max)");
    AddScriptParam(SCRIPT_PARAM::INT, &m_DamageTypeIdx, "Damage Type");

    AddScriptParam(SCRIPT_PARAM::BOOL, &m_bRepeatDamage, "Damage Repeat (stay trigger)");
    AddScriptParam(SCRIPT_PARAM::FLOAT, &m_RepeatTime, "Repeat Time");
}

CBossWeaponHitbox::~CBossWeaponHitbox()
{
}

void CBossWeaponHitbox::begin()
{
    m_Target = PLAYER;
    m_Owner = BOSS;
}

void CBossWeaponHitbox::tick()
{
    if (!m_Owner || !m_Target || !m_bRepeatDamage || !m_bOnTrigger)
        return;

    m_AccTime += DT;

    if (m_AccTime >= m_RepeatTime)
    {
        m_AccTime -= m_RepeatTime;
        AddDamage();
    }
}

void CBossWeaponHitbox::OnTriggerEnter(CCollider* _OtherCollider)
{
    m_bOnTrigger = true;
    m_AccTime = 0.f;
    AddDamage();
}

void CBossWeaponHitbox::OnTriggerExit(CCollider* _OtherCollider)
{
    m_bOnTrigger = false;
}

float CBossWeaponHitbox::GetRandDamage()
{
    float Damage = 5.f;
    Damage = GetRandomfloat(m_RandMin, m_RandMax);

    return Damage;
}

void CBossWeaponHitbox::AddDamage()
{
    if (!m_Owner || !m_Target)
        return;

    Vec3 HitDir = (m_Target->Transform()->GetWorldPos() - m_Owner->Transform()->GetWorldPos()).Normalize();
    UnitHit HitInfo = {(DAMAGE_TYPE)m_DamageTypeIdx, HitDir, GetRandDamage(), 0.f, 0.f};

    PLAYERUNIT->GetDamage(HitInfo);
    BOSSUNIT->AttackReward();
}

UINT CBossWeaponHitbox::SaveToLevelFile(FILE* _File)
{
    UINT MemoryByte = 0;

    fwrite(&m_RandMin, 1, sizeof(float), _File);
    fwrite(&m_RandMax, 1, sizeof(float), _File);
    fwrite(&m_DamageTypeIdx, 1, sizeof(int), _File);
    fwrite(&m_bRepeatDamage, 1, sizeof(bool), _File);
    fwrite(&m_RepeatTime, 1, sizeof(float), _File);

    MemoryByte += sizeof(float);
    MemoryByte += sizeof(float);
    MemoryByte += sizeof(int);
    MemoryByte += sizeof(bool);
    MemoryByte += sizeof(float);

    return MemoryByte;
}

UINT CBossWeaponHitbox::LoadFromLevelFile(FILE* _File)
{
    UINT MemoryByte = 0;

    fread(&m_RandMin, 1, sizeof(float), _File);
    fread(&m_RandMax, 1, sizeof(float), _File);
    fread(&m_DamageTypeIdx, 1, sizeof(int), _File);
    fread(&m_bRepeatDamage, 1, sizeof(bool), _File);
    fread(&m_RepeatTime, 1, sizeof(float), _File);

    MemoryByte += sizeof(float);
    MemoryByte += sizeof(float);
    MemoryByte += sizeof(int);
    MemoryByte += sizeof(bool);
    MemoryByte += sizeof(float);

    return MemoryByte;
}

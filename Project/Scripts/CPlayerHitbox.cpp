#include "pch.h"
#include "CPlayerHitbox.h"
#include "CPlayerMgr.h"
#include "CKirbyUnitScript.h"
#include "CCameraController.h"

CPlayerHitbox::CPlayerHitbox()
    : CScript(PLAYERHITBOX)
    , m_Instigator(nullptr)
    , m_Collider(nullptr)
    , m_bSummon(false)
    , m_bCallReward(true)
    , m_Damage(5.f)
    , m_DamageTypeIdx(0)
    , m_AccTime(0.f)
    , m_RepeatTime(2.f)
    , m_bRepeatDamage(true)
    , m_bRepeat(false)
    , m_bTimeScaling(false)
    , m_bDestroyCollision(false)
    , m_bCameraShake(false)
    , m_SoundAcc(0.f)
    , m_SoundCoolTime(0.1f)
{
    AddScriptParam(SCRIPT_PARAM::FLOAT, &m_Damage, "Damage");
    AddScriptParam(SCRIPT_PARAM::INT, &m_DamageTypeIdx, "Damage Type");
    AddScriptParam(SCRIPT_PARAM::FLOAT, &m_RepeatTime, "Repeat Time");
    AddScriptParam(SCRIPT_PARAM::BOOL, &m_bRepeatDamage, "Damage Repeat (stay trigger)");
    AddScriptParam(SCRIPT_PARAM::BOOL, &m_bCallReward, "Call Reward");
    AddScriptParam(SCRIPT_PARAM::BOOL, &m_bSummon, "Summon");
    AddScriptParam(SCRIPT_PARAM::BOOL, &m_bTimeScaling, "Time Scaling");
    AddScriptParam(SCRIPT_PARAM::BOOL, &m_bDestroyCollision, "Destroy Collision");
    AddScriptParam(SCRIPT_PARAM::BOOL, &m_bCameraShake, "Camera Shake");
}

CPlayerHitbox::CPlayerHitbox(const CPlayerHitbox& _Origin)
    : CScript(_Origin)
    , m_Instigator(nullptr)
    , m_Collider(nullptr)
    , m_bSummon(_Origin.m_bSummon)
    , m_bCallReward(_Origin.m_bCallReward)
    , m_Damage(_Origin.m_Damage)
    , m_DamageTypeIdx(_Origin.m_DamageTypeIdx)
    , m_AccTime(0.f)
    , m_RepeatTime(_Origin.m_RepeatTime)
    , m_bRepeatDamage(_Origin.m_bRepeatDamage)
    , m_bRepeat(false)
    , m_bTimeScaling(_Origin.m_bTimeScaling)
    , m_bDestroyCollision(_Origin.m_bDestroyCollision)
    , m_bCameraShake(_Origin.m_bCameraShake)
    , m_SoundAcc(0.f)
    , m_SoundCoolTime(0.1f)
{
    AddScriptParam(SCRIPT_PARAM::FLOAT, &m_Damage, "Damage");
    AddScriptParam(SCRIPT_PARAM::INT, &m_DamageTypeIdx, "Damage Type");
    AddScriptParam(SCRIPT_PARAM::FLOAT, &m_RepeatTime, "Repeat Time");
    AddScriptParam(SCRIPT_PARAM::BOOL, &m_bRepeatDamage, "Damage Repeat (stay trigger)");
    AddScriptParam(SCRIPT_PARAM::BOOL, &m_bCallReward, "Call Reward");
    AddScriptParam(SCRIPT_PARAM::BOOL, &m_bSummon, "Summon");
    AddScriptParam(SCRIPT_PARAM::BOOL, &m_bTimeScaling, "Time Scaling");
    AddScriptParam(SCRIPT_PARAM::BOOL, &m_bDestroyCollision, "Destroy Collision");
    AddScriptParam(SCRIPT_PARAM::BOOL, &m_bCameraShake, "Camera Shake");
}

CPlayerHitbox::~CPlayerHitbox()
{
}

void CPlayerHitbox::begin()
{
    m_Instigator = PLAYER;
    m_Collider = GetOwner()->GetComponent<CCollider>();
}

void CPlayerHitbox::tick()
{
    // case : repeat
    if (!m_Instigator || !m_bRepeatDamage)
        return;

    m_AccTime += DT;
    m_SoundAcc += DT;

    // On Repeat flag
    if (m_AccTime > m_RepeatTime)
    {
        m_AccTime = 0.f;
        m_bRepeat = true;
    }

    // Off Repeat flag (after repeat tick)
    else if (m_AccTime < m_RepeatTime && m_bRepeat)
    {
        m_bRepeat = false;
    }
}

// ������ : ���� �������� �浹�� ���� �������� ������ �ݺ���
// �ó����� :
// - �������Ƕ� ���� �ε���
// - 4�� ������ ������ Ȳ�Ҷ� �浹
// - ������ �ݺ����ַ��� üũ�ϴ� m_AccTime �ʱ�ȭ��
// - ���������� �浹�� ����(Ȳ��) ���� 5�� ��������
// - �浹�ߴ� ��� ����(��������, Ȳ��) ��� ������ �ٽ� ���� �� ����
// �����ǰ� : ������ �ݺ� �ð��� ª���� (0.1��~) ���� �ѷ��� �巯���� ������ �׳� ����
void CPlayerHitbox::OnTriggerEnter(CCollider* _OtherCollider)
{
    UINT Layer = _OtherCollider->GetOwner()->GetLayerIdx();
    wstring Name = _OtherCollider->GetOwner()->GetName();
    CGameObject* pMonster = nullptr;

    if (Layer == LAYER_MONSTER)
    {
        pMonster = _OtherCollider->GetOwner();
    }
    else if (Layer == LAYER_MONSTER_TRIGGER)
    {
        if (Name == L"Body Collider")
        {
            pMonster = _OtherCollider->GetOwner()->GetParent();
        }
        else
        {
            return;
        }
    }
    else
    {
        return;
    }

    // sound

    if (m_SoundAcc > m_SoundCoolTime)
    {
        GamePlayStatic::Play2DSound(L"sound\\wav\\HeroBasic\\0012.wav", 1, KIRBY_EFFECTSOUND * 1.5f, false);
        m_SoundAcc = 0.f;
    }


    m_AccTime = 0.f;
    AddDamage(pMonster);

    // �����
    if (m_bTimeScaling)
    {
        CTimeMgr::GetInst()->SetTimeScale(0.1f, 0.f);
    }

    // Destroy
    if (m_bDestroyCollision)
    {
        GamePlayStatic::DestroyGameObject(GetOwner());
    }

    // Camera Shake
    if (m_bCameraShake)
    {
        CAMERACTRL->Shake(0.1f, 15.f, 15.f);
    }
}

// Trigger�� ��� �浹�ϰ� �ִ� ���, Repeat Damage ó��
void CPlayerHitbox::OnTriggerStay(CCollider* _OtherCollider)
{
    if (!m_Instigator || !m_bRepeatDamage || !m_bRepeat)
        return;

    UINT Layer = _OtherCollider->GetOwner()->GetLayerIdx();
    wstring Name = _OtherCollider->GetOwner()->GetName();
    CGameObject* pMonster = nullptr;

    // Find Monster (Attack Target)
    if (Layer == LAYER_MONSTER)
    {
        pMonster = _OtherCollider->GetOwner();
    }
    else if (Layer == LAYER_MONSTER_TRIGGER)
    {
        if (Name == L"Body Collider")
        {
            pMonster = _OtherCollider->GetOwner()->GetParent();
        }
        else
        {
            return;
        }
    }
    else
    {
        return;
    }

    // Add Damage
    AddDamage(pMonster);

    // �����
    if (m_bTimeScaling)
    {
        CTimeMgr::GetInst()->SetTimeScale(0.1f, 0.f);
    }

    // Destroy
    if (m_bDestroyCollision)
    {
        GamePlayStatic::DestroyGameObject(GetOwner());
    }

    // Camera Shake
    if (m_bCameraShake)
    {
        CAMERACTRL->Shake(0.1f, 15.f, 15.f);
    }
}

void CPlayerHitbox::AddDamage(CGameObject* _Monster)
{
    CUnitScript* pUnit = _Monster->GetScript<CUnitScript>();

    if (!m_Instigator || !pUnit)
        return;

    // create HitInfo
    Vec3 HitDir = _Monster->Transform()->GetWorldPos();

    if (!m_bSummon)
    {
        HitDir -= m_Instigator->Transform()->GetWorldPos();
    }
    else
    {
        HitDir -= Transform()->GetWorldPos();
    }
    HitDir.Normalize();

    UnitHit HitInfo = {(DAMAGE_TYPE)m_DamageTypeIdx, HitDir, m_Damage, 0.f, 0.f};

    // Damage & Reward
    pUnit->GetDamage(HitInfo);

    if (m_bCallReward)
    {
        PLAYERUNIT->AttackReward();
    }
}

UINT CPlayerHitbox::SaveToLevelFile(FILE* _File)
{
    UINT MemoryByte = 0;

    fwrite(&m_Damage, 1, sizeof(float), _File);
    fwrite(&m_DamageTypeIdx, 1, sizeof(int), _File);
    fwrite(&m_bRepeatDamage, 1, sizeof(bool), _File);
    fwrite(&m_RepeatTime, 1, sizeof(float), _File);
    fwrite(&m_bSummon, 1, sizeof(bool), _File);
    fwrite(&m_bCallReward, 1, sizeof(bool), _File);
    fwrite(&m_bTimeScaling, 1, sizeof(bool), _File);
    fwrite(&m_bDestroyCollision, 1, sizeof(bool), _File);
    fwrite(&m_bCameraShake, 1, sizeof(bool), _File);

    MemoryByte += sizeof(float);
    MemoryByte += sizeof(int);
    MemoryByte += sizeof(bool);
    MemoryByte += sizeof(float);
    MemoryByte += sizeof(bool);
    MemoryByte += sizeof(bool);
    MemoryByte += sizeof(bool);
    MemoryByte += sizeof(bool);
    MemoryByte += sizeof(bool);

    return MemoryByte;
}

UINT CPlayerHitbox::LoadFromLevelFile(FILE* _File)
{
    UINT MemoryByte = 0;

    fread(&m_Damage, 1, sizeof(float), _File);
    fread(&m_DamageTypeIdx, 1, sizeof(int), _File);
    fread(&m_bRepeatDamage, 1, sizeof(bool), _File);
    fread(&m_RepeatTime, 1, sizeof(float), _File);
    fread(&m_bSummon, 1, sizeof(bool), _File);
    fread(&m_bCallReward, 1, sizeof(bool), _File);
    fread(&m_bTimeScaling, 1, sizeof(bool), _File);
    fread(&m_bDestroyCollision, 1, sizeof(bool), _File);
    fread(&m_bCameraShake, 1, sizeof(bool), _File);

    MemoryByte += sizeof(float);
    MemoryByte += sizeof(int);
    MemoryByte += sizeof(bool);
    MemoryByte += sizeof(float);
    MemoryByte += sizeof(bool);
    MemoryByte += sizeof(bool);
    MemoryByte += sizeof(bool);
    MemoryByte += sizeof(bool);
    MemoryByte += sizeof(bool);

    return MemoryByte;
}
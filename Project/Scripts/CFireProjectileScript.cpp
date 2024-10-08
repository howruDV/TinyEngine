#include "pch.h"
#include "CFireProjectileScript.h"

#include "CPlayerMgr.h"

#include "CUnitScript.h"

CFireProjectileScript::CFireProjectileScript()
    : CMonsterATKScript(FIREPROJECTILESCRIPT)
    , m_eState(FIREPROJECTILE_STATE::Attack)
    , m_Quat{}
    , m_fSpeed(200.f)
    , m_fAccRadian(0.f)
    , m_fFallSpeed(0.03f)
{
    AddScriptParam(SCRIPT_PARAM::FLOAT, &m_fSpeed, "Speed");
    AddScriptParam(SCRIPT_PARAM::FLOAT, &m_fFallSpeed, "FallSpeed");
    AddScriptParam(SCRIPT_PARAM::FLOAT, &m_fAccRadian, "AccRadian");
}

CFireProjectileScript::CFireProjectileScript(const CFireProjectileScript& _Origin)
    : CMonsterATKScript(_Origin)
    , m_eState(FIREPROJECTILE_STATE::Attack)
    , m_Quat{}
    , m_fSpeed(_Origin.m_fSpeed)
    , m_fAccRadian(0.f)
    , m_fFallSpeed(_Origin.m_fFallSpeed)
{
    AddScriptParam(SCRIPT_PARAM::FLOAT, &m_fSpeed, "Speed");
    AddScriptParam(SCRIPT_PARAM::FLOAT, &m_fFallSpeed, "FallSpeed");
    AddScriptParam(SCRIPT_PARAM::FLOAT, &m_fAccRadian, "AccRadian");
}

CFireProjectileScript::~CFireProjectileScript()
{
}

void CFireProjectileScript::begin()
{
    ChangeState(FIREPROJECTILE_STATE::Attack);
}

void CFireProjectileScript::tick()
{
    switch (m_eState)
    {
    case FIREPROJECTILE_STATE::Attack: {
        Attack();
    }
    break;
    case FIREPROJECTILE_STATE::Destroy: {
        Destroy();
    }
    break;
    default:
        break;
    }
}

void CFireProjectileScript::EnterState(FIREPROJECTILE_STATE _state)
{
    switch (m_eState)
    {
    case FIREPROJECTILE_STATE::Attack:
        break;
    case FIREPROJECTILE_STATE::Destroy:
        break;
    default:
        break;
    }
}

void CFireProjectileScript::ChangeState(FIREPROJECTILE_STATE _state)
{
    ExitState(m_eState);
    m_eState = _state;
    EnterState(m_eState);
}

void CFireProjectileScript::ExitState(FIREPROJECTILE_STATE _state)
{
    switch (m_eState)
    {
    case FIREPROJECTILE_STATE::Attack:
        break;
    case FIREPROJECTILE_STATE::Destroy:
        break;
    default:
        break;
    }
}

void CFireProjectileScript::Attack()
{
    m_fAccRadian += DT * m_fFallSpeed;
    Vec3 vDir = Transform()->GetWorldDir(DIR_TYPE::FRONT);
    if (m_fAccRadian >= 1.5f)
    {
        m_fAccRadian = 1.5f;
    }
    vDir.y = -39.5f * cosf(m_fAccRadian) * DT;

    Rigidbody()->SetVelocity(vDir * m_fSpeed);
}

void CFireProjectileScript::Destroy()
{
    // TODO : 대충 삭제 머시기 하고 삭제
    GamePlayStatic::DestroyGameObject(GetOwner());
}

void CFireProjectileScript::OnTriggerEnter(CCollider* _OtherCollider)
{
    // 1. Player Hit
    CGameObject* pObj = _OtherCollider->GetOwner();

    if (nullptr == pObj)
        return;

    UnitHit hitInfo = {DAMAGE_TYPE::NORMAL, Transform()->GetWorldDir(DIR_TYPE::FRONT), 6.f, 0.f, 0.f};
    LAYER_PLAYER == pObj->GetLayerIdx() && L"Main Player" == pObj->GetName() ? pObj->GetScript<CUnitScript>()->GetDamage(hitInfo) : void();

    // 2. Static Object Hit
    LAYER_STATIC == pObj->GetLayerIdx() ? ChangeState(FIREPROJECTILE_STATE::Destroy) : void();

    GamePlayStatic::Play2DSound(L"sound\\wav\\CharaBasic\\0008.wav", 1, MONSTER_EFFECTSOUND);
}

void CFireProjectileScript::OnTriggerExit(CCollider* _OtherCollider)
{
}

UINT CFireProjectileScript::SaveToLevelFile(FILE* _File)
{
    UINT MemoryByte = 0;

    MemoryByte += CMonsterATKScript::SaveToLevelFile(_File);
    fwrite(&m_fSpeed, sizeof(float), 1, _File);
    fwrite(&m_fFallSpeed, sizeof(float), 1, _File);

    MemoryByte += sizeof(float);
    MemoryByte += sizeof(float);

    return MemoryByte;
}

UINT CFireProjectileScript::LoadFromLevelFile(FILE* _File)
{
    UINT MemoryByte = 0;

    MemoryByte += CMonsterATKScript::LoadFromLevelFile(_File);
    fread(&m_fSpeed, sizeof(float), 1, _File);
    fread(&m_fFallSpeed, sizeof(float), 1, _File);

    MemoryByte += sizeof(float);
    MemoryByte += sizeof(float);

    return MemoryByte;
}
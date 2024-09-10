#include "pch.h"
#include "CUnitScript.h"

CUnitScript::CUnitScript(UINT _Type)
    : CScript(_Type)
    , m_InitInfo{}
    , m_PrevInfo{}
    , m_CurInfo{}
{
}

CUnitScript::CUnitScript(const CUnitScript& _Origin)
    : CScript(_Origin)
    , m_InitInfo{_Origin.m_InitInfo}
    , m_PrevInfo{}
    , m_CurInfo(_Origin.m_CurInfo)
{
}

CUnitScript::~CUnitScript()
{
}

void CUnitScript::begin()
{
    m_CurInfo = m_InitInfo;
}

void CUnitScript::tick()
{
    ClearHitDir();
    m_PrevInfo = m_CurInfo;

    // ���� �����ӿ� �޴� ���������� ������Ʈ
    float NewDamage = DamageProc();
    if (NewDamage > 0.f)
    {
        m_CurInfo.HP -= NewDamage;
    }

    if (m_CurInfo.HP < 0.f)
    {
        m_CurInfo.HP = 0.f;
    }

    if (m_CurInfo.HP > m_CurInfo.MAXHP)
    {
        m_CurInfo.HP = m_CurInfo.MAXHP;
    }
}

void CUnitScript::GetDamage(UnitHit _Damage)
{
    m_HitHistory.push_back(_Damage);
}

float CUnitScript::DamageProc()
{
    float CurDamage = 0.f;

    std::list<UnitHit>::iterator iter = m_HitHistory.begin();

    while (iter != m_HitHistory.end())
    {
        switch (iter->Type)
        {
        case DAMAGE_TYPE::NORMAL: {
            CurDamage += iter->Damage;
            m_HitDir += iter->HitDir;
            iter = m_HitHistory.erase(iter);
        }
        break;

        case DAMAGE_TYPE::DOT: {
            iter->Acc += DT;

            if (iter->Acc >= DOT_TERM)
            {
                CurDamage += iter->Damage;
                iter->Acc -= DOT_TERM;
                iter->Duration -= DOT_TERM;
                iter++;
            }

            if (iter->Duration <= 0.f)
            {
                iter = m_HitHistory.erase(iter);
            }
        }
        break;
        }
    }

    return CurDamage;
}

void CUnitScript::SetHitDirHorizen()
{
    m_HitDir.y = 0.f;
    m_HitDir.Normalize();
}

UINT CUnitScript::SaveToLevelFile(FILE* _File)
{
    UINT MemoryByte = 0;

    fwrite(&m_InitInfo, sizeof(UnitInfo), 1, _File);
    MemoryByte += sizeof(UnitInfo);

    return MemoryByte;
}

UINT CUnitScript::LoadFromLevelFile(FILE* _File)
{
    UINT MemoryByte = 0;

    fread(&m_InitInfo, sizeof(UnitInfo), 1, _File);
    m_CurInfo = m_InitInfo;

    MemoryByte += sizeof(UnitInfo);

    return MemoryByte;
}

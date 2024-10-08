#pragma once
#include <Engine\\CScript.h>
class CUnitScript;
class CFSMScript;
class CBossLevelFlowMgr;

class CBossMgr : public CScript
{
public:
    string m_BossName;

private:
    static CGameObject* m_Boss;
    static CUnitScript* m_BossUnit;
    static CFSMScript* m_BossFSM;
    static CBossLevelFlowMgr* m_FlowMgr;

public:
    virtual void begin() override;

public:
    void SetBoss();

    static CGameObject* GetBoss() { return m_Boss; }
    static CUnitScript* GetBossUnit() { return m_BossUnit; }
    static CFSMScript* GetBossFSM() { return m_BossFSM; }
    static CBossLevelFlowMgr* GetBossFlowMgr() { return m_FlowMgr; }

public:
    virtual UINT SaveToLevelFile(FILE* _File) override;
    virtual UINT LoadFromLevelFile(FILE* _File) override;

public:
    CLONE(CBossMgr);
    CBossMgr();
    virtual ~CBossMgr();
};

#include "pch.h"
#include "CElfilisFSM.h"
#include "CElfilisBigFSM.h"
#include "CState.h"
#include "CPlayerMgr.h"

CElfilisFSM::CElfilisFSM()
    : CFSMScript(ELFILISFSM)
    , m_CurStateGroup(ElfilisStateGroup::END)
    , m_Pattern(ElfilisPatternType::NONE)
    , m_Phase(1)
    , m_ComboLevel(0)
    , m_PatternStep(0)
    , m_bAttackRepeat(false)
    , m_bResist(false)
    , m_GroundAttackCount(0)
    , m_NearDist(5.f)
    , m_AirPosition(Vec3(0.f, 600.f, 700.f))
    , m_bGroundCollision(false)
    , m_BigElfilis(nullptr)
    , m_BigElfilisFSM(nullptr)
    , m_Weapon(nullptr)
    , m_Hitbox(nullptr)
    , m_MapFloorOffset(Vec3(0.f, 0.f, 5.f))
    , m_MapSizeRadius(25.f)
{
    for (UINT i = 0; i < (UINT)ElfilisStateGroup::END; ++i)
    {
        m_StateGroup[(ElfilisStateGroup)i][0] = vector<wstring>();
        m_StateGroup[(ElfilisStateGroup)i][1] = vector<wstring>();
    }

    m_DropStarPref = CAssetMgr::GetInst()->Load<CPrefab>(L"prefab\\BossDropStar.pref");
}

CElfilisFSM::CElfilisFSM(const CElfilisFSM& _Origin)
    : CFSMScript(_Origin)
    , m_CurStateGroup(ElfilisStateGroup::END)
    , m_Pattern(ElfilisPatternType::NONE)
    , m_Phase(_Origin.m_Phase)
    , m_ComboLevel(0)
    , m_PatternStep(0)
    , m_bAttackRepeat(false)
    , m_bResist(false)
    , m_GroundAttackCount(0)
    , m_NearDist(_Origin.m_NearDist)
    , m_AirPosition(_Origin.m_NearDist)
    , m_bGroundCollision(false)
    , m_BigElfilis(nullptr)
    , m_BigElfilisFSM(nullptr)
    , m_Weapon(nullptr)
    , m_Hitbox(nullptr)
    , m_MapFloorOffset(_Origin.m_MapFloorOffset)
    , m_MapSizeRadius(_Origin.m_MapSizeRadius)
{
    for (auto it : m_StateGroup)
    {
        m_StateGroup[(ElfilisStateGroup)it.first][0] = vector<wstring>();
        m_StateGroup[(ElfilisStateGroup)it.first][1] = vector<wstring>();
    }

    m_DropStarPref = CAssetMgr::GetInst()->Load<CPrefab>(L"prefab\\BossDropStar.pref");
}

CElfilisFSM::~CElfilisFSM()
{
}

void CElfilisFSM::ResetFSM()
{
    // FSM setting clear
    SetGlobalState(false);
    SetPattern(ElfilisPatternType::NONE);
    SetResist(false);
    ClearComboLevel();

    // Big Elfilis
    m_BigElfilisFSM->ChangeState(L"IDLE");
    m_BigElfilis->SetActive(false);

    // attack collider clear
    OffWeaponTrigger();
    m_Hitbox->BoxCollider()->SetSize(Vec3(1.f, 1.f, 1.f));
    m_Hitbox->BoxCollider()->SetCenter(Vec3::Zero);
    m_Hitbox->SetEnabled(false);
}

// USAGE : �����ϰ� State Group�� ����
void CElfilisFSM::ChangeStateGroup(ElfilisStateGroup _Group, const wstring& _State)
{
    if (m_Pattern != ElfilisPatternType::NONE)
    {
        ProcPatternStep();
        return;
    }

    if (_State.empty())
    {
        ChangeStateGroup_Random(_Group);
    }
    else
    {
        ChangeStateGroup_Set(_Group, _State);
    }
}

void CElfilisFSM::ChangeStateGroup_Random(ElfilisStateGroup _Group)
{
    if (m_CurStateGroup == _Group || m_StateGroup.find(_Group) == m_StateGroup.cend())
        return;

    // get min counted state
    UINT MinCount = UINT_MAX;
    vector<wstring> MinCountedStates;

    for (pair<wstring, UINT> iter : m_StateSelectionCount[(int)_Group])
    {
        if (iter.second > MinCount)
        {
            continue;
        }
        else if (iter.second == MinCount)
        {
            MinCountedStates.push_back(iter.first);
        }
        else
        {
            MinCount = iter.second;
            MinCountedStates.clear();
            MinCountedStates.push_back(iter.first);
        }
    }

    int Random = GetRandomInt(0, (int)MinCountedStates.size() - 1);
    wstring RandState = MinCountedStates[Random];

    SetStateGroup(_Group);
    ChangeState(RandState);
    m_StateSelectionCount[(int)_Group][RandState]++;
}

void CElfilisFSM::ChangeStateGroup_Set(ElfilisStateGroup _Group, const wstring& _State)
{
    if (m_StateGroup.find(_Group) == m_StateGroup.cend())
        return;

    vector<wstring>::iterator iter1 = find(m_StateGroup[_Group][0].begin(), m_StateGroup[_Group][0].end(), _State);
    vector<wstring>::iterator iter2 = find(m_StateGroup[_Group][1].begin(), m_StateGroup[_Group][1].end(), _State);
    if (iter1 == m_StateGroup[_Group][0].end() && iter2 == m_StateGroup[_Group][1].end())
        return;

    SetStateGroup(_Group);
    ChangeState(_State);
}

void CElfilisFSM::RepeatState(wstring _State)
{
    if (m_Pattern != ElfilisPatternType::NONE)
    {
        ProcPatternStep();
        return;
    }

    m_bAttackRepeat = true;

    if (_State != L"")
    {
        ChangeState(_State);
    }
    else
    {
        ChangeState(GetCurState()->GetName());
    }
}

// Usage : GroundAttackCount, Repeat ���� ���� üũ�� �ʿ��� ���� Group State�� ã��
ElfilisStateGroup CElfilisFSM::FindNextStateGroup() const
{
    switch (m_CurStateGroup)
    {
    case ElfilisStateGroup::GroundIdle: {
        return ElfilisStateGroup::GroundMove;
    }
    break;
    case ElfilisStateGroup::GroundMove:
    case ElfilisStateGroup::GroundMoveAtk: {
        float Rand = GetRandomfloat(1, 100);

        if (Rand <= 90.f || m_BigElfilis->IsActive())
        {
            if (IsNearPlayer())
            {
                return ElfilisStateGroup::GroundAtkNear;
            }
            else
            {
                return ElfilisStateGroup::GroundAtkFar;
            }
        }
        else
        {
            return ElfilisStateGroup::GroundToAir;
        }
    }
    break;

    case ElfilisStateGroup::GroundAtkNear:
    case ElfilisStateGroup::GroundAtkFar: {
        // case : �̹� ���� ������ 2�� �ݺ��� ����
        if (m_bAttackRepeat)
        {
            if (m_GroundAttackCount >= 2)
            {
                return ElfilisStateGroup::GroundToAir;
            }
            else
            {
                float Rand = GetRandomfloat(1, 100);

                if (Rand <= 80.f || m_BigElfilis->IsActive())
                {
                    return ElfilisStateGroup::GroundMove;
                }
                // else if (Rand <= 80.f)
                //{
                //     return ElfilisStateGroup::GroundMoveAtk;
                // }
                else
                {
                    return ElfilisStateGroup::GroundToAir;
                }
            }
        }

        // case : ���� ������ ���
        else
        {
            float Rand = GetRandomfloat(1, 100);

            if (Rand <= 80.f || m_BigElfilis->IsActive())
            {
                if (Rand <= 50.f)
                {
                    // ���� Group State�� retrun�Ǵ� ���, �ݺ��� �ǹ� (�Լ� ȣ���� �ʿ��� ���� �� Ȯ�� �� ó��)
                    return m_CurStateGroup;
                }
                else
                {
                    return ElfilisStateGroup::GroundMove;
                }
                // else if (Rand <= 80.f)
                //{
                //     return ElfilisStateGroup::GroundMoveAtk;
                // }
            }
            else
            {
                return ElfilisStateGroup::GroundToAir;
            }
        }
    }
    break;
    case ElfilisStateGroup::GroundToAir: {
        float Rand = GetRandomfloat(1, 100);

        if (Rand <= 75.f)
        {
            return ElfilisStateGroup::AirMove;
        }
        else
        {
            return ElfilisStateGroup::AirLargeAtk;
        }
    }
    break;
    case ElfilisStateGroup::AirIdle:
        break;
    case ElfilisStateGroup::AirMove: {
        if (m_Phase == 1)
        {
            return ElfilisStateGroup::AirSmallAtk1;
        }
        else
        {
            float Rand = GetRandomfloat(1, 100);

            if (Rand <= 20.f)
            {
                return ElfilisStateGroup::AirSmallAtk1;
            }
            else
            {
                return ElfilisStateGroup::AirSmallAtk2;
            }
        }
    }
    break;
    case ElfilisStateGroup::AirSmallAtk1:
    case ElfilisStateGroup::AirSmallAtk2: {
        // case : �̹� ���� ������ 2�� �ݺ��� ����
        if (m_bAttackRepeat)
        {
            return ElfilisStateGroup::AirToGround;
        }

        // case : ���� ������ ���
        else
        {
            float Rand = GetRandomfloat(1, 100);

            if (Rand <= 50.f)
            {
                // ���� Group State�� retrun�Ǵ� ���, �ݺ��� �ǹ� (�Լ� ȣ���� �ʿ��� ���� �� Ȯ�� �� ó��)
                return m_CurStateGroup;
            }
            else
            {
                return ElfilisStateGroup::AirToGround;
            }
        }
    }
    break;
    case ElfilisStateGroup::AirLargeAtk: {
        return ElfilisStateGroup::AirToGround;
    }
    break;
    case ElfilisStateGroup::AirToGround: {
        if (GetPlayerDist() > 100.f)
        {
            return ElfilisStateGroup::GroundAtkFar;
        }
        else
        {
            return ElfilisStateGroup::GroundAtkNear;
        }
    }
    break;
    }

    return ElfilisStateGroup::END;
}

void CElfilisFSM::SpawnDropStar(Vec3 _Pos)
{
    if (m_DropStarPref == nullptr)
        return;

    CGameObject* pDropStar = m_DropStarPref->Instantiate();
    pDropStar->Transform()->SetWorldPos(_Pos);
    GamePlayStatic::SpawnGameObject(pDropStar, LAYER_DYNAMIC);
}

void CElfilisFSM::DestroySumon()
{
    for (CGameObject* Obj : CLevelMgr::GetInst()->GetCurrentLevel()->GetLayer(LAYER_MONSTERATK)->GetParentObjects())
    {
        GamePlayStatic::DestroyGameObject(Obj);
    }

    for (CGameObject* Obj : CLevelMgr::GetInst()->GetCurrentLevel()->GetLayer(LAYER_MONSTERATK_TRIGGER)->GetParentObjects())
    {
        GamePlayStatic::DestroyGameObject(Obj);
    }
}

#include "CElfilisD_Appear.h"
#include "CElfilisD_Damage.h"
#include "CElfilisD_Jump.h"
#include "CElfilisD_Roar.h"
#include "CElfilisD_Resist.h"
#include "CElfilisD_ResistFail.h"
#include "CElfilisD_ResistSuccess.h"

#include "CElfilisG_Idle.h"
#include "CElfilisG_BackStep.h"
#include "CElfilisG_Teleport.h"
#include "CElfilisG_NormalAtk.h"
#include "CElfilisG_NormalAtkL.h"
#include "CElfilisG_NormalAtkR.h"
#include "CElfilisG_NormalAtkFinishL.h"
#include "CElfilisG_NormalAtkTeleportL.h"
#include "CElfilisG_NormalAtkTeleportR.h"
#include "CElfilisG_NormalAtkTeleportFinishL.h"
#include "CElfilisG_RayArrow.h"
#include "CElfilisG_SwordWaveRL.h"
#include "CElfilisG_SwordWaveLR.h"
#include "CElfilisG_SwordWaveFinishRL.h"
#include "CElfilisG_SwordWaveStorm.h"
// #include "CElfilisG_DimensionSpike.h"
#include "CElfilisG_GroundToAir.h"
#include "CElfilisG_ToAirTeleport.h"

#include "CElfilisA_Idle.h"
#include "CElfilisA_MoveL.h"
#include "CElfilisA_MoveR.h"
#include "CElfilisA_RayArrowUp.h"
// #include "CElfilisA_RayArrowDown.h"
#include "CElfilisA_StabCombo.h"
#include "CElfilisA_SlashCombo.h"
#include "CElfilisA_DimensionLaser.h"
#include "CElfilisA_DrawLaser.h"
#include "CElfilisA_Stab.h"
#include "CElfilisA_Teleport.h"
#include "CElfilisA_TeleportCombo.h"
void CElfilisFSM::begin()
{
    CFSMScript::begin();

    // add state
    AddGroupPublicState(ElfilisStateGroup::GroundIdle, L"GROUND_IDLE", new CElfilisG_Idle);
    AddGroupPublicState(ElfilisStateGroup::GroundMove, L"GROUND_MOVE_BACKSTEP", new CElfilisG_BackStep);
    AddGroupPublicState(ElfilisStateGroup::GroundMove, L"GROUND_MOVE_TELEPORT", new CElfilisG_Teleport);
    AddGroupPublicState(ElfilisStateGroup::GroundAtkNear, L"GROUND_ATK_NORMAL", new CElfilisG_NormalAtk);
    AddGroupPublicState(ElfilisStateGroup::GroundAtkFar, L"GROUND_ATK_NORMALTELEPORT", new CElfilisG_NormalAtk);
    AddGroupPublicState(ElfilisStateGroup::GroundAtkFar, L"GROUND_ATK_RAYARROW", new CElfilisG_RayArrow);
    AddGroupPublicState(ElfilisStateGroup::GroundToAir, L"GROUND_TOAIR", new CElfilisG_GroundToAir);
    AddGroupPublicState(ElfilisStateGroup::AirIdle, L"AIR_IDLE", new CElfilisA_Idle);
    AddGroupPublicState(ElfilisStateGroup::AirMove, L"AIR_MOVE_L", new CElfilisA_MoveL);
    AddGroupPublicState(ElfilisStateGroup::AirMove, L"AIR_MOVE_R", new CElfilisA_MoveR);
    AddGroupPublicState(ElfilisStateGroup::AirSmallAtk1, L"AIR_ATKS_RAYARROW_UP", new CElfilisA_RayArrowUp);
    // AddGroupPublicState(ElfilisStateGroup::AirSmallAtk, L"AIR_ATKS_RAYARROW_DOWN", new CElfilisA_RayArrowDown);
    AddGroupPublicState(ElfilisStateGroup::AirSmallAtk1, L"AIR_ATKS_STABCOMBO", new CElfilisA_StabCombo);
    AddGroupPublicState(ElfilisStateGroup::AirSmallAtk1, L"AIR_ATKS_SLASHCOMBO", new CElfilisA_SlashCombo);
    AddGroupPublicState(ElfilisStateGroup::AirSmallAtk2, L"AIR_ATKS_DIMENSIONLASER", new CElfilisA_DimensionLaser);
    AddGroupPublicState(ElfilisStateGroup::AirLargeAtk, L"AIR_ATKL_DRAWLASER", new CElfilisA_DrawLaser);
    AddGroupPublicState(ElfilisStateGroup::AirToGround, L"AIR_TOGROUND_TELEPORT", new CElfilisA_Teleport);
    AddGroupPublicState(ElfilisStateGroup::AirToGround, L"AIR_TOGROUND_TELEPORTCOMBO", new CElfilisA_TeleportCombo);

    AddGroupPrivateState(ElfilisStateGroup::DEMO, L"DEMO_APPEAR1", new CElfilisD_Appear);
    AddGroupPrivateState(ElfilisStateGroup::DEMO, L"DEMO_APPEAR2_DAMAGE", new CElfilisD_Damage);
    AddGroupPrivateState(ElfilisStateGroup::DEMO, L"DEMO_APPEAR2_JUMP", new CElfilisD_Jump);
    AddGroupPrivateState(ElfilisStateGroup::DEMO, L"DEMO_APPEAR2_ROAR", new CElfilisD_Roar);
    AddGroupPrivateState(ElfilisStateGroup::DEMO, L"DEMO_RESIST", new CElfilisD_Resist);
    AddGroupPrivateState(ElfilisStateGroup::DEMO, L"DEMO_RESIST_FAIL", new CElfilisD_ResistFail);
    AddGroupPrivateState(ElfilisStateGroup::DEMO, L"DEMO_RESIST_SUCCESS", new CElfilisD_ResistSuccess);
    AddGroupPrivateState(ElfilisStateGroup::GroundAtkNear, L"GROUND_ATK_NORMAL_L", new CElfilisG_NormalAtkL);
    AddGroupPrivateState(ElfilisStateGroup::GroundAtkNear, L"GROUND_ATK_NORMAL_R", new CElfilisG_NormalAtkR);
    AddGroupPrivateState(ElfilisStateGroup::GroundAtkNear, L"GROUND_ATK_NORMAL_FINISHL", new CElfilisG_NormalAtkFinishL);
    AddGroupPrivateState(ElfilisStateGroup::GroundAtkNear, L"GROUND_ATK_NORMALTELEPORT_L", new CElfilisG_NormalAtkTeleportL);
    AddGroupPrivateState(ElfilisStateGroup::GroundAtkNear, L"GROUND_ATK_NORMALTELEPORT_R", new CElfilisG_NormalAtkTeleportR);
    AddGroupPrivateState(ElfilisStateGroup::GroundAtkNear, L"GROUND_ATK_NORMALTELEPORT_FINISHL", new CElfilisG_NormalAtkTeleportFinishL);
    AddGroupPrivateState(ElfilisStateGroup::GroundAtkFar, L"GROUND_ATK_SWORDWAVE_RL", new CElfilisG_SwordWaveRL);
    AddGroupPrivateState(ElfilisStateGroup::GroundAtkFar, L"GROUND_ATK_SWORDWAVE_LR", new CElfilisG_SwordWaveLR);
    AddGroupPrivateState(ElfilisStateGroup::GroundAtkFar, L"GROUND_ATK_SWORDWAVE_FINISHLR", new CElfilisG_SwordWaveFinishRL);
    AddGroupPrivateState(ElfilisStateGroup::GroundAtkFar, L"GROUND_ATK_SWORDWAVE_STORM", new CElfilisG_SwordWaveStorm);
    // AddGroupPrivateState(ElfilisStateGroup::GroundAtk, L"GROUND_ATK_DIMENSIONSPIKE", new CElfilisG_DimensionSpike);
    AddGroupPrivateState(ElfilisStateGroup::GroundToAir, L"GROUND_TOAIR_TELEPORT", new CElfilisG_ToAirTeleport);
    AddGroupPrivateState(ElfilisStateGroup::AirToGround, L"AIR_TOGROUND_STAB", new CElfilisA_Stab);

    ChangeStateGroup(ElfilisStateGroup::GroundIdle);

    // find Big Elfilis
    wstring strName = GetOwner()->GetName() + L"Big";
    CGameObject* pBossBig = CLevelMgr::GetInst()->GetCurrentLevel()->FindObjectByName(strName);
    CElfilisBigFSM* pBossBigFSM = nullptr;

    if (pBossBig)
    {
        pBossBigFSM = pBossBig->GetScript<CElfilisBigFSM>();
    }

    if (pBossBig && pBossBigFSM)
    {
        m_BigElfilis = pBossBig;
        m_BigElfilisFSM = pBossBigFSM;
    }
    else
    {
        MessageBox(nullptr, L"Big Elfilis�� ã�� �� �����ϴ�", L"Big Elfilis ��� ����", MB_OK);
    }

    // get childs
    m_BodyCollider = GetOwner()->GetChildObject(L"Body Collider")->CapsuleCollider();
    m_Weapon = GetOwner()->GetChildObject(L"Halberd");

    CGameObject* Hitbox = GetOwner()->GetChildObject(L"Hitbox");
    if (Hitbox)
    {
        m_Hitbox = Hitbox->BoxCollider();
        Hitbox->SetActive(false);
    }

    // get mtrl
    for (int i = 0; i < (int)MeshRender()->GetMtrlCount(); ++i)
    {
        m_listBodyMtrl.push_back(MeshRender()->GetMaterial(i));
        m_listBodyEmissive.push_back(MeshRender()->GetMaterial(i)->GetEmission());
        m_listBodyEmissiveTex.push_back(MeshRender()->GetMaterial(i)->GetTexParam(TEX_PARAM::TEX_7));
    }

    if (m_Weapon)
    {
        for (int i = 0; i < (int)m_Weapon->MeshRender()->GetMtrlCount(); ++i)
        {
            m_listWeaponMtrl.push_back(m_Weapon->MeshRender()->GetMaterial(i));
            m_listWeaponEmissive.push_back(m_Weapon->MeshRender()->GetMaterial(i)->GetEmission());
            m_listWeaponEmissiveTex.push_back(MeshRender()->GetMaterial(i)->GetTexParam(TEX_PARAM::TEX_7));
        }

        m_Weapon->BoxCollider()->SetEnabled(false);
    }

    // set map size
    float ScaleFactor = Transform()->GetLocalScale().x;
    m_NearDist *= ScaleFactor;
    m_MapFloorOffset *= ScaleFactor;
    m_MapSizeRadius *= ScaleFactor;
}

void CElfilisFSM::tick()
{
    CFSMScript::tick();

    //if (KEY_TAP(KEY::_0))
    //{
    //    Rigidbody()->SetVelocity(Vec3());
    //    Rigidbody()->SetAngularVelocity(Vec3());
    //    ChangeStateGroup(ElfilisStateGroup::GroundToAir);
    //}
    //if (KEY_TAP(KEY::_9))
    //{
    //    Rigidbody()->SetVelocity(Vec3());
    //    Rigidbody()->SetAngularVelocity(Vec3());
    //    ChangeStateGroup(ElfilisStateGroup::AirToGround);
    //}
    //if (KEY_TAP(KEY::ENTER))
    //{
    //    Rigidbody()->SetVelocity(Vec3());
    //    Rigidbody()->SetAngularVelocity(Vec3());
    //    ChangeStateGroup(ElfilisStateGroup::AirSmallAtk2, L"AIR_ATKS_DIMENSIONLASER");
    //}
}

void CElfilisFSM::OnCollisionEnter(CCollider* _OtherCollider)
{
    int LayerIdx = _OtherCollider->GetOwner()->GetLayerIdx();

    if (m_CurStateGroup == ElfilisStateGroup::AirToGround)
    {
        if (LayerIdx == LAYER_STATIC && _OtherCollider->GetOwner()->GetName() == L"Floor")
        {
            m_bGroundCollision = true;
        }
        else if (LayerIdx == LAYER_PLAYER)
        {
            m_bGroundCollision = true;
        }
    }
}

void CElfilisFSM::SetPattern(ElfilisPatternType _Pattern)
{
    if (m_Pattern != ElfilisPatternType::NONE && _Pattern != ElfilisPatternType::NONE)
        return;

    m_PatternStep = 0;
    m_Pattern = _Pattern;
}

void CElfilisFSM::ProcPatternStep()
{
    if (m_Pattern == ElfilisPatternType::NONE)
        return;

    bool bFinish = false;

    switch (m_Pattern)
    {
    case ElfilisPatternType::Appear1: {
        if (m_PatternStep == 0)
        {
            ChangeStateGroup_Set(ElfilisStateGroup::GroundMove, L"GROUND_MOVE_TELEPORT");
            Animator()->Play(ANIMPREFIX("Wait"), false, false, 1.5f, 0.f);
            Animator()->SetClipFrameIndex(1);
        }
        else if (m_PatternStep == 1)
        {
            ChangeStateGroup_Set(ElfilisStateGroup::GroundMove, L"GROUND_MOVE_TELEPORT");
        }
        else if (m_PatternStep == 2)
        {
            ChangeStateGroup_Set(ElfilisStateGroup::GroundAtkNear, L"GROUND_ATK_NORMALTELEPORT_L");
            bFinish = true;
        }
    }
    break;
    case ElfilisPatternType::Appear2: {
        if (m_PatternStep == 0)
        {
            ChangeStateGroup_Random(ElfilisStateGroup::AirIdle);
        }
        else if (m_PatternStep == 1) // ���� : �ܺ�ȣ��
        {
            ChangeStateGroup_Set(ElfilisStateGroup::AirSmallAtk1, L"AIR_ATKS_RAYARROW_UP");
        }
        else if (m_PatternStep == 2)
        {
            ChangeStateGroup_Random(ElfilisStateGroup::AirIdle);
        }
        else if (m_PatternStep == 3) // ���� : �ܺ�ȣ��
        {
            ChangeStateGroup_Set(ElfilisStateGroup::AirToGround, L"AIR_TOGROUND_STAB");
            bFinish = true;
        }
    }
    break;
    case ElfilisPatternType::BigCombo: {
        if (m_PatternStep == 0)
        {
            ChangeStateGroup_Random(ElfilisStateGroup::GroundIdle);
        }
        else if (m_PatternStep == 1) // ���� : �ܺ�ȣ��
        {
            ChangeStateGroup_Set(ElfilisStateGroup::GroundToAir, L"GROUND_TOAIR_TELEPORT");
        }
        else if (m_PatternStep == 2)
        {
            ChangeStateGroup_Set(ElfilisStateGroup::AirIdle, L"AIR_IDLE");
        }
        else if (m_PatternStep == 3) // ���� : �ܺ�ȣ��
        {
            ChangeStateGroup_Set(ElfilisStateGroup::AirSmallAtk1, L"AIR_ATKS_RAYARROW_UP");
        }
        else if (m_PatternStep == 4)
        {
            ChangeStateGroup_Set(ElfilisStateGroup::AirSmallAtk1, L"AIR_ATKS_RAYARROW_UP");
        }
        else if (m_PatternStep == 5)
        {
            ChangeStateGroup_Random(ElfilisStateGroup::AirIdle);
        }
        else if (m_PatternStep == 6) // ���� : �ܺ�ȣ��
        {
            ChangeStateGroup_Set(ElfilisStateGroup::AirToGround, L"AIR_TOGROUND_STAB");
            bFinish = true;
        }
    }
    break;
    case ElfilisPatternType::StabCombo: {
        if (m_PatternStep == 0)
        {
            ChangeStateGroup_Set(ElfilisStateGroup::AirToGround, L"AIR_TOGROUND_STAB");
        }
        else if (m_PatternStep == 1)
        {
            ChangeStateGroup_Set(ElfilisStateGroup::GroundToAir, L"GROUND_TOAIR_TELEPORT");
        }
        else if (m_PatternStep == 2)
        {
            ChangeStateGroup_Set(ElfilisStateGroup::AirToGround, L"AIR_TOGROUND_STAB");
            bFinish = true;
        }
    }
    break;
    }

    if (bFinish)
    {
        SetPattern(ElfilisPatternType::NONE);
    }
    else
    {
        m_PatternStep++;
    }
}

void CElfilisFSM::ResetEmissive()
{
    for (int i = 0; i < m_listBodyMtrl.size(); ++i)
    {
        m_listBodyMtrl[i]->SetEmission(Vec4(m_listBodyEmissive[i], 0.f));
        m_listBodyMtrl[i]->SetTexParam(TEX_PARAM::TEX_7, m_listBodyEmissiveTex[i]);
    }

    for (int i = 0; i < m_listWeaponMtrl.size(); ++i)
    {
        m_listWeaponMtrl[i]->SetEmission(Vec4(m_listWeaponEmissive[i], 0.f));
        m_listWeaponMtrl[i]->SetTexParam(TEX_PARAM::TEX_7, m_listWeaponEmissiveTex[i]);
    }
}

void CElfilisFSM::EnableCollider()
{
    CapsuleCollider()->SetEnabled(true);

    if (m_BodyCollider)
    {
        m_BodyCollider->SetEnabled(true);
    }

    OnWeaponTrigger();
}

void CElfilisFSM::DisableCollider()
{
    CapsuleCollider()->SetEnabled(false);

    if (m_BodyCollider)
    {
        m_BodyCollider->SetEnabled(false);
    }

    OffWeaponTrigger();
}

void CElfilisFSM::ReleaseDynamicMtrl()
{
    if (!MeshRender())
        return;

    Vec4 TeleportInfo;

    // set body shader
    int BodyMtrlCount = MeshRender()->GetMtrlCount();
    for (int i = 0; i < BodyMtrlCount; ++i)
    {
        MeshRender()->GetSharedMaterial(i)->SetScalarParam(SCALAR_PARAM::VEC4_3, TeleportInfo);
    }

    // set weapon shader
    if (m_Weapon && m_Weapon->MeshRender())
    {
        m_Weapon->MeshRender()->GetSharedMaterial(0)->SetScalarParam(SCALAR_PARAM::VEC4_3, TeleportInfo);
    }
}

void CElfilisFSM::AddEmissive(Vec3 _Color)
{
    for (int i = 0; i < m_listBodyMtrl.size(); ++i)
    {
        Vec4 Color = Vec4(m_listBodyEmissive[i] + _Color, 0.f);
        m_listBodyMtrl[i]->SetEmission(Color);
        m_listBodyMtrl[i]->SetTexParam(TEX_PARAM::TEX_7, nullptr);
    }

    for (int i = 0; i < m_listWeaponMtrl.size(); ++i)
    {
        Vec4 Color = Vec4(m_listWeaponEmissive[i] + _Color, 0.f);
        m_listWeaponMtrl[i]->SetEmission(Color);
        m_listWeaponMtrl[i]->SetTexParam(TEX_PARAM::TEX_7, nullptr);
    }
}

void CElfilisFSM::OnWeaponTrigger()
{
    if (!m_Weapon)
        return;

    m_Weapon->BoxCollider()->SetEnabled(true);
}

void CElfilisFSM::OffWeaponTrigger()
{
    if (!m_Weapon)
        return;

    m_Weapon->BoxCollider()->SetEnabled(false);
}

void CElfilisFSM::Teleport(CGameObject* _TeleportObject, UINT _Flag, float _WorldY)
{
    if (!_TeleportObject->MeshRender() || _Flag > 2)
        return;

    Vec4 TeleportInfo{(float)_Flag, _WorldY, 15.f, 0.f};
    
    // set body shader
    int BodyMtrlCount = _TeleportObject->MeshRender()->GetMtrlCount();
    for (int i = 0; i < BodyMtrlCount; ++i)
    {
        _TeleportObject->MeshRender()->GetDynamicMaterial(i)->SetScalarParam(SCALAR_PARAM::VEC4_3, TeleportInfo);
    }

    // set weapon shader
    CGameObject* Weapon = _TeleportObject->GetChildObject(L"Halberd");
    if (Weapon && Weapon->MeshRender())
    {
        Weapon->MeshRender()->GetDynamicMaterial(0)->SetScalarParam(SCALAR_PARAM::VEC4_3, TeleportInfo);
    }
}

const vector<wstring>& CElfilisFSM::GetCurPublicStates() const
{
    map<ElfilisStateGroup, vector<wstring>[2]>::const_iterator it = m_StateGroup.find(m_CurStateGroup);
    if (it != m_StateGroup.end())
    {
        return it->second[0];
    }
    else
    {
        MessageBox(nullptr, L"���� State Group�� ã�� �� �����ϴ�!", L"Failed to CElfilisFSM::GetCurPublicStates()", MB_OK);
        static vector<wstring> ret = vector<wstring>();
        return ret;
    }
}

float CElfilisFSM::GetPlayerDist() const
{
    Vec3 Dist = PLAYER->Transform()->GetWorldPos() - GetOwner()->Transform()->GetWorldPos();
    Dist.y = 0.f;
    return Dist.Length();
}

void CElfilisFSM::SetStateGroup(ElfilisStateGroup _Group)
{
    if (m_CurStateGroup == _Group)
        return;

    // check count
    switch (_Group)
    {
    case ElfilisStateGroup::GroundIdle: {
        ClearComboLevel();
        m_bAttackRepeat = false;
        m_GroundAttackCount = 0;
    }
    break;
    case ElfilisStateGroup::GroundMove:
    case ElfilisStateGroup::GroundMoveAtk: {
        ClearComboLevel();
        m_bAttackRepeat = false;
    }
    break;
    case ElfilisStateGroup::GroundAtkNear:
    case ElfilisStateGroup::GroundAtkFar: {
        ClearComboLevel();
        m_bAttackRepeat = false;
        m_GroundAttackCount++;
    }
    break;
    case ElfilisStateGroup::GroundToAir: {
        ClearComboLevel();
        m_bAttackRepeat = false;
        m_GroundAttackCount = 0;
    }
    break;
    case ElfilisStateGroup::AirIdle:
    case ElfilisStateGroup::AirMove:
    case ElfilisStateGroup::AirSmallAtk1:
    case ElfilisStateGroup::AirSmallAtk2:
    case ElfilisStateGroup::AirLargeAtk:
        ClearComboLevel();
        m_bAttackRepeat = false;
        break;
    case ElfilisStateGroup::AirToGround: {
        ClearComboLevel();
        m_bAttackRepeat = false;
        m_GroundAttackCount = 0;
        m_bGroundCollision = false;
    }
    break;
    }

    m_CurStateGroup = _Group;
}

void CElfilisFSM::AddGroupPublicState(ElfilisStateGroup _Group, const wstring& _StateName, CState* _State)
{
    // �̹� �����ϴ� Group���� �߰�
    if (m_StateGroup.find(_Group) == m_StateGroup.cend())
        return;

    CFSMScript::AddState(_StateName, _State);
    m_StateGroup[_Group][0].push_back(_StateName);
    m_StateSelectionCount[(int)_Group][_StateName] = 0;
}

void CElfilisFSM::AddGroupPrivateState(ElfilisStateGroup _Group, const wstring& _StateName, CState* _State)
{
    // �̹� �����ϴ� Group���� �߰�
    if (m_StateGroup.find(_Group) == m_StateGroup.cend())
        return;

    CFSMScript::AddState(_StateName, _State);
    m_StateGroup[_Group][1].push_back(_StateName);
}

UINT CElfilisFSM::SaveToLevelFile(FILE* _File)
{
    UINT MemoryByte = 0;
    MemoryByte += CFSMScript::SaveToLevelFile(_File);

    return MemoryByte;
}

UINT CElfilisFSM::LoadFromLevelFile(FILE* _File)
{
    UINT MemoryByte = 0;
    MemoryByte += CFSMScript::SaveToLevelFile(_File);

    return MemoryByte;
}
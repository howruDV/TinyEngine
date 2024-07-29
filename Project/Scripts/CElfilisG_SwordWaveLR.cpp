#include "pch.h"
#include "CElfilisG_SwordWaveLR.h"
#include "CElfilisFSM.h"
#include "CElfilisSwordSlashScript.h"

CElfilisG_SwordWaveLR::CElfilisG_SwordWaveLR()
    : m_bComboSuccess(false)
    , m_bFrmEnter(true)
{
    m_SwordSlash = CAssetMgr::GetInst()->Load<CPrefab>(L"prefab\\ElfilisSwordSlash.pref", L"prefab\\ElfilisSwordSlash.pref");
}

CElfilisG_SwordWaveLR::~CElfilisG_SwordWaveLR()
{
}

void CElfilisG_SwordWaveLR::tick()
{
    switch (m_Step)
    {
    case StateStep::Start: {
        Start();
    }
    break;
    case StateStep::Wait: {
        Wait();
    }
    break;
    case StateStep::Progress: {
        Progress();
    }
    break;
    case StateStep::End: {
        End();
    }
    break;
    }
}

void CElfilisG_SwordWaveLR::Enter()
{
    if (ELFFSM->GetComboLevel() == 0)
    {
        m_Step = StateStep::Start;
    }
    else
    {
        m_Step = StateStep::Wait;
    }

    Enter_Step();
}

void CElfilisG_SwordWaveLR::Enter_Step()
{
    switch (m_Step)
    {
    case StateStep::Start: {
        GetOwner()->Animator()->Play(ANIMPREFIX("SwingRightStart"), false, false, 2.5f, 0.5f);
    }
    break;
    case StateStep::Wait: {
        GetOwner()->Animator()->Play(ANIMPREFIX("ChainLR"), false, false, 1.5f, 0.5f);
    }
    break;
    case StateStep::Progress: {
        GetOwner()->Animator()->Play(ANIMPREFIX("SwingRight"), false);
        m_bFrmEnter = true;
    }
    break;
    case StateStep::End: {
        GetOwner()->Animator()->Play(ANIMPREFIX("SwingRightEnd"), false, false, 1.5f);
    }
    break;
    }
}

void CElfilisG_SwordWaveLR::Exit_Step()
{
    switch (m_Step)
    {
    case StateStep::Start:
        break;
    case StateStep::Wait:
        break;
    case StateStep::Progress: {
        if (m_bComboSuccess)
        {
            ELFFSM->AddComboLevel();
        }
        else
        {
            ELFFSM->SetComboLevel(0);
        }
    }
    break;
    case StateStep::End:
        break;
    }
}

void CElfilisG_SwordWaveLR::Start()
{    
    // rotate
    RotateToPlayer();

    if (GetOwner()->Animator()->IsFinish())
    {
        ChangeStep(StateStep::Wait);
    }
}

void CElfilisG_SwordWaveLR::Wait()
{
    if (GetOwner()->Animator()->IsFinish())
    {
        ChangeStep(StateStep::Progress);
    }
}

void CElfilisG_SwordWaveLR::Progress()
{ 
    // Spawn SwordSlash
    if (m_bFrmEnter && GetOwner()->Animator()->GetClipFrameIndex() >= 12)
    {
        if (m_SwordSlash != nullptr)
        {
            CGameObject* pSlash = m_SwordSlash->Instantiate();
            CElfilisSwordSlashScript* pScript = pSlash->GetScript<CElfilisSwordSlashScript>();

            if (pScript)
            {
                Vec3 InitPos = GetOwner()->Transform()->GetWorldPos();
                InitPos += GetOwner()->Transform()->GetWorldDir(DIR_TYPE::FRONT) * 30.f;

                pSlash->Transform()->SetWorldPos(InitPos);
                pScript->SetInitDir(GetOwner()->Transform()->GetWorldDir(DIR_TYPE::FRONT));
            }

            GamePlayStatic::SpawnGameObject(pSlash, LAYER_MONSTERATK);
        }

        m_bFrmEnter = false;
    }

    if (GetOwner()->Animator()->IsFinish())
    {
        // Check Combo
        float Rand = 0.f;
        Rand = GetRandomfloat(1.f, 100.f);

        if (Rand <= 70.f)
        {
            m_bComboSuccess = true;
            ELFFSM->ChangeStateGroup_SetState(ElfilisStateGroup::GroundAtk, L"GROUND_ATK_SWORDWAVE_FINISHLR");
        }
        else
        {
            ChangeStep(StateStep::End);
        }
    }
}

void CElfilisG_SwordWaveLR::End()
{
    if (GetOwner()->Animator()->IsFinish())
    {
        ElfilisStateGroup NextState = ELFFSM->FindNextStateGroup();

        if (NextState == ELFFSM->GetCurStateGroup())
        {
            ELFFSM->RepeatState(L"GROUND_ATK_SWORDWAVE_RL");
        }
        else
        {
            ELFFSM->ChangeStateGroup_RandState(NextState);
        }
    }
}
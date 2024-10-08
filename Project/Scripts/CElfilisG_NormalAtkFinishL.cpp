#include "pch.h"
#include "CElfilisG_NormalAtkFinishL.h"
#include "CElfilisFSM.h"
#include "CCameraController.h"

CElfilisG_NormalAtkFinishL::CElfilisG_NormalAtkFinishL()
    : m_bFrmEnter(true)
{
}

CElfilisG_NormalAtkFinishL::~CElfilisG_NormalAtkFinishL()
{
}

void CElfilisG_NormalAtkFinishL::tick()
{
    switch (m_Step)
    {
    case StateStep::Start: {
        Start();
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

void CElfilisG_NormalAtkFinishL::Exit()
{
    Exit_Step();
    ELFFSM->OffWeaponTrigger();
    ELFFSM->ClearComboLevel();
}

void CElfilisG_NormalAtkFinishL::Enter_Step()
{
    switch (m_Step)
    {
    case StateStep::Start: {
        GetOwner()->Animator()->Play(ANIMPREFIX("SwingLeftStart"), false, false, 1.5f, 0.5f);

        // sound
        wstring Start = L"sound\\wav\\CharaBossChimera2\\0002_SwingStart.wav";
        GamePlayStatic::Play2DSound(Start, 1, SOUND_ELFILIS * 0.7f);
    }
    break;
    case StateStep::Progress: {
        GetOwner()->Animator()->Play(ANIMPREFIX("SwingFinishLeft"), false, false, 1.5f);
        ELFFSM->OnWeaponTrigger();
        m_bFrmEnter = true;

        // sound
        wstring Start = L"sound\\wav\\CharaBossChimera2\\0005_Swing.wav";
        GamePlayStatic::Play2DSound(Start, 1, SOUND_ELFILIS * 0.8f);
    }
    break;
    case StateStep::End: {
        GetOwner()->Animator()->Play(ANIMPREFIX("SwingFinishLeftEnd"), false, false, 1.5f);
    }
    break;
    }
}

void CElfilisG_NormalAtkFinishL::Exit_Step()
{
    switch (m_Step)
    {
    case StateStep::Start:
        break;
    case StateStep::Progress: {
        ELFFSM->OffWeaponTrigger();

        // Spawn DropStar
        Vec3 SpawnPos = GetOwner()->Transform()->GetWorldPos();

        for (int i = 0; i < 5; ++i)
        {
            float Rot = XMConvertToRadians(360.f / 5.f * i);
            Vec3 RotSpawnPos;
            RotSpawnPos.x = cosf(Rot) - sinf(Rot);
            RotSpawnPos.z = sinf(Rot) + cosf(Rot);
            RotSpawnPos.Normalize();

            RotSpawnPos = SpawnPos + RotSpawnPos * 100.f;
            ELFFSM->SpawnDropStar(RotSpawnPos);
        }
    }
    break;
    case StateStep::End:
        break;
    }
}

void CElfilisG_NormalAtkFinishL::Start()
{
    // rotate
    RotateToPlayer();

    if (GetOwner()->Animator()->IsFinish())
    {
        ChangeStep(StateStep::Progress);
    }
}

void CElfilisG_NormalAtkFinishL::Progress()
{
    // resize Hitbox
    CBoxCollider* pHitbox = ELFFSM->GetHitbox();
    if (pHitbox && !GetOwner()->Animator()->IsChainging())
    {
        if (GetOwner()->Animator()->GetClipFrameIndex() > 32 && GetOwner()->Animator()->GetClipFrameIndex() < 42)
        {
            if (m_bFrmEnter)
            {
                m_bFrmEnter = false;

                pHitbox->GetOwner()->SetActive(true);
                pHitbox->Transform()->SetLocalPos(Vec3(0.f, 1.f, 0.f));
                pHitbox->Transform()->SetLocalRotation(Vec3(0.f));
                pHitbox->Transform()->SetLocalScale(Vec3(7.5f, 1.f, 7.5f));

                CAMERACTRL->Shake(0.3f, 30.f, 30.f);
            }
        }
        else
        {
            pHitbox->GetOwner()->SetActive(false);
        }
    }

    if (GetOwner()->Animator()->IsFinish())
    {
        ChangeStep(StateStep::End);
    }
}

void CElfilisG_NormalAtkFinishL::End()
{
    if (GetOwner()->Animator()->IsFinish())
    {
        ElfilisStateGroup NextState = ELFFSM->FindNextStateGroup();

        if (NextState == ELFFSM->GetCurStateGroup())
        {
            ELFFSM->RepeatState(L"GROUND_ATK_NORMAL");
        }
        else
        {
            ELFFSM->ChangeStateGroup(NextState);
        }
    }
}
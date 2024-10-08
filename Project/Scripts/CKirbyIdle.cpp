#include "pch.h"
#include "CKirbyIdle.h"

CKirbyIdle::CKirbyIdle()
    : m_WaitingTime(0.f)
    , m_PrevAnim(L"")
    , m_bPlayAddMotion(false)
    , m_EmotionType(EmotionType::NONE)
{
    m_DefaultWaitAddAnim.push_back(L"WaitSit");
    m_DefaultWaitAddAnim.push_back(L"WaitStreach");
    m_DefaultWaitAddAnim.push_back(L"WaitVariationLookAround");
    m_DefaultWaitAddAnim.push_back(L"WaitVariationLookAround2");
    m_DefaultWaitAddAnim.push_back(L"WaitVariationStretch");
    m_DefaultWaitAddAnim.push_back(L"WaitVariationWaryAbout");
    m_DefaultWaitAddAnim.push_back(L"ListenMusic");
}

CKirbyIdle::~CKirbyIdle()
{
}

void CKirbyIdle::tick()
{
    // 감정표현
    if (PLAYERFSM->GetCurObjectIdx() == ObjectCopyType::NONE && m_EmotionType == EmotionType::NONE)
    {
        if (KEY_TAP(KEY::_1))
        {
            GamePlayStatic::Play2DSound(L"sound\\wav\\HeroVoice\\0057.wav",1,KIRBY_EFFECTSOUND);

            m_EmotionType = EmotionType::WaveHand;

            GetOwner()->Animator()->Play(ANIMPREFIX("WaveTwohands"), true, false, 2.5f, 0.3f);
            CPlayerMgr::ClearMouthMtrl();
            CPlayerMgr::SetPlayerMtrl(PLAYERMESH(MouthSmileOpen));
        }
        else if (KEY_TAP(KEY::_2))
        {
            m_EmotionType = EmotionType::Sit_Start;

            GetOwner()->Animator()->Play(ANIMPREFIX("SitStart"), false, false, 2.0f);
            CPlayerMgr::ClearMouthMtrl();
            CPlayerMgr::SetPlayerMtrl(PLAYERMESH(MouthSmileClose));
        }
        else if (KEY_TAP(KEY::_3))
        {
            m_EmotionType = EmotionType::Yay;

            m_YaySound = false;

            GetOwner()->Animator()->Play(ANIMPREFIX("WaitYay"), false, false, 2.0f, 0.3f);
            CPlayerMgr::ClearMouthMtrl();
            CPlayerMgr::SetPlayerMtrl(PLAYERMESH(MouthSmileOpen));
            CPlayerMgr::SetPlayerFace(FaceType::Frown);
        }

        if (m_EmotionType != EmotionType::NONE)
        {
            m_PrevAnim = ANIMPREFIX("Wait");
            m_WaitingTime = 0.f;
        }
    }
    else if (m_EmotionType != EmotionType::NONE)
    {
        bool bFinish = tick_Emotion();

        if (bFinish)
        {
            GetOwner()->Animator()->Play(m_PrevAnim, true, false, 2.5f, 0.3f);
            CPlayerMgr::ClearMouthMtrl();
            CPlayerMgr::SetPlayerMtrl(PLAYERMESH(MouthNormal));
            CPlayerMgr::SetPlayerFace(FaceType::Normal);

            m_WaitingTime = GetRandomfloat(3.f, 15.f);
            m_EmotionType = EmotionType::NONE;
            m_bPlayAddMotion = false;
        }
    }

    // 추가 대기모션 재생
    if (m_EmotionType == EmotionType::NONE)
    {
        if (!m_bPlayAddMotion)
        {
            if (m_WaitingTime > 0.f)
            {
                m_WaitingTime -= DT;
            }

            if (m_WaitingTime <= 0.f)
            {
                PlayWaitingAnim();
                m_bPlayAddMotion = true;
            }
        }
        else
        {
            if (GetOwner()->Animator()->IsFinish())
            {
                GetOwner()->Animator()->Play(m_PrevAnim, true, false, 2.5f, 0.3f);

                if (PLAYERFSM->GetCurObjectIdx() == ObjectCopyType::NONE)
                {
                    // change player face
                    CPlayerMgr::ClearMouthMtrl();
                    CPlayerMgr::SetPlayerMtrl(PLAYERMESH(MouthNormal));
                }

                m_WaitingTime = GetRandomfloat(3.f, 15.f);
                m_bPlayAddMotion = false;
            }
        }
    }

    // 각 State별로 수행할 작업 (상태전환 제외)
    PLAY_CURSTATE(Idle)

    // State Change
    if (PLAYERFSM->GetCurObjectIdx() != ObjectCopyType::NONE)
    {
        switch (PLAYERFSM->GetCurObjectIdx())
        {
        case ObjectCopyType::CONE: {
            if (KEY_TAP(KEY_ATK))
            {
                ChangeState(L"ATTACK_START");
            }
            else if ((KEY_TAP(KEY_JUMP) || (KEY_PRESSED(KEY_JUMP))) && (KEY_NONE(KEY_GUARD) || KEY_RELEASED(KEY_GUARD)))
            {
                ChangeState(L"JUMP_START");
            }
            else if (PLAYERFSM->GetYPressedTime() >= PLAYERFSM->GetDropCopyTime())
            {
                ChangeState(L"DROP_OBJECT_START");
            }
            else if (PLAYERCTRL->GetInput().Length() != 0.f)
            {
                ChangeState(L"RUN");
            }
            else if (!PLAYERCTRL->IsGround())
            {
                ChangeState(L"JUMP_FALL");
            }
        }
        break;
        case ObjectCopyType::VENDING_MACHINE: {
            if (KEY_TAP(KEY_ATK) || KEY_PRESSED(KEY_ATK))
            {
                UINT LeftCanCount = PLAYERFSM->GetCanCount();

                if (LeftCanCount > 0)
                {
                    if (KEY_NONE_ARROW)
                    {
                        ChangeState(L"ATTACK_START");
                    }
                    else
                    {
                        ChangeState(L"ATTACK_CHARGE1");
                    }
                }
                else
                {
                    ChangeState(L"ATTACK_FAILED");
                }
            }
            else if ((KEY_TAP(KEY_JUMP) || (KEY_PRESSED(KEY_JUMP))) && (KEY_NONE(KEY_GUARD) || KEY_RELEASED(KEY_GUARD)))
            {
                ChangeState(L"JUMP_START");
            }
            else if (PLAYERFSM->GetYPressedTime() >= PLAYERFSM->GetDropCopyTime())
            {
                ChangeState(L"DROP_OBJECT_START");
            }
            else if (PLAYERCTRL->GetInput().Length() != 0.f)
            {
                ChangeState(L"RUN");
            }
            else if (!PLAYERCTRL->IsGround())
            {
                ChangeState(L"JUMP_FALL");
            }
        }
        break;
        case ObjectCopyType::LIGHT: {
            if (KEY_TAP(KEY_ATK) && !PLAYERFSM->IsAttackEvent())
            {
                ChangeState(L"ATTACK");
            }
            else if ((KEY_RELEASED(KEY_ATK) || KEY_NONE(KEY_ATK)) && PLAYERFSM->IsAttackEvent())
            {
                ChangeState(L"ATTACK_END");
            }
            else if ((KEY_TAP(KEY_JUMP) || (KEY_PRESSED(KEY_JUMP))) && (KEY_NONE(KEY_GUARD) || KEY_RELEASED(KEY_GUARD)))
            {
                ChangeState(L"JUMP_START");
            }
            else if (PLAYERFSM->GetYPressedTime() >= PLAYERFSM->GetDropCopyTime())
            {
                ChangeState(L"DROP_OBJECT");
            }
            else if (PLAYERCTRL->GetInput().Length() != 0.f)
            {
                ChangeState(L"RUN");
            }
            else if (!PLAYERCTRL->IsGround())
            {
                ChangeState(L"JUMP_FALL");
            }
        }
        break;
        }
    }
    else
    {
        switch (PLAYERFSM->GetCurAbilityIdx())
        {
        case AbilityCopyType::NORMAL: {
            if (KEY_TAP(KEY_ATK))
            {
                ChangeState(L"VACUUM1_START");
            }
            else if ((KEY_TAP(KEY_JUMP) || (KEY_PRESSED(KEY_JUMP))) && (KEY_NONE(KEY_GUARD) || KEY_RELEASED(KEY_GUARD)))
            {
                ChangeState(L"JUMP_START");
            }
            else if (KEY_TAP(KEY_GUARD) || KEY_PRESSED(KEY_GUARD))
            {
                ChangeState(L"GUARD");
            }
            else if (PLAYERFSM->GetYPressedTime() >= PLAYERFSM->GetDropCopyTime())
            {
                ChangeState(L"DROP_ABILITY");
            }
            else if (PLAYERCTRL->GetInput().Length() != 0.f)
            {
                ChangeState(L"RUN_START");
            }
            else if (!PLAYERCTRL->IsGround())
            {
                ChangeState(L"JUMP_FALL");
            }
        }
        break;
        case AbilityCopyType::FIRE: {
            if (KEY_TAP(KEY_ATK) && PLAYERFSM->IsNearDeformObject())
            {
                ChangeState(L"VACUUM1_START");
            }
            else if (KEY_TAP(KEY_ATK))
            {
                ChangeState(L"ATTACK_CHARGE1_START");
            }
            else if ((KEY_TAP(KEY_JUMP) || (KEY_PRESSED(KEY_JUMP))) && (KEY_NONE(KEY_GUARD) || KEY_RELEASED(KEY_GUARD)))
            {
                ChangeState(L"JUMP_START");
            }
            else if (KEY_TAP(KEY_GUARD) || KEY_PRESSED(KEY_GUARD))
            {
                ChangeState(L"GUARD");
            }
            else if (PLAYERFSM->GetYPressedTime() >= PLAYERFSM->GetDropCopyTime())
            {
                ChangeState(L"DROP_ABILITY");
            }
            else if (PLAYERCTRL->GetInput().Length() != 0.f)
            {
                ChangeState(L"RUN_START");
            }
            else if (!PLAYERCTRL->IsGround())
            {
                ChangeState(L"JUMP_FALL");
            }
        }
        break;
        case AbilityCopyType::CUTTER: {
            if (KEY_TAP(KEY_ATK) && PLAYERFSM->IsNearDeformObject())
            {
                ChangeState(L"VACUUM1_START");
            }
            else if (KEY_TAP(KEY_ATK))
            {
                Vec3 RayStart = PLAYER->Transform()->GetWorldPos() + Vec3(0.f, 15.f, 0.f);
                RaycastHit Hit = CPhysicsMgr::GetInst()->RayCast(RayStart, PLAYER->Transform()->GetWorldDir(DIR_TYPE::FRONT), 100.f, {L"Monster"});
                if (Hit.pCollisionObj != nullptr)
                {
                    ChangeState(L"ATTACK_COMBO1");
                }
                else if (PLAYERFSM->CanBladeAttack())
                {
                    ChangeState(L"ATTACK");
                }
            }
            else if (PLAYERFSM->GetYPressedTime() >= PLAYERFSM->GetDropCopyTime())
            {
                ChangeState(L"DROP_ABILITY");
            }
            else if ((KEY_TAP(KEY_JUMP) || (KEY_PRESSED(KEY_JUMP))) && (KEY_NONE(KEY_GUARD) || KEY_RELEASED(KEY_GUARD)))
            {
                ChangeState(L"JUMP_START");
            }
            else if (KEY_TAP(KEY_GUARD) || KEY_PRESSED(KEY_GUARD))
            {
                ChangeState(L"GUARD");
            }
            else if (PLAYERCTRL->GetInput().Length() != 0.f)
            {
                ChangeState(L"RUN_START");
            }
            else if (!PLAYERCTRL->IsGround())
            {
                ChangeState(L"JUMP_FALL");
            }
        }
        break;
        case AbilityCopyType::SWORD: {
            if (KEY_TAP(KEY_ATK) && PLAYERFSM->IsNearDeformObject())
            {
                ChangeState(L"VACUUM1_START");
            }
            else if (KEY_TAP(KEY_ATK) || KEY_PRESSED(KEY_ATK))
            {
                UINT Combo = PLAYERFSM->GetComboLevel();

                if (Combo == 0)
                    ChangeState(L"ATTACK");
                else if (Combo == 1)
                    ChangeState(L"ATTACK_COMBO1");
                else if (Combo == 2)
                    ChangeState(L"ATTACK_COMBO2");
            }
            // else if (KEY_PRESSED(KEY_ATK))
            //{
            //     ChangeState(L"ATTACK_CHARGE1_START");
            // }
            else if ((KEY_TAP(KEY_JUMP) || (KEY_PRESSED(KEY_JUMP))) && (KEY_NONE(KEY_GUARD) || KEY_RELEASED(KEY_GUARD)))
            {
                ChangeState(L"JUMP_START");
            }
            else if (KEY_TAP(KEY_GUARD) || KEY_PRESSED(KEY_GUARD))
            {
                ChangeState(L"GUARD");
            }
            else if (PLAYERFSM->GetYPressedTime() >= PLAYERFSM->GetDropCopyTime())
            {
                ChangeState(L"DROP_ABILITY");
            }
            else if (PLAYERCTRL->GetInput().Length() != 0.f)
            {
                ChangeState(L"RUN_START");
            }
            else if (!PLAYERCTRL->IsGround())
            {
                ChangeState(L"JUMP_FALL");
            }
        }
        break;
        case AbilityCopyType::SLEEP: {
            if (KEY_TAP(KEY_ATK) && PLAYERFSM->IsNearDeformObject())
            {
                ChangeState(L"VACUUM1_START");
            }
            else if (KEY_TAP(KEY_ATK))
            {
                ChangeState(L"ATTACK_START");
            }
            else if (PLAYERFSM->GetYPressedTime() >= PLAYERFSM->GetDropCopyTime())
            {
                ChangeState(L"DROP_ABILITY");
            }
            else if (PLAYERCTRL->GetInput().Length() != 0.f)
            {
                ChangeState(L"RUN");
            }
            else if (!PLAYERCTRL->IsGround())
            {
                ChangeState(L"JUMP_FALL");
            }
        }
        break;
        }
    }
}

void CKirbyIdle::Enter()
{
    PLAY_CURSTATE(IdleEnter)

    PLAYERCTRL->LockMove();
    PLAYERFSM->SetDroppable(true);

    m_WaitingTime = GetRandomfloat(3.f, 15.f);
    m_EmotionType = EmotionType::NONE;
    m_bPlayAddMotion = false;
}

void CKirbyIdle::Exit()
{
    PLAY_CURSTATE(IdleExit)

    PLAYERCTRL->UnlockMove();
    PLAYERFSM->SetDroppable(false);
}

void CKirbyIdle::PlayWaitingAnim()
{
    // find prev anim name
    const vector<tMTAnimClip>* vecAnimClip = GetOwner()->Animator()->GetSkeletalMesh()->GetAnimClip();
    int CurClipIdx = GetOwner()->Animator()->GetCurClipIdx();
    const tMTAnimClip& CurClip = vecAnimClip->at(CurClipIdx);
    m_PrevAnim = CurClip.strAnimName;

    // select & play waiting anim
    CKirbyFSM* KirbyFSM = CPlayerMgr::GetPlayerFSM();

    if (KirbyFSM->GetCurObject())
    {
        int animCount = (int)KirbyFSM->GetCurObject()->GetWaitingAnim().size();
        if (animCount == 0)
            return;

        int animIdx = GetRandomInt(0, animCount - 1);
        GetOwner()->Animator()->Play(ANIMPREFIX_STR + KirbyFSM->GetCurObject()->GetWaitingAnim()[animIdx], false, false, 2.f);
    }
    else
    {
        int animIdx = GetRandomInt(0, (int)m_DefaultWaitAddAnim.size() - 1);
        GetOwner()->Animator()->Play(ANIMPREFIX_STR + m_DefaultWaitAddAnim[animIdx], false, false, 2.f);

        // change player face
        CPlayerMgr::ClearMouthMtrl();
        CPlayerMgr::SetPlayerMtrl(PLAYERMESH(MouthSmileClose));
    }
}

bool CKirbyIdle::tick_Emotion()
{
    switch (m_EmotionType)
    {
    case EmotionType::WaveHand: {
        m_WaitingTime += DT;

        if (m_WaitingTime >= 1.f)
        {
            return true;
        }
    }
    break;
    case EmotionType::Sit: {
        m_WaitingTime += DT;

        if (m_WaitingTime >= 2.f)
        {
            GetOwner()->Animator()->Play(ANIMPREFIX("SitEnd"), false, false, 2.0f);
            m_EmotionType = EmotionType::Sit_End;
        }
    }
    break;
    case EmotionType::Sit_Start: {
        if (GetOwner()->Animator()->IsFinish())
        {
            GetOwner()->Animator()->Play(ANIMPREFIX("Sit"), true, false, 2.0f);
            m_EmotionType = EmotionType::Sit;
        }
    }
    break;
    case EmotionType::Sit_End: {
        if (GetOwner()->Animator()->IsFinish())
        {
            return true;
        }
    }
    break;
    case EmotionType::Yay: {
        if (CHECK_ANIMFRM(PLAYER, 45) && m_YaySound == false)
        {
            GamePlayStatic::Play2DSound(L"sound\\wav\\HeroVoice\\0047.wav", 1, KIRBY_EFFECTSOUND);
            m_YaySound = true;
        }

        if (GetOwner()->Animator()->IsFinish())
        {
            
            return true;
        }
    }
    break;
    }

    return false;
}

#include "pch.h"
#include "CKirbyCutterBullet.h"

#include "CPlayerMgr.h"
#include "CKirbyFSM.h"
#include "CKirbyHatBlade.h"
#include "CMonsterUnitScript.h"

CKirbyCutterBullet::CKirbyCutterBullet()
    : CScript(KIRBYCUTTERBULLET)
    , m_IsBack(false)
    , m_Speed(35.f)
    , m_BurningSpeed(70.f)
    , m_Frequency(0.7f)
    , m_RotAcc(0.f)
    , m_Angle(45.f)
    , m_bPressedATK(true)
    , m_bBuning(false)
    , m_BuningStepUpDuration(0.3f)
    , m_CurStep(BuningStep::NONE)
{
}

CKirbyCutterBullet::~CKirbyCutterBullet()
{
}

void CKirbyCutterBullet::begin()
{
    if (!Rigidbody())
    {
        GetOwner()->AddComponent(new CRigidbody());
    }

    // 애니메이션 재생
    Animator()->Play(ANIMPREFIX("Spin"), false, false, 1.5f, 0);

    // 초기 값 세팅
    m_IsBack = false;
    m_CurState = BulletState::DRAG;
    m_Velocity = Vec3(0.f, 0.f, 0.f);

    m_Duration = 0.4f;
    m_Acc = 0.f;

    m_MoveDir = Transform()->GetWorldDir(DIR_TYPE::FRONT);

    m_BounceCount = 6;
    m_bPressedATK = true;
    m_bBuning = false;
    m_BuningStepUpDuration = 0.5f;
    m_CurStep = BuningStep::NONE;
}

void CKirbyCutterBullet::tick()
{

    // Burning check
    if (m_bPressedATK && (KEY_RELEASED(KEY_ATK) || KEY_NONE(KEY_ATK)))
    {
        m_bPressedATK = false;
    }

    // 좌우로 흔들리는 효과
    m_RotAcc += DT;

    float Angle = m_Angle * sinf((m_RotAcc / m_Frequency) * XM_PI * 2.f);

    Vec3 Front = Transform()->GetWorldDir(DIR_TYPE::FRONT);
    Front.Normalize();

    Vec3 AngularVelocity = Front * XMConvertToRadians(Angle);
    Rigidbody()->SetAngularVelocity(AngularVelocity);

    if (m_RotAcc > m_Frequency)
    {
        m_RotAcc -= m_Frequency;
    }

    if (Animator()->IsFinish())
    {
        Animator()->Play(ANIMPREFIX("SpinBig"), true, false, 1.5f, 0.1);
    }

    // Bullter 움직임
    switch (m_CurState)
    {
    case BulletState::HOLD_AIR: {
        m_Acc += DT;

        if (m_bPressedATK)
        {
            SetState(BulletState::BURNING_WAIT);
        }
        else if (m_Acc > m_Duration)
        {
            --m_BounceCount;

            if (m_IsBack)
            {
                GamePlayStatic::DestroyGameObject(GetOwner());
            }
            else
            {
                if (m_BounceCount == 0)
                {
                    GamePlayStatic::DestroyGameObject(GetOwner());
                }
                else
                {
                    SetState(BulletState::BACK);
                }
            }
        }
    }
    break;
    case BulletState::HOLD_WALL: {
        m_Acc += DT;

        Rigidbody()->SetVelocity({0.f, 0.f, 0.f});

        if (m_Acc > m_Duration)
        {
            --m_BounceCount;

            if (m_BounceCount == 0)
            {
                GamePlayStatic::DestroyGameObject(GetOwner());
            }
            else
            {
                if (m_bBuning)
                {
                    SetState(BulletState::BURNING);
                }
                else if (m_IsBack)
                {
                    SetState(BulletState::RAGE);
                }
                else
                {
                    SetState(BulletState::DRAG);
                }
            }
        }
    }
    break;
    case BulletState::DRAG: {
        m_Acc += DT;

        float phase = (m_Acc / m_Duration) * XM_PI;
        phase = clamp(phase, 0.f, XM_PI);
        phase = std::sin(phase);

        m_Velocity = m_MoveDir * m_Speed * phase;

        Rigidbody()->SetVelocity(m_Velocity);

        if (m_Acc > m_Duration)
        {
            SetState(BulletState::HOLD_AIR);
        }
    }
    break;
    case BulletState::BACK: {
        m_Acc += DT;

        Vec3 KirbyPos = PLAYER->Transform()->GetWorldPos();
        KirbyPos += Vec3(0.f, 50.f, 0.f);
        Vec3 CurPos = Transform()->GetWorldPos();

        Vec3 NewDir = KirbyPos - CurPos;
        NewDir.Normalize();

        if (m_MoveDir.Dot(NewDir) > 0.f)
        {
            NewDir.y = m_MoveDir.y;
            m_MoveDir = DirectX::SimpleMath::Vector3::Lerp(m_MoveDir, NewDir, 0.15f);
            m_MoveDir.Normalize();
        }

        if (m_MoveDir.Dot(NewDir) > 0.f)
        {
            RaycastHit Hit = CPhysicsMgr::GetInst()->RayCast(CurPos, Vec3(0.f, -1.f, 0.f), 100.f, {L"World Static"});

            float newY = Lerp(m_MoveDir.y, NewDir.y, 0.3f);
            NewDir.y = newY;

            m_MoveDir = DirectX::SimpleMath::Vector3::Lerp(m_MoveDir, NewDir, 0.15f);

            // 땅이랑 가까워질 경우 y축 이동속도를 0으로 한다.
            if (Hit.Distance < GetOwner()->Transform()->GetLocalScale().y)
            {
                m_MoveDir.y = 0.f;
            }

            m_MoveDir.Normalize();
        }
        else
        {
            if (m_MoveDir.y < 0.f)
            {
                m_MoveDir.y = 0.f;
            }
        }

        float phase = (m_Acc / 0.2f) * XM_PI / 2.f;
        phase = clamp(phase, 0.f, XM_PI / 2.f);
        phase = std::sin(phase);

        m_Velocity = m_MoveDir * m_Speed * phase;

        Rigidbody()->SetVelocity(m_Velocity);

        if (m_Acc > m_Duration)
        {
            GamePlayStatic::DestroyGameObject(GetOwner());
        }
    }
    break;
    case BulletState::RAGE: {
        m_Acc += DT;

        float phase = (m_Acc / 0.2f) * XM_PI / 2.f;
        phase = clamp(phase, 0.f, XM_PI / 2.f);
        phase = std::sinf(phase);

        m_Velocity = m_MoveDir * m_BurningSpeed * phase;

        Rigidbody()->SetVelocity(m_Velocity);

        if (m_Acc > m_Duration)
        {
            GamePlayStatic::DestroyGameObject(GetOwner());
        }
    }
    break;
    case BulletState::BURNING_WAIT: {
        m_Acc += DT;

        if (m_Acc > m_BuningStepUpDuration)
        {
            // StepUP 했을 경우
            if (StepUp())
            {
                m_Acc -= m_BuningStepUpDuration;
            }
        }

        if (m_Acc > m_Duration || KEY_RELEASED(KEY_ATK) || KEY_NONE(KEY_ATK))
        {
            SetState(BulletState::BACK);
        }
    }
    break;
    case BulletState::BURNING: {
        m_Acc += DT;

        float phase = (m_Acc / 0.2f) * XM_PI / 2.f;
        phase = clamp(phase, 0.f, XM_PI / 2.f);
        phase = std::sin(phase);

        m_Velocity = m_MoveDir * m_BurningSpeed * phase;

        Rigidbody()->SetVelocity(m_Velocity);

        if (m_Acc > m_Duration)
        {
            GamePlayStatic::DestroyGameObject(GetOwner());
        }
    }
    break;
    }
}

void CKirbyCutterBullet::SetState(BulletState _State)
{
    m_CurState = _State;
    m_Acc = 0.f;

    switch (m_CurState)
    {
    case BulletState::HOLD_AIR: {
        m_Duration = 0.7f;
    }
    break;
    case BulletState::HOLD_WALL: {
        m_Duration = 0.7f;
    }
    break;
    case BulletState::DRAG: {
        GamePlayStatic::Play3DSound(L"sound\\wav\\HeroCutterMetal\\AttackCutter.wav", Transform()->GetWorldPos(), 1, KIRBY_EFFECTSOUND, true, true);
        m_Duration = 0.7f;
    }
    break;
    case BulletState::BACK: {
        // 방향 설정
        Vec3 KirbyPos = PLAYER->Transform()->GetWorldPos();
        Vec3 test = Transform()->GetWorldPos();

        Vec3 Dir = KirbyPos - Transform()->GetWorldPos() + Vec3(0.f, 20.f, 0.f);
        Dir.Normalize();

        m_MoveDir = Dir;

        m_Duration = 3.f;
        m_IsBack = true;
    }
    break;
    case BulletState::RAGE: {
        GamePlayStatic::Play3DSound(L"sound\\wav\\HeroCutterMetal\\Cutter_Burning.wav", Transform()->GetWorldPos(), 1, KIRBY_EFFECTSOUND, true, true);
        m_Duration = 3.f;
    }
    break;
    case BulletState::BURNING_WAIT: {
        m_Duration = 4.f;
    }
    break;
    case BulletState::BURNING: {
        GamePlayStatic::Play3DSound(L"sound\\wav\\HeroCutterMetal\\Cutter_Burning.wav", Transform()->GetWorldPos(), 1, KIRBY_EFFECTSOUND, true, true);
        m_Duration = 3.f;
    }
    break;

    default:
        break;
    }
}

bool CKirbyCutterBullet::StepUp()
{

    switch (m_CurStep)
    {
    case BuningStep::NONE: {
        GamePlayStatic::Play2DSound(L"sound\\wav\\HeroCutterBaisc\\Cutter_Rage0.wav", 1, KIRBY_EFFECTSOUND);

        m_bBuning = true;
        m_CurStep = BuningStep::STEP_1;

        GetOwner()->Transform()->SetLocalScale(Vec3(25.f, 25.f, 25.f));
    }
    break;
    case BuningStep::STEP_1: {
        GamePlayStatic::Play2DSound(L"sound\\wav\\HeroCutterBaisc\\Cutter_Rage1.wav", 1, KIRBY_EFFECTSOUND);
        m_CurStep = BuningStep::STEP_2;

        GetOwner()->Transform()->SetLocalScale(Vec3(30.f, 30.f, 30.f));
    }
    break;
    case BuningStep::STEP_2: {
        return false;
    }
    break;
    case BuningStep::STEP_3: {
        return false;
    }
    break;
    }

    return true;
}

void CKirbyCutterBullet::OnCollisionEnter(CCollider* _OtherCollider)
{
    int LayerIdx = _OtherCollider->GetOwner()->GetLayerIdx();

    // WorldStatic
    if (LayerIdx == LAYER_STATIC)
    {
        GamePlayStatic::Play3DSound(L"sound\\wav\\HeroCutterMetal\\Cutter_Hit.wav", Transform()->GetWorldPos(), 1, 2.f * KIRBY_EFFECTSOUND, true,
                                    true);

        Vec3 RayDir = m_MoveDir;
        RayDir.y = 0.f;
        RayDir.Normalize();

        Vec3 RayStart = Transform()->GetWorldPos() - m_MoveDir * 10.f;

        RaycastHit Hit = CPhysicsMgr::GetInst()->RayCast(RayStart, RayDir, 1000.f, {L"World Static"});

        if (Hit.pCollisionObj != nullptr)
        {
            Vec3 Normal = Hit.Normal;
            m_MoveDir = RayDir + 2.f * Normal * (-RayDir.Dot(Normal));
        }
        else
        {
            m_MoveDir *= -1.f;
        }

        SetState(BulletState::HOLD_WALL);
    }
}

void CKirbyCutterBullet::OnTriggerEnter(CCollider* _OtherCollider)
{
    int LayerIdx = _OtherCollider->GetOwner()->GetLayerIdx();

    // WorldStatic
    if (LayerIdx == LAYER_PLAYER_TRIGGER && _OtherCollider->GetOwner()->GetName() == L"Body Collider")
    {
        // 커비한테 돌아가는 상태라면
        if (m_IsBack)
        {
            GamePlayStatic::Play3DSound(L"sound\\wav\\HeroCutterMetal\\ReturnCutter.wav", Transform()->GetWorldPos(), 1, KIRBY_EFFECTSOUND, true,
                                        true);
            GamePlayStatic::DestroyGameObject(GetOwner());

            if (PLAYERFSM->GetCurAbilityIdx() == AbilityCopyType::CUTTER && PLAYERFSM->GetCurObjectIdx() == ObjectCopyType::NONE)
            {
                CKirbyHatBlade* BladeScript = dynamic_cast<CKirbyHatBlade*>(PLAYERFSM->GetCurHatBlade()->GetScripts()[0]);

                if (BladeScript)
                {
                    BladeScript->Reset();
                }
            }
        }
    }
    // Monster
    else if (LayerIdx == LAYER_MONSTER_TRIGGER && _OtherCollider->GetOwner()->GetName() == L"Body Collider")
    {
        CUnitScript* Monster = _OtherCollider->GetOwner()->GetParent()->GetScript<CUnitScript>();
        if (nullptr != Monster)
        {
            Vec3 HitDir = _OtherCollider->Transform()->GetWorldPos() - Transform()->GetWorldPos();
            HitDir.Normalize();
            UnitHit HitInfo = {DAMAGE_TYPE::NORMAL, HitDir, 10.f, 0.f, 0.f};

            if (m_bBuning)
            {
                HitInfo.Damage = 20.f;
            }

            for (UINT i = 0; i < 2; i++)
            {
                SpawnSlashEffect(_OtherCollider->Transform()->GetWorldPos());
            }

            Monster->GetDamage(HitInfo);

            GamePlayStatic::Play3DSound(L"sound\\wav\\HeroCutterMetal\\Cutter_Hit.wav", Transform()->GetWorldPos(), 1, 2.f * KIRBY_EFFECTSOUND, true,
                                        true);
        }
    }
}

void CKirbyCutterBullet::OnTriggerExit(CCollider* _OtherCollider)
{
}

void CKirbyCutterBullet::SpawnSlashEffect(Vec3 _vPos)
{
    CGameObject* pSpawnEffect =
        CAssetMgr::GetInst()->Load<CPrefab>(L"prefab\\Effect_SlashEffect.pref", L"prefab\\Effect_SlashEffect.pref")->Instantiate();

    _vPos.y += 35.f;
    pSpawnEffect->Transform()->SetWorldPos(_vPos);
    GamePlayStatic::SpawnGameObject(pSpawnEffect, pSpawnEffect->GetLayerIdx());
}

UINT CKirbyCutterBullet::SaveToLevelFile(FILE* _File)
{
    UINT MemoryByte = 0;

    return MemoryByte;
}

UINT CKirbyCutterBullet::LoadFromLevelFile(FILE* _File)
{
    UINT MemoryByte = 0;

    return MemoryByte;
}

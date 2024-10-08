#include "pch.h"
#include "CMorphoAtkG_Teleport_Tornado.h"
#include "CMorphoFSM.h"
#include "CMomentaryObjScript.h"

CMorphoAtkG_Teleport_Tornado::CMorphoAtkG_Teleport_Tornado()
    : m_AccTime(0.f)
    , m_ChargeTime(1.f)
    , m_TornadoTime(5.f)
    , m_Tornado{false,}
    , m_TornadoSpeed(5.f)
    , m_bSpawnDropStar(false)
{
    m_ChargeEffect = CAssetMgr::GetInst()->Load<CPrefab>(L"prefab\\Effect_MorphoTornadoCharging.pref", L"prefab\\Effect_MorphoTornadoCharging.pref");
    m_TornadoPref = CAssetMgr::GetInst()->Load<CPrefab>(L"prefab\\MorphoTornado.pref", L"prefab\\MorphoTornado.pref");
}

CMorphoAtkG_Teleport_Tornado::~CMorphoAtkG_Teleport_Tornado()
{
}

void CMorphoAtkG_Teleport_Tornado::tick()
{
    switch (m_Step)
    {
    case StateStep::Charge: {
        Charge();
    }
    break;
    case StateStep::ChargeWait: {
        ChargeWait();
    }
    break;
    case StateStep::Start: {
        Start();
    }
    break;
    case StateStep::Progress: {
        Progress();
    }
    break;
    case StateStep::Wait: {
        Wait();
    }
    break;
    case StateStep::End: {
        End();
    }
    break;
    }
}

void CMorphoAtkG_Teleport_Tornado::Enter()
{
    m_Step = StateStep::Charge;
    Enter_Step();
}

void CMorphoAtkG_Teleport_Tornado::Exit()
{
    for (int i = 0; i < 6; ++i)
    {
        if (m_Tornado[i])
        {
            GamePlayStatic::DestroyGameObject(m_Tornado[i]);
            m_Tornado[i] = nullptr;
        }
    }

    Exit_Step();

    // Sound
    wstring Storm = L"sound\\wav\\CharaMorphoknight\\0031_Storm.wav";
    CSoundMgr::GetInst()->FadeSound(Storm, SOUND_MORPHO, 0.f, 0.5f, false);
}

void CMorphoAtkG_Teleport_Tornado::Enter_Step()
{
    switch (m_Step)
    {
    case StateStep::Charge: {
        GetOwner()->Animator()->Play(ANIMPREFIX("TornadoAttackCharge"), false, false, 1.5f);
    }
    break;
    case StateStep::ChargeWait: {
        GetOwner()->Animator()->Play(ANIMPREFIX("TornadoAttackChargeMax"), true, false, 1.5f);
        m_AccTime = 0.f;

        // spawn charging effect
        if (m_ChargeEffect != nullptr)
        {
            CGameObject* Effect = m_ChargeEffect->Instantiate();
            CMomentaryObjScript* Script = Effect->GetScript<CMomentaryObjScript>();
            Vec3 Pos = GetOwner()->GetChildObject(L"BossMorphoSwordR")->Transform()->GetWorldPos();
            Vec3 Dir = GetOwner()->Transform()->GetWorldDir(DIR_TYPE::FRONT);
            Dir.y = 0.f;
            Dir.Normalize();

            Effect->Transform()->SetWorldPos(Pos);
            Effect->Transform()->Slerp(Dir, 1.f);

            Script->SetPlayTime(m_ChargeTime);

            //@EFFECT 차징이펙트 재생

            GamePlayStatic::SpawnGameObject(Effect, LAYER_EFFECT);
        }

        // Sound
        wstring Sound = L"sound\\wav\\CharaMorphoknight\\ShockWaveCharging2.wav";
        GamePlayStatic::Play2DSound(Sound, 1, SOUND_MORPHO);
    }
    break;
    case StateStep::Start: {
        GetOwner()->Animator()->Play(ANIMPREFIX("TornadoAttackStart"), false, false, 1.5f);
        MRPFSM->OnWeaponRTrigger();
    }
    break;
    case StateStep::Progress: {
        GetOwner()->Animator()->Play(ANIMPREFIX("TornadoAttackWait"), true, false, 1.5f);
        m_AccTime = 0.f;

        // spawn Tornado
        for (int i = 0; i < 6; ++i)
        {
            if (m_TornadoPref != nullptr)
            {
                m_Tornado[i] = m_TornadoPref->Instantiate();

                // Transform 초기화
                Vec3 Pos = GetOwner()->Transform()->GetWorldPos();
                Pos.y = 0.f;
                Vec3 Dir = GetOwner()->Transform()->GetWorldDir(DIR_TYPE::FRONT);
                Dir.y = 0.f;
                Dir.Normalize();

                m_Tornado[i]->Transform()->SetWorldPos(Pos);
                m_Tornado[i]->Transform()->Slerp(Dir, 1.f);

                // Tornado idx 회전 반영
                Vec3 DeltaRot = m_Tornado[i]->Transform()->GetWorldRotation();
                DeltaRot.y += XMConvertToRadians(-60.f + (120.f / 5.f) * i);
                m_Tornado[i]->Transform()->SetWorldRotation(DeltaRot);

                Dir = m_Tornado[i]->Transform()->GetWorldDir(DIR_TYPE::FRONT);
                Pos += Dir * 60.f;
                m_Tornado[i]->Transform()->SetWorldPos(Pos);

                GamePlayStatic::SpawnGameObject(m_Tornado[i], LAYER_MONSTERATK);
            }
        }

        // Sound
        wstring Sound = L"sound\\wav\\CharaMorphoknight\\0031_Storm.wav";
        CSoundMgr::GetInst()->FadeSound(Sound, 0.f, SOUND_MORPHO, 0.5f, true);
    }
    break;
    case StateStep::Wait: {
        m_AccTime = 0.f;
        m_bSpawnDropStar = false;
    }
    break;
    case StateStep::End: {
        GetOwner()->Animator()->Play(ANIMPREFIX("TornadoAttackEnd"), false, false, 1.5f);
    }
    break;
    }
}

void CMorphoAtkG_Teleport_Tornado::Exit_Step()
{
    switch (m_Step)
    {
    case StateStep::Charge:
        break;
    case StateStep::ChargeWait:
        break;
    case StateStep::Start: {
        MRPFSM->OffWeaponRTrigger();
    }
    break;
    case StateStep::Progress:
        break;
    case StateStep::Wait: {
        // Sound
        wstring Storm = L"sound\\wav\\CharaMorphoknight\\0031_Storm.wav";
        CSoundMgr::GetInst()->FadeSound(Storm, SOUND_MORPHO, 0.f, 0.5f, false);
    }
    break;
    case StateStep::End:
        break;
    }
}

void CMorphoAtkG_Teleport_Tornado::Charge()
{
    RotateToPlayer(DT * 0.75f);

    if (GetOwner()->Animator()->IsFinish())
    {
        ChangeStep(StateStep::ChargeWait);
    }
}

void CMorphoAtkG_Teleport_Tornado::ChargeWait()
{
    m_AccTime += DT;
    RotateToPlayer(DT * 0.75f);

    if (m_AccTime > m_ChargeTime)
    {
        ChangeStep(StateStep::Start);
    }
}

void CMorphoAtkG_Teleport_Tornado::Start()
{
    RotateToPlayer(DT * 0.75f);

    if (GetOwner()->Animator()->IsFinish())
    {
        ChangeStep(StateStep::Progress);
    }
}

void CMorphoAtkG_Teleport_Tornado::Progress()
{
    float AppearTime = 0.5f;
    m_AccTime += DT;

    // appeal scaling
    if (m_AccTime <= AppearTime)
    {
        for (int i = 0; i < 6; ++i)
        {
            if (m_Tornado[i])
            {
                float t = m_AccTime / AppearTime;
                m_Tornado[i]->GetChildObject()[0]->Transform()->SetLocalScale(Vec3(t));
            }
        }
    }
    else
    {
        for (int i = 0; i < 6; ++i)
        {
            if (m_Tornado[i])
            {
                float t = m_AccTime / AppearTime;
                m_Tornado[i]->GetChildObject()[0]->Transform()->SetLocalScale(Vec3(1.f));
            }
        }

        ChangeStep(StateStep::Wait);
    }
}

void CMorphoAtkG_Teleport_Tornado::Wait()
{
    m_AccTime += DT;
    float DisappearTime = 0.5f;

    // move
    for (int i = 0; i < 6; ++i)
    {
        if (m_Tornado[i])
        {
            float t = m_AccTime / m_TornadoTime;
            float NewSpeed = 10.f * sinf(t * XM_PI);
            Vec3 NewVeloc = m_Tornado[i]->Transform()->GetWorldDir(DIR_TYPE::FRONT) * NewSpeed;
            m_Tornado[i]->Rigidbody()->SetVelocity(NewVeloc);
        }
    }

    // spawn dropstar
    if (!m_bSpawnDropStar && m_AccTime >= 2.f)
    {
        m_bSpawnDropStar = true;

        for (int i = 0; i < 6; ++i)
        {
            if (m_Tornado[i])
            {
                Vec3 SpawnPos = m_Tornado[i]->Transform()->GetWorldPos();
                MRPFSM->SpawnDropStar(SpawnPos);
            }
        }
    }

    // disappear scaling
    else if (m_AccTime > m_TornadoTime - DisappearTime)
    {
        for (int i = 0; i < 6; ++i)
        {
            if (m_Tornado[i])
            {
                float t = clamp((m_TornadoTime - m_AccTime) / DisappearTime, 0.f, 1.f);
                Vec3 NewScale = Vec3(t, 1.f, t);
                m_Tornado[i]->GetChildObject()[0]->Transform()->SetLocalScale(NewScale);
            }
        }
    }

    // change state
    if (m_AccTime > m_TornadoTime)
    {
        for (int i = 0; i < 6; ++i)
        {
            if (m_Tornado[i])
            {
                m_Tornado[i]->GetChildObject()[0]->Transform()->SetLocalScale(Vec3(0.f, 1.f, 0.f));
            }
        }

        ChangeStep(StateStep::End);
    }
}

void CMorphoAtkG_Teleport_Tornado::End()
{
    if (GetOwner()->Animator()->IsFinish())
    {
        MRPFSM->Move();
    }
}
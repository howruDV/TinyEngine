#include "pch.h"
#include "CMorphoMoveG_TeleportNear.h"
#include "CMorphoFSM.h"

CMorphoMoveG_TeleportNear::CMorphoMoveG_TeleportNear()
    : m_AccTime(0.f)
    , m_WaitTime(0.5f)
    , m_bParticleSpawn(false)
{
}

CMorphoMoveG_TeleportNear::~CMorphoMoveG_TeleportNear()
{
}

void CMorphoMoveG_TeleportNear::tick()
{
    switch (m_Step)
    {
    case StateStep::Start: {
        Start();
    }
    break;
    case StateStep::End: {
        End();
    }
    break;
    }
}

void CMorphoMoveG_TeleportNear::Exit()
{
    Exit_Step();

    MRPFSM->EnableRender();
    MRPFSM->EnableCollider();
    GetOwner()->Rigidbody()->SetUseGravity(true);
    GetOwner()->Animator()->SetPlay(true);
}

void CMorphoMoveG_TeleportNear::Enter_Step()
{
    switch (m_Step)
    {
    case StateStep::Start: {
        GetOwner()->Animator()->SetPlay(false);
        m_AccTime = 0.f;
        m_bParticleSpawn = false;

        // teleport pos
        Vec3 MapSize = MRPFSM->GetMapSize();
        Vec3 MapOffset = MRPFSM->GetMapFloorOffset();
        Vec3 Dist = Vec3(GetRandomfloat(-100.f, 100.f), 0.f, GetRandomfloat(-100.f, 100.f)).Normalize();
        if (Dist.Length() <= 0.f)
        {
            Dist = Vec3(1.f, 0.f, 1.f).Normalize();
        }

        Dist *= MRPFSM->GetNearDist() * (GetRandomfloat(50.f, 150.f) / 100.f);
        m_AfterPos = PLAYER->Transform()->GetWorldPos() + Dist;
        m_AfterPos.y = 0.f;

        // map limit
        if (m_AfterPos.x < 0 && m_AfterPos.x < -MapSize.x + MapOffset.x)
        {
            m_AfterPos.x = -MapSize.x + MapOffset.x;
        }
        else if (m_AfterPos.x > 0 && m_AfterPos.x > MapSize.x + MapOffset.x)
        {
            m_AfterPos.x = MapSize.x + MapOffset.x;
        }

        if (m_AfterPos.z < 0 && m_AfterPos.z < -MapSize.z + MapOffset.z)
        {
            m_AfterPos.z = -MapSize.z + MapOffset.z;
        }
        else if (m_AfterPos.z > 0 && m_AfterPos.z > MapSize.z)
        {
            m_AfterPos.z = MapSize.z + MapOffset.z;
        }

        // sound
        wstring Sound = L"sound\\wav\\CharaMorphoknight\\TeleportDisappear.wav";
        GamePlayStatic::Play2DSound(Sound, 1, SOUND_MORPHO * 1.2f);
    }
    break;
    case StateStep::End: {
        Vec3 Dir = PLAYER->Transform()->GetWorldPos() - m_AfterPos;
        Dir.y = 0.f;
        Dir.Normalize();

        GetOwner()->Transform()->SetWorldPos(m_AfterPos);
        GetOwner()->Transform()->Slerp(Dir, 1.f);
        m_AccTime = 0.f;
    }
    break;
    }
}

void CMorphoMoveG_TeleportNear::Exit_Step()
{
    switch (m_Step)
    {
    case StateStep::Start: {
        MRPFSM->DisableRender();
        MRPFSM->DisableCollider();
        MRPFSM->ResetEmissive();
        GetOwner()->Rigidbody()->SetUseGravity(false);
    }
    break;
    case StateStep::End:
        break;
    }
}

void CMorphoMoveG_TeleportNear::Start()
{
    m_AccTime += DT;

    // Disappear
    float delta = m_AccTime / MRPFSM->GetEmissiveTime();
    float t1 = delta;
    float t2 = sinf(delta * XM_PI / 2.f);
    Vec3 Color = Vec3(t1);
    Color.x = t2;

    MRPFSM->AddEmissive(Color);

    // Particle On
    if (!m_bParticleSpawn && m_AccTime > MRPFSM->GetEmissiveTime() - 0.25f)
    {
        m_bParticleSpawn = true;
        MRPFSM->EnableTeleportParticle(true);
    }

    // Change State
    if (m_AccTime > MRPFSM->GetEmissiveTime())
    {
        ChangeStep(StateStep::End);
    }
}

void CMorphoMoveG_TeleportNear::End()
{
    m_AccTime += DT;

    // Particle Off
    if (m_bParticleSpawn && m_AccTime > 0.25f)
    {
        m_bParticleSpawn = false;
        MRPFSM->EnableTeleportParticle(false);
    }

    // Change State
    if (m_AccTime > m_WaitTime)
    {
        MRPFSM->SetTeleportTime(true);
        MRPFSM->Attack();

        // sound
        wstring Sound = L"sound\\wav\\CharaMorphoknight\\TeleportAppear.wav";
        GamePlayStatic::Play2DSound(Sound, 1, SOUND_MORPHO);
    }
}
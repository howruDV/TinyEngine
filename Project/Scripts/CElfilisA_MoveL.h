#pragma once
#include "CBossState.h"

class CElfilisA_MoveL : public CBossState
{
private:
    Vec3 m_TargetPos;
    Vec3 m_StartPos;
    float m_PrevDrag;
    bool m_bMoveLeft;
    bool m_bMoveUp;

    // sound
    vector<wstring> m_SoundKeyMoveAir;

public:
    virtual void tick() override;
    virtual void Exit() override;

private:
    void Enter_Step();
    void Exit_Step();

    // tick
    void Start();
    void Progress();
    void End();

    void Fly();

public:
    CLONE(CElfilisA_MoveL)
    CElfilisA_MoveL();
    virtual ~CElfilisA_MoveL();
};

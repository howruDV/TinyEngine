#pragma once
#include "CState.h"
class CTestRight :
    public CState
{
public:
    virtual void tick();
    virtual void Enter();
    virtual void Exit();

public:
    CLONE(CTestRight)
    CTestRight();
    virtual ~CTestRight();
};


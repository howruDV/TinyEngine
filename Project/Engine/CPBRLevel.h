#pragma once
#include "CLevel.h"

class CPBRLevel : public CLevel
{
public:
    virtual void begin() override;
    virtual void tick() override;
    virtual void finaltick() override;

public:
    CPBRLevel();
    virtual ~CPBRLevel();
};

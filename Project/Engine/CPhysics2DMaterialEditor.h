#pragma once
#include "CEditor.h"

class CPhysics2DMaterialEditor : public CEditor
{
private:
    Ptr<CPhysics2DMaterial> m_Mtrl;

public:
    virtual void init() override{};
    virtual void tick() override{};
    virtual void finaltick() override{};
    virtual void render(bool* open) override;

private:
    void render();

private:
    void DrawDetails();

public:
    void SetMaterial(Ptr<CPhysics2DMaterial> _Mtrl) { m_Mtrl = _Mtrl; }

public:
    CPhysics2DMaterialEditor();
    virtual ~CPhysics2DMaterialEditor();
};

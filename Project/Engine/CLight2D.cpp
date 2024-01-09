#include "pch.h"
#include "CLight2D.h"

#include "CRenderMgr.h"

CLight2D::CLight2D()
    : CComponent(COMPONENT_TYPE::LIGHT2D)
    , m_Info{}
{
    m_Info.vColor = Vec4(1.f, 1.f, 1.f, 1.f);
    m_Info.vAmbient = Vec4(1.f, 1.f, 1.f, 1.f);
    m_Info.vSpecular = Vec4(1.f, 1.f, 1.f, 1.f);

    m_Info.fRadius = 0.f;
    m_Info.fAngle = 0.f;

    m_Info.LightType = -1;

    m_Info.fallOffStart = 0.f;
    m_Info.fallOffEnd = 1000.f;
    m_Info.spotPower = 100.f;
}

CLight2D::~CLight2D()
{
}

void CLight2D::finaltick()
{
    // ���� ���
    CRenderMgr::GetInst()->RegisterLight2D(this);
}

void CLight2D::SaveToLevelFile(FILE* _File)
{
    fwrite(&m_Info, sizeof(tLightInfo), 1, _File);
}

void CLight2D::LoadFromLevelFile(FILE* _File)
{
    fread(&m_Info, sizeof(tLightInfo), 1, _File);
}

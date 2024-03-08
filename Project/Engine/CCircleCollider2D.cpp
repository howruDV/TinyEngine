#include "pch.h"
#include "CCircleCollider2D.h"

#include "components.h"

CCircleCollider2D::CCircleCollider2D()
    : CCollider2D(COMPONENT_TYPE::CIRCLECOLLIDER2D)
    , m_Radius(1.f)

{
}

CCircleCollider2D::~CCircleCollider2D()
{
}

void CCircleCollider2D::finaltick()
{
    Vec3 sacle = Transform()->GetRelativeScale();

    Matrix matWorld = Transform()->GetWorldMat();
    Matrix matTranslation = XMMatrixTranslation(m_Offset.x, m_Offset.y, 0.0f);
    Matrix matScale = XMMatrixScaling(m_Radius * sacle.x, m_Radius * sacle.x, 0.f);

    Matrix matInvScale = XMMatrixScaling(1.f / sacle.x, 1.f / sacle.y, 1.f / sacle.z);

    GamePlayStatic::DrawDebugCircle(matScale * matTranslation * matInvScale * matWorld, Vec3(0.f, 1.f, 0.f), false);
}

void CCircleCollider2D::SaveToLevelFile(FILE* _File)
{
    CCollider2D::SaveToLevelFile(_File);
    fwrite(&m_Radius, sizeof(float), 1, _File);
}

void CCircleCollider2D::LoadFromLevelFile(FILE* _File)
{
    CCollider2D::LoadFromLevelFile(_File);
    fread(&m_Radius, sizeof(float), 1, _File);
}

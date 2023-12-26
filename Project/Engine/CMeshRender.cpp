#include "pch.h"
#include "CMeshRender.h"

#include "CMesh.h"
#include "CGraphicsShader.h"
#include "CMaterial.h"

#include "CGameObject.h"
#include "CTransform.h"

#include "CConstBuffer.h"
#include "CDevice.h"

#include "CLevelMgr.h"

#include "CAssetMgr.h"

CMeshRender::CMeshRender()
    : CRenderComponent(COMPONENT_TYPE::MESHRENDER)
    , m_bDrawNormalLine(false)
    , m_bDrawAsWire(false)
    , m_NormalLineScale(1.0f)
    , m_bUseTexture(false)
    , m_bUseRim(false)
    , m_RimColor(Vec3())
    , m_RimPower(1.f)

{
}

CMeshRender::~CMeshRender()
{
}

void CMeshRender::UpdateData()
{
    if (nullptr != GetShader())
    {
        RS_TYPE RStype = GetShader()->GetRSType();
        if (m_bDrawAsWire)
            GetShader()->SetRSType(RS_TYPE::WIRE_FRAME);

        GetShader()->UpdateData();

        GetShader()->SetRSType(RStype);
    }

    if (nullptr != GetMaterial())
    {
        GetMaterial()->UpdateData();
    }

    GetOwner()->Transform()->UpdateData();

    // Global Data 바인딩
    if (m_bDrawNormalLine)
        g_Global.NormalLineScale = m_NormalLineScale;

    g_Global.UseTexture = m_bUseTexture;
    g_Global.UseRim = m_bUseRim;
    g_Global.rimColor = m_RimColor;
    g_Global.rimPower = m_RimPower;

    CConstBuffer* pCB = CDevice::GetInst()->GetConstBuffer(CB_TYPE::GLOBAL_DATA);
    pCB->SetData(&g_Global);
    pCB->UpdateData(2);
}

void CMeshRender::render()
{
    if (0 == GetMeshes().size() || nullptr == GetShader())
        return;

    // Material 초기화
    CMaterial::Clear();

    UpdateData();

    // Render Pass
    for (const auto& mesh : GetMeshes())
    {
        mesh->render();
    }

    // outline pass
    if (CLevelMgr::GetInst()->GetSelectedObj() == GetOwner())
    {
        CAssetMgr::GetInst()->FindAsset<CGraphicsShader>(L"OutLine")->UpdateData();
        g_Global.thickness = 10; // 10픽셀
        g_Global.width = CDevice::GetInst()->GetRenderResolution().x;

        for (const auto& mesh : GetMeshes())
        {
            mesh->render();
        }
    }

    // Normal Line Pass
    if (m_bDrawNormalLine)
    {
        CAssetMgr::GetInst()->FindAsset<CGraphicsShader>(L"NormalLine")->UpdateData();
        for (const auto& mesh : GetMeshes())
        {
            mesh->renderDraw();
        }
    }
}

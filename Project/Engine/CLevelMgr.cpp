#include "pch.h"
#include "CDevice.h"

#include "CLevelMgr.h"
#include "CAssetMgr.h"

#include "CLevel.h"

#include "CGameObject.h"
#include "components.h"
#include "CPlayerScript.h"

CLevelMgr::CLevelMgr()
{
}

CLevelMgr::~CLevelMgr()
{
}

void CLevelMgr::init()
{
    m_CurLevel = new CLevel;

    m_CurLevel->begin();

    // GameObject ����
    CGameObject* pObj = nullptr;

    pObj = new CGameObject;
    pObj->SetName(L"Player");

    pObj->AddComponent(new CTransform);
    pObj->AddComponent(new CMeshRender);
    pObj->AddComponent(new CPlayerScript);

    pObj->Transform()->SetRelativePos(Vec3(-0.5f, 0.f, 0.f));
    pObj->Transform()->SetRelativeScale(Vec3(0.5f, 0.5f, 0.5f));

    pObj->MeshRender()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"RectMesh"));
    pObj->MeshRender()->SetShader(CAssetMgr::GetInst()->FindAsset<CGraphicsShader>(L"Std2DShader"));

    m_CurLevel->AddObject(pObj, 0);
}

void CLevelMgr::tick()
{
    if (nullptr != m_CurLevel)
        m_CurLevel->tick();

    if (nullptr != m_CurLevel)
        m_CurLevel->finaltick();
}

void CLevelMgr::render()
{
    float ClearColor[4] = {0.3f, 0.3f, 0.3f, 1.f};
    CDevice::GetInst()->ClearRenderTarget(ClearColor);

    if (nullptr != m_CurLevel)
        m_CurLevel->render();

    CDevice::GetInst()->Present();
}

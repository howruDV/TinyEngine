#include "pch.h"
#include "CRenderMgr.h"

#include "CStructuredBuffer.h"

#include "CTimeMgr.h"
#include "CAssetMgr.h"
#include "CLevelMgr.h"
#include "CEditorMgr.h"

#include "CDevice.h"
#include "components.h"

#include "CConstBuffer.h"

CRenderMgr::CRenderMgr()
    : m_mainCam(nullptr)
    , m_EditorCam(nullptr)
    , m_Light2DBuffer(nullptr)
    , m_Light3DBuffer(nullptr)
    , m_pDebugObj(nullptr)
    , m_bShowDebugRender(false)
    , m_bShowCollider(true)
    , m_vecNoiseTex{}
    , m_DepthOnlyTex{}
    , m_PostEffectObj(nullptr)
    , bloomLevels(5)
    , m_BloomTextures{}
    , m_BloomDownFilters{}
    , m_BloomUpFilters{}
    , m_ToneMappingObj(nullptr)

{
    RENDER_FUNC = &CRenderMgr::render_play;
}

CRenderMgr::~CRenderMgr()
{
    if (nullptr != m_pDebugObj)
    {
        delete m_pDebugObj;
        m_pDebugObj = nullptr;
    }

    if (nullptr != m_Light2DBuffer)
    {
        delete m_Light2DBuffer;
        m_Light2DBuffer = nullptr;
    }

    if (nullptr != m_Light3DBuffer)
    {
        delete m_Light3DBuffer;
        m_Light3DBuffer = nullptr;
    }

    Delete_Vec(m_BloomDownFilters);
    Delete_Vec(m_BloomUpFilters);

    if (nullptr != m_ToneMappingObj)
    {
        delete m_ToneMappingObj;
        m_ToneMappingObj = nullptr;
    }

    if (nullptr != m_PostEffectObj)
    {
        delete m_PostEffectObj;
        m_PostEffectObj = nullptr;
    }
}

void CRenderMgr::render()
{
    UpdateData();

    // Light Depth Map
    render_LightDepth();

    // Rendering
    (this->*RENDER_FUNC)();

    // Debug
    render_debug();

    Clear();
}

void CRenderMgr::render_play()
{
    if (m_vecCam.empty())
        return;

    m_mainCam = m_vecCam[0];

    for (size_t i = 0; i < m_vecCam.size(); ++i)
    {
        m_vecCam[i]->SortObject();
        m_vecCam[i]->render();
    }
}

void CRenderMgr::render_editor()
{
    if (nullptr == m_EditorCam)
        return;

    m_mainCam = m_EditorCam;

    m_EditorCam->SortObject();
    m_EditorCam->render();
}

void CRenderMgr::render_debug()
{
    if (m_vecCam.empty())
        return;

    g_Transform.matView = m_vecCam[0]->GetViewMat();
    g_Transform.matProj = m_vecCam[0]->GetProjMat();

    list<tDebugShapeInfo>::iterator iter = m_DbgShapeInfo.begin();
    for (; iter != m_DbgShapeInfo.end();)
    {
        switch ((*iter).eShape)
        {
        case DEBUG_SHAPE::RECT:
            m_pDebugObj->MeshRender()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"RectMesh_Debug"));
            break;
        case DEBUG_SHAPE::CIRCLE:
            m_pDebugObj->MeshRender()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"CircleMesh_Debug"));
            break;
        case DEBUG_SHAPE::CROSS:
            m_pDebugObj->MeshRender()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"CrosshairMesh"));
            break;
        case DEBUG_SHAPE::BOX:
            m_pDebugObj->MeshRender()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"WireBox"));
            break;
        case DEBUG_SHAPE::SPHERE:
            m_pDebugObj->MeshRender()->SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"WireSphere"));
            break;
        default:
            break;
        }

        Ptr<CMaterial> pMtrl = CAssetMgr::GetInst()->FindAsset<CMaterial>(L"DebugShapeMtrl");
        pMtrl->SetScalarParam(VEC4_0, (*iter).vColor);

        DS_TYPE PrevDSType = pMtrl->GetShader()->GetDSType();
        if ((*iter).bDepthTest)
        {
            pMtrl->GetShader()->SetDSType(DS_TYPE::LESS);
        }

        D3D11_PRIMITIVE_TOPOLOGY PrevTopology = pMtrl->GetShader()->GetTopology();
        if (DEBUG_SHAPE::CROSS == (*iter).eShape || DEBUG_SHAPE::BOX == (*iter).eShape || DEBUG_SHAPE::SPHERE == (*iter).eShape)
        {
            pMtrl->GetShader()->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        }

        m_pDebugObj->Transform()->SetWorldMat((*iter).matWorld);

        m_pDebugObj->render(pMtrl);

        pMtrl->GetShader()->SetDSType(PrevDSType);
        pMtrl->GetShader()->SetTopology(PrevTopology);

        // Duration Check
        (*iter).fLifeTime += DT;
        if ((*iter).fDuration <= (*iter).fLifeTime)
        {
            iter = m_DbgShapeInfo.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
}

void CRenderMgr::render_postprocess()
{
    // RTV(PostProcess), SRV(floatRTTex DepthOnlyTex)
    CONTEXT->OMSetRenderTargets(1, m_PostProcessTex_HDRI->GetRTV().GetAddressOf(), NULL);
    m_PostEffectObj->render();
    CTexture::Clear(0);
    CTexture::Clear(1);
    CONTEXT->CopyResource(m_FloatRTTex->GetTex2D().Get(), m_PostProcessTex_HDRI->GetTex2D().Get());

    // Bloom
    CDevice::GetInst()->SetFloatRenderTarget();
    CopyToPostProcessTex_HDRI();
    for (int i = 0; i < m_BloomDownFilters.size(); i++)
    {
        if (i == 0)
            m_BloomDownFilters[i]->MeshRender()->GetMaterial()->SetTexParam(TEX_0, m_PostProcessTex_HDRI);
        else
            m_BloomDownFilters[i]->MeshRender()->GetMaterial()->SetTexParam(TEX_0, m_BloomTextures[i - 1]);

        CDevice::GetInst()->SetViewport((float)m_BloomTextures[i]->GetWidth(), (float)m_BloomTextures[i]->GetHeight());
        CONTEXT->OMSetRenderTargets(1, m_BloomTextures[i]->GetRTV().GetAddressOf(), NULL);
        m_BloomDownFilters[i]->render();
        CTexture::Clear(0);
    }
    for (int i = 0; i < m_BloomUpFilters.size(); i++)
    {
        int level = bloomLevels - 2 - i;
        m_BloomUpFilters[i]->MeshRender()->GetMaterial()->SetTexParam(TEX_0, m_BloomTextures[level]);
        if (i == bloomLevels - 2)
        {
            CDevice::GetInst()->SetViewport((float)m_PostProcessTex_HDRI->GetWidth(), (float)m_PostProcessTex_HDRI->GetHeight());
            CONTEXT->OMSetRenderTargets(1, m_PostProcessTex_HDRI->GetRTV().GetAddressOf(), NULL);
        }
        else
        {
            CDevice::GetInst()->SetViewport((float)m_BloomTextures[level - 1]->GetWidth(), (float)m_BloomTextures[level - 1]->GetHeight());
            CONTEXT->OMSetRenderTargets(1, m_BloomTextures[level - 1]->GetRTV().GetAddressOf(), NULL);
        }
        m_BloomUpFilters[i]->render();
        CTexture::Clear(0);
    }

    // Tone Mapping + Bloom Combine
    CDevice::GetInst()->SetViewport();
    CDevice::GetInst()->SetRenderTarget();
    m_ToneMappingObj->render();
    CTexture::Clear(0);
    CTexture::Clear(1);
}

void CRenderMgr::render_LightDepth()
{
    // 그림자 적용 광원 최대갯수설정
    int dynamicShadowMaxCount = 3;

    for (int i = 0; i < m_vecLight3D.size(); i++)
    {
        m_vecLight3D[i]->SetShadowIdx(-1); // 초기화

        if (dynamicShadowMaxCount <= 0)
            break;

        const tLightInfo& info = m_vecLight3D[i]->GetLightInfo();
        if (!info.ShadowType)
            continue;

        // Rendering
        m_vecLight3D[i]->SetShadowIdx(3 - dynamicShadowMaxCount);
        m_vecLight3D[i]->render_LightDepth();

        // Output 에 설정된 DepthMap 리셋
        CONTEXT->OMSetRenderTargets(0, NULL, NULL);

        // Binding
        if (3 == dynamicShadowMaxCount)
            m_vecLight3D[i]->GetDepthMapTex()->UpdateData(23);
        else if (2 == dynamicShadowMaxCount)
            m_vecLight3D[i]->GetDepthMapTex()->UpdateData(24);
        else if (1 == dynamicShadowMaxCount)
            m_vecLight3D[i]->GetDepthMapTex()->UpdateData(25);

        dynamicShadowMaxCount--;
    }
}

void CRenderMgr::UpdateData()
{
    // GlobalData 에 광원 개수정보 세팅
    g_Global.Light2DCount = (UINT)m_vecLight2D.size();
    g_Global.Light3DCount = (UINT)m_vecLight3D.size();

    // 메인 카메라 위치 등록
    if (nullptr != m_mainCam)
        g_Global.eyeWorld = m_mainCam->Transform()->GetWorldPos();
    else
        g_Global.eyeWorld = Vec3();

    // 전역 상수 데이터 바인딩
    CConstBuffer* pGlobalBuffer = CDevice::GetInst()->GetConstBuffer(CB_TYPE::GLOBAL_DATA);
    pGlobalBuffer->SetData(&g_Global);
    pGlobalBuffer->UpdateData();
    pGlobalBuffer->UpdateData_CS();

    // Light2D
    static vector<tLightInfo> vecLight2DInfo;

    for (size_t i = 0; i < m_vecLight2D.size(); ++i)
    {
        const tLightInfo& info = m_vecLight2D[i]->GetLightInfo();
        vecLight2DInfo.push_back(info);
    }

    m_Light2DBuffer->SetData(vecLight2DInfo.data(), (UINT)vecLight2DInfo.size());
    m_Light2DBuffer->UpdateData(11);

    vecLight2DInfo.clear();

    // Light3D
    static vector<tLightInfo> vecLight3DInfo;

    for (size_t i = 0; i < m_vecLight3D.size(); ++i)
    {
        const tLightInfo& info = m_vecLight3D[i]->GetLightInfo();
        vecLight3DInfo.push_back(info);
    }

    m_Light3DBuffer->SetData(vecLight3DInfo.data(), (UINT)vecLight3DInfo.size());
    m_Light3DBuffer->UpdateData(12);

    vecLight3DInfo.clear();
}

void CRenderMgr::Clear()
{
    m_vecLight2D.clear();
    m_vecLight3D.clear();

    // Light DepthMap Clear
    CTexture::Clear(23);
    CTexture::Clear(24);
    CTexture::Clear(25);
}

void CRenderMgr::RegisterCamera(CCamera* _Cam, int _Idx)
{
    if (m_vecCam.size() <= _Idx + 1)
    {
        m_vecCam.resize(_Idx + 1);
    }

    m_vecCam[_Idx] = _Cam;
}

void CRenderMgr::ActiveEditorMode(bool _bActive)
{
    if (_bActive)
        RENDER_FUNC = &CRenderMgr::render_editor;
    else
        RENDER_FUNC = &CRenderMgr::render_play;
}

CCamera* CRenderMgr::GetCamera(int _Idx) const
{
    if (m_vecCam.size() <= _Idx)
        return nullptr;

    return m_vecCam[_Idx];
}

void CRenderMgr::Clear_Buffers(const Vec4& Color)
{
    CONTEXT->ClearRenderTargetView(m_IDMapTex->GetRTV().Get(), Color);
    CONTEXT->ClearDepthStencilView(m_IDMapDSTex->GetDSV().Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

    CONTEXT->ClearRenderTargetView(m_PostProcessTex_HDRI->GetRTV().Get(), Color);
    CONTEXT->ClearRenderTargetView(m_PostProcessTex_LDRI->GetRTV().Get(), Color);

    CONTEXT->ClearDepthStencilView(m_DepthOnlyTex->GetDSV().Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void CRenderMgr::CopyRTTexToRTCopyTex()
{
    Ptr<CTexture> pRTTex = CAssetMgr::GetInst()->FindAsset<CTexture>(L"RenderTargetTex");
    CONTEXT->CopyResource(m_RTCopyTex->GetTex2D().Get(), pRTTex->GetTex2D().Get());
}

void CRenderMgr::CopyToPostProcessTex_HDRI()
{
    CONTEXT->CopyResource(m_PostProcessTex_HDRI->GetTex2D().Get(), m_FloatRTTex->GetTex2D().Get());
}

void CRenderMgr::CopyToPostProcessTex_LDRI()
{
    CONTEXT->CopyResource(m_PostProcessTex_LDRI->GetTex2D().Get(), CDevice::GetInst()->GetRenderTargetTex()->GetTex2D().Get());
}

void CRenderMgr::CreateRTCopyTex(Vec2 Resolution)
{
    m_RTCopyTex = CAssetMgr::GetInst()->CreateTexture(L"RTCopyTex", (UINT)Resolution.x, (UINT)Resolution.y, DXGI_FORMAT_R8G8B8A8_UNORM,
                                                      D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DEFAULT);
}

void CRenderMgr::CreatePostProcessTex(Vec2 Resolution)
{
    m_PostProcessTex_HDRI =
        CAssetMgr::GetInst()->CreateTexture(L"PostProessTex_HDRI", (UINT)Resolution.x, (UINT)Resolution.y, DXGI_FORMAT_R16G16B16A16_FLOAT,
                                            D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DEFAULT);
    m_PostProcessTex_LDRI =
        CAssetMgr::GetInst()->CreateTexture(L"PostProessTex_LDRI", (UINT)Resolution.x, (UINT)Resolution.y, DXGI_FORMAT_R8G8B8A8_UNORM,
                                            D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DEFAULT);
}

void CRenderMgr::CreateBloomTextures(Vec2 Resolution)
{
    m_BloomTextures.clear();

    for (int i = 0; i < bloomLevels - 1; i++)
    {
        int div = int(pow(2, 1 + i));
        m_BloomTextures.push_back(CAssetMgr::GetInst()->CreateTexture(L"BloomTexture " + std::to_wstring(i), UINT(Resolution.x / div),
                                                                      UINT(Resolution.y / div), DXGI_FORMAT_R16G16B16A16_FLOAT,
                                                                      D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DEFAULT));
    }
}

void CRenderMgr::CreateIDMapTex(Vec2 Resolution)
{
    m_IDMapTex = CAssetMgr::GetInst()->CreateTexture(L"IDMapTex", (UINT)Resolution.x, (UINT)Resolution.y, DXGI_FORMAT_R8G8B8A8_UNORM,
                                                     D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, D3D11_USAGE_DEFAULT);

    m_IDMapDSTex = CAssetMgr::GetInst()->CreateTexture(L"IDMapDSTex", (UINT)Resolution.x, (UINT)Resolution.y, DXGI_FORMAT_D24_UNORM_S8_UINT,
                                                       D3D11_BIND_DEPTH_STENCIL, D3D11_USAGE_DEFAULT);
}

void CRenderMgr::CreateDepthOnlyTex(Vec2 Resolution)
{
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    ZeroMemory(&dsvDesc, sizeof(dsvDesc));
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    m_DepthOnlyTex =
        CAssetMgr::GetInst()->CreateTexture(L"DepthOnlyTex", (UINT)Resolution.x, (UINT)Resolution.y, DXGI_FORMAT_R32_TYPELESS,
                                            D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL, D3D11_USAGE_DEFAULT, &dsvDesc, nullptr, &srvDesc);
}

void CRenderMgr::Resize(Vec2 Resolution)
{
    CAssetMgr::GetInst()->DeleteAsset(ASSET_TYPE::TEXTURE, L"RTCopyTex");
    CAssetMgr::GetInst()->DeleteAsset(ASSET_TYPE::TEXTURE, L"IDMapTex");
    CAssetMgr::GetInst()->DeleteAsset(ASSET_TYPE::TEXTURE, L"IDMapDSTex");
    CAssetMgr::GetInst()->DeleteAsset(ASSET_TYPE::TEXTURE, L"DepthOnlyTex");
    CAssetMgr::GetInst()->DeleteAsset(ASSET_TYPE::TEXTURE, L"PostProessTex_HDRI");
    CAssetMgr::GetInst()->DeleteAsset(ASSET_TYPE::TEXTURE, L"PostProessTex_LDRI");

    for (int i = 0; i < bloomLevels - 1; i++)
    {
        CAssetMgr::GetInst()->DeleteAsset(ASSET_TYPE::TEXTURE, L"BloomTexture " + std::to_wstring(i));
    }

    m_RTCopyTex = nullptr;
    m_IDMapTex = nullptr;
    m_IDMapDSTex = nullptr;
    m_DepthOnlyTex = nullptr;
    m_PostProcessTex_HDRI = nullptr;
    m_PostProcessTex_LDRI = nullptr;
    m_FloatRTTex = nullptr;

    CreateRTCopyTex(Resolution);
    CreateIDMapTex(Resolution);
    CreateDepthOnlyTex(Resolution);
    CreatePostProcessTex(Resolution);
    CreateBloomTextures(Resolution);

    m_FloatRTTex = CAssetMgr::GetInst()->FindAsset<CTexture>(L"FloatRenderTargetTexture");

    for (size_t i = 0; i < m_vecCam.size(); i++)
    {
        m_vecCam[i]->Resize(Resolution);
    }

    m_ToneMappingObj->MeshRender()->GetMaterial()->SetTexParam(TEX_0, m_FloatRTTex);
    m_ToneMappingObj->MeshRender()->GetMaterial()->SetTexParam(TEX_1, m_PostProcessTex_HDRI);

    m_PostEffectObj->MeshRender()->GetMaterial()->SetTexParam(TEX_0, m_FloatRTTex);
    m_PostEffectObj->MeshRender()->GetMaterial()->SetTexParam(TEX_1, m_DepthOnlyTex);
}

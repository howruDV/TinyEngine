#include "pch.h"
#include "CMeshRender.h"

#include "CAssetMgr.h"
#include "CTimeMgr.h"
#include "CRenderMgr.h"
#include "CEditorMgr.h"

#include "CMesh.h"
#include "CGraphicsShader.h"
#include "CMaterial.h"

#include "CGameObject.h"
#include "CAnimator2D.h"
#include "CAnimator.h"
#include "CTransform.h"

#include "CConstBuffer.h"
#include "CDevice.h"

#include "CLevelMgr.h"

CMeshRender::CMeshRender()
    : CRenderComponent(COMPONENT_TYPE::MESHRENDER)
    , m_bMotionBlur(false)
{
}

CMeshRender::CMeshRender(const CMeshRender& origin)
    : CRenderComponent(origin)
    , m_bMotionBlur(origin.m_bMotionBlur)
{
}

CMeshRender::~CMeshRender()
{
}

void CMeshRender::finaltick()
{
    CRenderComponent::finaltick();

    // GamePlayStatic::DrawDebugCross(Transform()->GetWorldPos(), 1.f, Vec3(0.f, 1.f, 0.f), false);
}

void CMeshRender::render()
{
    if (nullptr == GetMesh())
        return;

    // Animatio2D 업데이트
    if (Animator2D())
    {
        Animator2D()->UpdateData();
    }

    // Animator 업데이트
    if (Animator() && Animator()->IsValid())
    {
        Animator()->UpdateData();

        for (UINT i = 0; i < GetMtrlCount(); ++i)
        {
            if (nullptr == GetMaterial(i))
                continue;

            GetMaterial(i)->SetAnim3D(true); // Animation Mesh 알리기
            GetMaterial(i)->SetBoneCount(Animator()->GetBoneCount());
        }
    }

    Transform()->UpdateData();

    for (UINT i = 0; i < GetMesh()->GetSubsetCount(); ++i)
    {
        if (nullptr == GetMaterial(i))
            continue;

        GetMaterial(i)->SetMotionBlur(m_bMotionBlur);
        GetMaterial(i)->UpdateData();
        GetMesh()->render(i);
        GetMaterial(i)->SetMotionBlur(false);
    }

    // Animation 관련 정보 제거
    if (Animator2D())
    {
        Animator2D()->Clear();
    }

    if (Animator() && Animator()->IsValid())
    {
        Animator()->ClearData();
    }
}

void CMeshRender::render(Ptr<CMaterial> _mtrl)
{
    if (nullptr == GetMesh() || nullptr == _mtrl)
        return;

    // Animatio2D 업데이트
    if (Animator2D())
    {
        Animator2D()->UpdateData();
    }

    // Animator 업데이트
    if (Animator() && Animator()->IsValid())
    {
        Animator()->UpdateData();

        _mtrl->SetAnim3D(true);
        _mtrl->SetBoneCount(Animator()->GetBoneCount());
    }

    Transform()->UpdateData();
    _mtrl->SetMotionBlur(m_bMotionBlur);
    _mtrl->UpdateData();

    for (UINT i = 0; i < GetMesh()->GetSubsetCount(); ++i)
    {
        if (nullptr == GetMaterial(i))
            continue;

        GetMesh()->render(i);
    }

    // Animation 관련 정보 제거
    if (Animator2D())
    {
        Animator2D()->Clear();
    }

    if (Animator() && Animator()->IsValid())
    {
        Animator()->ClearData();

        _mtrl->SetAnim3D(false);
        _mtrl->SetBoneCount(0);
    }

    _mtrl->SetMotionBlur(false);
}

void CMeshRender::render(UINT _Subset)
{
    if (nullptr == GetMesh() || nullptr == GetMaterial(_Subset))
        return;

    // Animatio2D 업데이트
    if (Animator2D())
    {
        Animator2D()->UpdateData();
    }

    // Animator 업데이트
    if (Animator() && Animator()->IsValid())
    {
        Animator()->UpdateData();

        GetMaterial(_Subset)->SetAnim3D(true); // Animation Mesh 알리기
        GetMaterial(_Subset)->SetBoneCount(Animator()->GetBoneCount());
    }

    Transform()->UpdateData();

    // 사용할 재질 업데이트
    GetMaterial(_Subset)->SetMotionBlur(m_bMotionBlur);
    GetMaterial(_Subset)->UpdateData();

    // 사용할 메쉬 업데이트 및 렌더링
    GetMesh()->render(_Subset);

    // Animation 관련 정보 제거
    if (Animator2D())
    {
        Animator2D()->Clear();
    }

    if (Animator() && Animator()->IsValid())
    {
        Animator()->ClearData();

        GetMaterial(_Subset)->SetAnim3D(false);
        GetMaterial(_Subset)->SetBoneCount(0);
    }

    GetMaterial(_Subset)->SetMotionBlur(false);
}

UINT CMeshRender::SaveToLevelFile(FILE* _File)
{
    UINT MemoryByte = 0;

    MemoryByte += CRenderComponent::SaveToLevelFile(_File);

    fwrite(&m_bMotionBlur, sizeof(bool), 1, _File);

    MemoryByte += sizeof(bool);

    return MemoryByte;
}

UINT CMeshRender::LoadFromLevelFile(FILE* _File)
{
    UINT MemoryByte = 0;

    MemoryByte += CRenderComponent::LoadFromLevelFile(_File);

    fread(&m_bMotionBlur, sizeof(bool), 1, _File);

    MemoryByte += sizeof(bool);

    return MemoryByte;
}

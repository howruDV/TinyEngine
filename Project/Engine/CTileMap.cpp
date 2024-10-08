#include "pch.h"
#include "CTileMap.h"

#include "CStructuredBuffer.h"

#include "CAssetMgr.h"
#include "CMesh.h"

#include "CTransform.h"

CTileMap::CTileMap()
    : CRenderComponent(COMPONENT_TYPE::TILEMAP)
    , m_iTileCountX(1)
    , m_iTileCountY(1)
    , m_vTileRenderSize()
    , m_TileAtlas()
    , m_vTilePixelSize()
    , m_vSliceSizeUV()
    , m_vecTileInfo{}
    , m_TileInfoBuffer(nullptr)
{
    SetMesh(CAssetMgr::GetInst()->FindAsset<CMesh>(L"RectMesh"));
    SetMaterial(CAssetMgr::GetInst()->FindAsset<CMaterial>(L"TileMapMtrl"), 0);

    m_TileInfoBuffer = new CStructuredBuffer;

    SetTileCount(m_iTileCountX, m_iTileCountY);

    SetFrustumCheck(false);
    SetCastShadow(false);
}

CTileMap::CTileMap(const CTileMap& origin)
    : CRenderComponent(origin)
    , m_iTileCountX(origin.m_iTileCountX)
    , m_iTileCountY(origin.m_iTileCountY)
    , m_vTileRenderSize(origin.m_vTileRenderSize)
    , m_TileAtlas(origin.m_TileAtlas)
    , m_vTilePixelSize(origin.m_vTilePixelSize)
    , m_vSliceSizeUV(origin.m_vSliceSizeUV)
    , m_vecTileInfo(origin.m_vecTileInfo)
    , m_TileInfoBuffer(nullptr)
{
    if (nullptr != origin.m_TileInfoBuffer)
    {
        m_TileInfoBuffer = origin.m_TileInfoBuffer->Clone();
    }
}

CTileMap::~CTileMap()
{
    if (nullptr != m_TileInfoBuffer)
        delete m_TileInfoBuffer;
}

void CTileMap::finaltick()
{
    CRenderComponent::finaltick();

    // (타일 개수 * 타일 사이즈) 로 사이즈를 변경처리한다.
    Vec3 vTileMapSize = Vec3(m_iTileCountX * m_vTileRenderSize.x, m_iTileCountY * m_vTileRenderSize.y, 1.f);
    Transform()->SetLocalScale(vTileMapSize);
}

void CTileMap::UpdateData()
{
    // 재질에 아틀라스 텍스쳐 전달.
    GetMaterial(0)->SetTexParam(TEX_0, m_TileAtlas);

    // 타일의 가로 세로 개수
    GetMaterial(0)->SetScalarParam(INT_0, m_iTileCountX);
    GetMaterial(0)->SetScalarParam(INT_1, m_iTileCountY);

    // 아틀라스 이미지에서 타일 1개의 자르는 사이즈(UV 기준)
    GetMaterial(0)->SetScalarParam(VEC2_0, m_vSliceSizeUV);

    // 각 타일 정보를 구조화 버퍼로 이동
    m_TileInfoBuffer->SetData(m_vecTileInfo.data(), (UINT)m_vecTileInfo.size());

    // 타일 구조화 버퍼를 t22 에 바인딩
    m_TileInfoBuffer->UpdateData(22);

    // 재질 업데이트
    GetMaterial(0)->UpdateData();

    Transform()->UpdateData();
}

void CTileMap::render()
{
    UpdateData();

    GetMesh()->render(0);
}

void CTileMap::render(Ptr<CMaterial> _mtrl)
{
    // 재질에 아틀라스 텍스쳐 전달.
    _mtrl->SetTexParam(TEX_0, m_TileAtlas);

    // 타일의 가로 세로 개수
    _mtrl->SetScalarParam(INT_0, m_iTileCountX);
    _mtrl->SetScalarParam(INT_1, m_iTileCountY);

    // 아틀라스 이미지에서 타일 1개의 자르는 사이즈(UV 기준)
    _mtrl->SetScalarParam(VEC2_0, m_vSliceSizeUV);

    // 각 타일 정보를 구조화 버퍼로 이동
    m_TileInfoBuffer->SetData(m_vecTileInfo.data(), (UINT)m_vecTileInfo.size());

    // 타일 구조화 버퍼를 t22 에 바인딩
    m_TileInfoBuffer->UpdateData(22);

    // 재질 업데이트
    _mtrl->UpdateData();

    Transform()->UpdateData();

    GetMesh()->render(0);
}

void CTileMap::render(UINT _Subset)
{
    render();
}

void CTileMap::SetTileAtlas(Ptr<CTexture> _Atlas, Vec2 _TilePixelSize)
{
    m_TileAtlas = _Atlas;
    m_vTilePixelSize = _TilePixelSize;

    m_vSliceSizeUV = Vec2(m_vTilePixelSize.x / m_TileAtlas->GetWidth(), m_vTilePixelSize.y / m_TileAtlas->GetHeight());
}

void CTileMap::SetTileCount(UINT _TileCountX, UINT _TileCountY)
{
    int originX = m_iTileCountX;
    int originY = m_iTileCountY;

    m_iTileCountX = _TileCountX;
    m_iTileCountY = _TileCountY;

    vector<tTileInfo> vecTemp;
    m_vecTileInfo.swap(vecTemp);
    m_vecTileInfo.resize(m_iTileCountX * m_iTileCountY);

    if (!vecTemp.empty())
    {
        // 타일맵의 크기가 확장된 경우
        if ((int)m_iTileCountX > originX || (int)m_iTileCountY > originY)
        {
            for (int y = 0; y < originY; y++)
            {
                for (int x = 0; x < originX; x++)
                {
                    m_vecTileInfo[y * m_iTileCountX + x] = vecTemp[y * originX + x];
                }
            }
        }
        else
        {
            // 타일맵의 크기가 축소된 경우
            for (UINT y = 0; y < m_iTileCountY; y++)
            {
                for (UINT x = 0; x < m_iTileCountX; x++)
                {
                    m_vecTileInfo[y * m_iTileCountX + x] = vecTemp[y * originX + x];
                }
            }
        }
    }

    m_TileInfoBuffer->Create(sizeof(tTileInfo), m_iTileCountX * m_iTileCountY, SB_TYPE::READ_ONLY, true);
}

void CTileMap::SetTileIndex(UINT _Row, UINT _Col, UINT _ImgIdx)
{
    if (nullptr == m_TileAtlas)
        return;

    UINT idx = _Row * m_iTileCountX + _Col;

    // 렌더링할 타일 정보
    UINT MaxCol = UINT(m_TileAtlas->GetWidth() / m_vTilePixelSize.x);
    UINT iRow = _ImgIdx / MaxCol;
    UINT iCol = _ImgIdx % MaxCol;

    m_vecTileInfo[idx].vLeftTopUV =
        Vec2((iCol * m_vTilePixelSize.x) / m_TileAtlas->GetWidth(), (iRow * m_vTilePixelSize.y) / m_TileAtlas->GetHeight());

    m_vecTileInfo[idx].bRender = 1;
    m_vecTileInfo[idx].ImageIndex = _ImgIdx;
}

UINT CTileMap::SaveToLevelFile(FILE* _File)
{
    UINT MemoryByte = 0;

    MemoryByte += CRenderComponent::SaveToLevelFile(_File);

    fwrite(&m_iTileCountX, sizeof(UINT), 1, _File);
    fwrite(&m_iTileCountY, sizeof(UINT), 1, _File);
    fwrite(&m_vTileRenderSize, sizeof(Vec2), 1, _File);

    MemoryByte += SaveAssetRef(m_TileAtlas, _File);

    fwrite(&m_vTilePixelSize, sizeof(Vec2), 1, _File);
    fwrite(&m_vSliceSizeUV, sizeof(Vec2), 1, _File);

    size_t InfoCount = m_vecTileInfo.size();
    fwrite(&InfoCount, sizeof(size_t), 1, _File);
    fwrite(m_vecTileInfo.data(), sizeof(tTileInfo), InfoCount, _File);

    MemoryByte += sizeof(UINT);
    MemoryByte += sizeof(UINT);
    MemoryByte += sizeof(Vec2);
    MemoryByte += sizeof(Vec2);
    MemoryByte += sizeof(Vec2);
    MemoryByte += sizeof(size_t);
    MemoryByte += UINT(sizeof(tTileInfo) * InfoCount);

    return MemoryByte;
}

UINT CTileMap::LoadFromLevelFile(FILE* _File)
{
    UINT MemoryByte = 0;

    MemoryByte += CRenderComponent::LoadFromLevelFile(_File);

    fread(&m_iTileCountX, sizeof(UINT), 1, _File);
    fread(&m_iTileCountY, sizeof(UINT), 1, _File);
    fread(&m_vTileRenderSize, sizeof(Vec2), 1, _File);

    MemoryByte += LoadAssetRef(m_TileAtlas, _File);

    fread(&m_vTilePixelSize, sizeof(Vec2), 1, _File);
    fread(&m_vSliceSizeUV, sizeof(Vec2), 1, _File);

    size_t InfoCount = 0;
    fread(&InfoCount, sizeof(size_t), 1, _File);
    m_vecTileInfo.resize(InfoCount);
    fread(m_vecTileInfo.data(), sizeof(tTileInfo), InfoCount, _File);

    MemoryByte += sizeof(UINT);
    MemoryByte += sizeof(UINT);
    MemoryByte += sizeof(Vec2);
    MemoryByte += sizeof(Vec2);
    MemoryByte += sizeof(Vec2);
    MemoryByte += sizeof(size_t);
    MemoryByte += UINT(sizeof(tTileInfo) * InfoCount);

    return MemoryByte;
}

#include "pch.h"
#include "CAssetMgr.h"

#include "CMesh.h"
#include "CGraphicsShader.h"
#include "CModleLoader.h"

#include "CGameObject.h"
#include "CTransform.h"
#include "CMeshRender.h"

#include "CDevice.h"

CAssetMgr::CAssetMgr()
{
}

CAssetMgr::~CAssetMgr()
{
}

void CAssetMgr::init()
{
    CreateDefaultMesh();
    CreateDefaultGraphicsShader();
    CreateDefaultTexture();
    CreateDefaultMaterial();
}

tMeshData CAssetMgr::MakeCrosshair()
{
    vector<Vec3> positions;
    vector<Vec3> colors;
    vector<Vec2> texcoords;

    tMeshData meshData;
    positions.push_back(Vec3(0.f, 0.5f, 0.f));
    positions.push_back(Vec3(0.f, -0.5f, 0.f));
    positions.push_back(Vec3(-0.5f, 0.f, 0.f));
    positions.push_back(Vec3(0.5f, 0.f, 0.f));

    texcoords.push_back(Vec2(0.f, 0.f));
    texcoords.push_back(Vec2(0.f, 0.f));
    texcoords.push_back(Vec2(0.f, 0.f));
    texcoords.push_back(Vec2(0.f, 0.f));

    colors.push_back(Vec3(0.f, 1.f, 0.f));
    colors.push_back(Vec3(0.f, 1.f, 0.f));
    colors.push_back(Vec3(0.f, 1.f, 0.f));
    colors.push_back(Vec3(0.f, 1.f, 0.f));

    for (size_t i = 0; i < positions.size(); i++)
    {
        Vtx v;
        v.vPos = positions[i];
        v.vUV = texcoords[i];
        v.vColor = colors[i];
        v.vColor.w = 1.f;

        meshData.vertices.push_back(v);
    }

    meshData.indices.push_back(0);
    meshData.indices.push_back(1);
    meshData.indices.push_back(2);
    meshData.indices.push_back(3);

    return meshData;
}

tMeshData CAssetMgr::MakePoint()
{
    tMeshData meshData;
    Vtx v;
    v.vPos = Vec3(0.f, 0.f, 0.f);
    v.vColor = Vec4(1.f, 1.f, 1.f, 1.f);
    v.vUV = Vec2(0.5f, 0.5f);
    meshData.vertices.push_back(v);
    meshData.indices.push_back(0);

    return meshData;
}

tMeshData CAssetMgr::MakeCircle(const float radius, const int numSlices)
{
    tMeshData meshData;

    Vtx v;

    // 중심 점
    v.vPos = Vec3(0.f, 0.f, 0.f);
    v.vColor = Vec4(1.f, 1.f, 1.f, 1.f);
    v.vUV = Vec2(0.5f, 0.5f);
    meshData.vertices.push_back(v);

    float fTheta = 0.f;

    for (int i = 0; i <= numSlices; ++i)
    {
        fTheta = (XM_2PI / numSlices) * i;

        v.vPos = Vec3(radius * cosf(fTheta), radius * sinf(fTheta), 0.f);
        v.vNormal = Vec3(0.f, 0.f, -1.f);
        v.vColor = Vec4(1.f, 1.f, 1.f, 1.f);
        v.vUV = Vec2(cosf(fTheta), sinf(fTheta));

        meshData.vertices.push_back(v);
    }

    for (int i = 0; i < numSlices; ++i)
    {
        meshData.indices.push_back(0);
        meshData.indices.push_back(i + 2);
        meshData.indices.push_back(i + 1);
    }

    return meshData;
}

tMeshData CAssetMgr::MakeRect(const float scale, const Vec2 texScale)
{
    vector<Vec3> positions;
    vector<Vec3> colors;
    vector<Vec3> normals;
    vector<Vec2> texcoords; // 텍스춰 좌표

    // 앞면
    positions.push_back(Vec3(-0.5f, 0.5f, 0.0f) * scale);
    positions.push_back(Vec3(0.5f, 0.5f, 0.0f) * scale);
    positions.push_back(Vec3(0.5f, -0.5f, 0.0f) * scale);
    positions.push_back(Vec3(-0.5f, -0.5f, 0.0f) * scale);
    colors.push_back(Vec3(1.0f, 1.0f, 1.0f));
    colors.push_back(Vec3(1.0f, 1.0f, 1.0f));
    colors.push_back(Vec3(1.0f, 1.0f, 1.0f));
    colors.push_back(Vec3(1.0f, 1.0f, 1.0f));
    normals.push_back(Vec3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vec3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vec3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vec3(0.0f, 0.0f, -1.0f));

    // Texture Coordinates (Direct3D 9)
    // https://learn.microsoft.com/en-us/windows/win32/direct3d9/texture-coordinates
    texcoords.push_back(Vec2(0.0f, 0.0f));
    texcoords.push_back(Vec2(1.0f, 0.0f));
    texcoords.push_back(Vec2(1.0f, 1.0f));
    texcoords.push_back(Vec2(0.0f, 1.0f));

    tMeshData meshData;

    for (size_t i = 0; i < positions.size(); i++)
    {
        Vtx v;
        v.vPos = positions[i];
        v.vNormal = normals[i];
        v.vUV = texcoords[i] * texScale;
        // v.tangentModel = Vec3(1.0f, 0.0f, 0.0f);
        v.vColor = colors[i];
        v.vColor.w = 1.f;

        meshData.vertices.push_back(v);
    }
    meshData.indices = {
        0, 1, 2, 0, 2, 3, // 앞면
    };

    return meshData;
}

tMeshData CAssetMgr::MakeDebugCircle(const float radius, const int numSlices)
{
    tMeshData meshData;

    Vtx v;

    // 중심 점
    v.vPos = Vec3(0.f, 0.f, 0.f);
    v.vColor = Vec4(1.f, 1.f, 1.f, 1.f);
    v.vUV = Vec2(0.5f, 0.5f);
    meshData.vertices.push_back(v);

    float fTheta = 0.f;

    for (int i = 0; i <= numSlices; ++i)
    {
        fTheta = (XM_2PI / numSlices) * i;

        v.vPos = Vec3(radius * cosf(fTheta), radius * sinf(fTheta), 0.f);
        v.vNormal = Vec3(0.f, 0.f, -1.f);
        v.vColor = Vec4(1.f, 1.f, 1.f, 1.f);
        v.vUV = Vec2(cosf(fTheta), sinf(fTheta));

        meshData.vertices.push_back(v);
    }

    for (int i = 1; i < meshData.vertices.size(); ++i)
    {
        meshData.indices.push_back(i);
    }

    return meshData;
}

tMeshData CAssetMgr::MakeDebugRect(const float scale, const Vec2 texScale)
{
    vector<Vec3> positions;
    vector<Vec3> colors;
    vector<Vec3> normals;
    vector<Vec2> texcoords; // 텍스춰 좌표

    // 앞면
    positions.push_back(Vec3(-0.5f, 0.5f, 0.0f) * scale);
    positions.push_back(Vec3(0.5f, 0.5f, 0.0f) * scale);
    positions.push_back(Vec3(0.5f, -0.5f, 0.0f) * scale);
    positions.push_back(Vec3(-0.5f, -0.5f, 0.0f) * scale);
    colors.push_back(Vec3(1.0f, 1.0f, 1.0f));
    colors.push_back(Vec3(1.0f, 1.0f, 1.0f));
    colors.push_back(Vec3(1.0f, 1.0f, 1.0f));
    colors.push_back(Vec3(1.0f, 1.0f, 1.0f));
    normals.push_back(Vec3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vec3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vec3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vec3(0.0f, 0.0f, -1.0f));

    // Texture Coordinates (Direct3D 9)
    // https://learn.microsoft.com/en-us/windows/win32/direct3d9/texture-coordinates
    texcoords.push_back(Vec2(0.0f, 0.0f));
    texcoords.push_back(Vec2(1.0f, 0.0f));
    texcoords.push_back(Vec2(1.0f, 1.0f));
    texcoords.push_back(Vec2(0.0f, 1.0f));

    tMeshData meshData;

    for (size_t i = 0; i < positions.size(); i++)
    {
        Vtx v;
        v.vPos = positions[i];
        v.vNormal = normals[i];
        v.vUV = texcoords[i] * texScale;
        // v.tangentModel = Vec3(1.0f, 0.0f, 0.0f);
        v.vColor = colors[i];
        v.vColor.w = 1.f;

        meshData.vertices.push_back(v);
    }

    meshData.indices = {0, 1, 2, 3, 0};

    return meshData;
}

tMeshData CAssetMgr::MakeSquareGrid(const int numSlices, const int numStacks, const float scale, const Vec2 texScale)
{
    tMeshData meshData;

    float dx = 2.0f / numSlices;
    float dy = 2.0f / numStacks;

    float y = 1.0f;
    for (int j = 0; j < numStacks + 1; j++)
    {
        float x = -1.0f;
        for (int i = 0; i < numSlices + 1; i++)
        {
            Vtx v;
            v.vPos = Vec3(x, y, 0.0f) * scale;
            v.vNormal = Vec3(0.0f, 0.0f, -1.0f);
            v.vUV = Vec2(x + 1.0f, y + 1.0f) * 0.5f * texScale;
            // v.tangentModel = Vec3(1.0f, 0.0f, 0.0f);

            meshData.vertices.push_back(v);

            x += dx;
        }
        y -= dy;
    }

    for (int j = 0; j < numStacks; j++)
    {
        for (int i = 0; i < numSlices; i++)
        {
            meshData.indices.push_back((numSlices + 1) * j + i);
            meshData.indices.push_back((numSlices + 1) * j + i + 1);
            meshData.indices.push_back((numSlices + 1) * (j + 1) + i);
            meshData.indices.push_back((numSlices + 1) * (j + 1) + i);
            meshData.indices.push_back((numSlices + 1) * j + i + 1);
            meshData.indices.push_back((numSlices + 1) * (j + 1) + i + 1);
        }
    }

    return meshData;
}

tMeshData CAssetMgr::MakeBox(const float scale)
{
    vector<Vec3> positions;
    vector<Vec3> colors;
    vector<Vec3> normals;
    vector<Vec2> texcoords; // 텍스춰 좌표

    // 윗면
    positions.push_back(Vec3(-1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vec3(-1.0f, 1.0f, 1.0f) * scale);
    positions.push_back(Vec3(1.0f, 1.0f, 1.0f) * scale);
    positions.push_back(Vec3(1.0f, 1.0f, -1.0f) * scale);
    colors.push_back(Vec3(1.0f, 0.0f, 0.0f));
    colors.push_back(Vec3(1.0f, 0.0f, 0.0f));
    colors.push_back(Vec3(1.0f, 0.0f, 0.0f));
    colors.push_back(Vec3(1.0f, 0.0f, 0.0f));
    normals.push_back(Vec3(0.0f, 1.0f, 0.0f));
    normals.push_back(Vec3(0.0f, 1.0f, 0.0f));
    normals.push_back(Vec3(0.0f, 1.0f, 0.0f));
    normals.push_back(Vec3(0.0f, 1.0f, 0.0f));
    texcoords.push_back(Vec2(0.0f, 0.0f));
    texcoords.push_back(Vec2(1.0f, 0.0f));
    texcoords.push_back(Vec2(1.0f, 1.0f));
    texcoords.push_back(Vec2(0.0f, 1.0f));

    // 아랫면
    positions.push_back(Vec3(-1.0f, -1.0f, -1.0f) * scale);
    positions.push_back(Vec3(1.0f, -1.0f, -1.0f) * scale);
    positions.push_back(Vec3(1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vec3(-1.0f, -1.0f, 1.0f) * scale);
    colors.push_back(Vec3(0.0f, 1.0f, 0.0f));
    colors.push_back(Vec3(0.0f, 1.0f, 0.0f));
    colors.push_back(Vec3(0.0f, 1.0f, 0.0f));
    colors.push_back(Vec3(0.0f, 1.0f, 0.0f));
    normals.push_back(Vec3(0.0f, -1.0f, 0.0f));
    normals.push_back(Vec3(0.0f, -1.0f, 0.0f));
    normals.push_back(Vec3(0.0f, -1.0f, 0.0f));
    normals.push_back(Vec3(0.0f, -1.0f, 0.0f));
    texcoords.push_back(Vec2(0.0f, 0.0f));
    texcoords.push_back(Vec2(1.0f, 0.0f));
    texcoords.push_back(Vec2(1.0f, 1.0f));
    texcoords.push_back(Vec2(0.0f, 1.0f));

    // 앞면
    positions.push_back(Vec3(-1.0f, -1.0f, -1.0f) * scale);
    positions.push_back(Vec3(-1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vec3(1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vec3(1.0f, -1.0f, -1.0f) * scale);
    colors.push_back(Vec3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vec3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vec3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vec3(0.0f, 0.0f, 1.0f));
    normals.push_back(Vec3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vec3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vec3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vec3(0.0f, 0.0f, -1.0f));
    texcoords.push_back(Vec2(0.0f, 0.0f));
    texcoords.push_back(Vec2(1.0f, 0.0f));
    texcoords.push_back(Vec2(1.0f, 1.0f));
    texcoords.push_back(Vec2(0.0f, 1.0f));

    // 뒷면
    positions.push_back(Vec3(-1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vec3(1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vec3(1.0f, 1.0f, 1.0f) * scale);
    positions.push_back(Vec3(-1.0f, 1.0f, 1.0f) * scale);
    colors.push_back(Vec3(0.0f, 1.0f, 1.0f));
    colors.push_back(Vec3(0.0f, 1.0f, 1.0f));
    colors.push_back(Vec3(0.0f, 1.0f, 1.0f));
    colors.push_back(Vec3(0.0f, 1.0f, 1.0f));
    normals.push_back(Vec3(0.0f, 0.0f, 1.0f));
    normals.push_back(Vec3(0.0f, 0.0f, 1.0f));
    normals.push_back(Vec3(0.0f, 0.0f, 1.0f));
    normals.push_back(Vec3(0.0f, 0.0f, 1.0f));
    texcoords.push_back(Vec2(0.0f, 0.0f));
    texcoords.push_back(Vec2(1.0f, 0.0f));
    texcoords.push_back(Vec2(1.0f, 1.0f));
    texcoords.push_back(Vec2(0.0f, 1.0f));

    // 왼쪽
    positions.push_back(Vec3(-1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vec3(-1.0f, 1.0f, 1.0f) * scale);
    positions.push_back(Vec3(-1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vec3(-1.0f, -1.0f, -1.0f) * scale);
    colors.push_back(Vec3(1.0f, 1.0f, 0.0f));
    colors.push_back(Vec3(1.0f, 1.0f, 0.0f));
    colors.push_back(Vec3(1.0f, 1.0f, 0.0f));
    colors.push_back(Vec3(1.0f, 1.0f, 0.0f));
    normals.push_back(Vec3(-1.0f, 0.0f, 0.0f));
    normals.push_back(Vec3(-1.0f, 0.0f, 0.0f));
    normals.push_back(Vec3(-1.0f, 0.0f, 0.0f));
    normals.push_back(Vec3(-1.0f, 0.0f, 0.0f));
    texcoords.push_back(Vec2(0.0f, 0.0f));
    texcoords.push_back(Vec2(1.0f, 0.0f));
    texcoords.push_back(Vec2(1.0f, 1.0f));
    texcoords.push_back(Vec2(0.0f, 1.0f));

    // 오른쪽
    positions.push_back(Vec3(1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vec3(1.0f, -1.0f, -1.0f) * scale);
    positions.push_back(Vec3(1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vec3(1.0f, 1.0f, 1.0f) * scale);
    colors.push_back(Vec3(1.0f, 0.0f, 1.0f));
    colors.push_back(Vec3(1.0f, 0.0f, 1.0f));
    colors.push_back(Vec3(1.0f, 0.0f, 1.0f));
    colors.push_back(Vec3(1.0f, 0.0f, 1.0f));
    normals.push_back(Vec3(1.0f, 0.0f, 0.0f));
    normals.push_back(Vec3(1.0f, 0.0f, 0.0f));
    normals.push_back(Vec3(1.0f, 0.0f, 0.0f));
    normals.push_back(Vec3(1.0f, 0.0f, 0.0f));
    texcoords.push_back(Vec2(0.0f, 0.0f));
    texcoords.push_back(Vec2(1.0f, 0.0f));
    texcoords.push_back(Vec2(1.0f, 1.0f));
    texcoords.push_back(Vec2(0.0f, 1.0f));

    tMeshData meshData;
    for (size_t i = 0; i < positions.size(); i++)
    {
        Vtx v;
        v.vPos = positions[i];
        v.vNormal = normals[i];
        v.vUV = texcoords[i];
        v.vColor = colors[i];
        v.vColor.w = 1.f;
        meshData.vertices.push_back(v);
    }

    meshData.indices = {
        0,  1,  2,  0,  2,  3,  // 윗면
        4,  5,  6,  4,  6,  7,  // 아랫면
        8,  9,  10, 8,  10, 11, // 앞면
        12, 13, 14, 12, 14, 15, // 뒷면
        16, 17, 18, 16, 18, 19, // 왼쪽
        20, 21, 22, 20, 22, 23  // 오른쪽
    };

    return meshData;
}

tMeshData CAssetMgr::MakeCylinder(const float bottomRadius, const float topRadius, float height, int numSlices)
{
    // Texture 좌표계때문에 (numSlices + 1) x 2 개의 버텍스 사용

    const float dTheta = -XM_2PI / float(numSlices);

    tMeshData meshData;

    vector<Vtx>& vertices = meshData.vertices;

    // 옆면의 바닥 버텍스들 (인덱스 0 이상 numSlices 미만)
    for (int i = 0; i <= numSlices; i++)
    {
        Vtx v;
        v.vPos = Vec3::Transform(Vec3(bottomRadius, -0.5f * height, 0.0f), Matrix::CreateRotationY(dTheta * float(i)));

        // std::cout << v.vPos.x << " " << v.vPos.z << std::endl;

        v.vNormal = v.vPos - Vec3(0.0f, -0.5f * height, 0.0f);
        v.vNormal.Normalize();
        v.vUV = Vec2(float(i) / numSlices, 1.0f);

        vertices.push_back(v);
    }

    // 옆면의 맨 위 버텍스들 (인덱스 numSlices 이상 2 * numSlices 미만)
    for (int i = 0; i <= numSlices; i++)
    {
        Vtx v;
        v.vPos = Vec3::Transform(Vec3(topRadius, 0.5f * height, 0.0f), Matrix::CreateRotationY(dTheta * float(i)));
        v.vNormal = v.vPos - Vec3(0.0f, 0.5f * height, 0.0f);
        v.vNormal.Normalize();
        v.vUV = Vec2(float(i) / numSlices, 0.0f);

        vertices.push_back(v);
    }

    vector<UINT>& indices = meshData.indices;

    for (int i = 0; i < numSlices; i++)
    {
        indices.push_back(i);
        indices.push_back(i + numSlices + 1);
        indices.push_back(i + 1 + numSlices + 1);

        indices.push_back(i);
        indices.push_back(i + 1 + numSlices + 1);
        indices.push_back(i + 1);
    }

    return meshData;
}

tMeshData CAssetMgr::MakeSphere(const float radius, const int numSlices, const int numStacks, const Vec2 texScale)
{

    // 참고: OpenGL Sphere
    // http://www.songho.ca/opengl/gl_sphere.html
    // Texture 좌표계때문에 (numSlices + 1) 개의 버텍스 사용 (마지막에 닫아주는
    // 버텍스가 중복) Stack은 y 위쪽 방향으로 쌓아가는 방식

    const float dTheta = -XM_2PI / float(numSlices);
    const float dPhi = -XM_PI / float(numStacks);

    tMeshData meshData;

    vector<Vtx>& vertices = meshData.vertices;

    for (int j = 0; j <= numStacks; j++)
    {

        // 스택에 쌓일 수록 시작점을 x-y 평면에서 회전 시켜서 위로 올리는 구조
        Vec3 stackStartPoint = Vec3::Transform(Vec3(0.0f, -radius, 0.0f), Matrix::CreateRotationZ(dPhi * j));

        for (int i = 0; i <= numSlices; i++)
        {
            Vtx v;

            // 시작점을 x-z 평면에서 회전시키면서 원을 만드는 구조
            v.vPos = Vec3::Transform(stackStartPoint, Matrix::CreateRotationY(dTheta * float(i)));

            v.vNormal = v.vPos; // 원점이 구의 중심
            v.vNormal.Normalize();
            v.vUV = Vec2(float(i) / numSlices, 1.0f - float(j) / numStacks) * texScale;

            // texcoord가 위로 갈수록 증가
            Vec3 biTangent = Vec3(0.0f, 1.0f, 0.0f);

            Vec3 normalOrth = v.vNormal - biTangent.Dot(v.vNormal) * v.vNormal;
            normalOrth.Normalize();

            // v.tangentModel = biTangent.Cross(normalOrth);
            // v.tangentModel.Normalize();

            /*    Vec3::Transform(Vec3(0.0f, 0.0f, 1.0f),
                                   Matrix::CreateRotationY(dTheta *
               float(i)));*/
            // v.biTangentModel = Vec3(0.0f, 1.0f, 0.0f);

            vertices.push_back(v);
        }
    }

    // cout << vertices.size() << endl;

    vector<UINT>& indices = meshData.indices;

    for (int j = 0; j < numStacks; j++)
    {
        const int offset = (numSlices + 1) * j;

        for (int i = 0; i < numSlices; i++)
        {
            indices.push_back(offset + i);
            indices.push_back(offset + i + numSlices + 1);
            indices.push_back(offset + i + 1 + numSlices + 1);

            indices.push_back(offset + i);
            indices.push_back(offset + i + 1 + numSlices + 1);
            indices.push_back(offset + i + 1);
        }
    }

    return meshData;
}

tMeshData CAssetMgr::MakeTetrahedron()
{
    // Regular Tetrahedron
    // https://mathworld.wolfram.com/RegularTetrahedron.html

    const float a = 1.0f;
    const float x = sqrt(3.0f) / 3.0f * a;
    const float d = sqrt(3.0f) / 6.0f * a; // = x / 2
    const float h = sqrt(6.0f) / 3.0f * a;

    vector<Vec3> points = {{0.0f, x, 0.0f}, {-0.5f * a, -d, 0.0f}, {+0.5f * a, -d, 0.0f}, {0.0f, 0.0f, h}};

    Vec3 center = Vec3(0.0f);

    for (int i = 0; i < 4; i++)
    {
        center += points[i];
    }
    center /= 4.0f;

    for (int i = 0; i < 4; i++)
    {
        points[i] -= center;
    }

    tMeshData meshData;

    for (int i = 0; i < points.size(); i++)
    {
        Vtx v;
        v.vPos = points[i];
        v.vNormal = v.vPos; // 중심이 원점
        v.vNormal.Normalize();

        meshData.vertices.push_back(v);
    }

    meshData.indices = {0, 1, 2, 3, 2, 1, 0, 3, 1, 0, 2, 3};

    return meshData;
}

tMeshData CAssetMgr::MakeIcosahedron()
{
    // Luna DX12 교재 참고
    // 등20면체
    // https://mathworld.wolfram.com/Isohedron.html

    const float X = 0.525731f;
    const float Z = 0.850651f;

    tMeshData newMesh;

    vector<Vec3> pos = {Vec3(-X, 0.0f, Z), Vec3(X, 0.0f, Z),  Vec3(-X, 0.0f, -Z), Vec3(X, 0.0f, -Z),
                        Vec3(0.0f, Z, X),  Vec3(0.0f, Z, -X), Vec3(0.0f, -Z, X),  Vec3(0.0f, -Z, -X),
                        Vec3(Z, X, 0.0f),  Vec3(-Z, X, 0.0f), Vec3(Z, -X, 0.0f),  Vec3(-Z, -X, 0.0f)};

    for (size_t i = 0; i < pos.size(); i++)
    {
        Vtx v;
        v.vPos = pos[i];
        v.vNormal = v.vPos;
        v.vNormal.Normalize();

        newMesh.vertices.push_back(v);
    }

    newMesh.indices = {1, 4,  0, 4, 9, 0,  4, 5, 9,  8, 5, 4,  1,  8,  4, 1, 10, 8,  10, 3,
                       8, 8,  3, 5, 3, 2,  5, 3, 7,  2, 3, 10, 7,  10, 6, 7, 6,  11, 7,  6,
                       0, 11, 6, 1, 0, 10, 1, 6, 11, 0, 9, 2,  11, 9,  5, 2, 9,  11, 2,  7};

    return newMesh;
}

tMeshData CAssetMgr::SubdivideToSphere(const float radius, tMeshData meshData)
{
    // 원점이 중심이라고 가정
    for (auto& v : meshData.vertices)
    {
        v.vPos = v.vNormal * radius;
    }

    // 구의 표면으로 옮기고 노멀과 texture 좌표 계산
    auto ProjectVertex = [&](Vtx& v) {
        v.vNormal = v.vPos;
        v.vNormal.Normalize();
        v.vPos = v.vNormal * radius;

        // 주의: 텍스춰가 이음매에서 깨집니다.
        // atan vs atan2
        // https://stackoverflow.com/questions/283406/what-is-the-difference-between-atan-and-atan2-in-c
        // const float theta = atan2f(v.vPos.z, v.vPos.x);
        // const float phi = acosf(v.vPos.y / radius);
        // v.vUV.x = theta / XM_2PI;
        // v.vUV.y = phi / XM_PI;
    };

    auto UpdateFaceNormal = [](Vtx& v0, Vtx& v1, Vtx& v2) {
        auto faceNormal = (v1.vPos - v0.vPos).Cross(v2.vPos - v0.vPos);
        faceNormal.Normalize();
        v0.vNormal = faceNormal;
        v1.vNormal = faceNormal;
        v2.vNormal = faceNormal;
    };

    // 버텍스가 중복되는 구조로 구현
    tMeshData newMesh;
    UINT count = 0;
    for (size_t i = 0; i < meshData.indices.size(); i += 3)
    {
        size_t i0 = meshData.indices[i];
        size_t i1 = meshData.indices[i + 1];
        size_t i2 = meshData.indices[i + 2];

        Vtx v0 = meshData.vertices[i0];
        Vtx v1 = meshData.vertices[i1];
        Vtx v2 = meshData.vertices[i2];

        Vtx v3;
        v3.vPos = (v0.vPos + v2.vPos) * 0.5f;
        v3.vUV = (v0.vUV + v2.vUV) * 0.5f;
        ProjectVertex(v3);

        Vtx v4;
        v4.vPos = (v0.vPos + v1.vPos) * 0.5f;
        v4.vUV = (v0.vUV + v1.vUV) * 0.5f;
        ProjectVertex(v4);

        Vtx v5;
        v5.vPos = (v1.vPos + v2.vPos) * 0.5f;
        v5.vUV = (v1.vUV + v2.vUV) * 0.5f;
        ProjectVertex(v5);

        // UpdateFaceNormal(v4, v1, v5);
        // UpdateFaceNormal(v0, v4, v3);
        // UpdateFaceNormal(v3, v4, v5);
        // UpdateFaceNormal(v3, v5, v2);

        newMesh.vertices.push_back(v4);
        newMesh.vertices.push_back(v1);
        newMesh.vertices.push_back(v5);

        newMesh.vertices.push_back(v0);
        newMesh.vertices.push_back(v4);
        newMesh.vertices.push_back(v3);

        newMesh.vertices.push_back(v3);
        newMesh.vertices.push_back(v4);
        newMesh.vertices.push_back(v5);

        newMesh.vertices.push_back(v3);
        newMesh.vertices.push_back(v5);
        newMesh.vertices.push_back(v2);

        for (UINT j = 0; j < 12; j++)
        {
            newMesh.indices.push_back(j + count);
        }
        count += 12;
    }

    return newMesh;
}

vector<tMeshData> CAssetMgr::ReadFromFile(std::string basePath, std::string filename, bool revertNormals)
{
    CModelLoader modelLoader;
    modelLoader.Load(basePath, filename, revertNormals);
    vector<tMeshData>& meshes = modelLoader.meshes;

    // Normalize vertices
    Vector3 vmin(1000, 1000, 1000);
    Vector3 vmax(-1000, -1000, -1000);
    for (auto& mesh : meshes)
    {
        for (auto& v : mesh.vertices)
        {
            vmin.x = XMMin(vmin.x, v.vPos.x);
            vmin.y = XMMin(vmin.y, v.vPos.y);
            vmin.z = XMMin(vmin.z, v.vPos.z);
            vmax.x = XMMax(vmax.x, v.vPos.x);
            vmax.y = XMMax(vmax.y, v.vPos.y);
            vmax.z = XMMax(vmax.z, v.vPos.z);
        }
    }

    float dx = vmax.x - vmin.x, dy = vmax.y - vmin.y, dz = vmax.z - vmin.z;
    float dl = XMMax(XMMax(dx, dy), dz);
    float cx = (vmax.x + vmin.x) * 0.5f, cy = (vmax.y + vmin.y) * 0.5f, cz = (vmax.z + vmin.z) * 0.5f;

    for (auto& mesh : meshes)
    {
        for (auto& v : mesh.vertices)
        {
            v.vPos.x = (v.vPos.x - cx) / dl;
            v.vPos.y = (v.vPos.y - cy) / dl;
            v.vPos.z = (v.vPos.z - cz) / dl;
        }
    }

    return meshes;
}

Ptr<CMaterial> CAssetMgr::LoadModelMaterial(Ptr<CMesh> _Mesh, const tMeshData& _MeshData)
{
    CMaterial* pMtrl = new CMaterial;
    pMtrl->SetMaterialCoefficient(Vec4(), Vec4(1.f, 1.f, 1.f, 1.f), Vec4(1.f, 1.f, 1.f, 1.f), Vec4());
    pMtrl->SetShader(FindAsset<CGraphicsShader>(L"BlinnPhongShader"));

    // 텍스쳐 로딩
    std::wstring path = stringToWstring(_MeshData.RelativeTextureFilePath);

    if (!_MeshData.AmbientTextureFilename.empty())
    {
        std::wcout << _Mesh->GetName() << " : ";
        std::cout << _MeshData.AmbientTextureFilename << std::endl;

        std::wstring name = stringToWstring(_MeshData.AmbientTextureFilename);

        pMtrl->SetTexParam(TEX_0, Load<CTexture>(name, path + name));
    }

    if (!_MeshData.AoTextureFilename.empty())
    {
        std::wcout << _Mesh->GetName() << " : ";
        std::cout << _MeshData.AoTextureFilename << std::endl;

        std::wstring name = stringToWstring(_MeshData.AoTextureFilename);

        pMtrl->SetTexParam(TEX_1, Load<CTexture>(name, path + name));
    }

    if (!_MeshData.NormalTextureFilename.empty())
    {
        std::wcout << _Mesh->GetName() << " : ";
        std::cout << _MeshData.NormalTextureFilename << std::endl;

        std::wstring name = stringToWstring(_MeshData.NormalTextureFilename);

        pMtrl->SetTexParam(TEX_2, Load<CTexture>(name, path + name));
    }

    if (!_MeshData.HeightTextureFilename.empty())
    {
        std::wcout << _Mesh->GetName() << " : ";
        std::cout << _MeshData.HeightTextureFilename << std::endl;

        std::wstring name = stringToWstring(_MeshData.HeightTextureFilename);

        pMtrl->SetTexParam(TEX_3, Load<CTexture>(name, path + name));
    }

    if (!_MeshData.MetallicTextureFilename.empty())
    {
        std::wcout << _Mesh->GetName() << " : ";
        std::cout << _MeshData.MetallicTextureFilename << std::endl;

        std::wstring name = stringToWstring(_MeshData.MetallicTextureFilename);

        pMtrl->SetTexParam(TEX_4, Load<CTexture>(name, path + name));
    }

    if (!_MeshData.RoughnessTextureFilename.empty())
    {
        std::wcout << _Mesh->GetName() << " : ";
        std::cout << _MeshData.RoughnessTextureFilename << std::endl;

        std::wstring name = stringToWstring(_MeshData.RoughnessTextureFilename);

        // gLTF 포맷은 Metallic과 Roghness 를 한이미지에 같이 넣어사용함
        // 앞에서 Load한 MetallicTexture가 Roghness가 다른 텍스춰인경우에만 Load
        Ptr<CTexture> pTex = FindAsset<CTexture>(name);
        if (nullptr == pTex)
        {
            pMtrl->SetTexParam(TEX_4, Load<CTexture>(name, path + name));
        }
    }

    if (!_MeshData.EmissiveTextureFilename.empty())
    {
        std::wcout << _Mesh->GetName() << " : ";
        std::cout << _MeshData.EmissiveTextureFilename << std::endl;

        std::wstring name = stringToWstring(_MeshData.EmissiveTextureFilename);

        pMtrl->SetTexParam(TEX_5, Load<CTexture>(name, path + name));
    }

    return Ptr<CMaterial>(pMtrl);
}

void CAssetMgr::CreateDefaultMesh()
{
    // Crosshair
    {
        auto mesh = MakeCrosshair();

        Ptr<CMesh> pMesh = new CMesh;
        pMesh->Create(mesh.vertices.data(), (UINT)mesh.vertices.size(), mesh.indices.data(), (UINT)mesh.indices.size());
        pMesh->SetName(L"CrosshairMesh");
        AddAsset(L"CrosshairMesh", pMesh);
    }

    // Point
    {
        auto mesh = MakePoint();

        Ptr<CMesh> pMesh = new CMesh;
        pMesh->Create(mesh.vertices.data(), (UINT)mesh.vertices.size(), mesh.indices.data(), (UINT)mesh.indices.size());
        pMesh->SetName(L"PointMesh");
        AddAsset(L"PointMesh", pMesh);
    }

    // Circle
    {
        auto mesh = MakeCircle(1.f, 40);

        Ptr<CMesh> pMesh = new CMesh;
        pMesh->Create(mesh.vertices.data(), (UINT)mesh.vertices.size(), mesh.indices.data(), (UINT)mesh.indices.size());
        pMesh->SetName(L"CircleMesh");
        AddAsset(L"CircleMesh", pMesh);
    }

    // Rect
    {
        auto mesh = MakeRect();

        Ptr<CMesh> pMesh = new CMesh;
        pMesh->Create(mesh.vertices.data(), (UINT)mesh.vertices.size(), mesh.indices.data(), (UINT)mesh.indices.size());
        pMesh->SetName(L"RectMesh");
        AddAsset(L"RectMesh", pMesh);
    }

    // Debug Circle
    {
        auto mesh = MakeDebugCircle(1.f, 40);

        Ptr<CMesh> pMesh = new CMesh;
        pMesh->Create(mesh.vertices.data(), (UINT)mesh.vertices.size(), mesh.indices.data(), (UINT)mesh.indices.size());
        pMesh->SetName(L"CircleMesh_Debug");
        AddAsset(L"CircleMesh_Debug", pMesh);
    }

    // Debug Rect
    {
        auto mesh = MakeDebugRect();

        Ptr<CMesh> pMesh = new CMesh;
        pMesh->Create(mesh.vertices.data(), (UINT)mesh.vertices.size(), mesh.indices.data(), (UINT)mesh.indices.size());
        pMesh->SetName(L"RectMesh_Debug");
        AddAsset(L"RectMesh_Debug", pMesh);
    }

    // SquareGrid
    {
        auto mesh = MakeSquareGrid(10, 10);

        Ptr<CMesh> pMesh = new CMesh;
        pMesh->Create(mesh.vertices.data(), (UINT)mesh.vertices.size(), mesh.indices.data(), (UINT)mesh.indices.size());
        pMesh->SetName(L"SquareGridMesh");
        AddAsset(L"SquareGridMesh", pMesh);
    }

    // Box
    {
        auto mesh = MakeBox();

        Ptr<CMesh> pMesh = new CMesh;
        pMesh->Create(mesh.vertices.data(), (UINT)mesh.vertices.size(), mesh.indices.data(), (UINT)mesh.indices.size());
        pMesh->SetName(L"BoxMesh");
        AddAsset(L"BoxMesh", pMesh);
    }

    // Cylinder
    {
        auto mesh = MakeCylinder(1, 1, 1, 100);
        Ptr<CMesh> pMesh = new CMesh;
        pMesh->Create(mesh.vertices.data(), (UINT)mesh.vertices.size(), mesh.indices.data(), (UINT)mesh.indices.size());
        pMesh->SetName(L"CylinderMesh");
        AddAsset(L"CylinderMesh", pMesh);
    }

    // Sphere
    {
        auto mesh = MakeSphere(1, 10, 10);

        Ptr<CMesh> pMesh = new CMesh;
        pMesh->Create(mesh.vertices.data(), (UINT)mesh.vertices.size(), mesh.indices.data(), (UINT)mesh.indices.size());
        pMesh->SetName(L"SphereMesh");
        AddAsset(L"SphereMesh", pMesh);
    }

    // Tetrahedron
    {
        auto mesh = MakeTetrahedron();
        Ptr<CMesh> pMesh = new CMesh;
        pMesh->Create(mesh.vertices.data(), (UINT)mesh.vertices.size(), mesh.indices.data(), (UINT)mesh.indices.size());
        pMesh->SetName(L"TetrahedronMesh");
        AddAsset(L"TetrahedronMesh", pMesh);
    }

    // Icosahedron
    {
        auto mesh = MakeIcosahedron();
        Ptr<CMesh> pMesh = new CMesh;
        pMesh->Create(mesh.vertices.data(), (UINT)mesh.vertices.size(), mesh.indices.data(), (UINT)mesh.indices.size());
        pMesh->SetName(L"IcosahedronMesh");
        AddAsset(L"IcosahedronMesh", pMesh);
    }

    // Sphere
    {
        auto mesh = MakeSphere(1, 25, 25);
        mesh = SubdivideToSphere(1.f, mesh);

        Ptr<CMesh> pMesh = new CMesh;
        pMesh->Create(mesh.vertices.data(), (UINT)mesh.vertices.size(), mesh.indices.data(), (UINT)mesh.indices.size());
        pMesh->SetName(L"SubdivideSphereMesh");
        AddAsset(L"SubdivideSphereMesh", pMesh);
    }
}

CGameObject* CAssetMgr::LoadModel(const std::string& _relativepath, const std::string& _filename,
                                  const std::wstring& _name, bool revertNormals)
{
    auto meshes = ReadFromFile(_relativepath, _filename, revertNormals);

    CGameObject* model = new CGameObject;
    model->SetName(_name);
    model->AddComponent(new CTransform);

    int idx = 0;
    for (auto& meshData : meshes)
    {
        CGameObject* Parts = new CGameObject;
        Parts->SetName(_name + L" Parts " + std::to_wstring(idx));

        Ptr<CMesh> pMesh = new CMesh;
        pMesh->SetName(_name + L" Parts " + std::to_wstring(idx));
        pMesh->Create(meshData.vertices.data(), (UINT)meshData.vertices.size(), meshData.indices.data(),
                      (UINT)meshData.indices.size());
        AddAsset<CMesh>(_name + L" Parts " + std::to_wstring(idx), pMesh);

        Ptr<CMaterial> material = LoadModelMaterial(pMesh, meshData);
        AddAsset<CMaterial>(_name + L" Parts " + std::to_wstring(idx) + L"Mtrl", material.Get());

        Parts->AddComponent(new CTransform);
        Parts->AddComponent(new CMeshRender);

        Parts->Transform()->SetAbsolute(false);
        Parts->MeshRender()->SetMesh(pMesh);
        Parts->MeshRender()->SetMaterial(material);

        model->AddChild(Parts);
        ++idx;
    }

    return model;
}

void CAssetMgr::CreateDefaultGraphicsShader()
{
    // =================================
    // Std2DShader
    // =================================
    {
        Ptr<CGraphicsShader> pShader = new CGraphicsShader;
        pShader->CreateVertexShader(L"shader\\std2d.fx", "VS_Std2D");
        pShader->CreatePixelShader(L"shader\\std2d.fx", "PS_Std2D");

        pShader->SetRSType(RS_TYPE::CULL_NONE);
        pShader->SetDSType(DS_TYPE::LESS);
        pShader->SetBSType(BS_TYPE::DEFAULT);

        pShader->SetDomain(SHADER_DOMAIN::DOMAIN_MASKED);

        pShader->SetName(L"Std2DShader");
        AddAsset(L"Std2DShader", pShader);
    }

    // =================================
    // EffectShader
    // =================================
    {
        Ptr<CGraphicsShader> pShader = new CGraphicsShader;
        pShader->CreateVertexShader(L"shader\\std2d.fx", "VS_Std2D");
        pShader->CreatePixelShader(L"shader\\std2d.fx", "PS_Std2D_Effect");

        pShader->SetRSType(RS_TYPE::CULL_NONE);
        pShader->SetDSType(DS_TYPE::LESS);
        pShader->SetBSType(BS_TYPE::ALPHA_BLEND);

        pShader->SetDomain(SHADER_DOMAIN::DOMAIN_TRANSPARENT);

        AddAsset(L"EffectShader", pShader);
    }

    // =================================
    // GrayFilter Shader
    // Mesh			: RectMesh
    // RS_TYPE		: CULL_BACK
    // DS_TYPE		: NO_TEST_NO_WRITE
    // BS_TYPE		: Default
    // Domain		: DOMAIN_POSTPROCESS
    // =================================
    {
        Ptr<CGraphicsShader> pShader = new CGraphicsShader;
        pShader->CreateVertexShader(L"shader\\postprocess.fx", "VS_GrayFilter");
        pShader->CreatePixelShader(L"shader\\postprocess.fx", "PS_GrayFilter");

        pShader->SetRSType(RS_TYPE::CULL_BACK);
        pShader->SetDSType(DS_TYPE::NO_TEST_NO_WRITE);
        pShader->SetDomain(SHADER_DOMAIN::DOMAIN_POSTPROCESS);

        AddAsset(L"GrayFilterShader", pShader);
    }

    // =================================
    // DebugShape Shader
    // =================================
    {
        Ptr<CGraphicsShader> pShader = new CGraphicsShader;
        pShader->CreateVertexShader(L"shader\\debug.fx", "VS_DebugShape");
        pShader->CreatePixelShader(L"shader\\debug.fx", "PS_DebugShape");

        pShader->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
        pShader->SetRSType(RS_TYPE::CULL_NONE);
        pShader->SetBSType(BS_TYPE::DEFAULT);
        pShader->SetDSType(DS_TYPE::NO_TEST_NO_WRITE);

        pShader->SetDomain(SHADER_DOMAIN::DOMAIN_DEBUG);

        pShader->SetName(L"DebugShapeShader");
        AddAsset(L"DebugShapeShader", pShader);
    }

    // =================================
    // Basic Shader
    // =================================
    {
        Ptr<CGraphicsShader> pShader = new CGraphicsShader;
        pShader->CreateVertexShader(L"shader\\BasicVS.hlsl", "main");
        pShader->CreatePixelShader(L"shader\\BasicPS.hlsl", "main");

        pShader->SetDomain(SHADER_DOMAIN::DOMAIN_MASKED);

        pShader->SetName(L"BasicShader");
        AddAsset(L"BasicShader", pShader);
    }

    // =================================
    // BlinnPhong Shader
    // =================================
    {
        Ptr<CGraphicsShader> pShader = new CGraphicsShader;
        pShader->CreateVertexShader(L"shader\\BlinnPhongVS.hlsl", "main");
        pShader->CreatePixelShader(L"shader\\BlinnPhongPS.hlsl", "main");

        pShader->SetRSType(RS_TYPE::CULL_BACK);
        pShader->SetDSType(DS_TYPE::LESS);
        pShader->SetBSType(BS_TYPE::DEFAULT);

        pShader->SetDomain(SHADER_DOMAIN::DOMAIN_OPAQUE);

        pShader->SetName(L"BlinnPhongShader");
        AddAsset(L"BlinnPhongShader", pShader);
    }

    // =================================
    // NormalLine Shader
    // =================================
    {
        Ptr<CGraphicsShader> pShader = new CGraphicsShader;
        pShader->CreateVertexShader(L"shader\\NormalLineVS.hlsl", "main");
        pShader->CreateGeometryShader(L"shader\\NormalLineGS.hlsl", "main");
        pShader->CreatePixelShader(L"shader\\NormalLinePS.hlsl", "main");

        pShader->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

        pShader->SetDomain(SHADER_DOMAIN::DOMAIN_MASKED);

        pShader->SetName(L"NormalLineShader");
        AddAsset(L"NormalLineShader", pShader);
    }

    // =================================
    // OutLine Shader
    // =================================
    {
        Ptr<CGraphicsShader> pShader = new CGraphicsShader;
        pShader->CreateVertexShader(L"shader\\OutLineVS.hlsl", "main");
        pShader->CreatePixelShader(L"shader\\OutLinePS.hlsl", "main");

        pShader->SetRSType(RS_TYPE::CULL_FRONT); // OutLine은 뒷면만 렌더링
        pShader->SetDSType(DS_TYPE::LESS);
        pShader->SetBSType(BS_TYPE::DEFAULT);

        pShader->SetDomain(SHADER_DOMAIN::DOMAIN_OPAQUE);

        pShader->SetName(L"OutLineShader");
        AddAsset(L"OutLineShader", pShader);
    }

    // =================================
    // SkyBox Shader
    // =================================
    {
        Ptr<CGraphicsShader> pShader = new CGraphicsShader;
        pShader->CreateVertexShader(L"shader\\SkyBoxVS.hlsl", "main");
        pShader->CreatePixelShader(L"shader\\SkyboxPS.hlsl", "main");

        pShader->SetRSType(RS_TYPE::CULL_FRONT); // SkyBox는 뒷면만 렌더링
        pShader->SetDSType(DS_TYPE::NO_WRITE);
        pShader->SetBSType(BS_TYPE::DEFAULT);

        pShader->SetDomain(SHADER_DOMAIN::DOMAIN_OPAQUE);

        pShader->SetName(L"SkyboxShader");
        AddAsset(L"SkyboxShader", pShader);
    }

    // =================================
    // BillBoardPoint Shader
    // =================================
    {
        Ptr<CGraphicsShader> pShader = new CGraphicsShader;
        pShader->CreateVertexShader(L"shader\\BillBoardPointVS.hlsl", "main");
        pShader->CreateGeometryShader(L"shader\\BillBoardPointGS.hlsl", "main");
        pShader->CreatePixelShader(L"shader\\BillBoardPointPS.hlsl", "main");

        pShader->SetRSType(RS_TYPE::CULL_NONE);
        pShader->SetDSType(DS_TYPE::LESS);
        pShader->SetBSType(BS_TYPE::DEFAULT);

        pShader->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

        pShader->SetDomain(SHADER_DOMAIN::DOMAIN_OPAQUE);

        pShader->SetName(L"BillBoardPointShader");
        AddAsset(L"BillBoardPointShader", pShader);
    }

    // =================================
    // IDMap Shader
    // =================================
    {
        Ptr<CGraphicsShader> pShader = new CGraphicsShader;
        pShader->CreateVertexShader(L"shader\\IDMapVS.hlsl", "main");
        pShader->CreatePixelShader(L"shader\\IDMapPS.hlsl", "main");

        pShader->SetRSType(RS_TYPE::CULL_NONE);
        pShader->SetDSType(DS_TYPE::LESS_EQUAL);
        pShader->SetBSType(BS_TYPE::DEFAULT);

        pShader->SetDomain(SHADER_DOMAIN::DOMAIN_OPAQUE);

        pShader->SetName(L"IDMapShader");
        AddAsset(L"IDMapShader", pShader);
    }

    // =================================
    // SkyBox IDMap Shader
    // =================================
    {
        Ptr<CGraphicsShader> pShader = new CGraphicsShader;
        pShader->CreateVertexShader(L"shader\\IDMapVS.hlsl", "main");
        pShader->CreatePixelShader(L"shader\\IDMapPS.hlsl", "main");

        pShader->SetRSType(RS_TYPE::CULL_FRONT);
        pShader->SetDSType(DS_TYPE::NO_WRITE);
        pShader->SetBSType(BS_TYPE::DEFAULT);

        pShader->SetDomain(SHADER_DOMAIN::DOMAIN_OPAQUE);

        pShader->SetName(L"SkyBox_IDMapShader");
        AddAsset(L"SkyBox_IDMapShader", pShader);
    }
}

void CAssetMgr::CreateDefaultTexture()
{
    Ptr<CTexture> ptex = Load<CTexture>(L"missing_texture", L"missing_texture.png");
    assert(ptex.Get());

    Load<CTexture>(L"DirectoryIconTex", L"Icons//ContentBrowser//DirectoryIcon.png");
    Load<CTexture>(L"FileIconTex", L"Icons//ContentBrowser//FileIcon.png");

    Load<CTexture>(L"PauseButtonTex", L"Icons//PauseButton.png");
    Load<CTexture>(L"PlayButtonTex", L"Icons//PlayButton.png");
    Load<CTexture>(L"SimulateButtonTex", L"Icons//SimulateButton.png");
    Load<CTexture>(L"StepButtonTex", L"Icons//StepButton.png");
    Load<CTexture>(L"StopButtonTex", L"Icons//StopButton.png");

    Load<CTexture>(L"DirectionalLightTex", L"Icons//DirectionalLight.png");
    Load<CTexture>(L"PointLightTex", L"Icons//PointLight.png");
    Load<CTexture>(L"SpotLightTex", L"Icons//SpotLight.png");

    Load<CTexture>(L"cubemapTex", L"Developers//Textures//Cubemaps//skybox//cubemap_bgra.dds");
    Load<CTexture>(L"cubemap_diffuseTex", L"Developers//Textures//Cubemaps//skybox//cubemap_diffuse.dds");
    Load<CTexture>(L"cubemap_specularTex", L"Developers//Textures//Cubemaps//skybox//cubemap_specular.dds");
}

void CAssetMgr::CreateDefaultMaterial()
{
    // Std2DMtrl
    {
        CMaterial* pMtrl = new CMaterial;
        pMtrl->SetShader(FindAsset<CGraphicsShader>(L"Std2DShader"));
        pMtrl->SetName(L"Std2DMtrl");
        AddAsset<CMaterial>(L"Std2DMtrl", pMtrl);
    }

    // GrayFilterMtrl
    {
        CMaterial* pMtrl = new CMaterial;
        pMtrl->SetShader(FindAsset<CGraphicsShader>(L"GrayFilterShader"));
        AddAsset<CMaterial>(L"GrayFilterMtrl", pMtrl);
    }

    // DebugShapeMtrl
    {
        CMaterial* pMtrl = new CMaterial;
        pMtrl->SetShader(FindAsset<CGraphicsShader>(L"DebugShapeShader"));
        pMtrl->SetName(L"DebugShapeMtrl");
        AddAsset<CMaterial>(L"DebugShapeMtrl", pMtrl);
    }

    // Basic
    {
        CMaterial* pMtrl = new CMaterial;
        pMtrl->SetShader(FindAsset<CGraphicsShader>(L"BasicShader"));
        pMtrl->SetMaterialCoefficient(Vec4(1.f, 1.f, 0.f, 1.f), Vec4(0.5f, 0.5f, 0.5f, 1.f),
                                      Vec4(0.5f, 0.5f, 0.5f, 1.f), Vec4());
        pMtrl->SetName(L"BasicMtrl");
        AddAsset<CMaterial>(L"BasicMtrl", pMtrl);
    }

    // BlinnPhong
    {
        CMaterial* pMtrl = new CMaterial;
        pMtrl->SetShader(FindAsset<CGraphicsShader>(L"BlinnPhongShader"));
        pMtrl->SetMaterialCoefficient(Vec4(), Vec4(0.5f, 0.5f, 0.5f, 1.f), Vec4(0.5f, 0.5f, 0.5f, 1.f), Vec4());
        pMtrl->SetTexParam(TEXCUBE_0, FindAsset<CTexture>(L"cubemap_diffuseTex"));
        pMtrl->SetTexParam(TEXCUBE_1, FindAsset<CTexture>(L"cubemap_specularTex"));
        pMtrl->SetName(L"BlinnPhongMtrl");
        AddAsset<CMaterial>(L"BlinnPhongMtrl", pMtrl);
    }

    // SkyBox
    {
        CMaterial* pMtrl = nullptr;
        pMtrl = new CMaterial;
        pMtrl->SetShader(FindAsset<CGraphicsShader>(L"SkyboxShader"));
        pMtrl->SetTexParam(TEXCUBE_0, FindAsset<CTexture>(L"cubemapTex"));
        pMtrl->SetName(L"SkyboxMtrl");
        AddAsset<CMaterial>(L"SkyboxMtrl", pMtrl);
    }

    // Directional Light
    {
        CMaterial* pMtrl = new CMaterial;
        pMtrl->SetShader(FindAsset<CGraphicsShader>(L"BillBoardPointShader"));
        pMtrl->SetTexParam(TEX_0, FindAsset<CTexture>(L"DirectionalLightTex"));
        pMtrl->SetName(L"DirectionalLightMtrl");
        AddAsset<CMaterial>(L"DirectionalLightMtrl", pMtrl);
    }

    // Point Light
    {
        CMaterial* pMtrl = new CMaterial;
        pMtrl->SetShader(FindAsset<CGraphicsShader>(L"BillBoardPointShader"));
        pMtrl->SetTexParam(TEX_0, FindAsset<CTexture>(L"PointLightTex"));
        pMtrl->SetName(L"PointLightMtrl");
        AddAsset<CMaterial>(L"PointLightMtrl", pMtrl);
    }

    // Spot Light
    {
        CMaterial* pMtrl = new CMaterial;
        pMtrl->SetShader(FindAsset<CGraphicsShader>(L"BillBoardPointShader"));
        pMtrl->SetTexParam(TEX_0, FindAsset<CTexture>(L"SpotLightTex"));
        pMtrl->SetName(L"SpotLightMtrl");
        AddAsset<CMaterial>(L"SpotLightMtrl", pMtrl);
    }
}

Ptr<CTexture> CAssetMgr::CreateTexture(const wstring& _strKey, UINT _Width, UINT _Height, DXGI_FORMAT _pixelformat,
                                       UINT _BindFlag, D3D11_USAGE _Usage)
{
    Ptr<CTexture> pTex = FindAsset<CTexture>(_strKey);

    assert(nullptr == pTex);

    pTex = new CTexture();
    if (FAILED(pTex->Create(_Width, _Height, _pixelformat, _BindFlag, _Usage)))
    {
        assert(nullptr);
    }

    AddAsset<CTexture>(_strKey, pTex);

    return pTex;
}

Ptr<CTexture> CAssetMgr::CreateTexture(const wstring& _strKey, ComPtr<ID3D11Texture2D> _Tex2D)
{
    Ptr<CTexture> pTex = FindAsset<CTexture>(_strKey);

    assert(nullptr == pTex);

    pTex = new CTexture();
    if (FAILED(pTex->Create(_Tex2D)))
    {
        assert(nullptr);
    }

    pTex->SetKey(_strKey);
    AddAsset<CTexture>(_strKey, pTex);

    return pTex;
}

void CAssetMgr::DeleteAsset(ASSET_TYPE _type, const wstring& _strKey)
{
    map<wstring, Ptr<CAsset>>::iterator iter = m_mapAsset[(UINT)_type].find(_strKey);

    assert(!(iter == m_mapAsset[(UINT)_type].end()));

    m_mapAsset[(UINT)_type].erase(iter);
}

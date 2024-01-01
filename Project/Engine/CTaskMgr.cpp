#include "pch.h"

#include "CTaskMgr.h"
#include "CLevelMgr.h"
#include "CLevel.h"
#include "CGameObject.h"
#include "CComponent.h"

#include "CEngine.h"
#include "CDevice.h"
#include "CEditorMgr.h"
#include "CRenderMgr.h"
#include "CAssetMgr.h"
#include "CKeyMgr.h"

// stb
#define _CRT_SECURE_NO_WARNINGS // stb_image_write compile error fix
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

CTaskMgr::CTaskMgr()
{
}

CTaskMgr::~CTaskMgr()
{
}

void CTaskMgr::tick()
{
    {
        // 윈도우 사이즈 체크
        RECT rect;
        if (GetClientRect(CEngine::GetInst()->GetMainWind(), &rect))
        {
            Vec2 Resolution = CEngine::GetInst()->GetResolution();
            int width = rect.right - rect.left;
            int height = rect.bottom - rect.top;

            if (Resolution.x != width || Resolution.y != height)
            {
                GamePlayStatic::WindowResize(width, height);
            }
        }

        // 스크린샷
        if (KEY_TAP(KEY::PRINT))
        {
            GamePlayStatic::ScreenShot();
        }

        // Mouse color Picking
        if (KEY_TAP(KEY::LBTN))
        {
            Vec2 MousePos = CKeyMgr::GetInst()->GetMousePos();
            if (nullptr != CEditorMgr::GetInst()->GetCurEditor())
            {
                MousePos = CEditorMgr::GetInst()->GetViewportMousePos();
            }

            GamePlayStatic::MouseColorPicking(MousePos);
        }
    }

    for (size_t i = 0; i < m_vecTask.size(); ++i)
    {
        switch (m_vecTask[i].Type)
        {
        case TASK_TYPE::CREATE_OBJECT:
            {
                int LayerIdx = (int)m_vecTask[i].Param_1;
                CGameObject* Object = (CGameObject*)m_vecTask[i].Param_2;

                CLevel* pCurLevel = CLevelMgr::GetInst()->GetCurrentLevel();
                pCurLevel->AddObject(Object, LayerIdx, true);

                /*if (LEVEL_STATE::PLAY == pCurLevel->GetState())
                {
                    Object->begin();
                }*/
            }
            break;
        case TASK_TYPE::DELETE_OBJECT:
            {
                CGameObject* pDeadObj = (CGameObject*)m_vecTask[i].Param_1;

                list<CGameObject*> queue;
                queue.push_back(pDeadObj);

                // 레이어에 입력되는 오브젝트 포함, 그 밑에 달린 자식들까지 모두 Dead 처리
                while (!queue.empty())
                {
                    CGameObject* pObject = queue.front();
                    queue.pop_front();

                    pObject->m_bDead = true;

                    for (size_t i = 0; i < pObject->m_vecChild.size(); ++i)
                    {
                        queue.push_back(pObject->m_vecChild[i]);
                    }
                }
            }
            break;
        case TASK_TYPE::LEVEL_CHANGE:
            {
            }
            break;
        case TASK_TYPE::ADD_CHILD:
            break;
        case TASK_TYPE::DISCONNECT_PARENT:
            break;
        case TASK_TYPE::WINDOW_RESIZE:
            {
                UINT width = (UINT)m_vecTask[i].Param_1;
                UINT height = (UINT)m_vecTask[i].Param_2;
                if (width <= 0 || height <= 0)
                    break;

                Vec2 resolution = Vec2(width, height);
                CEngine::GetInst()->SetResolution(resolution);

                // 즉시 삭제
                CAssetMgr::GetInst()->DeleteAsset(ASSET_TYPE::TEXTURE, L"RenderTargetTex");
                CAssetMgr::GetInst()->DeleteAsset(ASSET_TYPE::TEXTURE, L"DepthStencilTex");
                CAssetMgr::GetInst()->DeleteAsset(ASSET_TYPE::TEXTURE, L"RTCopyTex");
                CAssetMgr::GetInst()->DeleteAsset(ASSET_TYPE::TEXTURE, L"IDMap");

                CDevice::GetInst()->Resize(resolution);
                CRenderMgr::GetInst()->Resize(resolution);
            }
            break;
        case TASK_TYPE::DELETE_ASSET:
            {
                ASSET_TYPE type = (ASSET_TYPE)m_vecTask[i].Param_1;
                CAsset* pAsset = (CAsset*)m_vecTask[i].Param_2;
                CAssetMgr::GetInst()->DeleteAsset(type, pAsset->GetKey());
            }
            break;
        case TASK_TYPE::SCREENSHOT:
            {
                Vec2 Resolution = CDevice::GetInst()->GetRenderResolution();
                Ptr<CTexture> pTex = CAssetMgr::GetInst()->FindAsset<CTexture>(L"RenderTargetTex");

                //// MSAA 사용시 Texture2DMS → Texture2D
                // Ptr<CTexture> tempTex = CAssetMgr::GetInst()->CreateTexture(
                //     L"tempTex", (UINT)Resolution.x, (UINT)Resolution.y, DXGI_FORMAT_R8G8B8A8_UNORM,
                //     D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DEFAULT);
                //;
                // CONTEXT->ResolveSubresource(tempTex->GetTex2D().Get(), 0, pTex->GetTex2D().Get(), 0,
                //                             DXGI_FORMAT_R8G8B8A8_UNORM);

                // Texture2D → StagingTexture
                D3D11_TEXTURE2D_DESC desc;
                pTex->GetTex2D()->GetDesc(&desc);
                desc.SampleDesc.Count = 1;
                desc.SampleDesc.Quality = 0;
                desc.BindFlags = 0;
                desc.MiscFlags = 0;
                desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ; // CPU에서 읽기 가능
                desc.Usage = D3D11_USAGE_STAGING;            // GPU에서 CPU로 보낼 데이터를 임시 보관

                ComPtr<ID3D11Texture2D> stagingTexture;
                DEVICE->CreateTexture2D(&desc, nullptr, stagingTexture.GetAddressOf());

                // 전체 복사
                CONTEXT->CopyResource(stagingTexture.Get(), pTex->GetTex2D().Get());

                // GPU → CPU
                // R8G8B8A8 이라고 가정
                std::vector<uint8_t> pixels(desc.Width * desc.Height * 4);

                D3D11_MAPPED_SUBRESOURCE ms;
                CONTEXT->Map(stagingTexture.Get(), NULL, D3D11_MAP_READ, NULL,
                             &ms); // D3D11_MAP_READ 주의

                // 텍스춰가 작을 경우에는
                // ms.RowPitch가 width * sizeof(uint8_t) * 4보다 클 수도 있어서
                // for문으로 가로줄 하나씩 복사
                uint8_t* pData = (uint8_t*)ms.pData;
                for (unsigned int h = 0; h < desc.Height; h++)
                {
                    memcpy(&pixels[h * desc.Width * 4], &pData[h * ms.RowPitch], desc.Width * sizeof(uint8_t) * 4);
                }

                CONTEXT->Unmap(stagingTexture.Get(), NULL);
                string filename = WstringTostring(CPathMgr::GetOutputPath());
                filename += "screenshot\\";

                // Check if the folder exists, if not, create it
                if (!std::filesystem::exists(filename))
                {
                    std::filesystem::create_directory(filename);
                }

                filename += currentDateTime();
                filename += ".png";
                if (stbi_write_png(filename.c_str(), desc.Width, desc.Height, 4, pixels.data(), desc.Width * 4))
                {
                    std::cout << "Screenshot! FileName : " << filename << std::endl;
                }
            }
            break;
        case TASK_TYPE::MOUSE_COLOR_PICKING:
            {
                int MouseX = (int)m_vecTask[i].Param_1;
                int MouseY = (int)m_vecTask[i].Param_2;

                Vec2 WindowSize = CDevice::GetInst()->GetRenderResolution();

                if (nullptr != CEditorMgr::GetInst()->GetCurEditor())
                {
                    Vec2 ViewportSize = CEditorMgr::GetInst()->GetViewportSize();

                    float NdcMouseX = MouseX / ViewportSize.x;
                    float NdcMouseY = MouseY / ViewportSize.y;

                    MouseX = NdcMouseX * WindowSize.x;
                    MouseY = NdcMouseY * WindowSize.y;
                }

                if (MouseX < 0 || MouseY < 0 || MouseX > WindowSize.x || MouseY > WindowSize.y)
                    break;

                Ptr<CTexture> pIDMapTex = CAssetMgr::GetInst()->FindAsset<CTexture>(L"IDMap");

                // Texture2D → StagingTexture
                D3D11_TEXTURE2D_DESC desc;
                pIDMapTex->GetTex2D()->GetDesc(&desc);
                desc.SampleDesc.Count = 1;
                desc.SampleDesc.Quality = 0;
                desc.BindFlags = 0;
                desc.MiscFlags = 0;
                desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ; // CPU에서 읽기 가능
                desc.Usage = D3D11_USAGE_STAGING;            // GPU에서 CPU로 보낼 데이터를 임시 보관

                ComPtr<ID3D11Texture2D> stagingTexture;
                DEVICE->CreateTexture2D(&desc, nullptr, stagingTexture.GetAddressOf());

                // 마우스위치의 1픽셀만 복사
                D3D11_BOX box;
                box.left = MouseX;
                box.right = MouseX + 1;
                box.top = MouseY;
                box.bottom = MouseY + 1;
                box.front = 0;
                box.back = 1;
                CONTEXT->CopySubresourceRegion(stagingTexture.Get(), 0, 0, 0, 0, pIDMapTex->GetTex2D().Get(), 0, &box);

                // R8G8B8A8 이라고 가정
                D3D11_MAPPED_SUBRESOURCE ms;
                CONTEXT->Map(stagingTexture.Get(), NULL, D3D11_MAP_READ, NULL, &ms);
                uint8_t* pData = (uint8_t*)ms.pData;
                uint8_t m_pickColor[4] = {};
                memcpy(m_pickColor, pData, sizeof(uint8_t) * 4);
                CONTEXT->Unmap(stagingTexture.Get(), NULL);

                std::cout << (float)m_pickColor[0] << " " << (float)m_pickColor[1] << " " << (float)m_pickColor[2]
                          << " " << (float)m_pickColor[3] << std::endl;

                CLevel* pCurLevel = CLevelMgr::GetInst()->GetCurrentLevel();
                for (int i = 0; i < LAYER_MAX; ++i)
                {
                    CLayer* pLayer = pCurLevel->GetLayer(i);
                    const vector<CGameObject*>& vecObjects = pLayer->GetLayerObjects();
                    for (size_t i = 0; i < vecObjects.size(); ++i)
                    {
                        std::hash<std::wstring> hasher;
                        int HashID = (int)hasher(vecObjects[i]->GetName());
                        Vec4 colorID = HashIDToColor(HashID);
                        uint8_t colorIDInt[4] = {
                            static_cast<int>(colorID[0] * 255.f), static_cast<int>(colorID[1] * 255.f),
                            static_cast<int>(colorID[2] * 255.f), static_cast<int>(colorID[3] * 255.f)};

                        if (abs(m_pickColor[0] - colorIDInt[0]) < 1e-5 && abs(m_pickColor[1] - colorIDInt[1]) < 1e-5 &&
                            abs(m_pickColor[2] - colorIDInt[2]) < 1e-5 && abs(m_pickColor[3] - colorIDInt[3]) < 1e-5)
                        {
                            CLevelMgr::GetInst()->SetSelectObj(vecObjects[i]);
                            break;
                        }
                    }
                }
            }
        }
    }

    m_vecTask.clear();
}
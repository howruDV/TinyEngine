#include "pch.h"
#include "global.h"
#include "CSkyBox.h"

tTransform g_Transform = {};
tGlobalData g_Global = tGlobalData();

const char* COMPONENT_TYPE_STRING[(UINT)COMPONENT_TYPE::END]{"Transform",
                                                             "Animator 2D",
                                                             "Animator",
                                                             "Light 2D",
                                                             "Light",
                                                             "Camera",
                                                             "Rigidbody 2D",
                                                             "Box Collider 2D",
                                                             "Circle Collider 2D",
                                                             "Polygon Collider 2D",
                                                             "Edge Collider 2D",
                                                             "Rigidbody",
                                                             "Box Collider",
                                                             "Sphere Collider",
                                                             "Capsule Collider",
                                                             "Mesh Collider",
                                                             "Character Controller",
                                                             "Sound Listener",
                                                             "MeshRender",
                                                             "TileMap",
                                                             "ParticleSystem",
                                                             "Skybox",
                                                             "Decal",
                                                             "Landscape",
                                                             "TextRender"};

const char* ASSET_TYPE_STRING[(UINT)ASSET_TYPE::END]{
    "Mesh", "Meshdata", "Prefab", "Texture", "Compute Shader", "Graphics Shader", "Material", "Sound", "Physics2D Material", "Physic Material",
};

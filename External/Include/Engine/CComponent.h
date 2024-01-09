#pragma once
#include "CEntity.h"

#include "CGameObject.h"

#define GET_OTHER_COMPONENT(Type)                                                                                      \
    C##Type* Type()                                                                                                    \
    {                                                                                                                  \
        return m_Owner->Type();                                                                                        \
    }

class CGameObject;

class CComponent : public CEntity
{
private:
    const COMPONENT_TYPE m_Type;
    CGameObject* m_Owner;

public:
    virtual void begin() {}
    virtual void tick() {}
    virtual void finaltick() = 0;
    virtual void UpdateData(){};

public:
    COMPONENT_TYPE GetType() const { return m_Type; }
    CGameObject* GetOwner() const { return m_Owner; }

public:
    virtual void SaveToLevelFile(FILE* _File) = 0;
    virtual void LoadFromLevelFile(FILE* _File) = 0;

public:
    GET_OTHER_COMPONENT(Transform);
    GET_OTHER_COMPONENT(MeshRender);
    GET_OTHER_COMPONENT(Camera);
    GET_OTHER_COMPONENT(Collider2D);
    GET_OTHER_COMPONENT(Animator2D);
    GET_OTHER_COMPONENT(Light2D);

public:
    GET_OTHER_COMPONENT(Light3D);

public:
    CComponent(COMPONENT_TYPE _Type);
    virtual ~CComponent();

    friend class CGameObject;
};

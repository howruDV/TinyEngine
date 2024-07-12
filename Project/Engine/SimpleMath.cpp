//-------------------------------------------------------------------------------------
// SimpleMath.cpp -- Simplified C++ Math wrapper for DirectXMath
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkId=248929
// http://go.microsoft.com/fwlink/?LinkID=615561
//-------------------------------------------------------------------------------------

#include "pch.h"
#include "SimpleMath.h"

/****************************************************************************
 *
 * Constants
 *
 ****************************************************************************/

namespace DirectX
{
    namespace SimpleMath
    {
        const Vector2 Vector2::Zero = {0.f, 0.f};
        const Vector2 Vector2::One = {1.f, 1.f};
        const Vector2 Vector2::UnitX = {1.f, 0.f};
        const Vector2 Vector2::UnitY = {0.f, 1.f};

        const Vector3 Vector3::Zero = {0.f, 0.f, 0.f};
        const Vector3 Vector3::One = {1.f, 1.f, 1.f};
        const Vector3 Vector3::UnitX = {1.f, 0.f, 0.f};
        const Vector3 Vector3::UnitY = {0.f, 1.f, 0.f};
        const Vector3 Vector3::UnitZ = {0.f, 0.f, 1.f};
        const Vector3 Vector3::Up = {0.f, 1.f, 0.f};
        const Vector3 Vector3::Down = {0.f, -1.f, 0.f};
        const Vector3 Vector3::Right = {1.f, 0.f, 0.f};
        const Vector3 Vector3::Left = {-1.f, 0.f, 0.f};
        const Vector3 Vector3::Forward = {0.f, 0.f, -1.f};
        const Vector3 Vector3::Backward = {0.f, 0.f, 1.f};

        const Vector4 Vector4::Zero = {0.f, 0.f, 0.f, 0.f};
        const Vector4 Vector4::One = {1.f, 1.f, 1.f, 1.f};
        const Vector4 Vector4::UnitX = {1.f, 0.f, 0.f, 0.f};
        const Vector4 Vector4::UnitY = {0.f, 1.f, 0.f, 0.f};
        const Vector4 Vector4::UnitZ = {0.f, 0.f, 1.f, 0.f};
        const Vector4 Vector4::UnitW = {0.f, 0.f, 0.f, 1.f};

        const Matrix Matrix::Identity = {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};

        const Quaternion Quaternion::Identity = {0.f, 0.f, 0.f, 1.f};
    }
}

using namespace DirectX;
using namespace DirectX::SimpleMath;

/****************************************************************************
 *
 * Quaternion
 *
 ****************************************************************************/

void Quaternion::RotateTowards(const Quaternion& target, float maxAngle, Quaternion& result) const noexcept
{
    const XMVECTOR T = XMLoadFloat4(this);

    // We can use the conjugate here instead of inverse assuming q1 & q2 are normalized.
    const XMVECTOR R = XMQuaternionMultiply(XMQuaternionConjugate(T), target);

    const float rs = XMVectorGetW(R);
    const XMVECTOR L = XMVector3Length(R);
    const float angle = 2.f * atan2f(XMVectorGetX(L), rs);
    if (angle > maxAngle)
    {
        const XMVECTOR delta = XMQuaternionRotationAxis(R, maxAngle);
        const XMVECTOR Q = XMQuaternionMultiply(delta, T);
        XMStoreFloat4(&result, Q);
    }
    else
    {
        // Don't overshoot.
        result = target;
    }
}

void Quaternion::FromToRotation(const Vector3& fromDir, const Vector3& toDir, Quaternion& result) noexcept
{
    // Melax, "The Shortest Arc Quaternion", Game Programming Gems, Charles River Media (2000).

    const XMVECTOR F = XMVector3Normalize(fromDir);
    const XMVECTOR T = XMVector3Normalize(toDir);

    const float dot = XMVectorGetX(XMVector3Dot(F, T));
    if (dot >= 1.f)
    {
        result = Identity;
    }
    else if (dot <= -1.f)
    {
        XMVECTOR axis = XMVector3Cross(F, Vector3::Right);
        if (XMVector3NearEqual(XMVector3LengthSq(axis), g_XMZero, g_XMEpsilon))
        {
            axis = XMVector3Cross(F, Vector3::Up);
        }

        const XMVECTOR Q = XMQuaternionRotationAxis(axis, XM_PI);
        XMStoreFloat4(&result, Q);
    }
    else
    {
        const XMVECTOR C = XMVector3Cross(F, T);
        XMStoreFloat4(&result, C);

        const float s = sqrtf((1.f + dot) * 2.f);
        result.x /= s;
        result.y /= s;
        result.z /= s;
        result.w = s * 0.5f;
    }
}

void Quaternion::LookRotation(const Vector3& forward, const Vector3& up, Quaternion& result) noexcept
{
    Quaternion q1;
    FromToRotation(Vector3::Forward, forward, q1);

    const XMVECTOR C = XMVector3Cross(forward, up);
    if (XMVector3NearEqual(XMVector3LengthSq(C), g_XMZero, g_XMEpsilon))
    {
        // forward and up are co-linear
        result = q1;
        return;
    }

    const XMVECTOR U = XMQuaternionMultiply(q1, Vector3::Up);

    Quaternion q2;
    FromToRotation(U, up, q2);

    XMStoreFloat4(&result, XMQuaternionMultiply(q2, q1));
}

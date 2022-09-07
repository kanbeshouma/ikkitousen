#pragma once
#include <DirectXMath.h>

//**********************************************************************************************************************************
// 
// XMFloat2ÇÃÉTÉ|Å[Ég
// 
//**********************************************************************************************************************************

//--------------------<ñﬂÇËílÇ™XMFloat2>--------------------//

// ë´ÇµéZ
inline DirectX::XMFLOAT2 operator+ (const DirectX::XMFLOAT2 a, const DirectX::XMFLOAT2 b)
{
    using namespace DirectX;

    const XMVECTOR A = XMLoadFloat2(&a);
    const XMVECTOR B = XMLoadFloat2(&b);
    const XMVECTOR Ans = A + B;
    XMFLOAT2 ret{};

    XMStoreFloat2(&ret, Ans);
    return ret;
}

// à¯Ç´éZ
inline DirectX::XMFLOAT2 operator- (const DirectX::XMFLOAT2 a, const DirectX::XMFLOAT2 b)
{
    using namespace DirectX;

    const XMVECTOR A = XMLoadFloat2(&a);
    const XMVECTOR B = XMLoadFloat2(&b);
    const XMVECTOR Ans = A - B;
    XMFLOAT2 ret;

    XMStoreFloat2(&ret, Ans);
    return ret;
}

// ä|ÇØéZ
inline DirectX::XMFLOAT2 operator* (const DirectX::XMFLOAT2 a, const DirectX::XMFLOAT2 b)
{
    using namespace DirectX;

    const XMVECTOR A = XMLoadFloat2(&a);
    const XMVECTOR B = XMLoadFloat2(&b);
    const XMVECTOR Ans = A * B;
    XMFLOAT2 ret;

    XMStoreFloat2(&ret, Ans);
    return ret;
}

// äÑÇËéZ
inline DirectX::XMFLOAT2 operator/ (const DirectX::XMFLOAT2 a, const DirectX::XMFLOAT2 b)
{
    using namespace DirectX;

    const XMVECTOR A = XMLoadFloat2(&a);
    const XMVECTOR B = XMLoadFloat2(&b);
    const XMVECTOR Ans = A / B;
    XMFLOAT2 ret;

    XMStoreFloat2(&ret, Ans);
    return ret;
}


// ä|ÇØéZ
inline DirectX::XMFLOAT2 operator * (const DirectX::XMFLOAT2 a, const float b)
{
    using namespace DirectX;

    const DirectX::XMFLOAT2 ret = {
        a.x * b,
        a.y * b
    };

    return ret;
}

// Å{ÅÅ
inline void operator += (DirectX::XMFLOAT2& a, const DirectX::XMFLOAT2 b)
{
    using namespace DirectX;
    a.x += b.x;
    a.y += b.y;
}

// ÅñÅÅ
inline void operator *= (DirectX::XMFLOAT2& a, const float b)
{
    using namespace DirectX;
    a.x *= b;
    a.y *= b;
}


inline DirectX::XMFLOAT2 operator-(DirectX::XMFLOAT2 a)
{
    using namespace DirectX;
    XMFLOAT2 ret{
         -a.x,
     -a.y
    };
    return ret;

}

//**********************************************************************************************************************************
// 
// XMFloat3ÇÃÉTÉ|Å[Ég
// 
//**********************************************************************************************************************************

// ë´ÇµéZ
inline DirectX::XMFLOAT3 operator + (const DirectX::XMFLOAT3 a, const DirectX::XMFLOAT3 b)
{
    using namespace DirectX;

    const XMVECTOR A = XMLoadFloat3(&a);
    const XMVECTOR B = XMLoadFloat3(&b);
    const XMVECTOR Ans = A + B;
    XMFLOAT3 ret;

    XMStoreFloat3(&ret, Ans);
    return ret;
}

// à¯Ç´éZ
inline DirectX::XMFLOAT3 operator - (const DirectX::XMFLOAT3 a, const DirectX::XMFLOAT3 b)
{
    using namespace DirectX;

    const XMVECTOR A = XMLoadFloat3(&a);
    const XMVECTOR B = XMLoadFloat3(&b);
    const XMVECTOR Ans = A - B;
    XMFLOAT3 ret;

    XMStoreFloat3(&ret, Ans);
    return ret;
}

// ä|ÇØéZ
inline DirectX::XMFLOAT3 operator * (const DirectX::XMFLOAT3 a, const DirectX::XMFLOAT3 b)
{
    using namespace DirectX;

    const XMVECTOR A = XMLoadFloat3(&a);
    const XMVECTOR B = XMLoadFloat3(&b);
    const XMVECTOR Ans = A * B;
    XMFLOAT3 ret;

    XMStoreFloat3(&ret, Ans);
    return ret;
}

// äÑÇËéZ
inline DirectX::XMFLOAT3 operator / (const DirectX::XMFLOAT3 a, const DirectX::XMFLOAT3 b)
{
    using namespace DirectX;

    const XMVECTOR A = XMLoadFloat3(&a);
    const XMVECTOR B = XMLoadFloat3(&b);
    const XMVECTOR Ans = A / B;
    XMFLOAT3 ret;

    XMStoreFloat3(&ret, Ans);
    return ret;
}

// ä|ÇØéZ
inline DirectX::XMFLOAT3 operator * (const DirectX::XMFLOAT3 a, const float b)
{
    using namespace DirectX;

    const DirectX::XMFLOAT3 ret = {
        a.x * b,
        a.y * b,
        a.z * b
    };

    return ret;
}

// ä|ÇØéZ
inline DirectX::XMFLOAT3 operator * (const float b, const DirectX::XMFLOAT3 a)
{
    using namespace DirectX;

    const DirectX::XMFLOAT3 ret = {
        a.x * b,
        a.y * b,
        a.z * b
    };

    return ret;
}

// Å{ÅÅ
inline void operator += (DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3 b)
{
    using namespace DirectX;


    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
}
// Å{ÅÅ
inline void operator -= (DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3 b)
{
    using namespace DirectX;


    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
}



// *=
inline void operator *= (DirectX::XMFLOAT3& a, const float b)
{
    using namespace DirectX;


    a.x *= b;
    a.y *= b;
    a.z *= b;
}

// -=
inline void operator -= (DirectX::XMFLOAT3& a, DirectX::XMFLOAT3& b)
{
    using namespace DirectX;

    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
}


inline DirectX::XMFLOAT3 operator-(DirectX::XMFLOAT3 a)
{
    using namespace DirectX;
    XMFLOAT3 ret{
         -a.x,
     -a.y,
     -a.z
    };
    return ret;

}


//**********************************************************************************************************************************
// 
//  XMFloat4ÇÃÉTÉ|Å[Ég
// 
//**********************************************************************************************************************************

// ë´ÇµéZ
inline DirectX::XMFLOAT4 operator + (const DirectX::XMFLOAT4 a, const DirectX::XMFLOAT4 b)
{
    using namespace DirectX;

    const XMVECTOR A = XMLoadFloat4(&a);
    const XMVECTOR B = XMLoadFloat4(&b);
    const XMVECTOR Ans = A + B;
    XMFLOAT4 ret;

    XMStoreFloat4(&ret, Ans);
    return ret;
}

// à¯Ç´éZ
inline DirectX::XMFLOAT4 operator - (const DirectX::XMFLOAT4 a, const DirectX::XMFLOAT4 b)
{
    using namespace DirectX;

    const XMVECTOR A = XMLoadFloat4(&a);
    const XMVECTOR B = XMLoadFloat4(&b);
    const XMVECTOR Ans = A - B;
    XMFLOAT4 ret;

    XMStoreFloat4(&ret, Ans);
    return ret;
}

// ä|ÇØéZ
inline DirectX::XMFLOAT4 operator * (const DirectX::XMFLOAT4 a, const DirectX::XMFLOAT4 b)
{
    using namespace DirectX;

    const XMVECTOR A = XMLoadFloat4(&a);
    const XMVECTOR B = XMLoadFloat4(&b);
    const XMVECTOR Ans = A * B;
    XMFLOAT4 ret;

    XMStoreFloat4(&ret, Ans);
    return ret;
}

// äÑÇËéZ
inline DirectX::XMFLOAT4 operator / (const DirectX::XMFLOAT4 a, const DirectX::XMFLOAT4 b)
{
    using namespace DirectX;

    const XMVECTOR A = XMLoadFloat4(&a);
    const XMVECTOR B = XMLoadFloat4(&b);
    const XMVECTOR Ans = A - B;
    XMFLOAT4 ret;

    XMStoreFloat4(&ret, Ans);
    return ret;
}


// *=
inline void operator *= (DirectX::XMFLOAT4& a, const float b)
{
    using namespace DirectX;


    a.x *= b;
    a.y *= b;
    a.z *= b;
    a.w *= b;
}

//****************************************************************
// 
// çsóÒåvéZÇÃÉTÉ|Å[Ég 
// 
//****************************************************************

inline DirectX::XMFLOAT4X4 operator*(const DirectX::XMFLOAT4X4 A_, const DirectX::XMFLOAT4X4 B_)
{
    DirectX::XMFLOAT4X4 result{};

    DirectX::XMMATRIX Ma = DirectX::XMLoadFloat4x4(&A_);
    DirectX::XMMATRIX Mb = DirectX::XMLoadFloat4x4(&B_);
    auto Res = DirectX::XMMatrixMultiply(Ma, Mb);
    DirectX::XMStoreFloat4x4(&result, Res);
    return result;
}

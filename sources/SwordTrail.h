#pragma once
#include"constants.h"
#include<memory>
#include<d3d11.h>
#include<DirectXMath.h>
#include<vector>
#include<wrl.h>
//****************************************************************
//
// ���̋O�ՃN���X
//
//****************************************************************

class CatmullRomSpline
{
private:
    std::vector<DirectX::XMFLOAT3> control_points;
public:
    CatmullRomSpline(const std::vector<DirectX::XMFLOAT3>& data);
    virtual ~CatmullRomSpline() = default;
    CatmullRomSpline(const CatmullRomSpline&) = delete;
    CatmullRomSpline& operator=(const CatmullRomSpline&) = delete;
    CatmullRomSpline(CatmullRomSpline&&) noexcept = delete;
    CatmullRomSpline& operator=(CatmullRomSpline&&) noexcept = delete;

    const size_t segment_count;
    void interpolate(std::vector<DirectX::XMFLOAT3>& interpolated_data, size_t steps);
    void interpolate(size_t segment, std::vector<DirectX::XMFLOAT3>& interpolated_data, size_t steps);
};


class SwordTrail final
{
   //****************************************************************
   //
   // �\����
   //
   //****************************************************************
    struct TrailData
    {
        DirectX::XMFLOAT3 mTopPoint{}; // �O�Ղ̒��_
        DirectX::XMFLOAT3 mBottomPoint{};
    };
    struct TrailVertex
    {
        DirectX::XMFLOAT3 mPosition{};  // �ʒu
        DirectX::XMFLOAT2 mTexCoord{};  // UV
        DirectX::XMFLOAT3 mNormal{};    // �@��
    };

    struct TrailConstantBuffer
    {
        float mThreshold{};
        DirectX::XMFLOAT3 mPad0;
        DirectX::XMFLOAT4 color;
    };

    //****************************************************************
    //
    //  �֐�
    //
    //****************************************************************
public:
    SwordTrail() = default;
    ~SwordTrail() = default;

    void fInitialize(ID3D11Device* pDevice_, const wchar_t* FileName_, const wchar_t* ColorMapName_);
    void fUpdate(float elapsedTime_, size_t steps);
    void fRender(ID3D11DeviceContext* pDeviceContext_, DirectX::XMFLOAT4 color = { 1.0f, 0.2f, 0.2f, 1.0f });

    void fAddTrailPoint(DirectX::XMFLOAT3 Top_, DirectX::XMFLOAT3 Bottom_);
    void fEraseTrailPoint(float elapsedTime_);
private:
    void fInterpolate(size_t steps);

    //****************************************************************
    //
    // �ϐ�
    //
    //****************************************************************
    std::vector<TrailData> mDataVec{}; // �O�Ճf�[�^�̃R���e�i
    std::vector<TrailVertex> mTrailVertexVec{}; // ���_�f�[�^�̃R���e�i

    std::vector<DirectX::XMFLOAT3> mTopPoints;
    std::vector<DirectX::XMFLOAT3> mIinterpolatedTopPoints;
    std::vector<DirectX::XMFLOAT3> mBottomPoints;
    std::vector<DirectX::XMFLOAT3> mIinterpolatedBottomPoints;
    std::unique_ptr<Constants<TrailConstantBuffer>> mConstantBuffer{};

    float mEraseTimer{};

    Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader{ nullptr };
    Microsoft::WRL::ComPtr<ID3D11PixelShader> mPixelShader{ nullptr };
    Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout{ nullptr };
    Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexBuffer{ nullptr };
    Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> mShaderResourceView{ nullptr };
    Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> mTrailColorSrv{ nullptr };
    D3D11_TEXTURE2D_DESC mTexture2DDesc{};
    Microsoft::WRL::ComPtr<ID3D11GeometryShader> mGeometryShader{ nullptr };

    //****************************************************************
    //
    // �萔
    //
    //****************************************************************
    const int mMaxTrailCount = 6030;
    const float mEraseSeparateTime = 2.0f;
};

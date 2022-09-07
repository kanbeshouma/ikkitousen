#pragma once
#include"graphics_pipeline.h"
#include"skinned_mesh.h"
#include"constants.h"
//**********************************************************
// 
//  レーザービーム
// 
//****************************************************************
class LaserBeam final
{
    //****************************************************************
    // 
    // 構造体
    // 
    //****************************************************************
    struct Data
    {
        DirectX::XMFLOAT4 mColor;
    };


    //****************************************************************
    // 
    //  関数
    // 
    //****************************************************************
public:
    LaserBeam() = default;
    ~LaserBeam() = default;

    void fInitialize(ID3D11Device* pDevice_, const wchar_t* TextureName_);
    void fUpdate();
    void fRender(GraphicsPipeline& Graphics_);
    //--------------------<セッター関数>--------------------//
    void fSetPosition(DirectX::XMFLOAT3 Start_, DirectX::XMFLOAT3 End_);
    void fSetRadius(float Radius_);
    void fSetColor(DirectX::XMFLOAT4 Color_);
    void fSetAlpha(float Alpha_);
    void fSetLengthThreshold(float Threshold_);

    //--------------------<ゲッター関数>--------------------//
    DirectX::XMFLOAT3 fGetStart()const
    {
        return mStartPoint;
    }
    DirectX::XMFLOAT3 fGetEnd()const
    {
        return mEndPoint;
    }
    float fGetRadius()const
    {
        return mRadius;
    }


private:
    void fCalcTransform(); // 姿勢の情報を算出
    void fGuiMenu();
private:
    //****************************************************************
    // 
    // 変数
    // 
    //****************************************************************

    DirectX::XMFLOAT3 mStartPoint{}; // ビームの開始地点
    DirectX::XMFLOAT3 mEndPoint{}; // ビームの開始地点
    float mRadius{};  // 半径
    float mLerpSpeed{}; // 補完の速さ
    float mLengthThreshold{}; // 長さの補完率
    DirectX::XMFLOAT4 mOrientation{}; // 回転
    DirectX::XMFLOAT3 mScale{}; // 大きさ
    std::unique_ptr<SkinnedMesh> mpSkinnedMesh{ nullptr };
    std::unique_ptr<Constants<Data>> mConstantBuffer{ nullptr };
};


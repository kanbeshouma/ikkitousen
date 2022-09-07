#pragma once
#include"graphics_pipeline.h"
#include"skinned_mesh.h"
#include"constants.h"
//**********************************************************
// 
//  ���[�U�[�r�[��
// 
//****************************************************************
class LaserBeam final
{
    //****************************************************************
    // 
    // �\����
    // 
    //****************************************************************
    struct Data
    {
        DirectX::XMFLOAT4 mColor;
    };


    //****************************************************************
    // 
    //  �֐�
    // 
    //****************************************************************
public:
    LaserBeam() = default;
    ~LaserBeam() = default;

    void fInitialize(ID3D11Device* pDevice_, const wchar_t* TextureName_);
    void fUpdate();
    void fRender(GraphicsPipeline& Graphics_);
    //--------------------<�Z�b�^�[�֐�>--------------------//
    void fSetPosition(DirectX::XMFLOAT3 Start_, DirectX::XMFLOAT3 End_);
    void fSetRadius(float Radius_);
    void fSetColor(DirectX::XMFLOAT4 Color_);
    void fSetAlpha(float Alpha_);
    void fSetLengthThreshold(float Threshold_);

    //--------------------<�Q�b�^�[�֐�>--------------------//
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
    void fCalcTransform(); // �p���̏����Z�o
    void fGuiMenu();
private:
    //****************************************************************
    // 
    // �ϐ�
    // 
    //****************************************************************

    DirectX::XMFLOAT3 mStartPoint{}; // �r�[���̊J�n�n�_
    DirectX::XMFLOAT3 mEndPoint{}; // �r�[���̊J�n�n�_
    float mRadius{};  // ���a
    float mLerpSpeed{}; // �⊮�̑���
    float mLengthThreshold{}; // �����̕⊮��
    DirectX::XMFLOAT4 mOrientation{}; // ��]
    DirectX::XMFLOAT3 mScale{}; // �傫��
    std::unique_ptr<SkinnedMesh> mpSkinnedMesh{ nullptr };
    std::unique_ptr<Constants<Data>> mConstantBuffer{ nullptr };
};


#include"LaserBeam.h"
#include"user.h"
#include"Operators.h"
void LaserBeam::fInitialize(ID3D11Device* pDevice_, const wchar_t* TextureName_)
{
    // ������
    mpSkinnedMesh = std::make_unique<SkinnedMesh>(pDevice_, "./resources/Models/Bullet/LaserBeam.fbx");
    mOrientation = { 0.0f,0.0f,0.0f,1.0f };
    mConstantBuffer = std::make_unique<Constants<Data>>(pDevice_);
    mConstantBuffer->data.mColor = { 1.0f,0.0f,0.0f,1.0f };
}

void LaserBeam::fUpdate()
{
	fCalcTransform();
	//fGuiMenu();
}

void LaserBeam::fRender(GraphicsPipeline& Graphics_)
{
    if (mLengthThreshold <= 0.0f)
    {
        return;
    }
    mConstantBuffer->bind(Graphics_.get_dc().Get(), 10);
	Graphics_.set_pipeline_preset(SHADER_TYPES::LaserBeam);
	DirectX::XMFLOAT4X4 worldMat= Math::calc_world_matrix(mScale, mOrientation, mStartPoint);
    
	mpSkinnedMesh->render(Graphics_.get_dc().Get(), worldMat, { 1.0f,1.0f,1.0f,1.0 });
}

void LaserBeam::fSetPosition(DirectX::XMFLOAT3 Start_, DirectX::XMFLOAT3 End_)
{
    mStartPoint = Start_;
    mEndPoint = End_;
}

void LaserBeam::fSetRadius(float Radius_)
{
    mRadius = Radius_;
}

void LaserBeam::fSetColor(DirectX::XMFLOAT4 Color_)
{
    mConstantBuffer->data.mColor = Color_;
}

void LaserBeam::fSetAlpha(float Alpha_)
{
    mConstantBuffer->data.mColor.w = Alpha_;
}

void LaserBeam::fSetLengthThreshold(float Threshold_)
{
    mLengthThreshold = Threshold_;
}


void LaserBeam::fCalcTransform()
{
    // �X�P�[�����Z�o
    mScale.x = mScale.z = mRadius;

    // �I�_���v�Z����
    const DirectX::XMFLOAT3 endPoint = Math::lerp(mStartPoint, mEndPoint, mLengthThreshold);
	mScale.y = Math::Length( endPoint-mStartPoint);
    
    mLerpSpeed = Math::Saturate(mLerpSpeed);

    // ��]���Z�o
	// ���݂̏�x�N�g��
    DirectX::XMFLOAT3 up{ 0.001f,1.0f,0.0f };
    up = Math::Normalize(up);
    // �I�_�Ƃ̃x�N�g��
    DirectX::XMFLOAT3 cylinderUp = { mEndPoint-mStartPoint };
    cylinderUp = Math::Normalize(cylinderUp);

    // �O�ςŉ�]�����Z�o
    auto cross = Math::Cross(up, cylinderUp);
	cross = Math::Normalize(cross);
    // �������O�̎��N���b�V������̂��߉��̒l����
    if(Math::Length(cross)<=0.0f)
    {
        cross = { 0.0f,1.0f,0.0f };
    }
    // ���ςŉ�]�p���Z�o
    auto dot = Math::Dot(up, cylinderUp);
	dot = acosf(dot);
	DirectX::XMFLOAT4 dummy{ 0.0f,0.0f,0.0f,1.0f };
    auto rotQua=Math::RotQuaternion(dummy, cross, dot);

    // �⊮
      auto res= DirectX::XMQuaternionSlerp(
        DirectX::XMLoadFloat4(&mOrientation),
        DirectX::XMLoadFloat4(&rotQua),
        1.0f);
      
      DirectX::XMStoreFloat4(&mOrientation, res);
}

void LaserBeam::fGuiMenu()
{
#ifdef USE_IMGUI
	ImGui::Begin("LaserBeam");
	ImGui::DragFloat3("StartPoint", &mStartPoint.x);
	ImGui::DragFloat3("EndPoint", &mEndPoint.x);
    ImGui::SliderFloat("LerpSpeed", &mLerpSpeed,0.0f,1.0f);
	ImGui::End();
#endif


}

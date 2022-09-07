#include"LaserBeam.h"
#include"user.h"
#include"Operators.h"
void LaserBeam::fInitialize(ID3D11Device* pDevice_, const wchar_t* TextureName_)
{
    // 初期化
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
    // スケールを算出
    mScale.x = mScale.z = mRadius;

    // 終点を計算する
    const DirectX::XMFLOAT3 endPoint = Math::lerp(mStartPoint, mEndPoint, mLengthThreshold);
	mScale.y = Math::Length( endPoint-mStartPoint);
    
    mLerpSpeed = Math::Saturate(mLerpSpeed);

    // 回転を算出
	// 現在の上ベクトル
    DirectX::XMFLOAT3 up{ 0.001f,1.0f,0.0f };
    up = Math::Normalize(up);
    // 終点とのベクトル
    DirectX::XMFLOAT3 cylinderUp = { mEndPoint-mStartPoint };
    cylinderUp = Math::Normalize(cylinderUp);

    // 外積で回転軸を算出
    auto cross = Math::Cross(up, cylinderUp);
	cross = Math::Normalize(cross);
    // 長さが０の時クラッシュ回避のため仮の値を代入
    if(Math::Length(cross)<=0.0f)
    {
        cross = { 0.0f,1.0f,0.0f };
    }
    // 内積で回転角を算出
    auto dot = Math::Dot(up, cylinderUp);
	dot = acosf(dot);
	DirectX::XMFLOAT4 dummy{ 0.0f,0.0f,0.0f,1.0f };
    auto rotQua=Math::RotQuaternion(dummy, cross, dot);

    // 補完
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

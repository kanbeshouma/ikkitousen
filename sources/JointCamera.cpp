#include "camera.h"

// eyeとtarget(focus)を外から指定できるカメラ
// up は{0,1,0}


void JointCamera::Initialize(GraphicsPipeline& graphics)
{
	HRESULT hr{ S_OK };
	//----定数バッファ----//
	// カメラ関連
	scene_constants = std::make_unique<Constants<SceneConstants>>(graphics.get_device().Get());
}

void JointCamera::Update(float elapsedTime)
{
	DebugGUI();
	UpdateViewProjection();
}
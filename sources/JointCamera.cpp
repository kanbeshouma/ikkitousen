#define _WINSOCKAPI_  // windows.h���`�����ۂɁAwinsock.h�������I�ɃC���N���[�h���Ȃ�

#include "camera.h"

// eye��target(focus)���O����w��ł���J����
// up ��{0,1,0}


void JointCamera::Initialize(GraphicsPipeline& graphics)
{
	HRESULT hr{ S_OK };
	//----�萔�o�b�t�@----//
	// �J�����֘A
	scene_constants = std::make_unique<Constants<SceneConstants>>(graphics.get_device().Get());
}

void JointCamera::Update(float elapsedTime)
{
	DebugGUI();
	UpdateViewProjection();
}
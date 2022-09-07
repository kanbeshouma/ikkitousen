#pragma once

#include "skinned_mesh.h"
#include "graphics_pipeline.h"
#include "constants.h"

class PointLights
{
public:
	//--------< �R���X�g���N�^/�֐��� >--------//
	PointLights(GraphicsPipeline& graphics);
	~PointLights();

	void render(GraphicsPipeline& graphics, float elapsed_time);
	void finalize(GraphicsPipeline& graphics);
	//--------<getter/setter>--------//
	void set_all_parameters(int index, const DirectX::XMFLOAT4& pos, const DirectX::XMFLOAT4& color, float range, float luminous)
	{
		assert(index < POINT_LIGHT_COUNT && "�|�C���g���C�g�̃C���f�b�N�X���|�C���g���C�g�̍ő吔�������Ă��܂�");
		plig_constants->data.point_lights[index].position           = pos;
		plig_constants->data.point_lights[index].color              = color;
		plig_constants->data.point_lights[index].range              = range;
		plig_constants->data.point_lights[index].luminous_intensity = luminous;
	}
	void set_position(int index, const DirectX::XMFLOAT4& pos)
	{
		assert(index < POINT_LIGHT_COUNT && "�|�C���g���C�g�̃C���f�b�N�X���|�C���g���C�g�̍ő吔�������Ă��܂�");
		plig_constants->data.point_lights[index].position = pos;
	}
	void set_color(int index, const DirectX::XMFLOAT4& color)
	{
		assert(index < POINT_LIGHT_COUNT && "�|�C���g���C�g�̃C���f�b�N�X���|�C���g���C�g�̍ő吔�������Ă��܂�");
		plig_constants->data.point_lights[index].color = color;
	}
	void set_range(int index, float range)
	{
		assert(index < POINT_LIGHT_COUNT && "�|�C���g���C�g�̃C���f�b�N�X���|�C���g���C�g�̍ő吔�������Ă��܂�");
		plig_constants->data.point_lights[index].range = range;
	}
private:
	// �������x�I�ɍő��32���炢
	static const int POINT_LIGHT_COUNT = 8;
	struct PointLightParameters
	{
		DirectX::XMFLOAT4 position{ 0, 0, 0, 0 };
		DirectX::XMFLOAT4 color{ 1, 1, 1, 1 };
		float range{ 0 };
		float luminous_intensity{ 1.0f };
		DirectX::XMFLOAT2 pad{};
	};
	struct PointLightConstants
	{
		PointLightParameters point_lights[POINT_LIGHT_COUNT];
		int unique_id{};
		DirectX::XMINT3 pad;
	};
	std::unique_ptr<Constants<PointLightConstants>> plig_constants;
	// �������f��
	std::unique_ptr<SkinnedMesh> light_sources[POINT_LIGHT_COUNT];
};
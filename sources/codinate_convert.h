#pragma once

#include <DirectXMath.h>
#include <d3d11.h>

//  3D �� 2D�ւ̍��W�ϊ�
const DirectX::XMFLOAT2 conversion_2D(ID3D11DeviceContext* dc, const DirectX::XMFLOAT3& coordinate);
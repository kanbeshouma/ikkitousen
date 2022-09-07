#include <memory>

#include "debug_figures.h"
#include "shader.h"
#include "misc.h"
#include "user.h"
#include "debug_flags.h"

DebugFigures::DebugFigures(ID3D11Device* device)
{
    HRESULT hr{ S_OK };
    // vs
    {
        // 入力レイアウト
        D3D11_INPUT_ELEMENT_DESC input_element_desc[]
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
            D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        create_vs_from_cso(device, "shaders/debug_figure_vs.cso", vertex_shader.GetAddressOf(),
            input_layout.GetAddressOf(), input_element_desc, ARRAYSIZE(input_element_desc));
    }
    // ps
    {
        create_ps_from_cso(device, "shaders/debug_figure_ps.cso", pixel_shader.GetAddressOf());
    }
    // 定数バッファ
	figure_constants = std::make_unique<Constants<FigureConstants>>(device);
    // 球メッシュ作成
    create_sphere_mesh(device, 1.0f, 16, 16);
    // 円柱メッシュ作成
    create_cylinder_mesh(device, 1.0f, 1.0f, 0.0f, 1.0f, 16, 1);
    // 立方体メッシュ作成
	create_cuboid_mesh(device, 1.0f, 1.0f, 1.0f);
    // カプセルメッシュ作成
	create_capsule_mesh(device, 1.0f, 1.0f, { 0,0,0 }, { 0,1,0 }, 16);
}

void DebugFigures::render_all_figures(ID3D11DeviceContext* context)
{
#ifdef _DEBUG
	// シェーダー設定
	context->VSSetShader(vertex_shader.Get(), nullptr, 0);
	context->PSSetShader(pixel_shader.Get(), nullptr, 0);
	context->IASetInputLayout(input_layout.Get());
	// プリミティブ設定
	UINT stride = sizeof(DirectX::XMFLOAT3);
	UINT offset = 0;
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	// 球描画
	context->IASetVertexBuffers(0, 1, sphere_vertex_buffer.GetAddressOf(), &stride, &offset);
	for (const Sphere& sphere : spheres)
	{
		// ワールドビュープロジェクション行列作成
		DirectX::XMMATRIX S = DirectX::XMMatrixScaling(sphere.radius, sphere.radius, sphere.radius);
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(sphere.center.x, sphere.center.y, sphere.center.z);
		DirectX::XMMATRIX W = S * T;
		// 定数バッファ更新
		figure_constants->data.material_color = sphere.color;
		DirectX::XMStoreFloat4x4(&figure_constants->data.world, W);

		figure_constants->bind(context, 0, CB_FLAG::VS);
		if (DebugFlags::get_wireframe_switching()) context->Draw(sphere_vertex_count, 0);
	}
	spheres.clear();
	// 円柱描画
	context->IASetVertexBuffers(0, 1, cylinder_vertex_buffer.GetAddressOf(), &stride, &offset);
	for (const Cylinder& cylinder : cylinders)
	{
		// ワールドビュープロジェクション行列作成
		DirectX::XMMATRIX S = DirectX::XMMatrixScaling(cylinder.radius, cylinder.height, cylinder.radius);
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(cylinder.position.x, cylinder.position.y, cylinder.position.z);
		DirectX::XMMATRIX W = S * T;
		// 定数バッファ更新
		figure_constants->data.material_color = cylinder.color;
		DirectX::XMStoreFloat4x4(&figure_constants->data.world, W);

		figure_constants->bind(context, 0, CB_FLAG::VS);
		if (DebugFlags::get_wireframe_switching()) context->Draw(cylinder_vertex_count, 0);
	}
	cylinders.clear();
	// 直方体描画
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetVertexBuffers(0, 1, cuboid_vertex_buffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(cuboid_index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	for (const Cuboid& cuboid : cuboids)
	{
		// ワールドビュープロジェクション行列作成
		DirectX::XMMATRIX S = DirectX::XMMatrixScaling(cuboid.radius.x, cuboid.radius.y, cuboid.radius.z);
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(cuboid.position.x, cuboid.position.y, cuboid.position.z);
		DirectX::XMMATRIX W = S * T;
		// 定数バッファ更新
		figure_constants->data.material_color = cuboid.color;
		DirectX::XMStoreFloat4x4(&figure_constants->data.world, W);

		figure_constants->bind(context, 0, CB_FLAG::VS);
		D3D11_BUFFER_DESC buffer_desc{};
		cuboid_index_buffer->GetDesc(&buffer_desc);
		if (DebugFlags::get_wireframe_switching()) context->DrawIndexed(buffer_desc.ByteWidth / sizeof(uint32_t), 0, 0);
	}
	cuboids.clear();
	// カプセル描画
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	context->IASetVertexBuffers(0, 1, capsule_vertex_buffer.GetAddressOf(), &stride, &offset);
	for (const Capsule& capsule : capsules)
	{
		using namespace DirectX;
		// ワールドビュープロジェクション行列作成
		XMFLOAT3 n(0, 1, 0); // 軸（正規化）
		XMFLOAT4 orientation = {
			sinf(XMConvertToRadians(0) / 2) * n.x,
			sinf(XMConvertToRadians(0) / 2) * n.y,
			sinf(XMConvertToRadians(0) / 2) * n.z,
			cosf(XMConvertToRadians(0) / 2)
		};
		// XMVECTORクラスへ変換
		XMVECTOR orientation_vec = XMLoadFloat4(&orientation);
		XMFLOAT3 d               = { capsule.end.x - capsule.start.x, capsule.end.y - capsule.start.y, capsule.end.z - capsule.start.z };
		XMVECTOR d_norm          = XMVector3Normalize(XMLoadFloat3(&d));
		XMVECTOR d_length_vec    = XMVector3Length(XMLoadFloat3(&d));
		float d_length;
		XMStoreFloat(&d_length, d_length_vec);
		if (d_length <= capsule.radius * 2) continue;
		XMFLOAT3 d2 = { FLT_EPSILON, capsule.end.y - capsule.start.y + FLT_EPSILON, FLT_EPSILON };
		XMVECTOR d2_norm = XMVector3Normalize(XMLoadFloat3(&d2));
		XMVECTOR dot = XMVector3Dot(d2_norm, d_norm);
		float angle;
		XMStoreFloat(&angle, dot);
		angle = acosf(angle);
		if (fabsf(angle + FLT_EPSILON) > DirectX::XMConvertToRadians(0.1f))
		{
		    XMVECTOR axis = XMVector3Cross(d2_norm, d_norm);
			XMVECTOR q = XMQuaternionRotationAxis(axis, angle);
			orientation_vec = XMQuaternionMultiply(orientation_vec, q);
		}

		XMVECTOR up_vec = { 0,1,0 };
		float decision_dot;
		XMStoreFloat(&decision_dot, XMVector3Dot(-d_norm, up_vec));
		XMMATRIX S;
		if (decision_dot <= 0) { S = XMMatrixScaling(capsule.radius, d_length, capsule.radius); }
		else { S = XMMatrixScaling(capsule.radius, -d_length, capsule.radius); }
		XMMATRIX R{ XMMatrixRotationQuaternion(orientation_vec) };
		XMMATRIX T = XMMatrixTranslation(capsule.start.x, capsule.start.y, capsule.start.z);
		XMMATRIX W = S * R * T;
		// 定数バッファ更新
		figure_constants->data.material_color = capsule.color;
		XMStoreFloat4x4(&figure_constants->data.world, W);

		figure_constants->bind(context, 0, CB_FLAG::VS);
		if (DebugFlags::get_wireframe_switching()) context->Draw(capsule_vertex_count, 0);
	}
	capsules.clear();
#endif // _DEBUG
}

void DebugFigures::create_sphere(const DirectX::XMFLOAT3& center, float radius, const DirectX::XMFLOAT4& color)
{
#ifdef _DEBUG
	Sphere sphere;
	sphere.center = center;
	sphere.radius = radius;
	sphere.color = color;
	spheres.emplace_back(sphere);
#endif // _DEBUG
}

void DebugFigures::create_cylinder(const DirectX::XMFLOAT3& position, float radius, float height, const DirectX::XMFLOAT4& color)
{
#ifdef _DEBUG
	Cylinder cylinder;
	cylinder.position = position;
	cylinder.radius = radius;
	cylinder.height = height;
	cylinder.color = color;
	cylinders.emplace_back(cylinder);
#endif // _DEBUG
}

void DebugFigures::create_cuboid(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& radius, const DirectX::XMFLOAT4& color)
{
#ifdef _DEBUG
	Cuboid cuboid;
	cuboid.position = position;
	cuboid.radius = radius;
	cuboid.color = color;
	cuboids.emplace_back(cuboid);
#endif // _DEBUG
}

void DebugFigures::create_capsule(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, float radius, const DirectX::XMFLOAT4& color)
{
#ifdef _DEBUG
	Capsule capsule;
	capsule.start  = start;
	capsule.end    = end;
	capsule.radius = radius;
	capsule.color  = color;
	capsules.emplace_back(capsule);

	create_sphere(start, radius, color);
	create_sphere(end, radius, color);
#endif // _DEBUG
}

void DebugFigures::create_sphere_mesh(ID3D11Device* device, float radius, int slices, int stacks)
{
	sphere_vertex_count = stacks * slices * 2 + slices * stacks * 2;
	std::unique_ptr<DirectX::XMFLOAT3[]> vertices = std::make_unique<DirectX::XMFLOAT3[]>(sphere_vertex_count);

	float phiStep = DirectX::XM_PI / stacks;
	float thetaStep = DirectX::XM_2PI / slices;

	DirectX::XMFLOAT3* p = vertices.get();

	for (int i = 0; i < stacks; ++i)
	{
		float phi = i * phiStep;
		float y = radius * cosf(phi);
		float r = radius * sinf(phi);

		for (int j = 0; j < slices; ++j)
		{
			float theta = j * thetaStep;
			p->x = r * sinf(theta);
			p->y = y;
			p->z = r * cosf(theta);
			p++;

			theta += thetaStep;

			p->x = r * sinf(theta);
			p->y = y;
			p->z = r * cosf(theta);
			p++;
		}
	}
	thetaStep = DirectX::XM_2PI / stacks;
	for (int i = 0; i < slices; ++i)
	{
		DirectX::XMMATRIX M = DirectX::XMMatrixRotationY(i * thetaStep);
		for (int j = 0; j < stacks; ++j)
		{
			float theta = j * thetaStep;
			DirectX::XMVECTOR V1 = DirectX::XMVectorSet(radius * sinf(theta), radius * cosf(theta), 0.0f, 1.0f);
			DirectX::XMVECTOR P1 = DirectX::XMVector3TransformCoord(V1, M);
			DirectX::XMStoreFloat3(p++, P1);

			int n = (j + 1) % stacks;
			theta += thetaStep;

			DirectX::XMVECTOR V2 = DirectX::XMVectorSet(radius * sinf(theta), radius * cosf(theta), 0.0f, 1.0f);
			DirectX::XMVECTOR P2 = DirectX::XMVector3TransformCoord(V2, M);
			DirectX::XMStoreFloat3(p++, P2);
		}
	}
	// 頂点バッファ
	{
		D3D11_BUFFER_DESC desc = {};
		D3D11_SUBRESOURCE_DATA subresourceData = {};

		desc.ByteWidth = static_cast<UINT>(sizeof(DirectX::XMFLOAT3) * sphere_vertex_count);
		desc.Usage = D3D11_USAGE_IMMUTABLE;	// D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		subresourceData.pSysMem = vertices.get();
		subresourceData.SysMemPitch = 0;
		subresourceData.SysMemSlicePitch = 0;

		HRESULT hr = device->CreateBuffer(&desc, &subresourceData, sphere_vertex_buffer.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
}

void DebugFigures::create_cylinder_mesh(ID3D11Device* device, float radius1, float radius2, float start, float height, int slices, int stacks)
{
	cylinder_vertex_count = 2 * slices * (stacks + 1) + 2 * slices;
	std::unique_ptr<DirectX::XMFLOAT3[]> vertices = std::make_unique<DirectX::XMFLOAT3[]>(cylinder_vertex_count);

	DirectX::XMFLOAT3* p = vertices.get();

	float stackHeight = height / stacks;
	float radiusStep = (radius2 - radius1) / stacks;

	// vertices of ring
	float dTheta = DirectX::XM_2PI / slices;

	for (int i = 0; i < slices; ++i)
	{
		int n = (i + 1) % slices;

		float c1 = cosf(i * dTheta);
		float s1 = sinf(i * dTheta);

		float c2 = cosf(n * dTheta);
		float s2 = sinf(n * dTheta);

		for (int j = 0; j <= stacks; ++j)
		{
			float y = start + j * stackHeight;
			float r = radius1 + j * radiusStep;

			p->x = r * c1;
			p->y = y;
			p->z = r * s1;
			p++;

			p->x = r * c2;
			p->y = y;
			p->z = r * s2;
			p++;
		}

		p->x = radius1 * c1;
		p->y = start;
		p->z = radius1 * s1;
		p++;

		p->x = radius2 * c1;
		p->y = start + height;
		p->z = radius2 * s1;
		p++;
	}

	// 頂点バッファ
	{
		D3D11_BUFFER_DESC desc = {};
		D3D11_SUBRESOURCE_DATA subresourceData = {};

		desc.ByteWidth = static_cast<UINT>(sizeof(DirectX::XMFLOAT3) * cylinder_vertex_count);
		desc.Usage = D3D11_USAGE_IMMUTABLE;	// D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		subresourceData.pSysMem = vertices.get();
		subresourceData.SysMemPitch = 0;
		subresourceData.SysMemSlicePitch = 0;

		HRESULT hr = device->CreateBuffer(&desc, &subresourceData, cylinder_vertex_buffer.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
}

void DebugFigures::create_cuboid_mesh(ID3D11Device* device, float radius1, float radius2, float radius3)
{
	// vertex
	cuboid_vertex_count = 24;
	std::unique_ptr<DirectX::XMFLOAT3[]> vertices = std::make_unique<DirectX::XMFLOAT3[]>(cuboid_vertex_count);
	{
		/*0~3　上面*/
		{
			vertices[0] = { -radius1, radius2,  radius3 };
			vertices[1] = {  radius1, radius2,  radius3 };
			vertices[2] = {  radius1, radius2, -radius3 };
			vertices[3] = { -radius1, radius2, -radius3 };
		}
		/*4~7　右側面*/
		{
			vertices[4] = { radius1,  radius2,  radius3 };
			vertices[5] = { radius1, -radius2,  radius3 };
			vertices[6] = { radius1, -radius2, -radius3 };
			vertices[7] = { radius1,  radius2, -radius3 };
		}
		/*8~11　背面*/
		{
			vertices[8] = {  -radius1,  radius2, radius3 };
			vertices[9] = {   radius1,  radius2, radius3 };
			vertices[10] = {  radius1, -radius2, radius3 };
			vertices[11] = { -radius1, -radius2, radius3 };
		}
		/*12~15　左側面*/
		{
			vertices[12] = { -radius1,  radius2,  radius3 };
			vertices[13] = { -radius1, -radius2,  radius3 };
			vertices[14] = { -radius1, -radius2, -radius3 };
			vertices[15] = { -radius1,  radius2, -radius3 };
		}
		/*16~19　底面*/
		{
			vertices[16] = { -radius1, -radius2,  radius3 };
			vertices[17] = {  radius1, -radius2,  radius3 };
			vertices[18] = {  radius1, -radius2, -radius3 };
			vertices[19] = { -radius1, -radius2, -radius3 };
		}
		/*20~23　前面*/
		{
			vertices[20] = { -radius1,  radius2, -radius3 };
			vertices[21] = {  radius1,  radius2, -radius3 };
			vertices[22] = {  radius1, -radius2, -radius3 };
			vertices[23] = { -radius1, -radius2, -radius3 };
		}
	}
	// index
	cuboid_index_count = 36;
	std::unique_ptr<uint32_t[]> indices = std::make_unique<uint32_t[]>(cuboid_index_count);
	{
		/*上面*/
		{
			/*0~2*/
			indices[0] = 0;
			indices[1] = 1;
			indices[2] = 3;
			/*3~5*/
			indices[3] = 1;
			indices[4] = 2;
			indices[5] = 3;
		}
		/*右側面*/
		{
			/*6~8*/
			indices[6] = 4;
			indices[7] = 6;
			indices[8] = 7;
			/*9~11*/
			indices[9] = 4;
			indices[10] = 5;
			indices[11] = 6;
		}
		/*背面*/
		{
			/*12~14*/
			indices[12] = 11;
			indices[13] = 9;
			indices[14] = 8;
			/*15~17*/
			indices[15] = 11;
			indices[16] = 10;
			indices[17] = 9;
		}
		/*左側面*/
		{
			/*18~20*/
			indices[18] = 14;
			indices[19] = 12;
			indices[20] = 15;
			/*21~23*/
			indices[21] = 14;
			indices[22] = 13;
			indices[23] = 12;
		}
		/*底面*/
		{
			/*24~26*/
			indices[24] = 19;
			indices[25] = 17;
			indices[26] = 16;
			/*27~29*/
			indices[27] = 19;
			indices[28] = 18;
			indices[29] = 17;
		}
		/*前面*/
		{
			/*30~32*/
			indices[30] = 20;
			indices[31] = 21;
			indices[32] = 23;
			/*33~35*/
			indices[33] = 21;
			indices[34] = 22;
			indices[35] = 23;
		}
	}
	//----bufferの作成----//
	D3D11_BUFFER_DESC desc = {};
	D3D11_SUBRESOURCE_DATA subresourceData = {};
	// 頂点バッファ
	{

		desc.ByteWidth = static_cast<UINT>(sizeof(DirectX::XMFLOAT3) * cuboid_vertex_count);
		desc.Usage = D3D11_USAGE_IMMUTABLE;	// D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		subresourceData.pSysMem = vertices.get();
		subresourceData.SysMemPitch = 0;
		subresourceData.SysMemSlicePitch = 0;

		HRESULT hr = device->CreateBuffer(&desc, &subresourceData, cuboid_vertex_buffer.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
	// インデックスバッファ
	{
		desc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * cuboid_index_count);
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		subresourceData.pSysMem = indices.get();
		HRESULT hr = device->CreateBuffer(&desc, &subresourceData, cuboid_index_buffer.ReleaseAndGetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
}

void DebugFigures::create_capsule_mesh(ID3D11Device* device, float radius1, float radius2,
	const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, int slices)
{
	capsule_vertex_count = 2 * slices * 2 + 2 * slices;
	std::unique_ptr<DirectX::XMFLOAT3[]> vertices = std::make_unique<DirectX::XMFLOAT3[]>(capsule_vertex_count);

	DirectX::XMFLOAT3* p = vertices.get();

	float height = end.y - start.y;
	float radiusStep = radius2 - radius1;

	// vertices of ring
	float dTheta = DirectX::XM_2PI / slices;

	for (int i = 0; i < slices; ++i)
	{
		int n = (i + 1) % slices;

		float c1 = cosf(i * dTheta);
		float s1 = sinf(i * dTheta);

		float c2 = cosf(n * dTheta);
		float s2 = sinf(n * dTheta);

		for (int j = 0; j <= 1; ++j)
		{
			float y = start.y + j * height;
			float r = radius1 + j * radiusStep;

			float add_x = end.x * j;
			float add_z = end.z * j;

			p->x = add_x + r * c1;
			p->y = y;
			p->z = add_z + r * s1;
			p++;

			p->x = add_x + r * c2;
			p->y = y;
			p->z = add_z + r * s2;
			p++;
		}

		p->x = start.x + radius1 * c1;
		p->y = start.y;
		p->z = start.z + radius1 * s1;
		p++;

		p->x = end.x + radius2 * c1;
		p->y = end.y;
		p->z = end.z + radius2 * s1;
		p++;
	}

	// 頂点バッファ
	{
		D3D11_BUFFER_DESC desc = {};
		D3D11_SUBRESOURCE_DATA subresourceData = {};

		desc.ByteWidth = static_cast<UINT>(sizeof(DirectX::XMFLOAT3) * capsule_vertex_count);
		desc.Usage = D3D11_USAGE_IMMUTABLE;	// D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		subresourceData.pSysMem = vertices.get();
		subresourceData.SysMemPitch = 0;
		subresourceData.SysMemSlicePitch = 0;

		HRESULT hr = device->CreateBuffer(&desc, &subresourceData, capsule_vertex_buffer.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
}
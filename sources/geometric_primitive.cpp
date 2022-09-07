#include "geometric_primitive.h"
#include "shader.h"
#include "misc.h"

GeometricPrimitive::GeometricPrimitive(ID3D11Device* device)
{
    vertex vertices[24]{};
    // �T�C�Y��1.0�̐������̃f�[�^���쐬����i�d�S�����_�ɂ���j�B�������̂̃R���g���[���|�C���g����8�A
    // 1�̃R���g���[���|�C���g�̈ʒu�ɂ͖@���̌������Ⴄ���_��3����̂Œ��_���̑�����8x3=24�A
    // ���_���z��ivertices�j�ɂ��ׂĒ��_�̈ʒu�E�@�������i�[����B
    /*0~3�@���*/
    {
        DirectX::XMFLOAT3 normal{ 0, 1, 0 };
        vertices[0].position = { -0.5f, 0.5f, 0.5f };
        vertices[0].normal   = normal;
        vertices[1].position = { 0.5f, 0.5f, 0.5f };
        vertices[1].normal   = normal;
        vertices[2].position = { 0.5f, 0.5f, -0.5f };
        vertices[2].normal   = normal;
        vertices[3].position = { -0.5f, 0.5f, -0.5f };
        vertices[3].normal   = normal;
    }
    /*4~7�@�E����*/
    {
        DirectX::XMFLOAT3 normal{ 1, 0, 0 };
        vertices[4].position = { 0.5f, 0.5f, 0.5f };
        vertices[4].normal   = normal;
        vertices[5].position = { 0.5f, -0.5f, 0.5f };
        vertices[5].normal   = normal;
        vertices[6].position = { 0.5f, -0.5f, -0.5f };
        vertices[6].normal   = normal;
        vertices[7].position = { 0.5f, 0.5f, -0.5f };
        vertices[7].normal   = normal;
    }
    /*8~11�@�w��*/
    {
        DirectX::XMFLOAT3 normal{ 0, 0, 1 };
        vertices[8].position  = { -0.5f, 0.5f, 0.5f };
        vertices[8].normal    = normal;
        vertices[9].position  = { 0.5f, 0.5f, 0.5f };
        vertices[9].normal    = normal;
        vertices[10].position = { 0.5f, -0.5f, 0.5f };
        vertices[10].normal   = normal;
        vertices[11].position = { -0.5f, -0.5f, 0.5f };
        vertices[11].normal   = normal;
    }
    /*12~15�@������*/
    {
        DirectX::XMFLOAT3 normal{ -1, 0, 0 };
        vertices[12].position  = { -0.5f, 0.5f, 0.5f };
        vertices[12].normal    = normal;
        vertices[13].position  = { -0.5f, -0.5f, 0.5f };
        vertices[13].normal    = normal;
        vertices[14].position = { -0.5f, -0.5f, -0.5f };
        vertices[14].normal   = normal;
        vertices[15].position = { -0.5f, 0.5f, -0.5f };
        vertices[15].normal   = normal;
    }
    /*16~19�@���*/
    {
        DirectX::XMFLOAT3 normal{ 0, -1, 0 };
        vertices[16].position  = { -0.5f, -0.5f, 0.5f };
        vertices[16].normal    = normal;
        vertices[17].position  = { 0.5f, -0.5f, 0.5f };
        vertices[17].normal    = normal;
        vertices[18].position = { 0.5f, -0.5f, -0.5f };
        vertices[18].normal   = normal;
        vertices[19].position = { -0.5f, -0.5f, -0.5f };
        vertices[19].normal   = normal;
    }
    /*20~23�@�O��*/
    {
        DirectX::XMFLOAT3 normal{ 0, 0, -1 };
        vertices[20].position  = { -0.5f, 0.5f, -0.5f };
        vertices[20].normal    = normal;
        vertices[21].position  = { 0.5f, 0.5f, -0.5f };
        vertices[21].normal    = normal;
        vertices[22].position = { 0.5f, -0.5f, -0.5f };
        vertices[22].normal   = normal;
        vertices[23].position = { -0.5f, -0.5f, -0.5f };
        vertices[23].normal   = normal;
    }

    uint32_t indices[36]{};
    // �������̂�6�ʎ����A1�̖ʂ�2��3�p�`�|���S���ō\�������̂�3�p�`�|���S���̑�����6x2=12�A
    // �������̂�`�悷�邽�߂�12���3�p�`�|���S���`�悪�K�v�A����ĎQ�Ƃ���钸�_����12x3=36��A
    // 3�p�`�|���S�����Q�Ƃ��钸�_���̃C���f�b�N�X�i���_�ԍ��j��`�揇�ɔz��iindices�j�Ɋi�[����B
    // ���v��肪�\�ʂɂȂ�悤�Ɋi�[���邱�ƁB
    /*���*/
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
    /*�E����*/
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
    /*�w��*/
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
    /*������*/
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
    /*���*/
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
    /*�O��*/
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

    create_com_buffers(device, vertices, 24, indices, 36);

    HRESULT hr{ S_OK };

    D3D11_INPUT_ELEMENT_DESC input_element_desc[]
    {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
            D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
            D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    create_vs_from_cso(device, "shaders/geometric_primitive_vs.cso", vertex_shader.GetAddressOf(),
        input_layout.GetAddressOf(), input_element_desc, ARRAYSIZE(input_element_desc));
    create_ps_from_cso(device, "shaders/geometric_primitive_ps.cso", pixel_shader.GetAddressOf());

    D3D11_BUFFER_DESC buffer_desc{};
    buffer_desc.ByteWidth = sizeof(constants);
    buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    hr = device->CreateBuffer(&buffer_desc, nullptr, constant_buffer.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

GeometricPrimitive::GeometricPrimitive(ID3D11Device* device,
    const DirectX::XMFLOAT3& min_position, const DirectX::XMFLOAT3& max_position)
{
    vertex vertices[24]{};
    // �T�C�Y��1.0�̐������̃f�[�^���쐬����i�d�S�����_�ɂ���j�B�������̂̃R���g���[���|�C���g����8�A
    // 1�̃R���g���[���|�C���g�̈ʒu�ɂ͖@���̌������Ⴄ���_��3����̂Œ��_���̑�����8x3=24�A
    // ���_���z��ivertices�j�ɂ��ׂĒ��_�̈ʒu�E�@�������i�[����B
    /*0~3�@���*/
    {
        DirectX::XMFLOAT3 normal{ 0, 1, 0 };
        vertices[0].position = { min_position.x, max_position.y, max_position.z };
        vertices[0].normal = normal;
        vertices[1].position = { max_position };
        vertices[1].normal = normal;
        vertices[2].position = { max_position.x, max_position.y, min_position.z };
        vertices[2].normal = normal;
        vertices[3].position = { min_position.x, max_position.y, min_position.z };
        vertices[3].normal = normal;
    }
    /*4~7�@�E����*/
    {
        DirectX::XMFLOAT3 normal{ 1, 0, 0 };
        vertices[4].position = { max_position };
        vertices[4].normal = normal;
        vertices[5].position = { max_position.x, min_position.y, max_position.z };
        vertices[5].normal = normal;
        vertices[6].position = { max_position.x, min_position.y, min_position.z };
        vertices[6].normal = normal;
        vertices[7].position = { max_position.x, max_position.y, min_position.z };
        vertices[7].normal = normal;
    }
    /*8~11�@�w��*/
    {
        DirectX::XMFLOAT3 normal{ 0, 0, 1 };
        vertices[8].position = { min_position.x, max_position.y, max_position.z };
        vertices[8].normal = normal;
        vertices[9].position = { max_position };
        vertices[9].normal = normal;
        vertices[10].position = { max_position.x, min_position.y, max_position.z };
        vertices[10].normal = normal;
        vertices[11].position = { min_position.x, min_position.y, max_position.z };
        vertices[11].normal = normal;
    }
    /*12~15�@������*/
    {
        DirectX::XMFLOAT3 normal{ -1, 0, 0 };
        vertices[12].position = { min_position.x, max_position.y, max_position.z };
        vertices[12].normal = normal;
        vertices[13].position = { min_position.x, min_position.y, max_position.z };
        vertices[13].normal = normal;
        vertices[14].position = { min_position };
        vertices[14].normal = normal;
        vertices[15].position = { min_position.x, max_position.y, min_position.z };
        vertices[15].normal = normal;
    }
    /*16~19�@���*/
    {
        DirectX::XMFLOAT3 normal{ 0, -1, 0 };
        vertices[16].position = { min_position.x, min_position.y, max_position.z };
        vertices[16].normal = normal;
        vertices[17].position = { max_position.x, min_position.y, max_position.z };
        vertices[17].normal = normal;
        vertices[18].position = { max_position.x, min_position.y, min_position.z };
        vertices[18].normal = normal;
        vertices[19].position = { min_position };
        vertices[19].normal = normal;
    }
    /*20~23�@�O��*/
    {
        DirectX::XMFLOAT3 normal{ 0, 0, -1 };
        vertices[20].position = { min_position.x, max_position.y, min_position.z };
        vertices[20].normal = normal;
        vertices[21].position = { max_position.x, max_position.y, min_position.z };
        vertices[21].normal = normal;
        vertices[22].position = { max_position.x, min_position.y, min_position.z };
        vertices[22].normal = normal;
        vertices[23].position = { min_position };
        vertices[23].normal = normal;
    }

    uint32_t indices[36]{};
    // �������̂�6�ʎ����A1�̖ʂ�2��3�p�`�|���S���ō\�������̂�3�p�`�|���S���̑�����6x2=12�A
    // �������̂�`�悷�邽�߂�12���3�p�`�|���S���`�悪�K�v�A����ĎQ�Ƃ���钸�_����12x3=36��A
    // 3�p�`�|���S�����Q�Ƃ��钸�_���̃C���f�b�N�X�i���_�ԍ��j��`�揇�ɔz��iindices�j�Ɋi�[����B
    // ���v��肪�\�ʂɂȂ�悤�Ɋi�[���邱�ƁB
    /*���*/
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
    /*�E����*/
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
    /*�w��*/
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
    /*������*/
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
    /*���*/
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
    /*�O��*/
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

    create_com_buffers(device, vertices, 24, indices, 36);

    HRESULT hr{ S_OK };

    D3D11_INPUT_ELEMENT_DESC input_element_desc[]
    {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
            D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
            D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    create_vs_from_cso(device, "shaders/geometric_primitive_vs.cso", vertex_shader.GetAddressOf(),
        input_layout.GetAddressOf(), input_element_desc, ARRAYSIZE(input_element_desc));
    create_ps_from_cso(device, "shaders/geometric_primitive_ps.cso", pixel_shader.GetAddressOf());

    D3D11_BUFFER_DESC buffer_desc{};
    buffer_desc.ByteWidth = sizeof(constants);
    buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    hr = device->CreateBuffer(&buffer_desc, nullptr, constant_buffer.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

void GeometricPrimitive::render(ID3D11DeviceContext* dc,
    const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4& material_color)
{
    uint32_t stride{ sizeof(vertex) };
    uint32_t offset{ 0 };
    dc->IASetVertexBuffers(0, 1, vertex_buffer.GetAddressOf(), &stride, &offset);
    dc->IASetIndexBuffer(index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    dc->IASetInputLayout(input_layout.Get());

    dc->VSSetShader(vertex_shader.Get(), nullptr, 0);
    dc->PSSetShader(pixel_shader.Get(), nullptr, 0);

    constants data{ world, material_color };
    dc->UpdateSubresource(constant_buffer.Get(), 0, 0, &data, 0, 0);
    dc->VSSetConstantBuffers(0, 1, constant_buffer.GetAddressOf());

    D3D11_BUFFER_DESC buffer_desc{};
    index_buffer->GetDesc(&buffer_desc);
    dc->DrawIndexed(buffer_desc.ByteWidth / sizeof(uint32_t), 0, 0);
}

void GeometricPrimitive::create_com_buffers(ID3D11Device* device, vertex* vertices,
    size_t vertex_count, uint32_t* indices, size_t index_count)
{
    HRESULT hr{ S_OK };

    D3D11_BUFFER_DESC buffer_desc{};
    D3D11_SUBRESOURCE_DATA subresource_data{};
    buffer_desc.ByteWidth = static_cast<UINT>(sizeof(vertex) * vertex_count);
    buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    buffer_desc.CPUAccessFlags = 0;
    buffer_desc.MiscFlags = 0;
    buffer_desc.StructureByteStride = 0;
    subresource_data.pSysMem = vertices;
    subresource_data.SysMemPitch = 0;
    subresource_data.SysMemSlicePitch = 0;
    hr = device->CreateBuffer(&buffer_desc, &subresource_data, vertex_buffer.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    buffer_desc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * index_count);
    buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    subresource_data.pSysMem = indices;
    hr = device->CreateBuffer(&buffer_desc, &subresource_data, index_buffer.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}
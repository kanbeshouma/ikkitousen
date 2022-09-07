#include <WICTextureLoader.h>
using namespace DirectX;

#include <memory>
using namespace std;

#include "misc.h"
#include "shader.h"

HRESULT create_vs_from_cso(ID3D11Device* device, const char* cso_name, ID3D11VertexShader** vertex_shader,
    ID3D11InputLayout** input_layout, D3D11_INPUT_ELEMENT_DESC* input_element_desc, UINT num_elements)
{
    FILE* fp{ nullptr };
    fopen_s(&fp, cso_name, "rb");
    _ASSERT_EXPR_A(fp, "CSO File not found");

    fseek(fp, 0, SEEK_END);
    long cso_sz{ ftell(fp) };
    fseek(fp, 0, SEEK_SET);

    unique_ptr<unsigned char[]> cso_data{ make_unique<unsigned char[]>(cso_sz) };
    fread(cso_data.get(), cso_sz, 1, fp);
    fclose(fp);

    HRESULT hr{ S_OK };
    hr = device->CreateVertexShader(cso_data.get(), cso_sz, nullptr, vertex_shader);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    if (input_layout)
    {
        hr = device->CreateInputLayout(input_element_desc, num_elements,
            cso_data.get(), cso_sz, input_layout);
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    }

    return hr;
}

HRESULT create_ps_from_cso(ID3D11Device* device, const char* cso_name, ID3D11PixelShader** pixel_shader)
{
    FILE* fp{ nullptr };
    fopen_s(&fp, cso_name, "rb");
    _ASSERT_EXPR_A(fp, "CSO File not found");

    fseek(fp, 0, SEEK_END);
    long cso_sz{ ftell(fp) };
    fseek(fp, 0, SEEK_SET);

    unique_ptr<unsigned char[]> cso_data{ make_unique<unsigned char[]>(cso_sz) };
    fread(cso_data.get(), cso_sz, 1, fp);
    fclose(fp);

    HRESULT hr{ S_OK };
    hr = device->CreatePixelShader(cso_data.get(), cso_sz, nullptr, pixel_shader);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    return hr;
}

HRESULT create_gs_from_cso(ID3D11Device* device, const char* cso_name, ID3D11GeometryShader** geometry_shader)
{
    FILE* fp = nullptr;
    fopen_s(&fp, cso_name, "rb");
    _ASSERT_EXPR_A(fp, "CSO File not found");

    fseek(fp, 0, SEEK_END);
    long cso_sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    std::unique_ptr<unsigned char[]> cso_data = std::make_unique<unsigned char[]>(cso_sz);
    fread(cso_data.get(), cso_sz, 1, fp);
    fclose(fp);

    HRESULT hr = device->CreateGeometryShader(cso_data.get(), cso_sz, nullptr, geometry_shader);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    return hr;
}

HRESULT create_cs_from_cso(ID3D11Device* device, const char* cso_name, ID3D11ComputeShader** compute_shader)
{
    FILE* fp = nullptr;
    fopen_s(&fp, cso_name, "rb");
    _ASSERT_EXPR_A(fp, "CSO File not found");

    fseek(fp, 0, SEEK_END);
    long cso_sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    std::unique_ptr<unsigned char[]> cso_data = std::make_unique<unsigned char[]>(cso_sz);
    fread(cso_data.get(), cso_sz, 1, fp);
    fclose(fp);

    HRESULT hr = device->CreateComputeShader(cso_data.get(), cso_sz, nullptr, compute_shader);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    return hr;
}
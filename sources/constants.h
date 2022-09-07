#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <cstdint>
#include <DirectXMath.h>

#include "misc.h"

enum class CB_FLAG : uint8_t
{
    PS = 0x01,
    VS = 0x02,
    CS = 0x03,
    GS = 0x04,
    PS_VS = PS | VS,
    PS_CS = PS | CS,
    PS_GS = PS | GS,
    VS_CS = VS | CS,
    VS_GS = VS | GS,
    CS_GS = CS | GS,
    PS_VS_CS = PS | VS | CS,
    PS_VS_GS = PS | VS | GS,
    PS_CS_GS = PS | CS | GS,
    VS_CS_GS = VS | CS | GS,
    PS_VS_CS_GS = PS | VS | CS | GS,
};

inline bool operator&(CB_FLAG lhs, CB_FLAG rhs)
{
    return static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs);
}

template <class T>
class Constants
{
public:
    //--------<constructor/destructor>--------//
    Constants(ID3D11Device* device)
    {
        _ASSERT_EXPR(sizeof(T) % 16 == 0, L"constant buffer's need to be 16 byte aligned");
        HRESULT hr{ S_OK };
        D3D11_BUFFER_DESC buffer_desc{};
        buffer_desc.ByteWidth = sizeof(T);
        buffer_desc.Usage = D3D11_USAGE_DEFAULT;
        buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        buffer_desc.CPUAccessFlags = 0;
        buffer_desc.MiscFlags = 0;
        buffer_desc.StructureByteStride = 0;
        hr = device->CreateBuffer(&buffer_desc, nullptr, constant_buffer.ReleaseAndGetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
    }
    ~Constants() {}
    //--------< ŠÖ” >--------//
    void bind(ID3D11DeviceContext* dc, UINT slot, CB_FLAG flags = CB_FLAG::PS)
    {
        dc->UpdateSubresource(constant_buffer.Get(), 0, 0, &data, 0, 0);
        if (flags & CB_FLAG::PS) dc->PSSetConstantBuffers(slot, 1, constant_buffer.GetAddressOf());
        if (flags & CB_FLAG::VS) dc->VSSetConstantBuffers(slot, 1, constant_buffer.GetAddressOf());
        if (flags & CB_FLAG::CS) dc->CSSetConstantBuffers(slot, 1, constant_buffer.GetAddressOf());
        if (flags & CB_FLAG::GS) dc->GSSetConstantBuffers(slot, 1, constant_buffer.GetAddressOf());
    }
    //--------< •Ï” >--------//
    T data;
private:
    Microsoft::WRL::ComPtr<ID3D11Buffer> constant_buffer;
};
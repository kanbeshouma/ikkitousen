#include <WICTextureLoader.h>
using namespace DirectX;

 #include <wrl.h>
using namespace Microsoft::WRL;

 #include <string>
 #include <map>
using namespace std;

#include <memory>

#include <filesystem>
#include <DDSTextureLoader.h>

#include "misc.h"
#include "texture.h"

 static map<wstring, ComPtr<ID3D11ShaderResourceView>> resources;

 HRESULT load_texture_from_file(ID3D11Device* device, const wchar_t* filename,
     ID3D11ShaderResourceView** shader_resource_view, D3D11_TEXTURE2D_DESC* texture2d_desc)
 {
     HRESULT hr{ S_OK };
     ComPtr<ID3D11Resource> resource;

     std::filesystem::path dds_filename(filename);
     dds_filename.replace_extension("dds");
     if (std::filesystem::exists(dds_filename.c_str()))
     {
         Microsoft::WRL::ComPtr<ID3D11DeviceContext> dc;
         device->GetImmediateContext(dc.GetAddressOf());
         hr = DirectX::CreateDDSTextureFromFile(device, dc.Get(), dds_filename.c_str(),
             resource.GetAddressOf(), shader_resource_view);
         _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
     }
     else
     {
         hr = CreateWICTextureFromFile(device, filename, resource.GetAddressOf(), shader_resource_view);
         _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
         resources.insert(make_pair(filename, *shader_resource_view));
     }

     ComPtr<ID3D11Texture2D> texture2d;
     hr = resource.Get()->QueryInterface<ID3D11Texture2D>(texture2d.GetAddressOf());
     _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
     texture2d->GetDesc(texture2d_desc);

     return hr;
 }

 HRESULT make_dummy_texture(ID3D11Device* device,
     ID3D11ShaderResourceView** shader_resource_view, DWORD value/*0xAABBGGRR*/, UINT dimension)
 {
     HRESULT hr{ S_OK };

     D3D11_TEXTURE2D_DESC texture2d_desc{};
     texture2d_desc.Width = dimension;
     texture2d_desc.Height = dimension;
     texture2d_desc.MipLevels = 1;
     texture2d_desc.ArraySize = 1;
     texture2d_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
     texture2d_desc.SampleDesc.Count = 1;
     texture2d_desc.SampleDesc.Quality = 0;
     texture2d_desc.Usage = D3D11_USAGE_DEFAULT;
     texture2d_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

     size_t texels = dimension * dimension;
     std::unique_ptr<DWORD[]> sysmem{ std::make_unique< DWORD[]>(texels) };
     for (size_t i = 0; i < texels; ++i) sysmem[i] = value;

     D3D11_SUBRESOURCE_DATA subresource_data{};
     subresource_data.pSysMem = sysmem.get();
     subresource_data.SysMemPitch = sizeof(DWORD) * dimension;

     Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d;
     hr = device->CreateTexture2D(&texture2d_desc, &subresource_data, &texture2d);
     _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

     D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc{};
     shader_resource_view_desc.Format = texture2d_desc.Format;
     shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
     shader_resource_view_desc.Texture2D.MipLevels = 1;
     hr = device->CreateShaderResourceView(texture2d.Get(), &shader_resource_view_desc,
         shader_resource_view);
     _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

     return hr;
 }


 void release_all_textures()
 {
     resources.clear();
 }
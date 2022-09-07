#pragma once

//--プロトタイプ宣言--//
HRESULT load_texture_from_file(ID3D11Device* device, const wchar_t* filename,
    ID3D11ShaderResourceView** shader_resource_view, D3D11_TEXTURE2D_DESC* texture2d_desc);

// ダミーテクスチャの作成
HRESULT make_dummy_texture(ID3D11Device* device, ID3D11ShaderResourceView** shader_resource_view,
    DWORD value/*0xAABBGGRR*/, UINT dimension);

void release_all_textures();
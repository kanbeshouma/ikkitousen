#include <WICTextureLoader.h>
#include "misc.h"
#include "Font.h"
#include "user.h"


Font::Font(ID3D11Device* device, const char* filename, int maxSpriteCount)
{
	// 頂点シェーダー
	{
		// ファイルを開く
		FILE* fp = nullptr;
		fopen_s(&fp, "shaders\\font_vs.cso", "rb");
		_ASSERT_EXPR_A(fp, "CSO File not found");

		// ファイルのサイズを求める
		fseek(fp, 0, SEEK_END);
		long csoSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		// メモリ上に頂点シェーダーデータを格納する領域を用意する
		std::unique_ptr<u_char[]> csoData = std::make_unique<u_char[]>(csoSize);
		fread(csoData.get(), csoSize, 1, fp);
		fclose(fp);

		// 頂点シェーダー生成
		HRESULT hr = device->CreateVertexShader(csoData.get(), csoSize, nullptr, vertexShader.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		// 入力レイアウト
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		hr = device->CreateInputLayout(inputElementDesc, ARRAYSIZE(inputElementDesc), csoData.get(), csoSize, inputLayout.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
	// ピクセルシェーダー
	{
		// ファイルを開く
		FILE* fp = nullptr;
		fopen_s(&fp, "shaders\\font_ps.cso", "rb");
		_ASSERT_EXPR_A(fp, "CSO File not found");

		// ファイルのサイズを求める
		fseek(fp, 0, SEEK_END);
		long csoSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		// メモリ上に頂点シェーダーデータを格納する領域を用意する
		std::unique_ptr<u_char[]> csoData = std::make_unique<u_char[]>(csoSize);
		fread(csoData.get(), csoSize, 1, fp);
		fclose(fp);

		// ピクセルシェーダー生成
		HRESULT hr = device->CreatePixelShader(csoData.get(), csoSize, nullptr, pixelShader.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
	// サンプラステート
	{
		D3D11_SAMPLER_DESC desc;
		::memset(&desc, 0, sizeof(desc));
		desc.MipLODBias = 0.0f;
		desc.MaxAnisotropy = 1;
		desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		desc.MinLOD = -FLT_MAX;
		desc.MaxLOD = FLT_MAX;
		desc.BorderColor[0] = 1.0f;
		desc.BorderColor[1] = 1.0f;
		desc.BorderColor[2] = 1.0f;
		desc.BorderColor[3] = 1.0f;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

		HRESULT hr = device->CreateSamplerState(&desc, samplerState.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
	// 頂点バッファ
	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * maxSpriteCount * 4);
		//bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		HRESULT hr = device->CreateBuffer(&bufferDesc, nullptr, vertexBuffer.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
	// インデックスバッファ
	{
		D3D11_BUFFER_DESC bufferDesc = {};
		D3D11_SUBRESOURCE_DATA subresourceData = {};

		std::unique_ptr<UINT[]> indices = std::make_unique<UINT[]>(maxSpriteCount * 6);
		// 四角形 を 三角形 ２つに展開
		// 0---1      0---1  4
		// |   |  →  |／  ／|
		// 2---3      2  3---5
		{
			UINT* p = indices.get();
			for (int i = 0; i < maxSpriteCount * 4; i += 4)
			{
				p[0] = i + 0;
				p[1] = i + 1;
				p[2] = i + 2;
				p[3] = i + 2;
				p[4] = i + 1;
				p[5] = i + 3;
				p += 6;
			}
		}

		bufferDesc.ByteWidth = static_cast<UINT>(sizeof(UINT) * maxSpriteCount * 6);
		//bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;
		subresourceData.pSysMem = indices.get();
		subresourceData.SysMemPitch = 0; //Not use for index buffers.
		subresourceData.SysMemSlicePitch = 0; //Not use for index buffers.
		HRESULT hr = device->CreateBuffer(&bufferDesc, &subresourceData, indexBuffer.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
	// .fnt 読み込み
	{
		FILE* fp = nullptr;
		fopen_s(&fp, filename, "rb");
		_ASSERT_EXPR_A(fp, "FNT File not found");

		fseek(fp, 0, SEEK_END);
		long fntSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		std::unique_ptr<char[]> fntData = std::make_unique<char[]>(fntSize);
		fread(fntData.get(), fntSize, 1, fp);
		fclose(fp);

		// (***.fnt)ヘッダ解析
		char* context = nullptr;	// 内部的に使用するので深く考えない
		const char* delimiter = " ,=\"\r\n";
		const char* delimiter2 = ",=\"\r\n";
		char* pToken = ::strtok_s(fntData.get(), delimiter, &context);
		_ASSERT_EXPR_A(::strcmp(pToken, "info") == 0, "FNT Format");

		// face
		::strtok_s(nullptr, delimiter, &context);
		const char* face = ::strtok_s(nullptr, delimiter2, &context);
		// size
		::strtok_s(nullptr, delimiter, &context);
		int size = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// bold
		::strtok_s(nullptr, delimiter, &context);
		int bold = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// italic
		::strtok_s(nullptr, delimiter, &context);
		int italic = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// charset
		::strtok_s(nullptr, delimiter, &context);
		const char* charset = ::strtok_s(nullptr, delimiter, &context);
		// unicode
		if (::strcmp(charset, "unicode") == 0)
			charset = "";
		else
			::strtok_s(nullptr, delimiter, &context);
		int unicode = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// stretchH
		::strtok_s(nullptr, delimiter, &context);
		int stretchH = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// smooth
		::strtok_s(nullptr, delimiter, &context);
		int smooth = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// aa
		::strtok_s(nullptr, delimiter, &context);
		int aa = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// padding
		::strtok_s(nullptr, delimiter, &context);
		int padding1 = ::atoi(::strtok_s(nullptr, delimiter, &context));
		int padding2 = ::atoi(::strtok_s(nullptr, delimiter, &context));
		int padding3 = ::atoi(::strtok_s(nullptr, delimiter, &context));
		int padding4 = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// spacing
		::strtok_s(nullptr, delimiter, &context);
		int spacing1 = ::atoi(::strtok_s(nullptr, delimiter, &context));
		int spacing2 = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// outline
		::strtok_s(nullptr, delimiter, &context);
		int outline = ::atoi(::strtok_s(nullptr, delimiter, &context));

		// common
		::strtok_s(nullptr, delimiter, &context);
		// lineHeight
		::strtok_s(nullptr, delimiter, &context);
		int lineHeight = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// base
		::strtok_s(nullptr, delimiter, &context);
		int base = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// scaleW
		::strtok_s(nullptr, delimiter, &context);
		int scaleW = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// scaleH
		::strtok_s(nullptr, delimiter, &context);
		int scaleH = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// pages
		::strtok_s(nullptr, delimiter, &context);
		int pages = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// packed
		::strtok_s(nullptr, delimiter, &context);
		int packed = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// alphaChnl
		::strtok_s(nullptr, delimiter, &context);
		int alphaChnl = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// redChnl
		::strtok_s(nullptr, delimiter, &context);
		int redChnl = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// greenChnl
		::strtok_s(nullptr, delimiter, &context);
		int greenChnl = ::atoi(::strtok_s(nullptr, delimiter, &context));
		// blueChnl
		::strtok_s(nullptr, delimiter, &context);
		int blueChnl = ::atoi(::strtok_s(nullptr, delimiter, &context));

		// ディレクトリパス取得
		char dirname[256];
		::_splitpath_s(filename, nullptr, 0, dirname, 256, nullptr, 0, nullptr, 0);
		shaderResourceViews.resize(pages);
		for (int i = 0; i < pages; i++)
		{
			// page
			::strtok_s(nullptr, delimiter, &context);
			// id
			::strtok_s(nullptr, delimiter, &context);
			int id = ::atoi(::strtok_s(nullptr, delimiter, &context));
			// file
			::strtok_s(nullptr, delimiter, &context);
			const char* file = ::strtok_s(nullptr, delimiter2, &context);

			// 相対パスの解決
			char fname[256];
			::_makepath_s(fname, 256, nullptr, dirname, file, nullptr);

			// マルチバイト文字からワイド文字へ変換
			size_t length;
			wchar_t wfname[256];
			::mbstowcs_s(&length, wfname, 256, fname, _TRUNCATE);

			// テクスチャ読み込み
			Microsoft::WRL::ComPtr<ID3D11Resource> resource;
			HRESULT hr = DirectX::CreateWICTextureFromFile(device, wfname, resource.GetAddressOf(), shaderResourceViews.at(i).GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}

		// chars
		::strtok_s(nullptr, delimiter, &context);
		// count
		::strtok_s(nullptr, delimiter, &context);
		int count = ::atoi(::strtok_s(nullptr, delimiter, &context));

		// データ格納
		fontWidth = static_cast<float>(size);
		fontHeight = static_cast<float>(lineHeight);
		textureCount = pages;
		characterCount = count + 1;
		characterInfos.resize(characterCount);
		characterIndices.resize(0xFFFF);
		::memset(characterIndices.data(), 0, sizeof(WORD) * characterIndices.size());

		characterIndices.at(0x00) = CharacterInfo::EndCode;
		characterIndices.at(0x0a) = CharacterInfo::ReturnCode;
		characterIndices.at(0x09) = CharacterInfo::TabCode;
		characterIndices.at(0x20) = CharacterInfo::SpaceCode;

		// 文字情報解析
		int n = 1;
		for (int i = 0; i < count; i++)
		{
			// char
			::strtok_s(nullptr, delimiter, &context);
			// id
			::strtok_s(nullptr, delimiter, &context);
			int id = ::atoi(::strtok_s(nullptr, delimiter, &context));
			// x
			::strtok_s(nullptr, delimiter, &context);
			int x = ::atoi(::strtok_s(nullptr, delimiter, &context));
			// y
			::strtok_s(nullptr, delimiter, &context);
			int y = ::atoi(::strtok_s(nullptr, delimiter, &context));
			// width
			::strtok_s(nullptr, delimiter, &context);
			int width = ::atoi(::strtok_s(nullptr, delimiter, &context));
			// height
			::strtok_s(nullptr, delimiter, &context);
			int height = ::atoi(::strtok_s(nullptr, delimiter, &context));
			// xoffset
			::strtok_s(nullptr, delimiter, &context);
			int xoffset = ::atoi(::strtok_s(nullptr, delimiter, &context));
			// yoffset
			::strtok_s(nullptr, delimiter, &context);
			int yoffset = ::atoi(::strtok_s(nullptr, delimiter, &context));
			// xadvance
			::strtok_s(nullptr, delimiter, &context);
			int xadvance = ::atoi(::strtok_s(nullptr, delimiter, &context));
			// page
			::strtok_s(nullptr, delimiter, &context);
			int page = ::atoi(::strtok_s(nullptr, delimiter, &context));
			// chnl
			::strtok_s(nullptr, delimiter, &context);
			int chnl = ::atoi(::strtok_s(nullptr, delimiter, &context));

			// 文字コード格納
			if (id >= 0x10000) continue;

			CharacterInfo& info = characterInfos.at(n);

			characterIndices.at(id) = static_cast<WORD>(i + 1);

			// 文字情報格納
			info.left = static_cast<float>(x) / static_cast<float>(scaleW);
			info.top = static_cast<float>(y) / static_cast<float>(scaleH);
			info.right = static_cast<float>(x + width) / static_cast<float>(scaleW);
			info.bottom = static_cast<float>(y + height) / static_cast<float>(scaleH);
			info.xoffset = static_cast<float>(xoffset);
			info.yoffset = static_cast<float>(yoffset);
			info.xadvance = static_cast<float>(xadvance);
			info.width = static_cast<float>(width);
			info.height = static_cast<float>(height);
			info.page = page;
			info.ascii = (id < 0x100);

			n++;
		}
	}
}

void Font::Begin(ID3D11DeviceContext* context)
{
	// スクリーンサイズ取得
	D3D11_VIEWPORT viewport;
	UINT num_viewports = 1;
	context->RSGetViewports(&num_viewports, &viewport);
	screenWidth = viewport.Width;
	screenHeight = viewport.Height;

	// 頂点編集開始
	D3D11_MAPPED_SUBRESOURCE mapped_subresource;
	context->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);

	currentVertex = reinterpret_cast<Vertex*>(mapped_subresource.pData);
	currentIndexCount = 0;
	currentPage = -1;
	subsets.clear();
}

void Font::Draw(std::wstring string, DirectX::XMFLOAT2 pos, const DirectX::XMFLOAT2 scale,
	const DirectX::XMFLOAT4 color, float angle, TEXT_ALIGN align, DirectX::XMFLOAT2& Length)
{
	size_t length = ::wcslen(string.c_str());
	size_t maximum_length = length; // 改行も含めた一行当たりの最大文字数
	size_t judg_maximum_length = 0; // 判定用
	size_t previous_line_break = 0; // 一個前に改行した文字数
	int number_of_rows = 1;         // 行数

	float start_x = pos.x;
	float start_y = pos.y;
	fontWidth *= scale.x;
	//fontHeight *= scale.y;
	float space = fontWidth;

	for (size_t i = 0; i < length; ++i)
	{
		// 予め行数、改行された文字列の最大文字数を計算しておく
		WORD word = static_cast<WORD>(string[i]);
		WORD code = characterIndices.at(word);
		if (code == CharacterInfo::ReturnCode)
		{
			int now_number_of_words = (i + 1) - number_of_rows; // 今の文字数
			if (judg_maximum_length <= now_number_of_words - previous_line_break) {
				maximum_length = now_number_of_words - previous_line_break;
				judg_maximum_length = maximum_length;
			}
			previous_line_break = now_number_of_words;
			number_of_rows++;
		}
	}
	for (size_t i = 0; i < length; ++i)
	{
		// 文字値から文字情報が格納されているコードを取得
		WORD word = static_cast<WORD>(string[i]);
		WORD code = characterIndices.at(word);
		// 特殊制御用コードの処理
		if (code == CharacterInfo::EndCode)
		{
			break;
		}
		else if (code == CharacterInfo::ReturnCode)
		{
			pos.x = start_x;
			pos.y += fontHeight * scale.y;
			continue;
		}
		else if (code == CharacterInfo::TabCode)
		{
			pos.x += space * 4;
			continue;
		}
		else if (code == CharacterInfo::SpaceCode)
		{
			pos.x += space;
			continue;
		}
		// 文字情報を取得し、頂点データを編集
		const CharacterInfo& info = characterInfos.at(code);
		// 各種パラメーター
		float positionX = pos.x + info.xoffset;// + 0.5f;
		float positionY = pos.y + info.yoffset;// + 0.5f;
		// 原点変更
		DirectX::XMFLOAT2 align_pos = adjust_text_origin(align, { positionX, positionY },
			maximum_length * 64 * scale.x, number_of_rows * 64 * scale.y);
		// 文字列の長さ保存
		Length = { maximum_length * info.xadvance * scale.x,  number_of_rows * info.xadvance * scale.y };
		// 0---1
		// |   |
		// 2---3
		currentVertex[0].position.x = align_pos.x;
		currentVertex[0].position.y = align_pos.y;
		currentVertex[0].position.z = 0.0f;
		currentVertex[0].texcoord.x = info.left;
		currentVertex[0].texcoord.y = info.top;
		currentVertex[0].color.x = color.x;
		currentVertex[0].color.y = color.y;
		currentVertex[0].color.z = color.z;
		currentVertex[0].color.w = color.w;

		currentVertex[1].position.x = align_pos.x + info.width * scale.x;
		currentVertex[1].position.y = align_pos.y;
		currentVertex[1].position.z = 0.0f;
		currentVertex[1].texcoord.x = info.right;
		currentVertex[1].texcoord.y = info.top;
		currentVertex[1].color.x = color.x;
		currentVertex[1].color.y = color.y;
		currentVertex[1].color.z = color.z;
		currentVertex[1].color.w = color.w;

		currentVertex[2].position.x = align_pos.x;
		currentVertex[2].position.y = align_pos.y + info.height * scale.y;
		currentVertex[2].position.z = 0.0f;
		currentVertex[2].texcoord.x = info.left;
		currentVertex[2].texcoord.y = info.bottom;
		currentVertex[2].color.x = color.x;
		currentVertex[2].color.y = color.y;
		currentVertex[2].color.z = color.z;
		currentVertex[2].color.w = color.w;

		currentVertex[3].position.x = align_pos.x + info.width * scale.x;
		currentVertex[3].position.y = align_pos.y + info.height * scale.y;
		currentVertex[3].position.z = 0.0f;
		currentVertex[3].texcoord.x = info.right;
		currentVertex[3].texcoord.y = info.bottom;
		currentVertex[3].color.x = color.x;
		currentVertex[3].color.y = color.y;
		currentVertex[3].color.z = color.z;
		currentVertex[3].color.w = color.w;

		// 回転
		float cx = align_pos.x + info.width * 0.5f * scale.x;
		float cy = align_pos.y + info.height * 0.5f * scale.y;

		Math::rotate(currentVertex[0].position.x, currentVertex[0].position.y, cx, cy, angle);
		Math::rotate(currentVertex[1].position.x, currentVertex[1].position.y, cx, cy, angle);
		Math::rotate(currentVertex[2].position.x, currentVertex[2].position.y, cx, cy, angle);
		Math::rotate(currentVertex[3].position.x, currentVertex[3].position.y, cx, cy, angle);

		// NDC座標変換
		for (int j = 0; j < 4; ++j)
		{
			currentVertex[j].position.x = 2.0f * currentVertex[j].position.x / screenWidth - 1.0f;
			currentVertex[j].position.y = 1.0f - 2.0f * currentVertex[j].position.y / screenHeight;
		}
		currentVertex += 4;

		// 位置を1文字分ずらす
		{
			pos.x += info.xadvance * cosf(DirectX::XMConvertToRadians(angle)) * scale.x;
			pos.y += info.xadvance * sinf(DirectX::XMConvertToRadians(angle)) * scale.y;
		}

		// テクスチャが切り替わる度に描画する情報を設定
		if (currentPage != info.page)
		{
			currentPage = info.page;

			Subset subset;
			subset.shaderResourceView = shaderResourceViews.at(info.page).Get();
			subset.startIndex = currentIndexCount;
			subset.indexCount = 0;
			subsets.emplace_back(subset);
		}
		currentIndexCount += 6;
	}

}

void Font::End(ID3D11DeviceContext* context)
{
	// 頂点編集終了
	context->Unmap(vertexBuffer.Get(), 0);

	// サブセット調整
	size_t size = subsets.size();
	for (size_t i = 1; i < size; ++i)
	{
		Subset& prev = subsets.at(i - 1);
		Subset& next = subsets.at(i);
		prev.indexCount = next.startIndex - prev.startIndex;
	}
	Subset& last = subsets.back();
	last.indexCount = currentIndexCount - last.startIndex;

	// シェーダー設定
	context->VSSetShader(vertexShader.Get(), nullptr, 0);
	context->PSSetShader(pixelShader.Get(), nullptr, 0);
	context->IASetInputLayout(inputLayout.Get());

	// レンダーステート設定
	context->PSSetSamplers(0, 1, samplerState.GetAddressOf());

	// 頂点バッファ設定
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->PSSetSamplers(0, 1, samplerState.GetAddressOf());

	// 描画
	for (size_t i = 0; i < size; ++i)
	{
		Subset& subset = subsets.at(i);

		context->PSSetShaderResources(0, 1, &subset.shaderResourceView);
		context->DrawIndexed(subset.indexCount, subset.startIndex, 0);
	}
}

DirectX::XMFLOAT2 Font::adjust_text_origin(TEXT_ALIGN align, const DirectX::XMFLOAT2& v, float w, float h)
{
	DirectX::XMFLOAT2 pos = v;
	switch (align)
	{
	default:
	case TEXT_ALIGN::UPPER_LEFT:                                    break;
	case TEXT_ALIGN::UPPER_MIDDLE:  pos.x -= w / 2;                 break;
	case TEXT_ALIGN::UPPER_RIGHT:   pos.x -= w;                     break;
	case TEXT_ALIGN::MIDDLE_LEFT:   pos.y -= h / 2;                 break;
	case TEXT_ALIGN::MIDDLE:        pos.y -= h / 2; pos.x -= w / 2; break;
	case TEXT_ALIGN::MIDDLE_RIGHT:  pos.y -= h / 2; pos.x -= w;     break;
	case TEXT_ALIGN::LOWER_LEFT:    pos.y -= h;                     break;
	case TEXT_ALIGN::LOWER_MIDDLE:  pos.y -= h;     pos.x -= w / 2; break;
	case TEXT_ALIGN::LOWER_RIGHT:   pos.y -= h;     pos.x -= w;     break;
	}

	return pos;
}


FontInstances::FontInstances(ID3D11Device* device)
{
	// フォント初期化
	yu_gothic        = std::make_unique<Font>(device, "resources/Fonts/YuGothic.fnt", 2048);
	gothic           = std::make_unique<Font>(device, "resources/Fonts/MS Gothic_64.fnt", 2048);
	algerian_gold    = std::make_unique<Sprite>(device, L".\\resources\\Fonts\\Algerian_Gold(256x256).png");
}
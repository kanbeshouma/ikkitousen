#pragma once

#include <string>
#include <vector>
#include <memory>
#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>

#include "sprite.h"

// 文字描画のアライン
enum class TEXT_ALIGN
{
	UPPER_LEFT = 0, // 左上
	UPPER_MIDDLE,   // 上中央
	UPPER_RIGHT,    // 右上
	MIDDLE_LEFT,    // 左中央
	MIDDLE,         // 中央
	MIDDLE_RIGHT,   // 右中央
	LOWER_LEFT,     // 左下
	LOWER_MIDDLE,   // 下中央
	LOWER_RIGHT,    // 右下
};

class Font
{
public:
	Font(ID3D11Device* device, const char* filename, int maxSpriteCount);
	~Font() {}

	void Begin(ID3D11DeviceContext* context);
	void Draw(std::wstring string, DirectX::XMFLOAT2 pos, const DirectX::XMFLOAT2 scale, const DirectX::XMFLOAT4 color,
		float angle = 0, TEXT_ALIGN align = TEXT_ALIGN::MIDDLE_LEFT, DirectX::XMFLOAT2& Length = DirectX::XMFLOAT2(0, 0));
	void End(ID3D11DeviceContext* context);
private:
	//--------< 関数 >--------//
	DirectX::XMFLOAT2 adjust_text_origin(TEXT_ALIGN align, const DirectX::XMFLOAT2& v, float w, float h);
	//--------< 変数 >--------//
	Microsoft::WRL::ComPtr<ID3D11VertexShader>						vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>						pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>						inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer>							vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>							indexBuffer;

	Microsoft::WRL::ComPtr<ID3D11SamplerState>						samplerState;
	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>	shaderResourceViews;

	struct Vertex
	{
		DirectX::XMFLOAT3			position;
		DirectX::XMFLOAT4			color;
		DirectX::XMFLOAT2			texcoord;
	};

	struct CharacterInfo
	{
		static const WORD			NonCode = 0;			// コードなし
		static const WORD			EndCode = 0xFFFF;		// 終了コード
		static const WORD			ReturnCode = 0xFFFE;	// 改行コード
		static const WORD			TabCode = 0xFFFD;		// タブコード
		static const WORD			SpaceCode = 0xFFFC;		// 空白コード

		float						left;					// テクスチャ左座標
		float						top;					// テクスチャ上座標
		float						right;					// テクスチャ右座標
		float						bottom;					// テクスチャ下座標
		float						xoffset;				// オフセットX
		float						yoffset;				// オフセットY
		float						xadvance;				// プロポーショナル幅
		float						width;					// 画像幅
		float						height;					// 画像高さ
		int							page;					// テクスチャ番号
		bool						ascii;					// アスキー文字
	};

	float							fontWidth;
	float							fontHeight;
	int								textureCount;
	int								characterCount;
	std::vector<CharacterInfo>		characterInfos;
	std::vector<WORD>				characterIndices;

	struct Subset
	{
		ID3D11ShaderResourceView* shaderResourceView;
		UINT						startIndex;
		UINT						indexCount;
	};
	std::vector<Subset>				subsets;
	Vertex* currentVertex = nullptr;
	UINT							currentIndexCount;
	int								currentPage;

	float							screenWidth;
	float							screenHeight;
};

class FontInstances
{
public:
	FontInstances(ID3D11Device* device);
	~FontInstances() {}
public:
	//--font instance--//
	std::unique_ptr<Font> yu_gothic;
	std::unique_ptr<Font> gothic;
	std::unique_ptr<Sprite> algerian_gold;
};
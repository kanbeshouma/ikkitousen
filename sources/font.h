#pragma once

#include <string>
#include <vector>
#include <memory>
#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>

#include "sprite.h"

// �����`��̃A���C��
enum class TEXT_ALIGN
{
	UPPER_LEFT = 0, // ����
	UPPER_MIDDLE,   // �㒆��
	UPPER_RIGHT,    // �E��
	MIDDLE_LEFT,    // ������
	MIDDLE,         // ����
	MIDDLE_RIGHT,   // �E����
	LOWER_LEFT,     // ����
	LOWER_MIDDLE,   // ������
	LOWER_RIGHT,    // �E��
};

class Font
{
public:
	Font(ID3D11Device* device, const char* filename, int maxSpriteCount);
	~Font() {}

	void Begin(ID3D11DeviceContext* context);
	void Draw(std::wstring string, DirectX::XMFLOAT2 pos, const DirectX::XMFLOAT2 scale, const DirectX::XMFLOAT4 color,
		float angle = 0, TEXT_ALIGN align = TEXT_ALIGN::MIDDLE_LEFT, DirectX::XMFLOAT2 Length = {});
	void End(ID3D11DeviceContext* context);
private:
	//--------< �֐� >--------//
	DirectX::XMFLOAT2 adjust_text_origin(TEXT_ALIGN align, const DirectX::XMFLOAT2& v, float w, float h);
	//--------< �ϐ� >--------//
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
		static const WORD			NonCode = 0;			// �R�[�h�Ȃ�
		static const WORD			EndCode = 0xFFFF;		// �I���R�[�h
		static const WORD			ReturnCode = 0xFFFE;	// ���s�R�[�h
		static const WORD			TabCode = 0xFFFD;		// �^�u�R�[�h
		static const WORD			SpaceCode = 0xFFFC;		// �󔒃R�[�h

		float						left;					// �e�N�X�`�������W
		float						top;					// �e�N�X�`������W
		float						right;					// �e�N�X�`���E���W
		float						bottom;					// �e�N�X�`�������W
		float						xoffset;				// �I�t�Z�b�gX
		float						yoffset;				// �I�t�Z�b�gY
		float						xadvance;				// �v���|�[�V���i����
		float						width;					// �摜��
		float						height;					// �摜����
		int							page;					// �e�N�X�`���ԍ�
		bool						ascii;					// �A�X�L�[����
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
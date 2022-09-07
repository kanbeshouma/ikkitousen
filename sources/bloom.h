#pragma once

#include <memory>
#include <vector>

#include "framebuffer.h"
#include "fullscreen_quad.h"
#include "shader.h"

class Bloom : FullScreenQuad
{
public:
	Bloom(ID3D11Device* device, uint32_t width, uint32_t height);
	~Bloom() = default;
	Bloom(const Bloom&) = delete;
	Bloom& operator =(const Bloom&) = delete;
	Bloom(Bloom&&) noexcept = delete;
	Bloom& operator =(Bloom&&) noexcept = delete;

	void Bloom::make(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* color_map);
	void blit(ID3D11DeviceContext* dc);
private:
	std::unique_ptr<FrameBuffer> glow_extraction;

	static const size_t downsampled_count = 3;
	std::unique_ptr<FrameBuffer> gaussian_blur[downsampled_count][3];

private:
	Microsoft::WRL::ComPtr<ID3D11PixelShader> bloom_extraction_ps;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> gaussian_blur_horizontal_ps;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> gaussian_blur_vertical_ps;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> gaussian_blur_convolution_ps;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> gaussian_blur_downsampling_ps;
};
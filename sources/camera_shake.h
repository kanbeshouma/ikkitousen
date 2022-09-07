#pragma once

#include <d3d11.h>
#include <directxmath.h>
#include "perlin_noise.h"
#include "misc.h"
#include "graphics_pipeline.h"

class CameraShake
{
public:
    //--------< コンストラクタ/関数等 >--------//
    CameraShake();
    ~CameraShake();
    void shake(GraphicsPipeline& graphics, float elapsed_time);
    void shake(GraphicsPipeline& graphics, float elapsed_time,
        float max_skew, float max_sway_x, float max_sway_y, float seed_shifting_factor);
    void reset(GraphicsPipeline& graphics);
    //--------<getter/setter>--------//
    void set_max_skew(float v) { max_skew = v; }
    void set_max_sway_x(float v) { max_sway_x = v; }
    void set_max_sway_y(float v) { max_sway_y = v; }
    void set_seed_shifting_factor(float v) { seed_shifting_factor = v; }
    const DirectX::XMFLOAT4X4 get_shake_matrix() const { return shake_matrix; }



    // 毎フレーム一回のみ呼び出すこと
private:
    //--------< 変数 >--------//
    float max_skew = 0.00f;		// unit is degrees angle.
    float max_sway_x = 12.00f;	// unit is pixels in screen space.
    float max_sway_y = 33.00f;	// unit is pixels in screen space.
    PerlinNoise pn;
    float seed = 0;
    float seed_shifting_factor = 3.00f;
    DirectX::XMFLOAT4X4 shake_matrix{ 1, 0, 0, 0,
                                      0, 1, 0, 0,
                                      0, 0, 1, 0,
                                      0, 0, 0, 1 };
    
};

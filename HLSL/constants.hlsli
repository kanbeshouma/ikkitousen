static const int MAX_BONES = 256;
cbuffer OBJECT_CONSTANTS : register(b0)
{
    row_major float4x4 world;
    float4 material_color;
    row_major float4x4 bone_transforms[MAX_BONES];
    float4 emissive_color; // xyz:emissive_color w:emissive_strength
    float4 dissolve_threshold; // x:縁が近るディゾルブ y:光の筋ディゾルブ
    float glow_thickness; // glowの幅
    float3 object_pad1;
    float sub_color_threshold_purple;
    float sub_color_threshold_red;
    float2 object_pad2;
};

cbuffer SCENE_CONSTANTS : register(b1)
{
    row_major float4x4 view_projection;
    float4 light_direction;
    float4 light_color;
    float4 camera_position;
    row_major float4x4 shake_matrix;
    row_major float4x4 inverse_view_projection;
};

cbuffer SPRITE_CONSTANTS : register(b2)
{
    float4 threshold;
};

static const int POINT_LIGHT_COUNT = 8;
struct PointLightParameters
{
    float4 position;
    float4 color;
    float range;
    float luminous_intensity;
    float2 pad;
};
cbuffer POINT_LIGHT_CONSTANTS : register(b3)
{
    PointLightParameters point_lights[POINT_LIGHT_COUNT];
    int point_light_unique_id;
    int3 point_light_pad1;
};

cbuffer POST_EFFECT_CONSTANTS : register(b5)
{
    // luminance
    float4 luminance_min_max;
    float4 luminance_dot_rgb;
    // blur
    float blur_gaussian_sigma;
    float3 post_effect_pad1;
    float blur_bloom_intensity;
    float3 post_effect_pad2;
    // white_noise
    float white_noise_time;
    float3 post_effect_pad3;
    // low_resolution
    float low_resolution;
    float low_resolution_number_of_divisions;
    float2 post_effect_pad4;
    // scan_line
    float scan_line_time;
    float3 post_effect_pad5;
    // game_boy
    float4 border_color1;
    float4 border_color2;
    float4 border_color3;
    float4 border_color4;
    // barrel_shaped
    float barrel_shaped_distortion;
    float3 post_effect_pad6;
    // glitch
    float2 glitch_pase;
    float glitch_step_value;
    float glitch_time;
    // vignette
    float2 vignetting_pos;
    float vignetting_scope;
    float post_effect_pad7;
    // dash_blur
    float4 reference_position;
    float zoom_power;
    int focus_detail;
    float2 post_effect_pad8;
    // lockon
    float lockon_scope;
    float lockon_thickness;
    float lockon_alpha;
    float post_effect_pad9;
    float3 lockon_color;
    float post_effect_pad10;
    // slashing attack
    float slashing_power;
    float3 post_effect_pad11;
    // zoom rgb shift
    float2 rgb_shift_target_point;
    float rgb_shift_zoom_power;
    float post_effect_pad12;
    // wipe
    float wipe_threshold;
    float3 post_effect_pad13;
    //color filter
       // 色相調整
    float hueShift;
    // 彩度調整
    float saturation;
    // 明度調整
    float brightness;
    float pad14;
};

cbuffer SHADOWMAP_CONSTANTS : register(b6)
{
    row_major float4x4 light_view_projection;
    float3 shadow_color;
    float shadow_bias;

    int number_of_trials;
    int3 shadow_map_pad1;
    float search_width_magnification;
    float3 shadow_map_pad2;
};

cbuffer MIST_CONSTANTS : register(b7)
{
    float4 fog_color;
    float4 fog_range; // x:near y:far
};

cbuffer BLOOM_CONSTANTS : register(b8)
{
    float bloom_extraction_threshold;
    float blur_convolution_intensity;
    float2 bloom_pad1;
};

cbuffer PARTICLE_CONSTANTS : register(b9)
{
    uint particle_count;
    float particle_size;
    float particle_option;
    float delta_time;
    float4 position_on_near_plane;
    float4 eye_position;
};



cbuffer FIELD_CONSTANTS : register(b10)
{
    float field_time;
    float2 field_resolution;
    float field_pad1;
};
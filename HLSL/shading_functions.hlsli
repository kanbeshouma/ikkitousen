//--------------------------------------------
//	ランバート拡散反射計算関数
//--------------------------------------------
// N:法線(正規化済み)
// L:入射ベクトル(正規化済み)
// C:入射光(色・強さ)
// K:反射率
float3 calc_lambert(float3 N, float3 L, float3 C)
{
    float power = saturate(dot(N, -L));
    return C * power;
}

//--------------------------------------------
//	フォンの鏡面反射計算関数
//--------------------------------------------
// N:法線(正規化済み)
// L:入射ベクトル(正規化済み)
// E:視線ベクトル(正規化済み)
// C:入射光(色・強さ)
// K:反射率
float3 calc_phong_specular(float3 N, float3 L, float3 E, float3 C)
{
    float3 R = reflect(L, N);
    float power = max(dot(-E, R), 0);
    power = pow(power, 128);
    return C * power;
}

//--------------------------------------------
//	ハーフランバート拡散反射計算関数
//--------------------------------------------
// N:法線(正規化済み)
// L:入射ベクトル(正規化済み)
// C:入射光(色・強さ)
// K:反射率
float3 clac_half_lambert(float3 N, float3 L, float3 C)
{
    float D = saturate(dot(N, -L) * 0.5f + 0.5f);
    return C * D;
}
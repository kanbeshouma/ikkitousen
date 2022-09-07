#include "constants.hlsli"
#include"SwordTrail.hlsli"

SamplerState gSampler : register(s0);
Texture2D<float4> gTexNormal : register(t0);
Texture2D<float4> gTexColor : register(t1);

cbuffer Data :register(b0)
{
	float yugamiThreshold;
	float3 Pad0;
}

float4 main(PsInput PsInput_) : SV_TARGET
{
	float4 outPutColor = { 1.0f, 0.2f, 0.2f, 1.0f };
	float c = 1.0f - PsInput_.Texcoord.y;
	c = lerp(0.8f,c, step(0.8f, c));
	if(c>0.85f)
	{
		float a = step(0.0f, c);
	    //          [F‚Ì”Z‚³]   [”Z‚¢•”•ª‚Ì•]
		float b = (pow(5, 10 * c - 9) * a);
		outPutColor = outPutColor * b;
	}
	else
	{
		outPutColor = lerp(0.0f, 0.25f, 1.0f - PsInput_.Texcoord.y);
	}
	outPutColor.xyz *= 3.0f;
	// ƒuƒ‹[ƒ€‚Å–\‘–‚µ‚È‚¢‚æ‚¤‚É‹­§
    outPutColor.xyz = min(outPutColor.xyz, 6.0);

	return outPutColor;
}
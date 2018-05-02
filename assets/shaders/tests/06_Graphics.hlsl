struct VSInput
{
	float3 position : POSITION;
	float2 texCoord : TEXCOORD0;
	float3 objectPosition : TEXCOORD1;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD0;
};

cbuffer ConstantBuffer : register(b0)
{
	float4 color;
};

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

PSInput VSMain(VSInput input)
{
	PSInput output;
	output.position = float4(input.position + input.objectPosition, 1.0);
	output.texCoord = input.texCoord;
	return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	return color * Texture.Sample(Sampler, input.texCoord);
}
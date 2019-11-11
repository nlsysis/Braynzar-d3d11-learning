cbuffer cbPerObject
{
	float4x4 WVP;
	float4x4 World;
};

struct Light
{
	float3 dir;
	float4 ambient;
	float4 diffuse;
};

cbuffer cbPerFrame
{
	Light light;
};

Texture2D ObjTexture;
SamplerState ObjSamplerState;
TextureCube SkyMap;   //it is kind of Texture2D array

struct  SKYMAP_VS_OUTPUT
{
	float4 Pos: SV_POSITION;
	float3 texCoord : TEXCOORD;
};
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 normal : NORMAL;
};

VS_OUTPUT VS(float4 inPos : POSITION, float2 texcoord : TEXCOORD,float3 normal : NORMAL)
{
	VS_OUTPUT output;

	output.Pos = mul(inPos,WVP);
	output.TexCoord = texcoord;
	output.normal = mul(normal,World);
	//output.worldPos = mul(inPos, World);

	return output;
}

float4 PS(VS_OUTPUT input) : SV_TARGET
{
	input.normal = normalize(input.normal);

	float4 diffuse = ObjTexture.Sample(ObjSamplerState, input.TexCoord);

	float3 finalColor;

	finalColor = diffuse * light.ambient;
	finalColor += saturate(dot(light.dir, input.normal) * light.diffuse * diffuse);

	return float4(finalColor, diffuse.a);
}

//float4 D2D_PS(VS_OUTPUT input) : SV_TARGET
//{
//	float4 diffuse = ObjTexture.Sample(ObjSamplerState, input.TexCoord);
//
//	return diffuse;
//}

SKYMAP_VS_OUTPUT SKYMAP_VS(float3 inPos : POSITION, float2 inTexCoord : TEXCOORD, float3 normal : NORMAL)
{
	SKYMAP_VS_OUTPUT output = (SKYMAP_VS_OUTPUT)0;
	//set Pos to xyww instead of xyzw.so the z will always 1
	output.Pos = mul(float4(inPos, 1.0f), WVP).xyww;
	output.texCoord = inPos;
	return output;
}

float4 SKYMAP_PS(SKYMAP_VS_OUTPUT input) : SV_Target
{
	return SkyMap.Sample(ObjSamplerState,input.texCoord);
}
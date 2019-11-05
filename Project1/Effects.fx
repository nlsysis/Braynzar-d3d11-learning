cbuffer cbPerObject
{
	float4x4 WVP;
	float4x4 World;
};

struct Light
{
	float3 dir;
	float3 pos;
	float range;
	float3 att;
	float4 ambient;
	float4 diffuse;
};

cbuffer cbPerFrame
{
	Light light;
};

Texture2D ObjTexture;
SamplerState ObjSamplerState;

struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 normal : NORMAL;
	float4 worldPos : POSITION;
};

VS_OUTPUT VS(float4 inPos : POSITION, float2 texcoord : TEXCOORD,float3 normal : NORMAL)
{
	VS_OUTPUT output;

	output.Pos = mul(inPos,WVP);
	output.TexCoord = texcoord;
	output.normal = mul(normal,World);
	output.worldPos = mul(inPos, World);

	return output;
}

float4 PS(VS_OUTPUT input) : SV_TARGET
{
	input.normal = normalize(input.normal);
	float4 diffuse = ObjTexture.Sample(ObjSamplerState, input.TexCoord);
	float3 finalColor = float3(0.0f,0.0f,0.0f);
	float4  testAmbient = float4(0.5f, 0.5f, 0.5f, 1.0f);
	//Create vector from light to pixel
	float3 lightToPixelVec = light.pos - input.worldPos;

	//the length of the vector
	float d = length(lightToPixelVec);

	//create the ambient light
	float3 finalAmbient = diffuse * light.ambient;

	//if pixel is too far return pixel color with ambient
	if (d > light.range)
		return float4(finalAmbient, diffuse.a);
	

	//get  unit length vector
	lightToPixelVec /= d;

	float howMuchLight = dot(lightToPixelVec, input.normal);

	if (howMuchLight > 0.0f)
	{
		//Add light to the finalColor of the pixel
		finalColor += howMuchLight * diffuse * light.diffuse;

		//Calculate light falloff factor
		finalColor /= light.att[0] + (light.att[1] * d) + (light.att[2] * (d * d));
	}

	finalColor = saturate(finalColor + finalAmbient);

	return float4(finalColor, diffuse.a);
}

//float4 D2D_PS(VS_OUTPUT input) : SV_TARGET
//{
//	float4 diffuse = ObjTexture.Sample(ObjSamplerState, input.TexCoord);
//
//	return diffuse;
//}
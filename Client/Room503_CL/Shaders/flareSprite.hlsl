#include "LightingUtil.hlsl"

Texture2D gFlareMap : register(t0);

SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

cbuffer cbPerObject : register(b0)
{
	float4x4 gWorld;
	float4x4 gTexTransform;
	uint gTextureAniIndex;
	uint Pad0;
	uint Pad1;
	uint Pad2;
};

cbuffer cbPass : register(b1)
{
	float4x4 gView;
	float4x4 gInvView;
	float4x4 gProj;
	float4x4 gInvProj;
	float4x4 gViewProj;
	float4x4 gInvViewProj;
	/*float4x4 gViewMini;
	float4x4 gProjMini;
	float4x4 gViewProjMini;*/
	float3 gEyePosW;
	float cbPerObjectPad1;
	//float3 gEyePosWMini;
	//float cbPerObjectPad2;
	float2 gRenderTargetSize;
	float2 gInvRenderTargetSize;
	float gNearZ;
	float gFarZ;
	float gTotalTime;
	float gDeltaTime;
	float4 gAmbientLight;

	float4 gFogColor;
	float gFogStart;
	float gFogRange;
	float2 cbPerObjectPad3;

	Light gLights[MaxLights];
};

cbuffer cbMaterial : register(b2)
{
	float4   gDiffuseAlbedo;
    float3   gFresnelR0;
    float    gRoughness;
	float4x4 gMatTransform;
	uint     DiffuseMapIndex;
};
 
struct VertexIn
{
	float3 PosW  : POSITION;
	float2 SizeW : SIZE;
};

struct VertexOut
{
	float3 CenterW : POSITION;
	float2 SizeW   : SIZE;
};

struct GeoOut
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
    float2 TexC    : TEXCOORD;
    uint   PrimID  : SV_PrimitiveID;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	float4 posW = mul(float4(vin.PosW, 1.0f), gWorld);
	vout.CenterW = posW.xyz;
	vout.SizeW   = vin.SizeW;

	return vout;
}
 
// �� ���� �簢�� ���� Ȯ��. ������Ƽ�� ȣ��� �ִ� ��� ���� ���� 4
[maxvertexcount(4)]
void GS(point VertexOut gin[1],
	uint primID : SV_PrimitiveID,
	inout TriangleStream<GeoOut> triStream)
{
	//
	// ���
	//
	float3 up = float3(0.0f, 1.0f, 0.0f);
	float3 look = gEyePosW - gin[0].CenterW;
	look = normalize(look);
	float3 right = cross(up, look);

	//
	// ������� ������ �ﰢ�� �� �������� ��� (�簢�� �����ϴ� �ﰢ��)
	//
	float halfWidth = 0.5f*gin[0].SizeW.x;
	float halfHeight = 0.5f*gin[0].SizeW.y;

	float4 v[4];
	v[0] = float4(gin[0].CenterW + halfWidth*right - halfHeight*up, 1.0f); //�����ʾƷ�
	v[1] = float4(gin[0].CenterW + halfWidth*right + halfHeight*up, 1.0f); //��������;
	v[2] = float4(gin[0].CenterW - halfWidth*right - halfHeight*up, 1.0f); //���ʾƷ�
	v[3] = float4(gin[0].CenterW - halfWidth*right + halfHeight*up, 1.0f);

	//
	//
	int indexX = gTextureAniIndex % 5;
	int indexY = gTextureAniIndex / 5;
	float2 texC[4] =
	{
		float2(indexX*0.2f, 0.2f* indexY + 0.2f),
		float2(indexX*0.2f, 0.2f* indexY),
		float2(indexX*0.2f + 0.2f, 0.2f* indexY + 0.2f),
		float2(indexX*0.2f + 0.2f, 0.2f* indexY)
	};

	GeoOut gout;
	[unroll]
	for (int i = 0; i < 4; ++i)
	{
		gout.PosH = mul(v[i], gViewProj);
		gout.PosW = v[i].xyz;
		gout.NormalW = look;
		gout.TexC = texC[i];
		gout.PrimID = primID;

		triStream.Append(gout);
	}
}

float4 PS(GeoOut pin) : SV_Target
{
    float4 diffuseAlbedo = gFlareMap.Sample(gsamAnisotropicWrap, pin.TexC) * gDiffuseAlbedo;
	if(diffuseAlbedo.x < 0.05f && diffuseAlbedo.y < 0.05f && diffuseAlbedo.z < 0.05f) diffuseAlbedo.a = 0;
#ifdef ALPHA_TEST
	// �� ������ �ִ��� ���� �����ϴ� ���� �ٶ�����.
	clip(diffuseAlbedo.a - 0.1f);
#endif

	// ������ ���� �� �ٽ� ����ȭ
	pin.NormalW = normalize(pin.NormalW);

	// ���� �Ǵ� ������ �������� ����
	float3 toEyeW = gEyePosW - pin.PosW;
	float distToEye = length(toEyeW);
	toEyeW /= distToEye; // normalize

	 // �������
	float4 effectAmbi = float4(1.0f, 0.5f, 0.2f, 1.0f);
    float4 ambient = effectAmbi *diffuseAlbedo;

    const float shininess = 1.0f - gRoughness;

    float4 litColor = ambient ;

#ifdef FOG
	float fogAmount = saturate((distToEye - gFogStart) / gFogRange);
	litColor = lerp(litColor, gFogColor, fogAmount);
#endif

    // Common convention to take alpha from diffuse albedo.
    litColor.a = diffuseAlbedo.a;

    return litColor;
}



//=============================================================================
// Performs edge detection using Sobel operator.
//=============================================================================

Texture2D gInput            : register(t0);
RWTexture2D<float4> gOutput : register(u0);


// ���� ������ ���� ���� ���� �ΰ����� ������ �ֵ� �ٻ簪
float CalcLuminance(float3 color)
{
    return dot(color, float3(0.299f, 0.587f, 0.114f));
}

[numthreads(16, 16, 1)]
void SobelCS(int3 dispatchThreadID : SV_DispatchThreadID)
{
    // �ȼ� �ֺ� �̿� �ȼ����� ����
	float4 c[3][3];
	for(int i = 0; i < 3; ++i)
	{
		for(int j = 0; j < 3; ++j)
		{
			int2 xy = dispatchThreadID.xy + int2(-1 + j, -1 + i);
			c[i][j] = gInput[xy]; 
		}
	}

	// �� ���� ä�ο� ����, �Һ� ���� �̿��� x y�� ��̺� ����� �����Ѵ�.
	float4 Gx = -1.0f*c[0][0] - 2.0f*c[1][0] - 1.0f*c[2][0] + 1.0f*c[0][2] + 2.0f*c[1][2] + 1.0f*c[2][2];

	float4 Gy = -1.0f*c[2][0] - 2.0f*c[2][1] - 1.0f*c[2][1] + 1.0f*c[0][0] + 2.0f*c[0][1] + 1.0f*c[0][2];

	// gx,gy�� �� �ȼ��� ����Ⱑ �ȴ�. �� ���� ä�ο� ���� ��ȭ���� �ִ밡 �Ǵ� ũ�⸦ ���Ѵ�.
	float4 mag = sqrt(Gx*Gx + Gy*Gy);

	// ������ �ȼ��� �˰�, �ƴϸ� ��� �����.
	mag = 1.0f - saturate(CalcLuminance(mag.rgb));
	if (mag.r <= 0.5f || mag.g <= 0.5f || mag.b <= 0.5f) mag = float4(0, 0, 0, 1);
	gOutput[dispatchThreadID.xy] = mag;
}

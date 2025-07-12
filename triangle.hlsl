// triangle.hlsl - ���_�V�F�[�_�ƃs�N�Z���V�F�[�_

struct VSInput
{
    float3 position : POSITION;
};

struct PSInput
{
    float4 position : SV_POSITION;
};

// ���_�V�F�[�_
PSInput VSMain(VSInput input)
{
    PSInput output;
    output.position = float4(input.position, 1.0);
    return output;
}

// �s�N�Z���V�F�[�_
float4 PSMain(PSInput input) : SV_TARGET
{
    return float4(1.0, 0.0, 0.0, 1.0); // �ԐF
}

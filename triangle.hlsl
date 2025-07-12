// triangle.hlsl - ���_�V�F�[�_�ƃs�N�Z���V�F�[�_

struct VSInput
{
    float3 position : POSITION;
    float4 color : COLOR; 
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

// ���_�V�F�[�_
PSInput VSMain(VSInput input)
{
    PSInput output;
    output.position = float4(input.position, 1.0);
    output.color = input.color; // ���͂̐F�����̂܂܏o��
    return output;
}

// �s�N�Z���V�F�[�_
float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color; // ���͂̐F�����̂܂܏o��
}

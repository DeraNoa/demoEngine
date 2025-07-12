// triangle.hlsl - 頂点シェーダとピクセルシェーダ

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

// 頂点シェーダ
PSInput VSMain(VSInput input)
{
    PSInput output;
    output.position = float4(input.position, 1.0);
    output.color = input.color; // 入力の色をそのまま出力
    return output;
}

// ピクセルシェーダ
float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color; // 入力の色をそのまま出力
}

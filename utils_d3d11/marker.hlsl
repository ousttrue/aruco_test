struct VS_IN
{
    float4 Position: POSITION;
    float4 Color: COLOR;
	float2 Tex: TEXCOORD0;
};

struct VS_OUT
{
    float4 Position: SV_POSITION;
    float4 Color: COLOR;
	float2 Tex: TEXCOORD0;
};

typedef VS_OUT PS_IN;

row_major matrix ModelMatrix;
row_major matrix ViewMatrix;
row_major matrix ProjectionMatrix;

VS_OUT VS(VS_IN input)
{
    VS_OUT Output;
	Output.Position = mul(input.Position, mul(ModelMatrix, mul(ViewMatrix, ProjectionMatrix)));
    Output.Color = input.Color;
	Output.Tex = input.Tex;
    return Output;    
}

float4 PS(PS_IN input) : SV_TARGET
{
    return input.Color;
}

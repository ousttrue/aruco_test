Texture2D	tex0		: register(t0);
SamplerState	sample0		: register(s0);

//////////////////////////////////
// VS
//////////////////////////////////
struct VS_OUT
{
};

VS_OUT VS()
{
	VS_OUT output = (VS_OUT)0;
	return output;
}

//////////////////////////////////
// GS
//////////////////////////////////
typedef VS_OUT GS_IN;

struct GS_OUT
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD;
};

[maxvertexcount(4)]
void GS(point GS_IN particles[1]
	, uint primID : SV_PrimitiveID
	, inout TriangleStream<GS_OUT> triStream)
{
	GS_OUT output;

	// 0
	output.pos = float4(-1, -1, 0, 1);
	output.tex = float2(0, 1);
	triStream.Append(output);
	// 1
	output.pos = float4(-1, 1, 0, 1);
	output.tex = float2(0, 0);
	triStream.Append(output);
	// 2
	output.pos = float4(1, -1, 0, 1);
	output.tex = float2(1, 1);
	triStream.Append(output);
	// 3
	output.pos = float4(1, 1, 0, 1);
	output.tex = float2(1, 0);
	triStream.Append(output);
}

//////////////////////////////////
// PS
//////////////////////////////////
typedef GS_OUT PS_IN;

struct PS_OUT
{
	float4 col : SV_Target;
};

PS_OUT PS(PS_IN input)
{
	PS_OUT output = (PS_OUT)0;

	float4	texcol = tex0.Sample(sample0, input.tex);
	output.col = texcol;
	//output.col = float4(1, 0, 0, 1);

	return output;
}

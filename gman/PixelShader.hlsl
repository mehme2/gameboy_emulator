Texture2D tex;
SamplerState sam;

float4 main(float2 texCoord : TEXCOORD) : SV_TARGET
{
	return tex.Sample(sam, texCoord);
}
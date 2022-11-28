struct VSout
{
    float2 coord : TEXCOORD;
    float4 pos : SV_Position;
};

VSout main( float4 pos : POSITION, float2 texCoord : TEXCOORD )
{
    VSout vso;
    vso.pos = pos;
    vso.coord = texCoord;
	return vso;
}
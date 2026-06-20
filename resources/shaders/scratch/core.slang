static float2 positions[3] = float2[]
(
    float2(0.0, -0.5),
    float2(0.5, 0.5),
    float2(-0.5, 0.5)
);

static float3 colors[3] = float3[]
(
    float3(1.0, 0.0, 0.0),
    float3(0.0, 1.0, 0.0),
    float3(0.0, 0.0, 1.0)
);

struct vertex_output_t 
{
    float3 color;
    float4 sv_position : SV_Position;
};

[shader("vertex")] vertex_output_t 
vertex_main(uint32_t vid: SV_VertexID)
{
    vertex_output_t output = {};
    output.sv_position = float4(positions[vid], 0.0, 1.0);
    output.color = colors[vid];
    return output;
}

[shader("fragment")] float4 
fragment_main(vertex_output_t in_vertex) : SV_Target
{
    float3 color = in_vertex.color;
    return float4(color, 1.0);
}
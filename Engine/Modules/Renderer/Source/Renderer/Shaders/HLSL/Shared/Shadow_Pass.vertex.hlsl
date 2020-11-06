#include <../Deferred_Rendering/Deferred_Rendering.hlsli>	

// Entry Point
// -----------
VS_OUTPUT_SHADOWPASS main(VS_INPUT_SHADOWPASS vs_in)
{
    VS_OUTPUT_SHADOWPASS vs_out;
    
    //vs_in.position.x *= 0.5;
    //vs_in.position.y *= 0.5;
    //vs_in.position.z *= 0.5;
    
    float4 worldPos = mul(float4(vs_in.position, 1.0), world);
    float4x4 wvpLightSpace = mul(mul(world, dirLight.lightSpaceView), dirLight.lightSpaceProj);
        
    vs_out.sv_position = mul(float4(vs_in.position, 1.0), wvpLightSpace);
    vs_out.fragPos = worldPos.xyz;
    
    return vs_out;
}
#include <..//Deferred_Rendering/Deferred_Rendering.hlsli>

// Texture Inputs
// --------------
Texture2D t_AlbedoGBuffer               : register(t0);
Texture2D t_NormalGBuffer               : register(t1);
Texture2D t_RoughnessMetallicAOGBuffer  : register(t2);
Texture2D t_PositionGBuffer             : register(t3);
Texture2D t_SceneDepthGBuffer           : register(t4);

Texture2D t_ShadowDepthPass : register(t10);
Texture2D t_LightPassResult : register(t15);

RWTexture2D<float4> rw_BloomPassResult : register(u1);

// Samplers
// --------
sampler s_PointClampSampler         : register(s0);
SamplerState s_LinearWrapSampler    : register(s1);

// Function Signatures
// -------------------
float3 AddFilmGrain(float3 sourceColor, float2 texCoords);
float3 AddVignette(float3 sourceColor, float2 texCoords);
float3 AddChromaticAberration(float3 sourceColor, float2 texCoords);
float3 AddBloom(float3 sourceColor, float2 texCoords);
void LinearizeDepth(inout float depth);

void HDRToneMap(inout float3 target);
void GammaCorrect(inout float3 target);
// Pixel Shader Return Value
// -------------------------
struct PS_INPUT_POSTFX
{
    float4 sv_position : SV_POSITION;
    float2 texCoords : TEXCOORD;
};

float4 main(PS_INPUT_POSTFX ps_in) : SV_TARGET
{
    float3 LightPassResult = t_LightPassResult.Sample(s_PointClampSampler, ps_in.texCoords).rgb;
    //float3 LightPassResult = t_AlbedoGBuffer.Sample(s_PointClampSampler, ps_in.texCoords).rgb;
    
    float3 result = LightPassResult;
    
    if(blEnabled)
    {
        result = AddBloom(result, ps_in.texCoords);
    }
    if (vnEnabled)
    {
        result = AddVignette(result, ps_in.texCoords);
    }
    if (fgEnabled)
    {
        result = AddFilmGrain(result, ps_in.texCoords);
    }
    if (caEnabled)
    {
        result = AddChromaticAberration(result, ps_in.texCoords);
    }
    
    HDRToneMap(result);
    GammaCorrect(result);
    
    return float4(result, 1.0);
}

void HDRToneMap(inout float3 target)
{
    target = float3(1.0, 1.0, 1.0) - exp(-target * cbCameraExposure);
}

void GammaCorrect(inout float3 target)
{
    const float gamma = 2.2;
    target = pow(target.rgb, float3(1.0 / gamma, 1.0 / gamma, 1.0 / gamma));
}

float mod(float x, float y)
{
    return (x - y * floor(x / y));
}

float3 AddBloom(float3 sourceColor, float2 texCoords)
{
    float2 PixelCoords = (texCoords * cbScreenSize) / 2;
    float3 BloomPassResult = rw_BloomPassResult.Load(int3(PixelCoords, 0.0)).rgb;
 
    return mad(blCombineCoefficient, BloomPassResult, sourceColor);
}

float3 AddChromaticAberration(float3 sourceColor, float2 texCoords)
{
    float2 texel = 1.0 / cbScreenSize;
    float2 coords = (texCoords - 0.5) * 2.0;
    float coordsDot = dot(coords, coords);
    
    float2 precompute = caIntensity * coordsDot * coords;
    float2 uvR = texCoords - texel.xy * precompute;
    float2 uvB = texCoords + texel.xy * precompute;
    
    // TODO: this effect overwrites other effects because it adds the color texture directly. Fix it
    sourceColor.r = t_LightPassResult.Sample(s_LinearWrapSampler, uvR).r;
    sourceColor.g = t_LightPassResult.Sample(s_LinearWrapSampler, texCoords).g;
    sourceColor.b = t_LightPassResult.Sample(s_LinearWrapSampler, uvB).b;
    
    return sourceColor;
}

float3 AddFilmGrain(float3 sourceColor, float2 texCoords)
{
    float x = (texCoords.x + 4.0) * (texCoords.y + 4.0) * (cbTime * 10.0);
    float grain = mod((mod(x, 13.0) + 1.0) * (mod(x, 123.0) + 1.0), 0.01) - 0.005;
    float4 grainAmount = float4(grain, grain, grain, grain) * fgStrength;
    
    grainAmount = 1.0 - grainAmount;
    return grainAmount.rgb * sourceColor;
}

float3 AddVignette(float3 sourceColor, float2 texCoords)
{
    float2 centerUV = texCoords - float2(0.5, 0.5);
    float3 color = float3(1.0, 1.0, 1.0);

    color.rgb *= 1.0 - smoothstep(vnInnerRadius, vnOuterRadius, length(centerUV));
    color *= sourceColor;
    color = lerp(sourceColor, color, vnOpacity);
    
    return color;
}

void LinearizeDepth(inout float depth)
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    depth = (2.0 * cbCameraNearZ * cbCameraFarZ) / (cbCameraFarZ + cbCameraNearZ - z * (cbCameraFarZ - cbCameraNearZ)) / cbCameraFarZ;
}

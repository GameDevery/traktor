#include "World/WorldHandles.h"

namespace traktor
{
    namespace world
    {

// Techniques
const render::Handle s_techniqueDeferredColor(L"World_DeferredColor");
const render::Handle s_techniqueDeferredGBufferWrite(L"World_DeferredGBufferWrite");
const render::Handle s_techniqueForwardColor(L"World_ForwardColor");
const render::Handle s_techniqueForwardGBufferWrite(L"World_ForwardGBufferWrite");
const render::Handle s_techniqueSimpleColor(L"World_SimpleColor");
const render::Handle s_techniqueReflectionWrite(L"World_ReflectionWrite");
const render::Handle s_techniqueIrradianceWrite(L"World_IrradianceWrite");
const render::Handle s_techniqueVelocityWrite(L"World_VelocityWrite");
const render::Handle s_techniqueShadow(L"World_ShadowWrite");

// Shader parameters.
const render::Handle s_handleColorMap(L"World_ColorMap");
const render::Handle s_handleDecalParams(L"World_DecalParams");
const render::Handle s_handleDepthEnable(L"World_DepthEnable");
const render::Handle s_handleDepthMap(L"World_DepthMap");
const render::Handle s_handleExposure(L"World_Exposure");
const render::Handle s_handleExtent(L"World_Extent");
const render::Handle s_handleFogColor(L"World_FogColor");
const render::Handle s_handleFogDistanceAndDensity(L"World_FogDistanceAndDensity");
const render::Handle s_handleFogEnable(L"World_FogEnable");
const render::Handle s_handleGamma(L"World_Gamma");
const render::Handle s_handleGammaInverse(L"World_GammaInverse");
const render::Handle s_handleIrradianceEnable(L"World_IrradianceEnable");
const render::Handle s_handleIrradianceGridBoundsMax(L"World_IrradianceGridBoundsMax");
const render::Handle s_handleIrradianceGridBoundsMin(L"World_IrradianceGridBoundsMin");
const render::Handle s_handleIrradianceGridSBuffer(L"World_IrradianceGridSBuffer");
const render::Handle s_handleIrradianceGridSize(L"World_IrradianceGridSize");
const render::Handle s_handleLastWorld(L"World_LastWorld");
const render::Handle s_handleLastWorldView(L"World_LastWorldView");
const render::Handle s_handleLightCount(L"World_LightCount");
const render::Handle s_handleLightDiffuseMap(L"World_LightDiffuseMap");
const render::Handle s_handleLightMap(L"World_LightMap");
const render::Handle s_handleLightSBuffer(L"World_LightSBuffer");
const render::Handle s_handleLightSpecularMap(L"World_LightSpecularMap");
const render::Handle s_handleMagicCoeffs(L"World_MagicCoeffs");
const render::Handle s_handleMiscMap(L"World_MiscMap");
const render::Handle s_handleNormalMap(L"World_NormalMap");
const render::Handle s_handleOcclusionMap(L"World_OcclusionMap");
const render::Handle s_handleProbeDiffuse(L"World_ProbeDiffuse");
const render::Handle s_handleProbeIntensity(L"World_ProbeIntensity");
const render::Handle s_handleProbeLocal(L"World_ProbeLocal");
const render::Handle s_handleProbeSpecular(L"World_ProbeSpecular");
const render::Handle s_handleProbeSpecularMips(L"World_ProbeSpecularMips");
const render::Handle s_handleProbeTexture(L"World_ProbeTexture");
const render::Handle s_handleProbeTextureMips(L"World_ProbeTextureMips");
const render::Handle s_handleProbeVolumeCenter(L"World_ProbeVolumeCenter");
const render::Handle s_handleProbeVolumeExtent(L"World_ProbeVolumeExtent");
const render::Handle s_handleProjection(L"World_Projection");
const render::Handle s_handleReflectionMap(L"World_ReflectionMap");
const render::Handle s_handleReflectionsEnable(L"World_ReflectionsEnable");
const render::Handle s_handleScreenMap(L"World_ScreenMap");
const render::Handle s_handleShadowEnable(L"World_ShadowEnable");
const render::Handle s_handleShadowMapAtlas(L"World_ShadowMapAtlas");
const render::Handle s_handleShadowMapCascade(L"World_ShadowMapCascade");
const render::Handle s_handleShadowMask(L"World_ShadowMask");
const render::Handle s_handleTileSBuffer(L"World_TileSBuffer");
const render::Handle s_handleTime(L"World_Time");
const render::Handle s_handleViewDistance(L"World_ViewDistance");
const render::Handle s_handleView(L"World_View");
const render::Handle s_handleViewInverse(L"World_ViewInverse");
const render::Handle s_handleWorld(L"World_World");
const render::Handle s_handleWorldView(L"World_WorldView");
const render::Handle s_handleWorldViewInv(L"World_WorldViewInv");
const render::Handle s_handleJitter(L"World_Jitter");

// ImageGraph inputs.
const render::Handle s_handleInputColor(L"InputColor");
const render::Handle s_handleInputColorLast(L"InputColorLast");
const render::Handle s_handleInputDepth(L"InputDepth");
const render::Handle s_handleInputNormal(L"InputNormal");
const render::Handle s_handleInputVelocity(L"InputVelocity");
const render::Handle s_handleInputShadowMap(L"InputShadowMap");
const render::Handle s_handleInputRoughness(L"InputRoughness");

    }
}

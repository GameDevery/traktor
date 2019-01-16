/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Shader.h"
#include "Render/Types.h"
#include "Render/Context/ProgramParameters.h"
#include "World/WorldRenderView.h"
#include "World/Deferred/WorldRenderPassDeferred.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

enum { MaxForwardLightCount = 2 };

bool s_handlesInitialized = false;
render::handle_t s_techniqueDeferredColor;
render::handle_t s_techniqueVelocityWrite;
render::handle_t s_handleWorld;
render::handle_t s_handleWorldView;
render::handle_t s_handleLastWorld;
render::handle_t s_handleLastWorldView;
render::handle_t s_handleFogEnable;
render::handle_t s_handleDepthEnable;
render::handle_t s_handleLightPositionAndType;
render::handle_t s_handleLightDirectionAndRange;
render::handle_t s_handleLightColor;

void initializeHandles()
{
	if (s_handlesInitialized)
		return;

	s_techniqueDeferredColor = render::getParameterHandle(L"World_DeferredColor");
	s_techniqueVelocityWrite = render::getParameterHandle(L"World_VelocityWrite");

	s_handleWorld = render::getParameterHandle(L"World_World");
	s_handleWorldView = render::getParameterHandle(L"World_WorldView");
	s_handleLastWorld = render::getParameterHandle(L"World_LastWorld");
	s_handleLastWorldView = render::getParameterHandle(L"World_LastWorldView");
	s_handleFogEnable = render::getParameterHandle(L"World_FogEnable");
	s_handleDepthEnable = render::getParameterHandle(L"World_DepthEnable");
	s_handleLightPositionAndType = render::getParameterHandle(L"World_LightPositionAndType");
	s_handleLightDirectionAndRange = render::getParameterHandle(L"World_LightDirectionAndRange");
	s_handleLightColor = render::getParameterHandle(L"World_LightColor");

	s_handlesInitialized = true;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldRenderPassDeferred", WorldRenderPassDeferred, IWorldRenderPass)

WorldRenderPassDeferred::WorldRenderPassDeferred(
	render::handle_t technique,
	const WorldRenderView& worldRenderView,
	uint32_t passFlags,
	bool fogEnabled,
	bool depthEnable
)
:	m_technique(technique)
,	m_worldRenderView(worldRenderView)
,	m_passFlags(passFlags)
,	m_fogEnabled(fogEnabled)
,	m_depthEnable(depthEnable)
{
	initializeHandles();
}

WorldRenderPassDeferred::WorldRenderPassDeferred(
	render::handle_t technique,
	const WorldRenderView& worldRenderView,
	uint32_t passFlags
)
:	m_technique(technique)
,	m_worldRenderView(worldRenderView)
,	m_passFlags(passFlags)
,	m_fogEnabled(false)
,	m_depthEnable(false)
{
	initializeHandles();
}

render::handle_t WorldRenderPassDeferred::getTechnique() const
{
	return m_technique;
}

uint32_t WorldRenderPassDeferred::getPassFlags() const
{
	return m_passFlags;
}

void WorldRenderPassDeferred::setShaderTechnique(render::Shader* shader) const
{
	shader->setTechnique(m_technique);
}

void WorldRenderPassDeferred::setShaderCombination(render::Shader* shader) const
{
	if (m_technique == s_techniqueDeferredColor)
	{
		shader->setCombination(s_handleFogEnable, m_fogEnabled);
		shader->setCombination(s_handleDepthEnable, m_depthEnable);
	}
}

void WorldRenderPassDeferred::setShaderCombination(render::Shader* shader, const Transform& world, const Aabb3& bounds) const
{
	if (m_technique == s_techniqueDeferredColor)
	{
		shader->setCombination(s_handleFogEnable, m_fogEnabled);
		shader->setCombination(s_handleDepthEnable, m_depthEnable);
	}
}

void WorldRenderPassDeferred::setProgramParameters(render::ProgramParameters* programParams) const
{
	setWorldProgramParameters(programParams, Transform::identity(), Transform::identity());
	if (m_technique == s_techniqueDeferredColor)
		setLightProgramParameters(programParams);
}

void WorldRenderPassDeferred::setProgramParameters(render::ProgramParameters* programParams, const Transform& lastWorld, const Transform& world, const Aabb3& bounds) const
{
	setWorldProgramParameters(programParams, lastWorld, world);
	if (m_technique == s_techniqueDeferredColor)
		setLightProgramParameters(programParams);
}

void WorldRenderPassDeferred::setWorldProgramParameters(render::ProgramParameters* programParams, const Transform& lastWorld, const Transform& world) const
{
	Matrix44 w = world.toMatrix44();
	programParams->setMatrixParameter(s_handleWorld, w);
	programParams->setMatrixParameter(s_handleWorldView, m_worldRenderView.getView() * w);

	if (m_technique == s_techniqueVelocityWrite)
	{
		Matrix44 w0 = lastWorld.toMatrix44();
		programParams->setMatrixParameter(s_handleLastWorld, w0);
		programParams->setMatrixParameter(s_handleLastWorldView, m_worldRenderView.getLastView() * w0);
	}
}

void WorldRenderPassDeferred::setLightProgramParameters(render::ProgramParameters* programParams) const
{
	const Matrix44& view = m_worldRenderView.getView();

	// Pack light parameters.
	Vector4 lightPositionAndType[MaxForwardLightCount], *lightPositionAndTypePtr = lightPositionAndType;
	Vector4 lightDirectionAndRange[MaxForwardLightCount], *lightDirectionAndRangePtr = lightDirectionAndRange;
	Vector4 lightColor[MaxForwardLightCount], *lightColorPtr = lightColor;

	int lightCount = std::min< int >(m_worldRenderView.getLightCount(), MaxForwardLightCount);
	for (int i = 0; i < lightCount; ++i)
	{
		const Light& light = m_worldRenderView.getLight(i);
		*lightPositionAndTypePtr++ = (view * light.position).xyz0() + Vector4(0.0f, 0.0f, 0.0f, float(light.type));
		*lightDirectionAndRangePtr++ = (view * light.direction).xyz0() + Vector4(0.0f, 0.0f, 0.0f, light.range);
		*lightColorPtr++ = light.color;
	}

	// Disable excessive lights.
	for (int i = lightCount; i < MaxForwardLightCount; ++i)
	{
		const static Vector4 c_typeDisabled(0.0f, 0.0f, 0.0f, float(LtDisabled));
		*lightPositionAndTypePtr++ = c_typeDisabled;
		*lightDirectionAndRangePtr++ = Vector4::zero();
		*lightColorPtr++ = Vector4::zero();
	}

	// Finally set shader parameters.
	programParams->setVectorArrayParameter(s_handleLightPositionAndType, lightPositionAndType, MaxForwardLightCount);
	programParams->setVectorArrayParameter(s_handleLightDirectionAndRange, lightDirectionAndRange, MaxForwardLightCount);
	programParams->setVectorArrayParameter(s_handleLightColor, lightColor, MaxForwardLightCount);
}

	}
}

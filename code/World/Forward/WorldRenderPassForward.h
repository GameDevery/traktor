/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_world_WorldRenderPassForward_H
#define traktor_world_WorldRenderPassForward_H

#include "World/IWorldRenderPass.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ISimpleTexture;

	}

	namespace world
	{

class WorldRenderView;

/*! \brief World render pass.
 * \ingroup World
 */
class T_DLLCLASS WorldRenderPassForward : public IWorldRenderPass
{
	T_RTTI_CLASS;
	
public:
	WorldRenderPassForward(
		render::handle_t technique,
		const WorldRenderView& worldRenderView,
		uint32_t passFlags,
		bool fogEnabled,
		float fogDistanceY,
		float fogDistanceZ,
		float fogDensityY,
		float fogDensityZ,
		const Vector4& fogColor,
		render::ISimpleTexture* colorMap,
		render::ISimpleTexture* depthMap,
		render::ISimpleTexture* shadowMask
	);

	WorldRenderPassForward(
		render::handle_t technique,
		const WorldRenderView& worldRenderView,
		uint32_t passFlags,
		render::ISimpleTexture* colorMap,
		render::ISimpleTexture* depthMap
	);

	virtual render::handle_t getTechnique() const override final;

	virtual uint32_t getPassFlags() const override final;

	virtual void setShaderTechnique(render::Shader* shader) const override final;

	virtual void setShaderCombination(render::Shader* shader) const override final;

	virtual void setShaderCombination(render::Shader* shader, const Transform& world, const Aabb3& bounds) const override final;

	virtual void setProgramParameters(render::ProgramParameters* programParams) const override final;

	virtual void setProgramParameters(render::ProgramParameters* programParams, const Transform& lastWorld, const Transform& world, const Aabb3& bounds) const override final;

private:
	render::handle_t m_technique;
	const WorldRenderView& m_worldRenderView;
	Matrix44 m_viewInverse;
	uint32_t m_passFlags;
	bool m_fogEnabled;
	float m_fogDistanceY;
	float m_fogDistanceZ;
	float m_fogDensityY;
	float m_fogDensityZ;
	Vector4 m_fogColor;
	render::ISimpleTexture* m_colorMap;
	render::ISimpleTexture* m_depthMap;
	render::ISimpleTexture* m_shadowMask;

	void setWorldProgramParameters(render::ProgramParameters* programParams, const Transform& world) const;

	void setLightProgramParameters(render::ProgramParameters* programParams) const;

	void setLightProgramParameters(render::ProgramParameters* programParams, const Transform& world, const Aabb3& bounds) const;

	void setFogProgramParameters(render::ProgramParameters* programParams) const;

	void setColorMapProgramParameters(render::ProgramParameters* programParams) const;

	void setShadowMapProgramParameters(render::ProgramParameters* programParams) const;

	void setDepthMapProgramParameters(render::ProgramParameters* programParams) const;
};
	
	}
}

#endif	// traktor_world_WorldRenderPassForward_H

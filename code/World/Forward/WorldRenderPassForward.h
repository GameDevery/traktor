#pragma once

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
	namespace world
	{

class WorldRenderView;

/*! World render pass.
 * \ingroup World
 */
class T_DLLCLASS WorldRenderPassForward : public IWorldRenderPass
{
	T_RTTI_CLASS;

public:
	WorldRenderPassForward() = delete;

	WorldRenderPassForward(
		render::handle_t technique,
		render::ProgramParameters* sharedParams,
		const WorldRenderView& worldRenderView,
		uint32_t passFlags,
		bool irradianceEnable,
		bool fogEnable,
		bool shadowEnable,
		bool reflectionsEnable
	);

	WorldRenderPassForward(
		render::handle_t technique,
		render::ProgramParameters* sharedParams,
		const WorldRenderView& worldRenderView,
		uint32_t passFlags
	);

	virtual render::handle_t getTechnique() const override final;

	virtual uint32_t getPassFlags() const override final;

	virtual render::Shader::Permutation getPermutation(const render::Shader* shader) const override final;

	virtual void setProgramParameters(render::ProgramParameters* programParams) const override final;

	virtual void setProgramParameters(render::ProgramParameters* programParams, const Transform& lastWorld, const Transform& world, const Aabb3& bounds) const override final;

private:
	render::handle_t m_technique;
	render::ProgramParameters* m_sharedParams;
	const WorldRenderView& m_worldRenderView;
	uint32_t m_passFlags = 0;
	bool m_irradianceEnable = false;
	bool m_fogEnable = false;
	bool m_shadowEnable = false;
	bool m_reflectionsEnable = false;

	void setWorldProgramParameters(render::ProgramParameters* programParams, const Transform& lastWorld, const Transform& world) const;
};

	}
}

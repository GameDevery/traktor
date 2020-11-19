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
class T_DLLCLASS WorldRenderPassDeferred : public IWorldRenderPass
{
	T_RTTI_CLASS;

public:
	WorldRenderPassDeferred() = delete;

	WorldRenderPassDeferred(
		render::handle_t technique,
		render::ProgramParameters* sharedParams,
		const WorldRenderView& worldRenderView,
		uint32_t passFlags,
		bool fogEnabled,
		bool depthEnable,
		bool irradianceEnable
	);

	virtual render::handle_t getTechnique() const override final;

	virtual uint32_t getPassFlags() const override final;

	virtual render::Shader::Permutation getPermutation(const render::Shader* shader) const override final;

	virtual void setProgramParameters(render::ProgramParameters* programParams) const override final;

	virtual void setProgramParameters(render::ProgramParameters* programParams, const Transform& lastWorld, const Transform& world) const override final;

private:
	render::handle_t m_technique;
	render::ProgramParameters* m_sharedParams;
	const WorldRenderView& m_worldRenderView;
	uint32_t m_passFlags;
	bool m_fogEnabled;
	bool m_depthEnable;
	bool m_irradianceEnable;

	void setWorldProgramParameters(render::ProgramParameters* programParams, const Transform& lastWorld, const Transform& world) const;
};

	}
}


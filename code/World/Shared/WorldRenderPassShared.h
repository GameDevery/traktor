/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <initializer_list>
#include "World/IWorldRenderPass.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class WorldRenderView;

/*! World render pass.
 * \ingroup World
 */
class T_DLLCLASS WorldRenderPassShared : public IWorldRenderPass
{
	T_RTTI_CLASS;

public:
	struct TechniqueFlag
	{
		render::handle_t handle;
		bool enable;
	};

	WorldRenderPassShared() = delete;

	explicit WorldRenderPassShared(
		render::handle_t technique,
		render::ProgramParameters* sharedParams,
		const WorldRenderView& worldRenderView,
		uint32_t passFlags
	);

	explicit WorldRenderPassShared(
		render::handle_t technique,
		render::ProgramParameters* sharedParams,
		const WorldRenderView& worldRenderView,
		uint32_t passFlags,
		const std::initializer_list< TechniqueFlag >& techniqueFlags
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
	std::initializer_list< TechniqueFlag > m_techniqueFlags;

	void setWorldProgramParameters(render::ProgramParameters* programParams, const Transform& lastWorld, const Transform& world) const;
};

}

/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Mesh/MeshComponent.h"
#include "Resource/Proxy.h"
#include "World/Entity/RTWorldComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::mesh
{

class StaticMesh;

/*! Static mesh component.
 * \ingroup Mesh
 */
class T_DLLCLASS StaticMeshComponent : public MeshComponent
{
	T_RTTI_CLASS;

public:
	explicit StaticMeshComponent(const resource::Proxy< StaticMesh >& mesh);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setWorld(world::World* world) override final;

	virtual void setState(const world::EntityState& state, const world::EntityState& mask, bool includeChildren) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass) override final;

private:
	resource::Proxy< StaticMesh > m_mesh;
	world::World* m_world = nullptr;
	world::RTWorldComponent::Instance* m_rtwInstance = nullptr;
	Transform m_lastTransform; //!< Last rendered transform.
};

}

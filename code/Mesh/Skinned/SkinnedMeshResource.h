/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Guid.h"
#include "Mesh/MeshResource.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class ITexture;
class Shader;

}

namespace traktor::mesh
{

/*! Skinned mesh persistent data.
 * \ingroup Mesh
 */
class T_DLLCLASS SkinnedMeshResource : public MeshResource
{
	T_RTTI_CLASS;

public:
	struct T_DLLCLASS Part
	{
		std::wstring shaderTechnique;
		uint32_t meshPart = 0;

		void serialize(ISerializer& s);
	};

	virtual Ref< IMesh > createMesh(
		const std::wstring& name,
		IStream* dataStream,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		render::MeshFactory* meshFactory) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class SkinnedMeshConverter;
	typedef AlignedVector< Part > parts_t;

	resource::Id< render::Shader > m_shader;
	AlignedVector< resource::Id< render::ITexture > > m_albedoTextures;
	SmallMap< std::wstring, parts_t > m_parts;
	SmallMap< std::wstring, int32_t > m_jointMap;
};

}

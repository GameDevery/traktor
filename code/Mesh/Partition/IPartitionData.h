#pragma once

#include "Core/Ref.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

class IPartition;

class T_DLLCLASS IPartitionData : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual Ref< IPartition > createPartition() const = 0;
};

	}
}


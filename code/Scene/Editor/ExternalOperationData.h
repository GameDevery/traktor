#pragma once

#include "Core/Guid.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace scene
	{

class T_DLLCLASS ExternalOperationData : public ISerializable
{
	T_RTTI_CLASS;

public:
	const Guid& getExternalDataId() const { return m_externalDataId; }

	virtual void serialize(ISerializer& s) override final;

private:
	Guid m_externalDataId;
};

	}
}

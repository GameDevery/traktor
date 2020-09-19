#pragma once

#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IRuntimeClass;

	namespace resource
	{

class IResourceManager;

	}

	namespace world
	{

class ScriptComponent;

/*! Script entity component data.
 * \ingroup World
 */
class T_DLLCLASS ScriptComponentData : public IEntityComponentData
{
	T_RTTI_CLASS;

public:
	ScriptComponentData() = default;

	explicit ScriptComponentData(const resource::Id< IRuntimeClass >& _class);

	Ref< ScriptComponent > createComponent(resource::IResourceManager* resourceManager) const;

	virtual void setTransform(const EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	const resource::Id< IRuntimeClass >& getRuntimeClass() const { return m_class;  }

	bool getEditorSupport() const { return m_editorSupport; }

private:
	resource::Id< IRuntimeClass > m_class;
	bool m_editorSupport = false;
};

	}
}


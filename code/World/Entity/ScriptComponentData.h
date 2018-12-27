/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_world_ScriptComponentData_H
#define traktor_world_ScriptComponentData_H

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

/*! \brief Script entity component data.
 * \ingroup World
 */
class T_DLLCLASS ScriptComponentData : public IEntityComponentData
{
	T_RTTI_CLASS;

public:
	ScriptComponentData();

	explicit ScriptComponentData(const resource::Id< IRuntimeClass >& _class);

	Ref< ScriptComponent > createComponent(resource::IResourceManager* resourceManager) const;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class EntityComponentPipeline;

	resource::Id< IRuntimeClass > m_class;
};

	}
}

#endif	// traktor_world_ScriptComponentData_H

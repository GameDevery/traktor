#ifndef traktor_spray_SpawnEffectEventData_H
#define traktor_spray_SpawnEffectEventData_H

#include "World/IEntityEventData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class EntityData;

	}

	namespace spray
	{

/*! \brief
 * \ingroup Spray
 */
class T_DLLCLASS SpawnEffectEventData : public world::IEntityEventData
{
	T_RTTI_CLASS;

public:
	SpawnEffectEventData();

	virtual Ref< world::IEntityEvent > create(const world::IEntityBuilder* entityBuilder) const;

	virtual bool serialize(ISerializer& s);

private:
	Ref< world::EntityData > m_effectData;
	bool m_follow;
};

	}
}

#endif	// traktor_spray_SpawnEffectEventData_H

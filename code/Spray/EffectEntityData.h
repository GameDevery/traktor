#ifndef traktor_spray_EffectEntityData_H
#define traktor_spray_EffectEntityData_H

#include "Resource/Id.h"
#include "World/Entity/EntityData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace sound
	{

class ISoundPlayer;

	}

	namespace spray
	{

class Effect;
class EffectEntity;

/*! \brief Effect entity data.
 * \ingroup Spray
 */
class T_DLLCLASS EffectEntityData : public world::EntityData
{
	T_RTTI_CLASS;

public:
	Ref< EffectEntity > createEntity(resource::IResourceManager* resourceManager, sound::ISoundPlayer* soundPlayer) const;

	virtual bool serialize(ISerializer& s);

	const resource::Id< Effect >& getEffect() const { return m_effect; }

private:
	resource::Id< Effect > m_effect;
};

	}
}

#endif	// traktor_spray_EffectEntityData_H

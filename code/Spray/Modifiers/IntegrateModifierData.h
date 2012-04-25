#ifndef traktor_spray_IntegrateModifierData_H
#define traktor_spray_IntegrateModifierData_H

#include "Spray/ModifierData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spray
	{

/*! \brief Integrate modifier persistent data.
 * \ingroup Spray
 */
class T_DLLCLASS IntegrateModifierData : public ModifierData
{
	T_RTTI_CLASS;

public:
	IntegrateModifierData();

	virtual Ref< Modifier > createModifier(resource::IResourceManager* resourceManager) const;

	virtual bool serialize(ISerializer& s);

private:
	float m_timeScale;
};

	}
}

#endif	// traktor_spray_IntegrateModifierData_H

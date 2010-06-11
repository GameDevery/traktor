#ifndef traktor_animation_StatePoseControllerData_H
#define traktor_animation_StatePoseControllerData_H

#include "Core/Math/Range.h"
#include "Animation/IPoseControllerData.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

class StateGraph;

/*! \brief Animation evaluation controller data.
 * \ingroup Animation
 */
class T_DLLCLASS StatePoseControllerData : public IPoseControllerData
{
	T_RTTI_CLASS;

public:
	StatePoseControllerData();

	virtual Ref< IPoseController > createInstance(
		resource::IResourceManager* resourceManager,
		physics::PhysicsManager* physicsManager,
		const Skeleton* skeleton,
		const Transform& worldTransform
	);

	virtual bool serialize(ISerializer& s);

	inline const resource::Proxy< StateGraph >& getStateGraph() const { return m_stateGraph; }

private:
	resource::Proxy< StateGraph > m_stateGraph;
	Range< float > m_randomTimeOffset;
};

	}
}

#endif	// traktor_animation_StatePoseControllerData_H

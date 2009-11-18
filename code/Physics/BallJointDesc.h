#ifndef traktor_physics_BallJointDesc_H
#define traktor_physics_BallJointDesc_H

#include "Physics/JointDesc.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*! \brief Ball joint description.
 * \ingroup Physics
 */
class T_DLLCLASS BallJointDesc : public JointDesc
{
	T_RTTI_CLASS;

public:
	BallJointDesc();

	void setAnchor(const Vector4& anchor);

	const Vector4& getAnchor() const;

	virtual bool serialize(ISerializer& s);

private:
	Vector4 m_anchor;
};

	}
}

#endif	// traktor_physics_BallJointDesc_H

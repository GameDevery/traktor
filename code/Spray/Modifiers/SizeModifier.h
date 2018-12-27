/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spray_SizeModifier_H
#define traktor_spray_SizeModifier_H

#include "Spray/Modifier.h"

namespace traktor
{
	namespace spray
	{

/*! \brief Particle size modifier.
 * \ingroup Spray
 */
class SizeModifier : public Modifier
{
	T_RTTI_CLASS;

public:
	SizeModifier(float adjustRate);

#if defined(T_MODIFIER_USE_PS3_SPURS)
	virtual void update(SpursJobQueue* jobQueue, const Scalar& deltaTime, const Transform& transform, PointVector& points) const override final;
#else
	virtual void update(const Scalar& deltaTime, const Transform& transform, PointVector& points, size_t first, size_t last) const override final;
#endif

private:
	float m_adjustRate;
};

	}
}

#endif	// traktor_spray_SizeModifier_H

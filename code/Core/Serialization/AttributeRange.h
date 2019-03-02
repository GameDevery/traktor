#pragma once

#include <limits>
#include "Core/Serialization/Attribute.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Member numeric range attribute.
 * \ingroup Core
 */
class T_DLLCLASS AttributeRange : public Attribute
{
	T_RTTI_CLASS;

public:
	AttributeRange(
		float min = std::numeric_limits< float >::min(),
		float max = std::numeric_limits< float >::max()
	);

	float getMin() const;

	float getMax() const;

private:
	float m_min;
	float m_max;
};

}


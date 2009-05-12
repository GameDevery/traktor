#ifndef traktor_render_Sampler_H
#define traktor_render_Sampler_H

#include "Core/Object.h"
#include "Core/Math/Vector4.h"

namespace traktor
{
	namespace render
	{

/*! \brief Texture sampler interface.
 * \ingroup SW
 */
class AbstractSampler : public Object
{
public:
	virtual Vector4 get(const Vector4& texCoord) const = 0;
};

	}
}

#endif	// traktor_render_Sampler_H

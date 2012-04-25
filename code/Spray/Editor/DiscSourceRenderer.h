#ifndef traktor_spray_DiscSourceRenderer_H
#define traktor_spray_DiscSourceRenderer_H

#include "Spray/Editor/SourceRenderer.h"

namespace traktor
{
	namespace spray
	{

class DiscSourceRenderer : public SourceRenderer
{
	T_RTTI_CLASS;

public:
	virtual void render(render::PrimitiveRenderer* primitiveRenderer, const Source* source) const;
};

	}
}

#endif	// traktor_spray_DiscSourceRenderer_H

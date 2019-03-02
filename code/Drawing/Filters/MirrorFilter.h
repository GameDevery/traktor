#pragma once

#include "Drawing/IImageFilter.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace drawing
	{

/*! \brief Mirror image filter.
 * \ingroup Drawing
 *
 * Flip image horizontally and/or vertically.
 */
class T_DLLCLASS MirrorFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	MirrorFilter(bool flipHorizontal, bool flipVertical);

protected:
	virtual void apply(Image* image) const override final;

private:
	bool m_flipHorizontal;
	bool m_flipVertical;
};

	}
}


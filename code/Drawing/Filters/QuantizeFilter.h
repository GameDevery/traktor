/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_drawing_QuantizeFilter_H
#define traktor_drawing_QuantizeFilter_H

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
	
/*! \brief Quantize filter.
 * \ingroup Drawing
 */
class T_DLLCLASS QuantizeFilter : public IImageFilter
{
	T_RTTI_CLASS;

public:
	QuantizeFilter(int steps);

protected:
	virtual void apply(Image* image) const override final;

private:
	int m_steps;
};
	
	}
}

#endif	// traktor_drawing_QuantizeFilter_H

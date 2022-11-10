/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Containers/AlignedVector.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/ConvolutionFilter.h"
#include "Drawing/Filters/SharpenFilter.h"

namespace traktor::drawing
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.SharpenFilter", SharpenFilter, IImageFilter)

SharpenFilter::SharpenFilter(int radius, float strength)
:	m_radius(radius)
,	m_strength(strength)
{
}

void SharpenFilter::apply(Image* image) const
{
	Ref< Image > unsharpenMask = image->clone();

	switch (m_radius)
	{
	case 3:
		unsharpenMask->apply(ConvolutionFilter::createGaussianBlur3());
		break;

	case 5:
		unsharpenMask->apply(ConvolutionFilter::createGaussianBlur5());
		break;

	default:
		unsharpenMask->apply(ConvolutionFilter::createGaussianBlur(m_radius));
		break;
	}

	AlignedVector< Color4f > span(image->getWidth());
	AlignedVector< Color4f > spanMask(image->getWidth());

	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		image->getSpanUnsafe(y, span.ptr());
		unsharpenMask->getSpanUnsafe(y, spanMask.ptr());

		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			Color4f& in = span[x];
			Scalar alpha = in.getAlpha();
			in = in + in - spanMask[x];
			in.setAlpha(alpha);
		}

		image->setSpanUnsafe(y, span.c_ptr());
	}
}

}

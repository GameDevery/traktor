#include "Drawing/Filters/GrayscaleFilter.h"
#include "Drawing/Image.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.GrayScaleFilter", GrayscaleFilter, IImageFilter)

Ref< Image > GrayscaleFilter::apply(const Image* image)
{
	Ref< Image > final = new Image(image->getPixelFormat(), image->getWidth(), image->getHeight(), image->getPalette());
	Color in;
	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			image->getPixelUnsafe(x, y, in);
			float luminance = 0.2126f * in.getRed() + 0.7152f * in.getGreen() + 0.0722f * in.getBlue();
			Color out(luminance, luminance, luminance, in.getAlpha());
			final->setPixelUnsafe(x, y, out);
		}
	}
	return final;
}

	}
}

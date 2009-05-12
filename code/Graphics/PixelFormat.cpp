#include "Graphics/PixelFormat.h"

namespace traktor
{
	namespace graphics
	{

int getByteSize(PixelFormatEnum pixelFormat)
{
	const int c_byteSize[] = { 0, 2, 2, 2, 3, 4 };
	return c_byteSize[int(pixelFormat)];
}

int getColorBits(PixelFormatEnum pixelFormat)
{
	const int c_colorBits[] = { 0, 15, 15, 16, 24, 32 };
	return c_colorBits[int(pixelFormat)];
}

int getAlphaBits(PixelFormatEnum pixelFormat)
{
	const int c_alphaBits[] = { 0, 0, 1, 0, 0, 8 };
	return c_alphaBits[int(pixelFormat)];
}

bool hasAlpha(PixelFormatEnum pixelFormat)
{
	const bool c_hasAlpha[] = { false, false, true, false, false, true };
	return c_hasAlpha[int(pixelFormat)];
}

void T_DLLCLASS convertPixel(
	PixelFormatEnum targetPixelFormat,
	void* targetPixel,
	PixelFormatEnum sourcePixelFormat,
	const void* sourcePixel
)
{
	const uint8_t* sp = static_cast< const uint8_t* >(sourcePixel);
	uint8_t* tp = static_cast< uint8_t* >(targetPixel);
	uint8_t rgb[3] = { 0, 0, 0 };

	switch (sourcePixelFormat)
	{
	case PfeR5G5B5:
		break;

	case PfeA1R5G5B5:
		break;

	case PfeR5G6B5:
		break;

	case PfeR8G8B8:
	case PfeA8R8G8B8:
		rgb[0] = sp[2];
		rgb[1] = sp[1];
		rgb[2] = sp[0];
		break;
	}

	switch (targetPixelFormat)
	{
	case PfeR5G5B5:
		break;

	case PfeA1R5G5B5:
		break;

	case PfeR5G6B5:
		break;

	case PfeR8G8B8:
	case PfeA8R8G8B8:
		tp[2] = rgb[0];
		tp[1] = rgb[1];
		tp[0] = rgb[2];
		break;
	}
}

	}
}

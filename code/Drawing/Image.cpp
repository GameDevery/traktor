/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Core/Io/BufferedStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Drawing/Image.h"
#include "Drawing/Palette.h"
#include "Drawing/IImageFormat.h"
#include "Drawing/IImageFilter.h"
#include "Drawing/ITransferFunction.h"

namespace traktor::drawing
{
	namespace
	{

#if defined(DRAWING_CHECK_DATA)
const uint32_t c_wallSize = 64;
#else
const uint32_t c_wallSize = 0;
#endif

void checkData(uint8_t* ptr, size_t size)
{
#if defined(DRAWING_CHECK_DATA)
	if (!ptr || !size)
		return;

	uint8_t* wp = ptr - c_wallSize;
	for (uint32_t i = 0; i < c_wallSize; ++i)
	{
		if (wp[i] != 0xcc)
			T_FATAL_ERROR;
		if (wp[c_wallSize + size + i] != 0x55)
			T_FATAL_ERROR;
	}
#endif
}

uint8_t* allocData(size_t size)
{
	uint8_t* ptr = (uint8_t*)getAllocator()->alloc(size + c_wallSize * 2, 16, T_FILE_LINE);
	if (!ptr)
		T_FATAL_ERROR;
#if defined(DRAWING_CHECK_DATA)
	for (uint32_t i = 0; i < c_wallSize; ++i)
	{
		ptr[i] = 0xcc;
		ptr[c_wallSize + size + i] = 0x55;
	}
#endif
	return ptr + c_wallSize;
}

void freeData(uint8_t* ptr, size_t size)
{
	if (ptr)
	{
		checkData(ptr, size);
		ptr -= c_wallSize;
		getAllocator()->free(ptr);
	}
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.Image", Image, Object)

Image::Image(const Image& src)
:	m_pixelFormat(src.m_pixelFormat)
,	m_width(src.m_width)
,	m_height(src.m_height)
,	m_pitch(src.m_pitch)
,	m_palette(src.m_palette)
,	m_own(true)
,	m_imageInfo(src.m_imageInfo)
{
	m_size = src.m_size;
	m_data = allocData(m_size);
	std::memcpy(m_data, src.m_data, m_size);
}

Image::Image(const PixelFormat& pixelFormat, uint32_t width, uint32_t height, Palette* palette)
:	m_pixelFormat(pixelFormat)
,	m_width(width)
,	m_height(height)
,	m_pitch(width * pixelFormat.getByteSize())
,	m_palette(palette)
,	m_own(true)
{
	m_size = m_height * m_pitch;
	m_data = allocData(m_size);
}

Image::Image(void* data, const PixelFormat& pixelFormat, uint32_t width, uint32_t height, Palette* palette)
:	m_pixelFormat(pixelFormat)
,	m_width(width)
,	m_height(height)
,	m_pitch(width * pixelFormat.getByteSize())
,	m_palette(palette)
,	m_data((uint8_t*)data)
{
	m_size = m_height * m_pitch;
}

Image::~Image()
{
	if (m_own)
		freeData(m_data, m_size);
}

Ref< Image > Image::clone(bool includeData) const
{
	Ref< Image > clone = new Image(m_pixelFormat, m_width, m_height, m_palette);
	if (includeData)
	{
		std::memcpy(clone->m_data, m_data, m_height * m_pitch);
		checkData(clone->m_data, clone->m_size);
	}
	return clone;
}

void Image::copy(const Image* src, int32_t x, int32_t y, int32_t width, int32_t height)
{
	copy(src, 0, 0, x, y, width, height);
}

void Image::copy(const Image* src, int32_t dx, int32_t dy, int32_t x, int32_t y, int32_t width, int32_t height)
{
	int32_t srcWidth = int32_t(src->getWidth());
	int32_t srcHeight = int32_t(src->getHeight());

	if (x < 0)
	{
		dx += -x;
		width -= -x;
		x = 0;
	}
	if (y < 0)
	{
		dy += -y;
		height -= y;
		y = 0;
	}
	if (dx < 0)
	{
		x += -dx;
		width -= -dx;
		dx = 0;
	}
	if (dy < 0)
	{
		y += -dy;
		height -= -dy;
		dy = 0;
	}

	if (x >= srcWidth || y >= srcHeight)
		return;

	if (x + width > srcWidth)
		width = srcWidth - x;
	if (y + height > srcHeight)
		height = srcHeight - y;

	const int32_t mx = m_width - dx;
	const int32_t my = m_height - dy;

	if (width > mx)
		width = mx;
	if (height > my)
		height = my;

	if (width <= 0 || height <= 0)
		return;

	T_ASSERT(x >= 0 && y >= 0);
	T_ASSERT(width >= 0 && height >= 0);

	const PixelFormat& pf = src->m_pixelFormat;
	for (int32_t yy = 0; yy < height; ++yy)
	{
		const uint8_t* sd = &src->m_data[x * pf.getByteSize() + (y + yy) * src->m_pitch];
		uint8_t* dd = &m_data[dx * m_pixelFormat.getByteSize() + (dy + yy) * m_pitch];
		pf.convert(0, sd, m_pixelFormat, 0, dd, width);
	}

	checkData(m_data, m_size);
}

void Image::copy(const Image* src, int32_t x, int32_t y, int32_t width, int32_t height, const ITransferFunction& tf)
{
	copy(src, 0, 0, x, y, width, height, tf);
}

void Image::copy(const Image* src, int32_t dx, int32_t dy, int32_t x, int32_t y, int32_t width, int32_t height, const ITransferFunction& tf)
{
	const int32_t srcWidth = int32_t(src->getWidth());
	const int32_t srcHeight = int32_t(src->getHeight());
	Color4f sp, dp;

	if (x < 0)
	{
		dx += -x;
		width -= -x;
		x = 0;
	}
	if (y < 0)
	{
		dy += -y;
		height -= y;
		y = 0;
	}
	if (dx < 0)
	{
		x += -dx;
		width -= -dx;
		dx = 0;
	}
	if (dy < 0)
	{
		y += -dy;
		height -= -dy;
		dy = 0;
	}

	if (x >= srcWidth || y >= srcHeight)
		return;

	if (x + width > srcWidth)
		width = srcWidth - x;
	if (y + height > srcHeight)
		height = srcHeight - y;

	const int32_t mx = m_width - dx;
	const int32_t my = m_height - dy;

	if (width > mx)
		width = mx;
	if (height > my)
		height = my;

	if (width <= 0 || height <= 0)
		return;

	T_ASSERT(x >= 0 && y >= 0);
	T_ASSERT(width >= 0 && height >= 0);

	const PixelFormat& pf = src->m_pixelFormat;
	for (int32_t yy = 0; yy < height; ++yy)
	{
		for (int32_t xx = 0; xx < width; ++xx)
		{
			const uint8_t* sd = &src->m_data[(x + xx) * pf.getByteSize() + (y + yy) * src->m_pitch];
			uint8_t* dd = &m_data[(dx + xx) * m_pixelFormat.getByteSize() + (dy + yy) * m_pitch];
			pf.convertTo4f(0, sd, &sp, pf.getByteSize(), 1);
			pf.convertTo4f(0, dd, &dp, m_pixelFormat.getByteSize(), 1);
			tf.evaluate(sp, dp);
			pf.convertFrom4f(&dp, 0, dd, m_pixelFormat.getByteSize(), 1);
		}
	}

	checkData(m_data, m_size);
}

void Image::clear(const Color4f& color)
{
	float T_MATH_ALIGN16 tmp[4];
	color.storeAligned(tmp);

	const uint32_t byteSize = m_pixelFormat.getByteSize();
	AlignedVector< uint8_t > c(byteSize);

	PixelFormat::getRGBAF32().convert(
		0,
		tmp,
		m_pixelFormat,
		m_palette,
		&c[0],
		1
	);

	for (int32_t i = 0; i < m_height * m_pitch; i += byteSize)
		std::memcpy(&m_data[i], &c[0], byteSize);

	checkData(m_data, m_size);
}

void Image::clearAlpha(float alpha)
{
	const Scalar sa(alpha);
	Color4f cl;

	for (int32_t y = 0; y < m_height; ++y)
	{
		for (int32_t x = 0; x < m_width; ++x)
		{
			getPixelUnsafe(x, y, cl);
			cl.setAlpha(sa);
			setPixelUnsafe(x, y, cl);
		}
	}
}

bool Image::getPixel(int32_t x, int32_t y, Color4f& outColor) const
{
	if (x < 0  || x >= m_width || y < 0 || y >= m_height)
		return false;

	getPixelUnsafe(x, y, outColor);
	return true;
}

bool Image::setPixel(int32_t x, int32_t y, const Color4f& color)
{
	if (x < 0  || x >= m_width || y < 0 || y >= m_height)
		return false;

	setPixelUnsafe(x, y, color);
	return true;
}

bool Image::setPixelAlphaBlend(int32_t x, int32_t y, const Color4f& color)
{
	if (x < 0 || x >= m_width || y < 0 || y >= m_height)
		return false;

	setPixelAlphaBlendUnsafe(x, y, color);
	return true;
}

void Image::getPixelUnsafe(int32_t x, int32_t y, Color4f& outColor) const
{
	m_pixelFormat.convertTo4f(
		m_palette,
		&m_data[x * m_pixelFormat.getByteSize() + y * m_pitch],
		&outColor,
		1,
		1
	);
}

void Image::setPixelUnsafe(int32_t x, int32_t y, const Color4f& color)
{
	m_pixelFormat.convertFrom4f(
		&color,
		m_palette,
		&m_data[x * m_pixelFormat.getByteSize() + y * m_pitch],
		1,
		1
	);
}

void Image::setPixelAlphaBlendUnsafe(int32_t x, int32_t y, const Color4f& color)
{
	Color4f target;
	getPixelUnsafe(x, y, target);
	setPixelUnsafe(x, y, (color * color.getAlpha() + target * (Scalar(1.0f) - color.getAlpha())).rgb1());
}

void Image::getSpanUnsafe(int32_t y, Color4f* outSpan) const
{
	const int32_t offset = y * m_pitch;
	m_pixelFormat.convertTo4f(
		m_palette,
		&m_data[offset],
		outSpan,
		1,
		m_width
	);
}

void Image::setSpanUnsafe(int32_t y, const Color4f* span)
{
	const int32_t offset = y * m_pitch;
	m_pixelFormat.convertFrom4f(
		span,
		m_palette,
		&m_data[offset],
		1,
		m_width
	);
}

void Image::getVerticalSpanUnsafe(int32_t x, Color4f* outSpan) const
{
	const int32_t offset = x * m_pixelFormat.getByteSize();
	m_pixelFormat.convertTo4f(
		m_palette,
		&m_data[offset],
		outSpan,
		m_width,
		m_height
	);
}

void Image::setVerticalSpanUnsafe(int32_t x, const Color4f* span)
{
	const int32_t offset = x * m_pixelFormat.getByteSize();
	m_pixelFormat.convertFrom4f(
		span,
		m_palette,
		&m_data[offset],
		m_width,
		m_height
	);
}

void Image::apply(const IImageFilter* imageFilter)
{
	imageFilter->apply(this);
	checkData(m_data, m_size);
}

void Image::convert(const PixelFormat& intoPixelFormat, Palette* intoPalette)
{
	// If pixel size match then convert in-place.
	if (m_pixelFormat.getByteSize() == intoPixelFormat.getByteSize())
	{
		m_pixelFormat.convert(
			m_palette,
			m_data,
			intoPixelFormat,
			intoPalette,
			m_data,
			m_width * m_height
		);
	}
	else
	{
		const size_t size = m_width * m_height * intoPixelFormat.getByteSize();
		uint8_t* tmp = allocData(size);

		m_pixelFormat.convert(
			m_palette,
			m_data,
			intoPixelFormat,
			intoPalette,
			tmp,
			m_width * m_height
		);

		freeData(m_data, m_size);

		m_pitch = m_width * intoPixelFormat.getByteSize();
		m_size = size;
		m_data = tmp;
	}

	m_pixelFormat = intoPixelFormat;
	checkData(m_data, m_size);
}

void Image::swap(Image* source)
{
	freeData(m_data, m_size);

	m_pixelFormat = source->m_pixelFormat;
	m_width = source->m_width;
	m_height = source->m_height;
	m_pitch = source->m_pitch;
	m_palette = source->m_palette;
	m_size = source->m_size;
	m_data = source->m_data;
	m_imageInfo = source->m_imageInfo;

	source->m_palette = nullptr;
	source->m_data = nullptr;
	source->m_imageInfo = nullptr;
}

Ref< Image > Image::load(const Path& fileName)
{
	Ref< Image > image;

	Ref< IImageFormat > imageFormat = IImageFormat::determineFormat(fileName);
	if (imageFormat == nullptr)
		return nullptr;

	Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmRead);
	if (file == nullptr)
		return nullptr;

	BufferedStream bufferedFile(file);
	image = imageFormat->read(&bufferedFile);

	if (image)
		checkData(image->m_data, image->m_size);

	file->close();
	return image;
}

Ref< Image > Image::load(IStream* stream, const std::wstring& extension)
{
	Ref< Image > image;

	Ref< IImageFormat > imageFormat = IImageFormat::determineFormat(extension);
	if (imageFormat == nullptr)
		return nullptr;

	BufferedStream bufferedFile(stream);
	image = imageFormat->read(&bufferedFile);

	if (image)
		checkData(image->m_data, image->m_size);

	return image;
}

Ref< Image > Image::load(const void* resource, uint32_t size, const std::wstring& extension)
{
	Ref< Image > image;

	Ref< IImageFormat > imageFormat = IImageFormat::determineFormat(extension);
	if (imageFormat == nullptr)
		return nullptr;

	MemoryStream stream(resource, size);
	image = imageFormat->read(&stream);

	if (image)
		checkData(image->m_data, image->m_size);

	return image;
}

bool Image::save(const Path& fileName) const
{
	if (!m_data)
		return false;

	Ref< IImageFormat > imageFormat = IImageFormat::determineFormat(fileName);
	if (imageFormat == nullptr)
		return false;

	Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmWrite);
	if (file == nullptr)
		return false;

	bool result = imageFormat->write(file, this);
	checkData(m_data, m_size);

	file->close();
	return result;
}

bool Image::save(IStream* stream, const std::wstring& extension) const
{
	if (!m_data)
		return false;

	Ref< IImageFormat > imageFormat = IImageFormat::determineFormat(extension);
	if (imageFormat == nullptr)
		return false;

	bool result = imageFormat->write(stream, this);
	checkData(m_data, m_size);

	return result;
}

void Image::setImageInfo(const ImageInfo* imageInfo)
{
	m_imageInfo = imageInfo;
}

const ImageInfo* Image::getImageInfo() const
{
	return m_imageInfo;
}

Image& Image::operator = (const Image& src)
{
	freeData(m_data, m_size);

	m_pixelFormat = src.m_pixelFormat;
	m_width = src.m_width;
	m_height = src.m_height;
	m_pitch = src.m_pitch;
	m_palette = src.m_palette;
	m_imageInfo = src.m_imageInfo;

	m_size = src.m_size;
	m_data = allocData(m_size);
	std::memcpy(m_data, src.m_data, m_size);

	checkData(m_data, m_size);
	return *this;
}

}

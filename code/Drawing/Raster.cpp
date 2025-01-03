/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <agg_alpha_mask_u8.h>
#include <agg_conv_curve.h>
#include <agg_conv_stroke.h>
#include <agg_ellipse.h>
#include <agg_path_storage.h>
#include <agg_pixfmt_gray.h>
#include <agg_pixfmt_rgba.h>
#include <agg_rasterizer_compound_aa.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_renderer_base.h>
#include <agg_renderer_scanline.h>
#include <agg_rendering_buffer.h>
#include <agg_rounded_rect.h>
#include <agg_scanline_p.h>
#include <agg_scanline_u.h>
#include <agg_span_allocator.h>
#include "Core/Containers/AlignedVector.h"
#include "Core/Log/Log.h"
#include "Core/Math/Envelope.h"
#include "Core/Misc/Align.h"
#include "Drawing/Image.h"
#include "Drawing/Raster.h"

namespace traktor
{

Color4f operator * (float v, const Color4f& c)
{
	return c * Scalar(v);
}

Color4f operator * (const Color4f& c, float v)
{
	return c * Scalar(v);
}

}

namespace traktor::drawing
{

int32_t cycle(float f)
{
	int32_t n = int32_t(f);
	return f >= 0.0f ? n : n - 1;
}

/*! Rasterizer implementation interface. */
class IRasterImpl : public IRefCount
{
public:
	virtual void setMask(Image* image) = 0;

	virtual void clearStyles() = 0;

	virtual int32_t defineSolidStyle(const Color4f& color) = 0;

	virtual int32_t defineLinearGradientStyle(const Matrix33& gradientMatrix, const AlignedVector< std::pair< Color4f, float > >& colors) = 0;

	virtual int32_t defineRadialGradientStyle(const Matrix33& gradientMatrix, const AlignedVector< std::pair< Color4f, float > >& colors) = 0;

	virtual int32_t defineImageStyle(const Matrix33& imageMatrix, const Image* image, bool repeat) = 0;

	virtual void clear() = 0;

	virtual void moveTo(float x, float y) = 0;

	virtual void lineTo(float x, float y) = 0;

	virtual void quadricTo(float x1, float y1, float x, float y) = 0;

	virtual void quadricTo(float x, float y) = 0;

	virtual void cubicTo(float x1, float y1, float x2, float y2, float x, float y) = 0;

	virtual void cubicTo(float x2, float y2, float x, float y) = 0;

	virtual void close() = 0;

	virtual void rect(float x, float y, float width, float height, float radius) = 0;

	virtual void circle(float x, float y, float radius) = 0;

	virtual void fill(int32_t style0, int32_t style1, Raster::FillRule fillRule) = 0;

	virtual void stroke(int32_t style, float width, Raster::StrokeJoin join, Raster::StrokeCap cap) = 0;

	virtual void submit() = 0;
};

/*! Style interface. */
template < typename color_type >
class IStyle
{
public:
	virtual void generateSpan(color_type* span, int x, int y, unsigned len) const = 0;
};

/*! Solid style partial template. */
template < typename color_type >
class SolidStyle : public IStyle< color_type > {};

/*! Solid style for 8-bit gray color. */
template < >
class SolidStyle< agg::gray8 > : public IStyle< agg::gray8 >
{
public:
	SolidStyle(const Color4f& color)
	:	m_color(agg::int8u(color.getAlpha() * 255.0f))
	{
	}

	virtual void generateSpan(agg::gray8* span, int x, int y, unsigned len) const override final
	{
		for (unsigned i = 0; i < len; ++i)
			span[i] = m_color;
	}

private:
	agg::gray8 m_color;
};

/*! Solid style for 32-bit colors. */
template < >
class SolidStyle< agg::rgba8 > : public IStyle< agg::rgba8 >
{
public:
	SolidStyle(const Color4f& color)
	:	m_color(
			agg::int8u(color.getRed() * 255.0f),
			agg::int8u(color.getGreen() * 255.0f),
			agg::int8u(color.getBlue() * 255.0f),
			agg::int8u(color.getAlpha() * 255.0f)
		)
	{
	}

	virtual void generateSpan(agg::rgba8* span, int x, int y, unsigned len) const override final
	{
		for (unsigned i = 0; i < len; ++i)
			span[i] = m_color;
	}

private:
	agg::rgba8 m_color;
};

/*! Linear gradient style for 32-bit colors. */
class LinearGradientStyle : public IStyle< agg::rgba8 >
{
public:
	explicit LinearGradientStyle(const Matrix33& gradientMatrix, const AlignedVector< std::pair< Color4f, float > >& colors)
	:	m_gradientMatrix(gradientMatrix)
	{
		for (const auto& p : colors)
			m_envelope.addKey(p.second, p.first * 255.0_simd);
	}

	virtual void generateSpan(agg::rgba8* span, int x, int y, unsigned len) const override final
	{
		float s = 0.0f;
		float n = 1.0f;

		Vector2 pt = m_gradientMatrix * Vector2(float(x), float(y));
		Vector2 dt = m_gradientMatrix * Vector2(float(x + 1.0f), float(y)) - pt;

		for (unsigned i = 0; i < len; ++i)
		{
			const float f = clamp((pt.x - s) * n, 0.0f, 1.0f);
			const Color4f c = m_envelope(f);

			span[i] = agg::rgba8(
				agg::int8u(c.getRed()),
				agg::int8u(c.getGreen()),
				agg::int8u(c.getBlue()),
				agg::int8u(c.getAlpha())
			);

			pt.x += dt.x;
		}
	}

private:
	Matrix33 m_gradientMatrix;
	Envelope< Color4f, LinearEvaluator< Color4f > > m_envelope;
};

/*! Radial gradient style for 32-bit colors. */
class RadialGradientStyle : public IStyle< agg::rgba8 >
{
public:
	explicit RadialGradientStyle(const Matrix33& gradientMatrix, const AlignedVector< std::pair< Color4f, float > >& colors)
	:	m_gradientMatrix(gradientMatrix)
	{
		for (const auto& p : colors)
			m_envelope.addKey(p.second, p.first * 255.0_simd);
	}

	virtual void generateSpan(agg::rgba8* span, int x, int y, unsigned len) const override final
	{
		float s = 0.0f;
		float n = 1.0f;

		Vector2 pt = m_gradientMatrix * Vector2(float(x), float(y));
		Vector2 dt = m_gradientMatrix * Vector2(float(x + 1.0f), float(y)) - pt;

		for (unsigned i = 0; i < len; ++i)
		{
			const float f = clamp(((pt * pt).length() - s) * n, 0.0f, 1.0f);
			const Color4f c = m_envelope(f);

			span[i] = agg::rgba8(
				agg::int8u(c.getRed()),
				agg::int8u(c.getGreen()),
				agg::int8u(c.getBlue()),
				agg::int8u(c.getAlpha())
			);

			pt += dt;
		}
	}

private:
	Matrix33 m_gradientMatrix;
	Envelope< Color4f, LinearEvaluator< Color4f > > m_envelope;
};

/*! Image style for 32-bit colors. */
class ImageClampStyle : public IStyle< agg::rgba8 >
{
public:
	ImageClampStyle(const Matrix33& imageMatrix, const drawing::Image* image)
	:	m_imageMatrix(imageMatrix)
	,	m_image(image)
	{
	}

	virtual void generateSpan(agg::rgba8* span, int x, int y, unsigned len) const override final
	{
		Color4f c(0.0f, 0.0f, 0.0f, 0.0f);

		Vector2 pt = m_imageMatrix * Vector2(float(x), float(y));
		Vector2 dt = m_imageMatrix * Vector2(float(x + 1.0f), float(y)) - pt;

		const int32_t w = m_image->getWidth();
		const int32_t h = m_image->getHeight();

		for (unsigned i = 0; i < len; ++i)
		{
			int32_t sx = clamp(int32_t(pt.x), 0, w - 1);
			int32_t sy = clamp(int32_t(pt.y), 0, h - 1);

			m_image->getPixelUnsafe(sx, sy, c);

			span[i] = agg::rgba8(
				agg::int8u(c.getRed() * 255.0f),
				agg::int8u(c.getGreen() * 255.0f),
				agg::int8u(c.getBlue() * 255.0f),
				agg::int8u(c.getAlpha() * 255.0f)
			);

			pt += dt;
		}
	}

private:
	Matrix33 m_imageMatrix;
	Ref< const drawing::Image > m_image;
};

/*! Image style for 32-bit colors. */
class ImageRepeatStyle : public IStyle< agg::rgba8 >
{
public:
	ImageRepeatStyle(const Matrix33& imageMatrix, const drawing::Image* image)
	:	m_imageMatrix(imageMatrix)
	,	m_image(image)
	{
	}

	virtual void generateSpan(agg::rgba8* span, int x, int y, unsigned len) const override final
	{
		Color4f c(0.0f, 0.0f, 0.0f, 0.0f);

		Vector2 pt = m_imageMatrix * Vector2(float(x), float(y));
		Vector2 dt = m_imageMatrix * Vector2(float(x + 1.0f), float(y)) - pt;

		const int32_t w = m_image->getWidth();
		const int32_t h = m_image->getHeight();

		for (unsigned i = 0; i < len; ++i)
		{
			int32_t nx = cycle(pt.x / w);
			int32_t sx = int32_t(pt.x - nx * w);

			int32_t ny = cycle(pt.y / h);
			int32_t sy = int32_t(pt.y - ny * h);

			m_image->getPixelUnsafe(sx, sy, c);

			span[i] = agg::rgba8(
				agg::int8u(c.getRed() * 255.0f),
				agg::int8u(c.getGreen() * 255.0f),
				agg::int8u(c.getBlue() * 255.0f),
				agg::int8u(c.getAlpha() * 255.0f)
			);

			pt += dt;
		}
	}

private:
	Matrix33 m_imageMatrix;
	Ref< const drawing::Image > m_image;
};

/*! Style handler partial template. */
template< typename color_type >
class StyleHandler {};

/*! Style handler for 8-bit gray colors. */
template < >
class StyleHandler< agg::gray8 >
{
public:
	void clearStyles()
	{
		m_styles.resize(0);
	}

	int32_t defineSolidStyle(const Color4f& color)
	{
		m_styles.push_back(new SolidStyle< agg::gray8 >(color));
		return int32_t(m_styles.size() - 1);
	}

	int32_t defineLinearGradientStyle(const Matrix33& gradientMatrix, const AlignedVector< std::pair< Color4f, float > >& colors)
	{
		return defineSolidStyle(colors[0].first);
	}

	int32_t defineRadialGradientStyle(const Matrix33& gradientMatrix, const AlignedVector< std::pair< Color4f, float > >& colors)
	{
		return defineSolidStyle(colors[0].first);
	}

	int32_t defineImageStyle(const Matrix33& imageMatrix, const Image* image, bool repeat)
	{
		return defineSolidStyle(Color4f(0.0f, 0.0f, 0.0f, 0.0f));
	}

	bool is_solid(unsigned style) const
	{
		return false;
	}

	agg::gray8 color(unsigned style) const
	{
		T_FATAL_ERROR;
		return agg::gray8();
	}

    void generate_span(agg::gray8* span, int x, int y, unsigned len, unsigned style)
    {
		m_styles[style]->generateSpan(span, x, y, len);
	}

private:
	AlignedVector< IStyle< agg::gray8 >* > m_styles;
};

/*! Style handler for 32-bit colors. */
template < >
class StyleHandler< agg::rgba8 >
{
public:
	void clearStyles()
	{
		m_styles.resize(0);
	}

	int32_t defineSolidStyle(const Color4f& color)
	{
		m_styles.push_back(new SolidStyle< agg::rgba8 >(color));
		return int32_t(m_styles.size() - 1);
	}

	int32_t defineLinearGradientStyle(const Matrix33& gradientMatrix, const AlignedVector< std::pair< Color4f, float > >& colors)
	{
		m_styles.push_back(new LinearGradientStyle(gradientMatrix, colors));
		return int32_t(m_styles.size() - 1);
	}

	int32_t defineRadialGradientStyle(const Matrix33& gradientMatrix, const AlignedVector< std::pair< Color4f, float > >& colors)
	{
		m_styles.push_back(new RadialGradientStyle(gradientMatrix, colors));
		return int32_t(m_styles.size() - 1);
	}

	int32_t defineImageStyle(const Matrix33& imageMatrix, const Image* image, bool repeat)
	{
		if (repeat)
			m_styles.push_back(new ImageRepeatStyle(imageMatrix, image));
		else
			m_styles.push_back(new ImageClampStyle(imageMatrix, image));
		return int32_t(m_styles.size() - 1);
	}

	bool is_solid(unsigned style) const
	{
		return false;
	}

	agg::rgba8 color(unsigned style) const
	{
		T_FATAL_ERROR;
		return agg::rgba8();
	}

	void generate_span(agg::rgba8* span, int x, int y, unsigned len, unsigned style)
	{
		m_styles[style]->generateSpan(span, x, y, len);
	}

private:
	AlignedVector< IStyle< agg::rgba8 >* > m_styles;
};

/*! Rasterizer implementation. */
template < typename pixfmt_type, typename color_type >
class RasterImpl : public RefCountImpl< IRasterImpl >
{
public:
	explicit RasterImpl(Image* image)
	:	m_rbuffer((agg::int8u*)image->getData(), image->getWidth(), image->getHeight(), image->getWidth() * image->getPixelFormat().getByteSize())
	,	m_pf(m_rbuffer)
	,	m_renderer(m_pf)
	{
	}

	virtual void setMask(Image* image) override final
	{
		m_mask = image;
	}

	virtual void clearStyles() override final
	{
		m_styleHandler.clearStyles();
	}

	virtual int32_t defineSolidStyle(const Color4f& color) override final
	{
		return m_styleHandler.defineSolidStyle(color);
	}

	virtual int32_t defineLinearGradientStyle(const Matrix33& gradientMatrix, const AlignedVector< std::pair< Color4f, float > >& colors) override final
	{
		return m_styleHandler.defineLinearGradientStyle(gradientMatrix, colors);
	}

	virtual int32_t defineRadialGradientStyle(const Matrix33& gradientMatrix, const AlignedVector< std::pair< Color4f, float > >& colors) override final
	{
		return m_styleHandler.defineRadialGradientStyle(gradientMatrix, colors);
	}

	virtual int32_t defineImageStyle(const Matrix33& imageMatrix, const Image* image, bool repeat) override final
	{
		return m_styleHandler.defineImageStyle(imageMatrix, image, repeat);
	}

	virtual void clear() override final
	{
		m_paths.resize(0);
		m_current = nullptr;
	}

	virtual void moveTo(float x, float y) override final
	{
		current().move_to(x, y);
	}

	virtual void lineTo(float x, float y) override final
	{
		current().line_to(x, y);
	}

	virtual void quadricTo(float x1, float y1, float x, float y) override final
	{
		current().curve3(x1, y1, x, y);
	}

	virtual void quadricTo(float x, float y) override final
	{
		current().curve3(x, y);
	}

	virtual void cubicTo(float x1, float y1, float x2, float y2, float x, float y) override final
	{
		current().curve4(x1, y1, x2, y2, x, y);
	}

	virtual void cubicTo(float x2, float y2, float x, float y) override final
	{
		current().curve4(x2, y2, x, y);
	}

	virtual void close() override final
	{
		m_current->second = true;
		m_current = nullptr;
	}

	virtual void rect(float x, float y, float width, float height, float radius) override final
	{
		agg::rounded_rect r(x, y, x + width, y + height, radius);
		r.normalize_radius();
		current().concat_path(r);
	}

	virtual void circle(float x, float y, float radius) override final
	{
		agg::ellipse e(x, y, radius, radius);
		current().concat_path(e);
	}

	virtual void fill(int32_t style0, int32_t style1, Raster::FillRule fillRule) override final
	{
		for (auto& path : m_paths)
		{
			agg::path_storage p = path.first;
			p.align_all_paths();

			agg::conv_curve< agg::path_storage > curve(p);

			if (fillRule == Raster::FillRule::NonZero)
				m_rasterizer.filling_rule(agg::fill_non_zero);
			else // Raster::FillRule::OddEven
				m_rasterizer.filling_rule(agg::fill_even_odd);

			m_rasterizer.styles(style0, style1);
			m_rasterizer.add_path(curve);
		}
	}

	virtual void stroke(int32_t style, float width, Raster::StrokeJoin join, Raster::StrokeCap cap) override final
	{
		for (auto& path : m_paths)
		{
			agg::path_storage p = path.first;
			p.align_all_paths();
			
			if (path.second)
				p.close_polygon();

			agg::conv_curve< agg::path_storage > curve(p);
			agg::conv_stroke< agg::conv_curve< agg::path_storage > > outline(curve);
			outline.width(width);

			switch (join)
			{
			case Raster::StrokeJoin::Miter:
				outline.line_join(agg::miter_join);
				break;

			case Raster::StrokeJoin::Round:
				outline.line_join(agg::round_join);
				break;

			case Raster::StrokeJoin::Bevel:
				outline.line_join(agg::bevel_join);
				break;

			default:
				break;
			}

			switch (cap)
			{
			case Raster::StrokeCap::Butt:
				outline.line_cap(agg::butt_cap);
				break;

			case Raster::StrokeCap::Square:
				outline.line_cap(agg::square_cap);
				break;

			case Raster::StrokeCap::Round:
				outline.line_cap(agg::round_cap);
				break;

			default:
				break;
			}

			m_rasterizer.filling_rule(agg::fill_non_zero);
			m_rasterizer.styles(-1, style);
			m_rasterizer.add_path(outline);
		}
	}

	virtual void submit() override final
	{
		agg::span_allocator< color_type > alloc;
		if (!m_mask)
		{
			agg::scanline_u8 sl;
			agg::render_scanlines_compound_layered(m_rasterizer, sl, m_renderer, alloc, m_styleHandler);
		}
		else
		{
			agg::rendering_buffer mrb((agg::int8u*)m_mask->getData(), m_mask->getWidth(), m_mask->getHeight(), m_mask->getWidth() * m_mask->getPixelFormat().getByteSize());
			agg::alpha_mask_gray8 mask(mrb);
			agg::scanline_u8_am< agg::alpha_mask_gray8 > sl(mask);
			agg::render_scanlines_compound_layered(m_rasterizer, sl, m_renderer, alloc, m_styleHandler);
		}
		m_rasterizer.reset();
	}

private:
	StyleHandler< color_type > m_styleHandler;
	Ref< Image > m_mask;
	agg::rendering_buffer m_rbuffer;
	pixfmt_type m_pf;
	agg::renderer_base< pixfmt_type > m_renderer;
	agg::rasterizer_compound_aa<> m_rasterizer;
	AlignedVector< std::pair< agg::path_storage, bool > > m_paths;
	std::pair< agg::path_storage, bool >* m_current = nullptr;

	agg::path_storage& current()
	{
		if (!m_current)
		{
			m_paths.push_back();
			m_current = &m_paths.back();
			m_current->second = false;
		}
		T_FATAL_ASSERT(m_current->second == false);
		return m_current->first;
	}
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.Raster", Raster, Object)

Raster::Raster(Image* image)
{
	setImage(image);
}

bool Raster::valid() const
{
	return m_impl != nullptr;
}

bool Raster::setImage(Image* image)
{
	m_impl = nullptr;

	if (image->getPixelFormat() == PixelFormat::getA8B8G8R8())
		m_impl = new RasterImpl< agg::pixfmt_rgba32_plain, agg::rgba8 >(image);
	else if (image->getPixelFormat() == PixelFormat::getB8G8R8A8())
		m_impl = new RasterImpl< agg::pixfmt_argb32_plain, agg::rgba8 >(image);
	else if (image->getPixelFormat() == PixelFormat::getA8R8G8B8())
		m_impl = new RasterImpl< agg::pixfmt_bgra32_plain, agg::rgba8 >(image);
	else if (image->getPixelFormat() == PixelFormat::getR8G8B8A8())
		m_impl = new RasterImpl< agg::pixfmt_abgr32_plain, agg::rgba8 >(image);
	else if (image->getPixelFormat() == PixelFormat::getA8())
		m_impl = new RasterImpl< agg::pixfmt_gray8, agg::gray8 >(image);

	return valid();
}

void Raster::setMask(Image* image)
{
	m_impl->setMask(image);
}

void Raster::clearStyles()
{
	m_impl->clearStyles();
}

int32_t Raster::defineSolidStyle(const Color4f& color)
{
	return m_impl->defineSolidStyle(color);
}

int32_t Raster::defineLinearGradientStyle(const Matrix33& gradientMatrix, const AlignedVector< std::pair< Color4f, float > >& colors)
{
	return m_impl->defineLinearGradientStyle(gradientMatrix, colors);
}

int32_t Raster::defineRadialGradientStyle(const Matrix33& gradientMatrix, const AlignedVector< std::pair< Color4f, float > >& colors)
{
	return m_impl->defineRadialGradientStyle(gradientMatrix, colors);
}

int32_t Raster::defineImageStyle(const Matrix33& imageMatrix, const Image* image, bool repeat)
{
	return m_impl->defineImageStyle(imageMatrix, image, repeat);
}

void Raster::clear()
{
	m_impl->clear();
}

void Raster::moveTo(float x, float y)
{
	m_impl->moveTo(x, y);
}

void Raster::lineTo(float x, float y)
{
	m_impl->lineTo(x, y);
}

void Raster::quadricTo(float x1, float y1, float x, float y)
{
	m_impl->quadricTo(x1, y1, x, y);
}

void Raster::quadricTo(float x, float y)
{
	m_impl->quadricTo(x, y);
}

void Raster::cubicTo(float x1, float y1, float x2, float y2, float x, float y)
{
	m_impl->cubicTo(x1, y1, x2, y2, x, y);
}

void Raster::cubicTo(float x2, float y2, float x, float y)
{
	m_impl->cubicTo(x2, y2, x, y);
}

void Raster::close()
{
	m_impl->close();
}

void Raster::rect(float x, float y, float width, float height, float radius)
{
	m_impl->rect(x, y, width, height, radius);
}

void Raster::circle(float x, float y, float radius)
{
	m_impl->circle(x, y, radius);
}

void Raster::fill(int32_t style0, int32_t style1, FillRule fillRule)
{
	m_impl->fill(style0, style1, fillRule);
}

void Raster::stroke(int32_t style, float width, StrokeJoin join, StrokeCap cap)
{
	m_impl->stroke(style, width, join, cap);
}

void Raster::submit()
{
	m_impl->submit();
}

}

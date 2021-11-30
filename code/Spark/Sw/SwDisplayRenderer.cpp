#include "Drawing/Image.h"
#include "Drawing/Raster.h"
#include "Spark/ColorTransform.h"
#include "Spark/BitmapImage.h"
#include "Spark/Canvas.h"
#include "Spark/Dictionary.h"
#include "Spark/Font.h"
#include "Spark/Movie.h"
#include "Spark/Shape.h"
#include "Spark/Sw/SwDisplayRenderer.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

const static Matrix33 c_textureTS = translate(0.5f, 0.5f) * scale(1.0f / 32768.0f, 1.0f / 32768.0f);

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.SwDisplayRenderer", SwDisplayRenderer, IDisplayRenderer)

SwDisplayRenderer::SwDisplayRenderer(drawing::Image* image, bool clearBackground)
:	m_image(image)
,	m_transform(Matrix33::identity())
,	m_clearBackground(clearBackground)
,	m_writeMask(false)
,	m_writeEnable(true)
{
	m_raster = new drawing::Raster(m_image);
}

void SwDisplayRenderer::setTransform(const Matrix33& transform)
{
	m_transform = transform;
}

void SwDisplayRenderer::setImage(drawing::Image* image)
{
	T_ASSERT(image->getPixelFormat() == m_image->getPixelFormat());
	m_image = image;
	m_raster = new drawing::Raster(m_image);
}

bool SwDisplayRenderer::wantDirtyRegion() const
{
	return false;
}

void SwDisplayRenderer::begin(
	const Dictionary& dictionary,
	const Color4f& backgroundColor,
	const Aabb2& frameBounds,
	const Vector4& frameTransform,
	float viewWidth,
	float viewHeight,
	const Aabb2& dirtyRegion
)
{
	if (m_clearBackground)
		m_image->clear(backgroundColor.rgb0());
	m_frameBounds = frameBounds;
	m_frameTransform = frameTransform;
}

void SwDisplayRenderer::beginSprite(const SpriteInstance& sprite, const Matrix33& transform)
{
}

void SwDisplayRenderer::endSprite(const SpriteInstance& sprite, const Matrix33& transform)
{
}

void SwDisplayRenderer::beginEdit(const EditInstance& edit, const Matrix33& transform)
{
}

void SwDisplayRenderer::endEdit(const EditInstance& edit, const Matrix33& transform)
{
}

void SwDisplayRenderer::beginMask(bool increment)
{
	m_writeMask = true;
	if (increment)
	{
		m_writeEnable = true;
		m_mask.push_back(new drawing::Image(
			drawing::PixelFormat::getA8(),
			m_image->getWidth(),
			m_image->getHeight()
		));
		m_mask.back()->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));
		m_raster->setImage(m_mask.back());
		if (m_mask.size() >= 2)
			m_raster->setMask(m_mask[m_mask.size() - 2]);
		else
			m_raster->setMask(0);
	}
	else
	{
		m_writeEnable = false;
		T_FATAL_ASSERT (!m_mask.empty());
		m_mask.pop_back();
	}
}

void SwDisplayRenderer::endMask()
{
	T_FATAL_ASSERT(m_writeMask);
	m_writeMask = false;
	m_writeEnable = true;
	m_raster->setImage(m_image);
	if (!m_mask.empty())
		m_raster->setMask(m_mask.back());
	else
		m_raster->setMask(0);
}

void SwDisplayRenderer::renderShape(const Dictionary& dictionary, const Matrix33& transform, const Aabb2& clipBounds, const Shape& shape, const ColorTransform& cxform, uint8_t blendMode)
{
	if (!m_writeEnable)
		return;

	const Color4f& cxm = cxform.mul;
	const Color4f& cxa = cxform.add;
	int32_t width = m_image->getWidth();
	int32_t height = m_image->getHeight();
	float frameWidth = m_frameBounds.mx.x;
	float frameHeight = m_frameBounds.mx.y;

	Matrix33 rasterTransform =
		traktor::scale(width / frameWidth, height / frameHeight) *
		traktor::scale(m_frameTransform.z(), m_frameTransform.w()) *
		traktor::translate(m_frameTransform.x(), m_frameTransform.y()) *
		transform *
		m_transform;

	float strokeScale = std::min(width / frameWidth, height / frameHeight);

	const AlignedVector< FillStyle >& fillStyles = shape.getFillStyles();
	const AlignedVector< LineStyle >& lineStyles = shape.getLineStyles();
	int32_t lineStyleBase = 0;

	// Convert all styles used by this shape.
	m_raster->clearStyles();
	if (!m_writeMask)
	{
		for (uint32_t i = 0; i < uint32_t(fillStyles.size()); ++i)
		{
			const FillStyle& style = fillStyles[i];
			const AlignedVector< FillStyle::ColorRecord >& colorRecords = style.getColorRecords();

			const BitmapImage* bitmap = dynamic_type_cast< const BitmapImage* >(dictionary.getBitmap(style.getFillBitmap()));
			if (bitmap)
			{
				const drawing::Image* image = bitmap->getImage();
				T_ASSERT(image);

				m_raster->defineImageStyle(
					style.getFillBitmapMatrix().inverse() * rasterTransform.inverse(),
					image,
					style.getFillBitmapRepeat()
				);
			}
			else
			{
				if (colorRecords.size() == 1)
				{
					const Color4f& c = colorRecords[0].color;
					m_raster->defineSolidStyle(c * cxm + cxa);
				}
				else if (colorRecords.size() > 1)
				{
					switch (style.getGradientType())
					{
					case FillStyle::GradientType::Linear:
						{
							AlignedVector< std::pair< Color4f, float > > colors;
							for (uint32_t j = 0; j < colorRecords.size(); ++j)
							{
								const Color4f& c = colorRecords[j].color;
								colors.push_back(std::make_pair(c * cxm + cxa, colorRecords[j].ratio));
							}
							m_raster->defineLinearGradientStyle(
								c_textureTS * style.getGradientMatrix().inverse() * rasterTransform.inverse(),
								colors
							);
						}
						break;

					case FillStyle::GradientType::Radial:
						{
							AlignedVector< std::pair< Color4f, float > > colors;
							for (uint32_t j = 0; j < colorRecords.size(); ++j)
							{
								const Color4f& c = colorRecords[j].color;
								colors.push_back(std::make_pair(c * cxm + cxa, colorRecords[j].ratio));
							}
							m_raster->defineRadialGradientStyle(
								c_textureTS * style.getGradientMatrix().inverse() * rasterTransform.inverse(),
								colors
							);
						}
						break;

					default:
						m_raster->defineSolidStyle(Color4f(1.0f, 1.0f, 1.0f, 1.0f));
						break;
					}
				}
				else
				{
					m_raster->defineSolidStyle(Color4f(1.0f, 1.0f, 1.0f, 1.0f));
				}
			}
		}
		lineStyleBase = int32_t(fillStyles.size());
		for (uint32_t i = 0; i < uint32_t(lineStyles.size()); ++i)
		{
			const LineStyle& style = lineStyles[i];
			m_raster->defineSolidStyle(style.getLineColor() * cxm + cxa);
		}
	}
	else
	{
		m_raster->defineSolidStyle(Color4f(1.0f, 1.0f, 1.0f, 1.0f));
	}

	// Rasterize every path in shape.
	for (const auto& path : shape.getPaths())
	{
		const AlignedVector< Vector2 >& points = path.getPoints();
		const AlignedVector< SubPath >& subPaths = path.getSubPaths();
		for (const auto& subPath : subPaths)
		{
			const int32_t fs0 = subPath.fillStyle0 - 1;
			const int32_t fs1 = subPath.fillStyle1 - 1;
			const int32_t ls = subPath.lineStyle - 1;
			T_ASSERT(fs0 >= 0 || fs1 >= 0 || ls >= 0);

			m_raster->clear();

			for (const auto& segment : subPath.segments)
			{
				m_raster->moveTo(rasterTransform * points[segment.pointsOffset]);
				if (segment.type == SpgtLinear)
					m_raster->lineTo(rasterTransform * points[segment.pointsOffset + 1]);
				else
					m_raster->quadricTo(rasterTransform * points[segment.pointsOffset + 1], rasterTransform * points[segment.pointsOffset + 2]);
			}

			if (fs0 >= 0 || fs1 >= 0)
			{
				if (!m_writeMask)
					m_raster->fill(fs0, fs1, drawing::Raster::FrNonZero);
				else
					m_raster->fill(fs0 >= 0 ? 0 : -1, fs1 >= 0 ? 0 : -1, drawing::Raster::FrNonZero);
			}

			if (ls >= 0)
			{
				if (!m_writeMask)
					m_raster->stroke(lineStyleBase + ls, lineStyles[ls].getLineWidth() * strokeScale, drawing::Raster::ScSquare);
				else
					m_raster->stroke(0, lineStyles[ls].getLineWidth(), drawing::Raster::ScSquare);
			}
		}

		m_raster->submit();
	}
}

void SwDisplayRenderer::renderMorphShape(const Dictionary& dictionary, const Matrix33& transform, const Aabb2& clipBounds, const MorphShape& shape, const ColorTransform& cxform)
{
}

void SwDisplayRenderer::renderGlyph(
	const Dictionary& dictionary,
	const Matrix33& transform,
	const Aabb2& clipBounds,
	const Font* font,
	const Shape* glyph,
	float fontHeight,
	wchar_t character,
	const Color4f& color,
	const ColorTransform& cxform,
	uint8_t filter,
	const Color4f& filterColor
)
{
	// Only support embedded fonts.
	if (!glyph || !m_writeEnable)
		return;

	m_raster->clearStyles();
	m_raster->defineSolidStyle(color * cxform.mul + cxform.add);

	int32_t width = m_image->getWidth();
	int32_t height = m_image->getHeight();
	float frameWidth = m_frameBounds.mx.x;
	float frameHeight = m_frameBounds.mx.y;

	float coordScale = font->getCoordinateType() == Font::CtTwips ? 1.0f / 1000.0f : 1.0f / (20.0f * 1000.0f);
	float fontScale = coordScale * fontHeight;

	Matrix33 rasterTransform =
		traktor::scale(width / frameWidth, height / frameHeight) *
		traktor::scale(m_frameTransform.z(), m_frameTransform.w()) *
		traktor::translate(m_frameTransform.x(), m_frameTransform.y()) *
		transform *
		scale(fontScale, fontScale) *
		m_transform;

	// Rasterize every path in shape.
	for (const auto& path : glyph->getPaths())
	{
		const AlignedVector< Vector2 >& points = path.getPoints();
		const AlignedVector< SubPath >& subPaths = path.getSubPaths();
		for (const auto& subPath : subPaths)
		{
			const int32_t fs0 = subPath.fillStyle0 - 1;
			const int32_t fs1 = subPath.fillStyle1 - 1;
			T_ASSERT(fs0 >= 0 || fs1 >= 0);

			m_raster->clear();

			for (const auto& segment : subPath.segments)
			{
				m_raster->moveTo(rasterTransform * points[segment.pointsOffset]);
				if (segment.type == SpgtLinear)
					m_raster->lineTo(rasterTransform * points[segment.pointsOffset + 1]);
				else
					m_raster->quadricTo(rasterTransform * points[segment.pointsOffset + 1], rasterTransform * points[segment.pointsOffset + 2]);
			}

			if (fs0 >= 0 || fs1 >= 0)
				m_raster->fill(fs0 >= 0 ? 0 : -1, fs1 >= 0 ? 0 : -1, drawing::Raster::FrNonZero);
		}

		m_raster->submit();
	}
}

void SwDisplayRenderer::renderQuad(const Matrix33& transform, const Aabb2& bounds, const ColorTransform& cxform)
{
}

void SwDisplayRenderer::renderCanvas(const Matrix33& transform, const Canvas& canvas, const ColorTransform& cxform, uint8_t blendMode)
{
	if (!m_writeEnable)
		return;

	const Color4f& cxm = cxform.mul;
	const Color4f& cxa = cxform.add;
	int32_t width = m_image->getWidth();
	int32_t height = m_image->getHeight();
	float frameWidth = m_frameBounds.mx.x;
	float frameHeight = m_frameBounds.mx.y;

	Matrix33 rasterTransform =
		traktor::scale(width / frameWidth, height / frameHeight) *
		traktor::scale(m_frameTransform.z(), m_frameTransform.w()) *
		traktor::translate(m_frameTransform.x(), m_frameTransform.y()) *
		transform *
		m_transform;

	float strokeScale = std::min(width / frameWidth, height / frameHeight);

	const Dictionary& dictionary = canvas.getDictionary();
	const AlignedVector< FillStyle >& fillStyles = canvas.getFillStyles();
	const AlignedVector< LineStyle >& lineStyles = canvas.getLineStyles();
	int32_t lineStyleBase = 0;

	// Convert all styles used by this shape.
	m_raster->clearStyles();
	if (!m_writeMask)
	{
		for (uint32_t i = 0; i < uint32_t(fillStyles.size()); ++i)
		{
			const FillStyle& style = fillStyles[i];
			const AlignedVector< FillStyle::ColorRecord >& colorRecords = style.getColorRecords();

			const BitmapImage* bitmap = dynamic_type_cast< const BitmapImage* >(dictionary.getBitmap(style.getFillBitmap()));
			if (bitmap)
			{
				const drawing::Image* image = bitmap->getImage();
				T_ASSERT(image);

				m_raster->defineImageStyle(
					style.getFillBitmapMatrix().inverse() * rasterTransform.inverse(),
					image,
					style.getFillBitmapRepeat()
				);
			}
			else
			{
				if (colorRecords.size() == 1)
				{
					const Color4f& c = colorRecords[0].color;
					m_raster->defineSolidStyle(c * cxm + cxa);
				}
				else if (colorRecords.size() > 1)
				{
					switch (style.getGradientType())
					{
					case FillStyle::GradientType::Linear:
						{
							AlignedVector< std::pair< Color4f, float > > colors;
							for (uint32_t j = 0; j < colorRecords.size(); ++j)
							{
								const Color4f& c = colorRecords[j].color;
								colors.push_back(std::make_pair(c * cxm + cxa, colorRecords[j].ratio));
							}
							m_raster->defineLinearGradientStyle(
								c_textureTS * style.getGradientMatrix().inverse() * rasterTransform.inverse(),
								colors
							);
						}
						break;

					case FillStyle::GradientType::Radial:
						{
							AlignedVector< std::pair< Color4f, float > > colors;
							for (uint32_t j = 0; j < colorRecords.size(); ++j)
							{
								const Color4f& c = colorRecords[j].color;
								colors.push_back(std::make_pair(c * cxm + cxa, colorRecords[j].ratio));
							}
							m_raster->defineRadialGradientStyle(
								c_textureTS * style.getGradientMatrix().inverse() * rasterTransform.inverse(),
								colors
							);
						}
						break;

					default:
						m_raster->defineSolidStyle(Color4f(1.0f, 1.0f, 1.0f, 1.0f));
						break;
					}
				}
				else
				{
					m_raster->defineSolidStyle(Color4f(1.0f, 1.0f, 1.0f, 1.0f));
				}
			}
		}
		lineStyleBase = int32_t(fillStyles.size());
		for (uint32_t i = 0; i < uint32_t(lineStyles.size()); ++i)
		{
			const LineStyle& style = lineStyles[i];
			m_raster->defineSolidStyle(style.getLineColor() * cxm + cxa);
		}
	}
	else
	{
		m_raster->defineSolidStyle(Color4f(1.0f, 1.0f, 1.0f, 1.0f));
	}

	// Rasterize every path in shape.
	const AlignedVector< Path >& paths = canvas.getPaths();
	for (AlignedVector< Path >::const_iterator i = paths.begin(); i != paths.end(); ++i)
	{
		const AlignedVector< Vector2 >& points = i->getPoints();
		const AlignedVector< SubPath >& subPaths = i->getSubPaths();
		for (AlignedVector< SubPath >::const_iterator j = subPaths.begin(); j != subPaths.end(); ++j)
		{
			int32_t fs0 = j->fillStyle0 - 1;
			int32_t fs1 = j->fillStyle1 - 1;
			int32_t ls = j->lineStyle - 1;

			T_ASSERT(fs0 >= 0 || fs1 >= 0 || ls >= 0);

			m_raster->clear();

			const AlignedVector< SubPathSegment >& segments = j->segments;
			for (AlignedVector< SubPathSegment >::const_iterator k = segments.begin(); k != segments.end(); ++k)
			{
				m_raster->moveTo(rasterTransform * points[k->pointsOffset]);
				if (k->type == SpgtLinear)
					m_raster->lineTo(rasterTransform * points[k->pointsOffset + 1]);
				else
					m_raster->quadricTo(rasterTransform * points[k->pointsOffset + 1], rasterTransform * points[k->pointsOffset + 2]);
			}

			if (fs0 >= 0 || fs1 >= 0)
			{
				if (!m_writeMask)
					m_raster->fill(fs0, fs1, drawing::Raster::FrNonZero);
				else
					m_raster->fill(fs0 >= 0 ? 0 : -1, fs1 >= 0 ? 0 : -1, drawing::Raster::FrNonZero);
			}

			if (ls >= 0)
			{
				if (!m_writeMask)
					m_raster->stroke(lineStyleBase + ls, lineStyles[ls].getLineWidth() * strokeScale, drawing::Raster::ScSquare);
				else
					m_raster->stroke(0, lineStyles[ls].getLineWidth(), drawing::Raster::ScSquare);
			}
		}

		m_raster->submit();
	}
}

void SwDisplayRenderer::end()
{
}

	}
}

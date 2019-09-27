#include "Svg/Style.h"

namespace traktor
{
	namespace svg
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.svg.Style", Style, Object)

Style::Style()
:	m_fillEnable(false)
,	m_fill(1.0f, 1.0f, 1.0f, 1.0f)
,	m_strokeEnable(false)
,	m_strokeWidth(1.0f)
,	m_stroke(0.0f, 0.0f, 0.0f, 1.0f)
,	m_opacity(1.0f)
{
}

void Style::setFillEnable(bool fillEnable)
{
	m_fillEnable = fillEnable;
}

bool Style::getFillEnable() const
{
	return m_fillEnable;
}

void Style::setFillGradient(const Gradient* fillGradient)
{
	m_fillGradient = fillGradient;
}

const Gradient* Style::getFillGradient() const
{
	return m_fillGradient;
}

void Style::setFill(const Color4f& fill)
{
	m_fill = fill;
}

const Color4f& Style::getFill() const
{
	return m_fill;
}

void Style::setStrokeEnable(bool strokeEnable)
{
	m_strokeEnable = strokeEnable;
}

bool Style::getStrokeEnable() const
{
	return m_strokeEnable;
}

void Style::setStrokeGradient(const Gradient* strokeGradient)
{
	m_strokeGradient = strokeGradient;
}

const Gradient* Style::getStrokeGradient() const
{
	return m_strokeGradient;
}

void Style::setStrokeWidth(float strokeWidth)
{
	m_strokeWidth = strokeWidth;
}

float Style::getStrokeWidth() const
{
	return m_strokeWidth;
}

void Style::setStroke(const Color4f& stroke)
{
	m_stroke = stroke;
}

const Color4f& Style::getStroke() const
{
	return m_stroke;
}

void Style::setOpacity(float opacity)
{
	m_opacity = opacity;
}

float Style::getOpacity() const
{
	return m_opacity;
}

bool Style::operator == (const Style& other) const
{
	if (m_fillEnable != other.m_fillEnable)
		return false;

	if (m_fillEnable)
	{
		if (m_fillGradient != other.m_fillGradient)
			return false;
		if (m_fill != other.m_fill)
			return false;
	}

	if (m_strokeEnable != other.m_strokeEnable)
		return false;

	if (m_strokeEnable)
	{
		if (m_strokeGradient != other.m_strokeGradient)
			return false;
		if (m_strokeWidth != other.m_strokeWidth)
			return false;
		if (m_stroke != other.m_stroke)
			return false;
	}

	if (m_opacity != other.m_opacity)
		return false;

	return true;
}

	}
}

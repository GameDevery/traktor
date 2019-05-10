#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/TString.h"
#include "Ui/Application.h"
#include "Ui/X11/BitmapX11.h"
#include "Ui/X11/CanvasX11.h"

namespace traktor
{
	namespace ui
	{

CanvasX11::CanvasX11(cairo_t* cr)
:	m_cr(cr)
,	m_foreground(255, 255, 255, 255)
,	m_background(255, 255, 255, 255)
,	m_thickness(1)
,	m_underline(false)
{
	cairo_reset_clip(m_cr);
	cairo_set_line_width(m_cr, 1);
	cairo_set_source_rgba(m_cr, 1.0f, 1.0f, 1.0f, 1.0f);
}

CanvasX11::~CanvasX11()
{
}

void CanvasX11::setForeground(const Color4ub& foreground)
{
	m_foreground = foreground;
}

void CanvasX11::setBackground(const Color4ub& background)
{
	m_background = background;
}

void CanvasX11::setFont(const Font& font)
{
	cairo_select_font_face(
		m_cr,
		wstombs(font.getFace()).c_str(),
		CAIRO_FONT_SLANT_NORMAL,
		font.isBold() ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL
	);
	cairo_set_font_size(
		m_cr,
		dpi96(font.getSize())
	);
	m_underline = font.isUnderline();
}

const IFontMetric* CanvasX11::getFontMetric() const
{
	return this;
}

void CanvasX11::setLineStyle(LineStyle lineStyle)
{
}

void CanvasX11::setPenThickness(int thickness)
{
	cairo_set_line_width(m_cr, thickness);
	m_thickness = thickness;
}

void CanvasX11::setClipRect(const Rect& rc)
{
	cairo_reset_clip(m_cr);
	cairo_rectangle(m_cr, rc.left, rc.top, rc.getWidth(), rc.getHeight());
	cairo_clip(m_cr);
}

void CanvasX11::resetClipRect()
{
	cairo_reset_clip(m_cr);
}

void CanvasX11::drawPixel(int x, int y, const Color4ub& c)
{
	log::info << L"CanvasX11::drawPixel NI" << Endl;
}

void CanvasX11::drawLine(int x1, int y1, int x2, int y2)
{
	cairo_set_source_rgba(m_cr, m_foreground.e[0] / 255.0, m_foreground.e[1] / 255.0, m_foreground.e[2] / 255.0, m_foreground.e[3] / 255.0);
	cairo_move_to(m_cr, x1, y1);
	cairo_line_to(m_cr, x2, y2);
	cairo_stroke(m_cr);
}

void CanvasX11::drawLines(const Point* pnts, int npnts)
{
	cairo_set_source_rgba(m_cr, m_foreground.e[0] / 255.0, m_foreground.e[1] / 255.0, m_foreground.e[2] / 255.0, m_foreground.e[3] / 255.0);
	cairo_move_to(m_cr, pnts[0].x, pnts[0].y);
	for (int i = 1; i < npnts; ++i)
		cairo_line_to(m_cr, pnts[i].x, pnts[i].y);
	cairo_stroke(m_cr);
}

void CanvasX11::fillCircle(int x, int y, float radius)
{
	cairo_set_source_rgba(m_cr, m_background.e[0] / 255.0, m_background.e[1] / 255.0, m_background.e[2] / 255.0, m_background.e[3] / 255.0);
	cairo_move_to(m_cr, x, y);
	cairo_arc(m_cr, x, y, radius, 0.0, TWO_PI);
	cairo_fill(m_cr);
}

void CanvasX11::drawCircle(int x, int y, float radius)
{
	cairo_set_source_rgba(m_cr, m_foreground.e[0] / 255.0, m_foreground.e[1] / 255.0, m_foreground.e[2] / 255.0, m_foreground.e[3] / 255.0);
	cairo_move_to(m_cr, x, y);
	cairo_arc(m_cr, x, y, radius, 0.0, TWO_PI);
	cairo_stroke(m_cr);
}

void CanvasX11::drawEllipticArc(int x, int y, int w, int h, float start, float end)
{
	log::info << L"CanvasX11::drawEllipticArc NI" << Endl;
}

void CanvasX11::drawSpline(const Point* pnts, int npnts)
{
	log::info << L"CanvasX11::drawSpline NI" << Endl;
}

void CanvasX11::fillRect(const Rect& rc)
{
	cairo_set_source_rgba(m_cr, m_background.e[0] / 255.0, m_background.e[1] / 255.0, m_background.e[2] / 255.0, m_background.e[3] / 255.0);
	cairo_rectangle(m_cr, rc.left, rc.top, rc.getWidth(), rc.getHeight());
	cairo_fill(m_cr);
}

void CanvasX11::fillGradientRect(const Rect& rc, bool vertical)
{
	cairo_set_source_rgba(m_cr, m_background.e[0] / 255.0, m_background.e[1] / 255.0, m_background.e[2] / 255.0, m_background.e[3] / 255.0);
	cairo_rectangle(m_cr, rc.left, rc.top, rc.getWidth(), rc.getHeight());
	cairo_fill(m_cr);
}

void CanvasX11::drawRect(const Rect& rc)
{
	cairo_set_source_rgba(m_cr, m_foreground.e[0] / 255.0, m_foreground.e[1] / 255.0, m_foreground.e[2] / 255.0, m_foreground.e[3] / 255.0);
	cairo_rectangle(m_cr, rc.left, rc.top, rc.getWidth(), rc.getHeight());
	cairo_stroke(m_cr);
}

void CanvasX11::drawRoundRect(const Rect& rc, int radius)
{
	cairo_set_source_rgba(m_cr, m_foreground.e[0] / 255.0, m_foreground.e[1] / 255.0, m_foreground.e[2] / 255.0, m_foreground.e[3] / 255.0);
	cairo_rectangle(m_cr, rc.left, rc.top, rc.getWidth(), rc.getHeight());
	cairo_stroke(m_cr);
}

void CanvasX11::drawPolygon(const Point* pnts, int count)
{
	cairo_set_source_rgba(m_cr, m_foreground.e[0] / 255.0, m_foreground.e[1] / 255.0, m_foreground.e[2] / 255.0, m_foreground.e[3] / 255.0);
	cairo_move_to(m_cr, pnts[0].x, pnts[0].y);
	for (int i = 1; i < count; ++i)
		cairo_line_to(m_cr, pnts[i].x, pnts[i].y);
	cairo_line_to(m_cr, pnts[0].x, pnts[0].y);
	cairo_stroke(m_cr);
}

void CanvasX11::fillPolygon(const Point* pnts, int count)
{
	cairo_set_source_rgba(m_cr, m_background.e[0] / 255.0, m_background.e[1] / 255.0, m_background.e[2] / 255.0, m_background.e[3] / 255.0);
	cairo_move_to(m_cr, pnts[0].x, pnts[0].y);
	for (int i = 1; i < count; ++i)
		cairo_line_to(m_cr, pnts[i].x, pnts[i].y);
	cairo_line_to(m_cr, pnts[0].x, pnts[0].y);
	cairo_fill(m_cr);
}

void CanvasX11::drawBitmap(const Point& dstAt, const Point& srcAt, const Size& size, ISystemBitmap* bitmap, uint32_t blendMode)
{
	BitmapX11* bm = static_cast< BitmapX11* >(bitmap);
	T_ASSERT (bm);

	cairo_surface_t* cs = bm->getSurface();
	if (cs == nullptr)
		return;

	cairo_set_source_surface(m_cr, cs, dstAt.x - srcAt.x, dstAt.y - srcAt.y);
	cairo_rectangle(m_cr, dstAt.x, dstAt.y, size.cx, size.cy);
	cairo_fill(m_cr);
}

void CanvasX11::drawBitmap(const Point& dstAt, const Size& dstSize, const Point& srcAt, const Size& srcSize, ISystemBitmap* bitmap, uint32_t blendMode)
{
	BitmapX11* bm = static_cast< BitmapX11* >(bitmap);
	T_ASSERT (bm);

	cairo_surface_t* cs = bm->getSurface();
	if (cs == nullptr)
		return;

	float sx = float(dstSize.cx) / srcSize.cx;
	float sy = float(dstSize.cy) / srcSize.cy;

	cairo_scale(m_cr, sx, sy);
	cairo_set_source_surface(m_cr, cs, dstAt.x / sx - srcAt.x, dstAt.y / sy - srcAt.y);

	switch (blendMode)
	{
	case BmNone:
		cairo_set_operator(m_cr, CAIRO_OPERATOR_SOURCE);
		break;

	case BmAlpha:
	case BmModulate:
		break;

	default:
		return;
	}

	cairo_rectangle(m_cr, dstAt.x / sx, dstAt.y / sy, dstSize.cx / sx, dstSize.cy / sy);
	cairo_fill(m_cr);
	cairo_identity_matrix(m_cr);
}

void CanvasX11::drawText(const Point& at, const std::wstring& text)
{
	cairo_font_extents_t x;
	cairo_font_extents(m_cr, &x);

	cairo_set_source_rgba(m_cr, m_foreground.e[0] / 255.0, m_foreground.e[1] / 255.0, m_foreground.e[2] / 255.0, m_foreground.e[3] / 255.0);
	cairo_move_to(m_cr, at.x, at.y + x.ascent);

	cairo_show_text(m_cr, wstombs(text).c_str());

	if (m_underline)
	{
		cairo_text_extents_t tx;
		cairo_text_extents(m_cr, wstombs(text).c_str(), &tx);

		int32_t thickness = std::max< int32_t >(1, x.ascent / 10);
		cairo_set_line_width(m_cr, thickness);

		cairo_move_to(m_cr, at.x, at.y + x.ascent + thickness);
		cairo_line_to(m_cr, at.x + tx.width, at.y + x.ascent + thickness);
		cairo_stroke(m_cr);

		cairo_set_line_width(m_cr, m_thickness);
	}
}

void* CanvasX11::getSystemHandle()
{
	T_FATAL_ERROR;
	return 0;
}

void CanvasX11::getAscentAndDescent(int32_t& outAscent, int32_t& outDescent) const
{
	cairo_font_extents_t x;
	cairo_font_extents(m_cr, &x);
	outAscent = (int32_t)x.ascent;
	outDescent = (int32_t)x.descent;
}

int32_t CanvasX11::getAdvance(wchar_t ch, wchar_t next) const
{
	uint8_t uc[IEncoding::MaxEncodingSize + 1] = { 0 };
	int32_t nuc = Utf8Encoding().translate(&ch, 1, uc);
	if (nuc <= 0)
		return 0;

	cairo_text_extents_t tx;
	cairo_text_extents(m_cr, (const char*)uc, &tx);

	return (int32_t)tx.x_advance;
}

int32_t CanvasX11::getLineSpacing() const
{
	cairo_font_extents_t x;
	cairo_font_extents(m_cr, &x);
	return (int32_t)x.height;
}

Size CanvasX11::getExtent(const std::wstring& text) const
{
	cairo_font_extents_t fx;
	cairo_text_extents_t tx;
	cairo_font_extents(m_cr, &fx);
	cairo_text_extents(m_cr, wstombs(text).c_str(), &tx);
	return Size(tx.width, fx.height);
}

	}
}

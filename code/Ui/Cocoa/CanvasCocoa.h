#ifndef traktor_ui_CanvasCocoa_H
#define traktor_ui_CanvasCocoa_H

#import <Cocoa/Cocoa.h>

#include "Ui/Itf/ICanvas.h"

namespace traktor
{
	namespace ui
	{

class CanvasCocoa : public ICanvas
{
public:
	CanvasCocoa(NSView* view);
	
	virtual ~CanvasCocoa();
	
	virtual void setForeground(const Color& foreground);

	virtual void setBackground(const Color& background);

	virtual void setFont(const Font& font);

	virtual void setLineStyle(LineStyle lineStyle);

	virtual void setPenThickness(int thickness);

	virtual void setClipRect(const Rect& rc);

	virtual void resetClipRect();
	
	virtual void drawPixel(int x, int y, const Color& c);

	virtual void drawLine(int x1, int y1, int x2, int y2);

	virtual void drawLines(const Point* pnts, int npnts);

	virtual void fillCircle(int x, int y, float radius);

	virtual void drawCircle(int x, int y, float radius);

	virtual void drawEllipticArc(int x, int y, int w, int h, float start, float end);

	virtual void drawSpline(const Point* pnts, int npnts);

	virtual void fillRect(const Rect& rc);

	virtual void fillGradientRect(const Rect& rc, bool vertical = true);

	virtual void drawRect(const Rect& rc);

	virtual void drawRoundRect(const Rect& rc, int radius);

	virtual void drawPolygon(const Point* pnts, int count);

	virtual void fillPolygon(const Point* pnts, int count);
	
	virtual void drawBitmap(const Point& dstAt, const Point& srcAt, const Size& size, IBitmap* bitmap, uint32_t blendMode);

	virtual void drawBitmap(const Point& dstAt, const Size& dstSize, const Point& srcAt, const Size& srcSize, IBitmap* bitmap, uint32_t blendMode);

	virtual void drawText(const Point& at, const std::wstring& text);

	virtual void drawText(const Rect& rc, const std::wstring& text, Align halign = AnLeft, Align valign = AnTop);
	
	virtual Size getTextExtent(const std::wstring& text) const;
	
private:
	NSView* m_view;
	NSColor* m_foregroundColor;
	NSColor* m_backgroundColor;
	int32_t m_clipStack;
};
	
	}
}

#endif	// traktor_ui_CanvasCocoa_H

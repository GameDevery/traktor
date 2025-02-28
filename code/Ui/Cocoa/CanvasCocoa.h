/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#import <Cocoa/Cocoa.h>

#include "Ui/Itf/ICanvas.h"
#include "Ui/Itf/IFontMetric.h"

namespace traktor
{
	namespace ui
	{

class CanvasCocoa
:	public ICanvas
,	public IFontMetric
{
public:
	explicit CanvasCocoa(NSFont* font);

	virtual ~CanvasCocoa();

	virtual void setForeground(const Color4ub& foreground) override final;

	virtual void setBackground(const Color4ub& background) override final;

	virtual void setFont(const Font& font) override final;

	virtual const IFontMetric* getFontMetric() const override final;

	virtual void setLineStyle(LineStyle lineStyle) override final;

	virtual void setPenThickness(int thickness) override final;

	virtual void setClipRect(const Rect& rc) override final;

	virtual void resetClipRect() override final;

	virtual void drawPixel(int x, int y, const Color4ub& c) override final;

	virtual void drawLine(int x1, int y1, int x2, int y2) override final;

	virtual void drawLines(const Point* pnts, int npnts) override final;

	virtual void fillCircle(int x, int y, float radius) override final;

	virtual void drawCircle(int x, int y, float radius) override final;

	virtual void drawEllipticArc(int x, int y, int w, int h, float start, float end) override final;

	virtual void drawSpline(const Point* pnts, int npnts) override final;

	virtual void fillRect(const Rect& rc) override final;

	virtual void fillGradientRect(const Rect& rc, bool vertical = true) override final;

	virtual void drawRect(const Rect& rc) override final;

	virtual void drawRoundRect(const Rect& rc, int radius) override final;

	virtual void drawPolygon(const Point* pnts, int count) override final;

	virtual void fillPolygon(const Point* pnts, int count) override final;

	virtual void drawBitmap(const Point& dstAt, const Point& srcAt, const Size& size, ISystemBitmap* bitmap, uint32_t blendMode) override final;

	virtual void drawBitmap(const Point& dstAt, const Size& dstSize, const Point& srcAt, const Size& srcSize, ISystemBitmap* bitmap, uint32_t blendMode) override final;

	virtual void drawText(const Point& at, const std::wstring& text) override final;

	virtual void* getSystemHandle() override final;

	// IFontMetric

	virtual void getAscentAndDescent(int32_t& outAscent, int32_t& outDescent) const override final;

	virtual int32_t getAdvance(wchar_t ch, wchar_t next) const override final;

	virtual int32_t getLineSpacing() const override final;

	virtual Size getExtent(const std::wstring& text) const override final;

private:
	NSColor* m_foregroundColor = nullptr;
	NSColor* m_backgroundColor = nullptr;
	NSFont* m_font = nullptr;
	bool m_haveClipper = false;
};

	}
}


/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Application.h"
#include "Ui/StyleSheet.h"
#include "Ui/Panel.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Panel", Panel, Container)

bool Panel::create(Widget* parent, const std::wstring& text, Layout* layout)
{
	if (!Container::create(parent, WsNone, layout))
		return false;

	setText(text);

	addEventHandler< PaintEvent >(this, &Panel::eventPaint);

	m_focusEventHandler = Application::getInstance()->addEventHandler< FocusEvent >(this, &Panel::eventFocus);

	return true;
}

void Panel::destroy()
{
	Application::getInstance()->removeEventHandler< FocusEvent >(m_focusEventHandler);
	Widget::destroy();
}

Size Panel::getMinimumSize() const
{
	Size titleSize = getFontMetric().getExtent(getText());
	Size sz = Container::getMinimumSize();
	sz.cx += 2;
	sz.cy += 1 + titleSize.cy;
	return sz;
}

Size Panel::getPreferredSize(const Size& hint) const
{
	Size titleSize = getFontMetric().getExtent(getText());
	Size sz = Container::getPreferredSize(hint);
	sz.cx += 2;
	sz.cy += 3 + titleSize.cy;
	return sz;
}

Rect Panel::getInnerRect() const
{
	Size titleSize = getFontMetric().getExtent(getText());
	Rect rc = Container::getInnerRect();
	rc.left += 1;
	rc.top += titleSize.cy + 3;
	rc.right -= 1;
	rc.bottom -= 1;
	return rc;
}

void Panel::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const Rect rcInner = Widget::getInnerRect();
	const StyleSheet* ss = getStyleSheet();

	canvas.fillRect(rcInner);

	bool focus = containFocus();

	std::wstring text = getText();
	Size extent = canvas.getFontMetric().getExtent(text);

	Rect rcTitle(rcInner.left, rcInner.top, rcInner.right, rcInner.top + extent.cy + 4);

	canvas.setBackground(ss->getColor(this, focus ? L"caption-background-color-focus" : L"caption-background-color-no-focus"));
	canvas.fillRect(rcTitle);

	canvas.setForeground(ss->getColor(this, focus ? L"caption-color-focus" : L"caption-color-no-focus"));
	canvas.drawText(
		rcTitle.inflate(-4, 0),
		text,
		AnLeft,
		AnCenter
	);

	Point pntBorder[5] =
	{
		Point(rcInner.left, rcInner.top),
		Point(rcInner.right - 1, rcInner.top),
		Point(rcInner.right - 1, rcInner.bottom - 1),
		Point(rcInner.left, rcInner.bottom - 1),
		Point(rcInner.left, rcInner.top)
	};
	canvas.setForeground(Color4ub(128, 128, 128));
	canvas.drawLines(pntBorder, 5);

	event->consume();
}

void Panel::eventFocus(FocusEvent* event)
{
	update();
}

	}
}

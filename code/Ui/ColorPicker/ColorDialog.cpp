/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <sstream>
#include "Core/Math/MathUtils.h"
#include "Core/Misc/String.h"
#include "Ui/Application.h"
#include "Ui/Edit.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/ColorPicker/ColorDialog.h"
#include "Ui/ColorPicker/ColorGradientControl.h"
#include "Ui/ColorPicker/ColorSliderControl.h"
#include "Ui/ColorPicker/ColorControl.h"
#include "Ui/ColorPicker/ColorEvent.h"

namespace traktor::ui
{

struct ColorGradient : public ColorSliderControl::IGradient
{
	virtual Color4ub get(int32_t at) const
	{
		int32_t rgb[] = { 255, 0, 0 };
		int32_t i = 2;
		int32_t d = 6;

		for (int32_t y = 0; y < at; ++y)
		{
			rgb[i] += d;
			if ((d < 0 && rgb[i] <= 0) || (d > 0 && rgb[i] >= 255))
			{
				rgb[i] = std::max(  0, rgb[i]);
				rgb[i] = std::min(255, rgb[i]);

				i = (i + 1) % 3;
				d = -d;
			}
		}

		return Color4ub(rgb[0], rgb[1], rgb[2], 255);
	}
};

struct AlphaGradient : public ColorSliderControl::IGradient
{
	Color4ub color;

	virtual Color4ub get(int32_t at) const
	{
		return Color4ub(color.r, color.g, color.b, at);
	}
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ColorDialog", ColorDialog, ConfigDialog)

bool ColorDialog::create(Widget* parent, const std::wstring& text, int32_t style, const Color4f& initialColor)
{
	if (!ConfigDialog::create(
		parent,
		text,
		500_ut,
		400_ut,
		style,
		new TableLayout(L"*,*,*,*", L"*", 4_ut, 4_ut)
	))
		return false;

	Color4f cl = initialColor;

	float ev = 0.0f;
	if (style & WsHDR)
	{
		ev = cl.getEV();
		cl.setEV(0.0_simd);
	}

	Color4ub club = cl.saturated().toColor4ub();

	m_gradientControl = new ColorGradientControl();
	m_gradientControl->create(this, WsDoubleBuffer | WsTabStop, club);
	m_gradientControl->addEventHandler< ColorEvent >(this, &ColorDialog::eventGradientColorSelect);

	m_colorGradient = new ColorGradient();

	m_sliderColorControl = new ColorSliderControl();
	m_sliderColorControl->create(this, WsDoubleBuffer | WsTabStop, m_colorGradient);
	m_sliderColorControl->addEventHandler< ColorEvent >(this, &ColorDialog::eventSliderColorSelect);

	if (style & WsAlpha)
	{
		m_alphaGradient = new AlphaGradient();
		m_alphaGradient->color = club;

		m_sliderAlphaControl = new ColorSliderControl();
		m_sliderAlphaControl->create(this, WsDoubleBuffer | WsTabStop, m_alphaGradient);
		m_sliderAlphaControl->setMarker(club.a);
		m_sliderAlphaControl->addEventHandler< ColorEvent >(this, &ColorDialog::eventSliderAlphaSelect);
	}

	Ref< Container > container = new Container();
	container->create(this, WsNone, new TableLayout(L"*,100", L"*,*,*,*,*", 0_ut, 4_ut));

	Ref< Static > labelR = new Static();
	labelR->create(container, L"R:");

	m_editColor[0] = new Edit();
	m_editColor[0]->create(container, toString< int32_t >(club.r), WsTabStop, new NumericEditValidator(false, 0, 255, 0));
	m_editColor[0]->addEventHandler< FocusEvent >(this, &ColorDialog::eventEditFocus);

	Ref< Static > labelG = new Static();
	labelG->create(container, L"G:");

	m_editColor[1] = new Edit();
	m_editColor[1]->create(container, toString< int32_t >(club.g), WsTabStop, new NumericEditValidator(false, 0, 255, 0));
	m_editColor[1]->addEventHandler< FocusEvent >(this, &ColorDialog::eventEditFocus);

	Ref< Static > labelB = new Static();
	labelB->create(container, L"B:");

	m_editColor[2] = new Edit();
	m_editColor[2]->create(container, toString< int32_t >(club.b), WsTabStop, new NumericEditValidator(false, 0, 255, 0));
	m_editColor[2]->addEventHandler< FocusEvent >(this, &ColorDialog::eventEditFocus);

	if (style & WsAlpha)
	{
		Ref< Static > labelA = new Static();
		labelA->create(container, L"A:");

		m_editColor[3] = new Edit();
		m_editColor[3]->create(container, toString< int32_t >(club.a), WsTabStop, new NumericEditValidator(false, 0, 255, 0));
		m_editColor[3]->addEventHandler< FocusEvent >(this, &ColorDialog::eventEditFocus);
	}

	if (style & WsHDR)
	{
		Ref< Static > labelEV = new Static();
		labelEV->create(container, L"EV:");

		m_editColor[4] = new Edit();
		m_editColor[4]->create(container, toString< int32_t >(int32_t(ev + 0.5f)), WsTabStop, new NumericEditValidator(false, -10, 10, 0));
		m_editColor[4]->addEventHandler< FocusEvent >(this, &ColorDialog::eventEditFocus);
	}

	m_colorControl = new ColorControl();
	m_colorControl->create(container, WsTabStop);
	m_colorControl->setColor(club);

	m_color = initialColor;

	fit(Container::Both);
	return true;
}

Color4f ColorDialog::getColor() const
{
	return m_color;
}

void ColorDialog::updateControls()
{
	Color4f cl = m_color;

	const float ev = cl.getEV();
	cl.setEV(0.0_simd);

	const Color4ub club = cl.saturated().toColor4ub();

	m_editColor[0]->setText(toString< int32_t >(club.r));
	m_editColor[1]->setText(toString< int32_t >(club.g));
	m_editColor[2]->setText(toString< int32_t >(club.b));

	if (m_editColor[3])
		m_editColor[3]->setText(toString< int32_t >(club.a));

	if (m_editColor[4])
		m_editColor[4]->setText(toString< int32_t >(int32_t(ev + 0.5f)));

	if (m_alphaGradient)
	{
		m_alphaGradient->color = club;
		m_sliderAlphaControl->updateGradient();
		m_sliderAlphaControl->update();
	}

	m_colorControl->setColor(club);
	m_colorControl->update();

	m_gradientControl->setColor(club, false);
	m_gradientControl->update();
}

void ColorDialog::eventGradientColorSelect(ColorEvent* event)
{
	Color4ub color = event->getColor();
	color.a = m_editColor[3] ? parseString< int32_t >(m_editColor[3]->getText()) : 255;

	m_color = Color4f::fromColor4ub(color);
	if (m_editColor[4])
	{
		const int32_t ev = parseString< int32_t >(m_editColor[4]->getText());
		m_color.setEV(Scalar(ev));
	}

	updateControls();
}

void ColorDialog::eventSliderColorSelect(ColorEvent* event)
{
	Color4ub color = event->getColor();

	m_gradientControl->setColor(color, false);
	m_gradientControl->update();

	// Cycle color through gradient control as gradient colors are primary HSL colors.
	color = m_gradientControl->getColor();
	color.a = m_editColor[3] ? parseString< int32_t >(m_editColor[3]->getText()) : 255;

	// Just copy rgb as gradient control will reset alpha.
	m_color = Color4f::fromColor4ub(color);
	if (m_editColor[4])
	{
		int32_t ev = parseString< int32_t >(m_editColor[4]->getText());
		m_color.setEV(Scalar(ev));
	}

	updateControls();
}

void ColorDialog::eventSliderAlphaSelect(ColorEvent* event)
{
	Color4ub alpha = event->getColor();

	int32_t r = parseString< int32_t >(m_editColor[0]->getText());
	int32_t g = parseString< int32_t >(m_editColor[1]->getText());
	int32_t b = parseString< int32_t >(m_editColor[2]->getText());
	int32_t a = alpha.a;

	m_color = Color4f::fromColor4ub(Color4ub(r, g, b, a)).saturated();
	if (m_editColor[4])
	{
		int32_t ev = parseString< int32_t >(m_editColor[4]->getText());
		m_color.setEV(Scalar(ev));
	}

	updateControls();
}

void ColorDialog::eventEditFocus(FocusEvent* event)
{
	if (!event->lostFocus())
		return;

	int32_t r = parseString< int32_t >(m_editColor[0]->getText());
	int32_t g = parseString< int32_t >(m_editColor[1]->getText());
	int32_t b = parseString< int32_t >(m_editColor[2]->getText());
	int32_t a = m_editColor[3] ? parseString< int32_t >(m_editColor[3]->getText()) : 255;
	int32_t ev = m_editColor[4] ? parseString< int32_t >(m_editColor[4]->getText()) : 0;

	m_color = Color4f::fromColor4ub(Color4ub(r, g, b, a)).saturated();
	m_color.setEV(Scalar(ev));
	updateControls();
}

}

/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <sstream>
#include "Core/Math/MathUtils.h"
#include "Ui/Application.h"
#include "Ui/StyleSheet.h"
#include "Ui/Envelope/EnvelopeContentChangeEvent.h"
#include "Ui/Envelope/EnvelopeControl.h"
#include "Ui/Envelope/EnvelopeEvaluator.h"
#include "Ui/Envelope/EnvelopeKey.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

float scaleValue(float value, float minValue, float maxValue)
{
	return clamp((value - minValue) / (maxValue - minValue), minValue, maxValue);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.EnvelopeControl", EnvelopeControl, Widget)

bool EnvelopeControl::create(Widget* parent, EnvelopeEvaluator* evaluator, float minValue, float maxValue, int style)
{
	if (!Widget::create(parent, style))
		return false;

	addEventHandler< MouseButtonDownEvent >(this, &EnvelopeControl::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &EnvelopeControl::eventButtonUp);
	addEventHandler< MouseMoveEvent >(this, &EnvelopeControl::eventMouseMove);
	addEventHandler< PaintEvent >(this, &EnvelopeControl::eventPaint);

	m_evaluator = evaluator;
	m_minValue = minValue;
	m_maxValue = maxValue;

	return true;
}

void EnvelopeControl::insertKey(EnvelopeKey* key)
{
	for (auto it = m_keys.begin(); it != m_keys.end(); ++it)
	{
		if (key->getT() < (*it)->getT())
		{
			m_keys.insert(it, key);
			return;
		}
	}
	m_keys.push_back(key);
}

const RefArray< EnvelopeKey >& EnvelopeControl::getKeys() const
{
	return m_keys;
}

void EnvelopeControl::addRange(const Color4ub& color, float limit0, float limit1, float limit2, float limit3)
{
	const Range r = { color, { limit0, limit1, limit2, limit3 } };
	m_ranges.push_back(r);
}

void EnvelopeControl::eventButtonDown(MouseButtonDownEvent* event)
{
	const Point pt = event->getPosition();

	m_selectedKey = 0;

	if (event->getButton() == MbtLeft)
	{
		const int32_t sx = dpi96(2);
		const int32_t sy = dpi96(2);

		for (auto key : m_keys)
		{
			const int32_t x = m_rcEnv.left + (int32_t)(m_rcEnv.getWidth() * key->getT());
			const int32_t y = m_rcEnv.bottom - (int32_t)(m_rcEnv.getHeight() * scaleValue(key->getValue(), m_minValue, m_maxValue));
			if (Rect(x - sx, y - sy, x + sx, y + sy).inside(pt))
			{
				m_selectedKey = key;
				break;
			}
		}
	}
	else if (event->getButton() == MbtRight)
	{
		m_selectedKey = new EnvelopeKey(
			(float)(pt.x - m_rcEnv.left) / m_rcEnv.getWidth(),
			(float)(pt.y - m_rcEnv.top) * (m_minValue - m_maxValue) / m_rcEnv.getHeight() + m_maxValue
		);

		insertKey(m_selectedKey);

		//Event changeEvent(this, m_selectedKey);
		//raiseEvent(&changeEvent);

		update();
	}

	if (m_selectedKey)
		setCapture();
}

void EnvelopeControl::eventButtonUp(MouseButtonUpEvent* event)
{
	if (m_selectedKey)
	{
		releaseCapture();
		m_selectedKey = nullptr;
	}
}

void EnvelopeControl::eventMouseMove(MouseMoveEvent* event)
{
	if (!m_selectedKey)
		return;

	const Point pt = event->getPosition();

	float T = float(pt.x - m_rcEnv.left) / m_rcEnv.getWidth();
	float value = float(pt.y - m_rcEnv.top) * (m_minValue - m_maxValue) / m_rcEnv.getHeight() + m_maxValue;

	T = std::max< float >(0.0f, T);
	T = std::min< float >(1.0f, T);

	value = std::max< float >(m_minValue, value);
	value = std::min< float >(m_maxValue, value);

	if (m_selectedKey->getT() != T || m_selectedKey->getValue() != value)
	{
		if (!m_selectedKey->isFixedT())
			m_selectedKey->setT(T);

		if (!m_selectedKey->isFixedValue())
			m_selectedKey->setValue(value);

		EnvelopeContentChangeEvent changeEvent(this, m_selectedKey);
		raiseEvent(&changeEvent);
	}

	update();
}

void EnvelopeControl::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const StyleSheet* ss = getStyleSheet();

	const Rect rcInner = getInnerRect();

	std::wstringstream mnv, mxv;
	mnv << m_minValue;
	mxv << m_maxValue;

	const Size mne = canvas.getFontMetric().getExtent(mnv.str());
	const Size mxe = canvas.getFontMetric().getExtent(mxv.str());

	const int32_t x = std::max< int32_t >(mne.cx, mxe.cx);
	const int32_t y = std::max< int32_t >(mne.cy, mxe.cy);
	m_rcEnv = Rect(
		rcInner.left + x + 8,
		rcInner.top + y / 2 + 4,
		rcInner.right - 4,
		rcInner.bottom - y / 2 - 4
	);

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(rcInner);

	for (std::vector< Range >::const_iterator i = m_ranges.begin(); i != m_ranges.end(); ++i)
	{
		const int32_t y0 = m_rcEnv.bottom - int32_t(m_rcEnv.getHeight() * scaleValue(i->limits[0], m_minValue, m_maxValue) + 0.5f);
		const int32_t y1 = m_rcEnv.bottom - int32_t(m_rcEnv.getHeight() * scaleValue(i->limits[1], m_minValue, m_maxValue) + 0.5f);
		const int32_t y2 = m_rcEnv.bottom - int32_t(m_rcEnv.getHeight() * scaleValue(i->limits[2], m_minValue, m_maxValue) + 0.5f);
		const int32_t y3 = m_rcEnv.bottom - int32_t(m_rcEnv.getHeight() * scaleValue(i->limits[3], m_minValue, m_maxValue) + 0.5f);

		canvas.setForeground(i->color * Color4ub(255, 255, 255, 0));
		canvas.setBackground(i->color * Color4ub(255, 255, 255, 255));

		if (y0 > y1)
		{
			canvas.fillGradientRect(Rect(
				m_rcEnv.left,
				y0,
				m_rcEnv.right,
				y1
			));
		}

		if (y1 > y2)
		{
			canvas.fillRect(Rect(
				m_rcEnv.left,
				y1,
				m_rcEnv.right,
				y2
			));
		}

		if (y2 > y3)
		{
			canvas.fillGradientRect(Rect(
				m_rcEnv.left,
				y3,
				m_rcEnv.right,
				y2
			));
		}
	}

	canvas.setForeground(ss->getColor(this, L"color"));
	canvas.drawRect(m_rcEnv.inflate(1, 1));

	canvas.drawText(
		Point(m_rcEnv.left - mne.cx - 4, m_rcEnv.bottom - mne.cy / 2),
		mnv.str()
	);
	canvas.drawText(
		Point(m_rcEnv.left - mxe.cx - 4, m_rcEnv.top - mxe.cy / 2),
		mxv.str()
	);

	const int32_t zero = m_rcEnv.bottom - int32_t(m_rcEnv.getHeight() * scaleValue(0, m_minValue, m_maxValue));
	canvas.drawLine(m_rcEnv.left, zero, m_rcEnv.right, zero);

	if (!m_keys.empty())
	{
		if (m_selectedKey)
		{
			const int32_t sx = m_rcEnv.left + int32_t(m_rcEnv.getWidth() * m_selectedKey->getT() + 0.5f);
			const int32_t sy = m_rcEnv.bottom - int32_t(m_rcEnv.getHeight() * scaleValue(m_selectedKey->getValue(), m_minValue, m_maxValue) + 0.5f);
			canvas.setForeground(ss->getColor(this, L"color-cursor"));
			canvas.drawLine(m_rcEnv.left, sy, sx, sy);
			canvas.drawLine(sx, m_rcEnv.bottom, sx, sy);
		}

		canvas.setForeground(ss->getColor(this, L"color-line"));

		const float dT = 1.0f / (rcInner.getSize().cx / 4.0f);

		int32_t px = m_rcEnv.left;
		int32_t py = m_rcEnv.bottom - int32_t(m_rcEnv.getHeight() * scaleValue(m_evaluator->evaluate(m_keys, 0.0f), m_minValue, m_maxValue));
		for (float T = dT; T <= 1.0f; T += dT)
		{
			const int32_t sx = m_rcEnv.left + int32_t(m_rcEnv.getWidth() * T + 0.5f);
			const int32_t sy = m_rcEnv.bottom - int32_t(m_rcEnv.getHeight() * scaleValue(m_evaluator->evaluate(m_keys, T), m_minValue, m_maxValue) + 0.5f);
			canvas.drawLine(px, py, sx, sy);
			px = sx;
			py = sy;
		}

		canvas.setBackground(ss->getColor(this, L"color-key"));
		for (auto key : m_keys)
		{
			const int32_t sx = m_rcEnv.left + int32_t(m_rcEnv.getWidth() * key->getT() + 0.5f);
			const int32_t sy = m_rcEnv.bottom - int32_t(m_rcEnv.getHeight() * scaleValue(key->getValue(), m_minValue, m_maxValue) + 0.5f);
			canvas.fillRect(Rect(sx - 2, sy - 2, sx + 3, sy + 3));
		}
	}

	event->consume();
}

	}
}

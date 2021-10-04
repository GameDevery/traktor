#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/ColorPicker/ColorEvent.h"
#include "Ui/ColorPicker/ColorGradientControl.h"
#include "Ui/ColorPicker/ColorUtilities.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ColorGradientControl", ColorGradientControl, Widget)

bool ColorGradientControl::create(Widget* parent, int style, const Color4ub& color)
{
	if (!Widget::create(parent, style))
		return false;

	addEventHandler< MouseButtonDownEvent >(this, &ColorGradientControl::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &ColorGradientControl::eventButtonUp);
	addEventHandler< MouseMoveEvent >(this, &ColorGradientControl::eventMouseMove);
	addEventHandler< PaintEvent >(this, &ColorGradientControl::eventPaint);

	m_gradientImage = new drawing::Image(drawing::PixelFormat::getR8G8B8(), 256, 256);
	m_gradientBitmap = new Bitmap(256, 256);

	setColor(color, true);
	update();

	return true;
}

Size ColorGradientControl::getPreferredSize(const Size& hint) const
{
	const int32_t size = dpi96(256);
	return Size(size, size);
}

void ColorGradientControl::setColor(const Color4ub& color, bool updateCursor)
{
	float rgba[4];
	color.getRGBA32F(rgba);

	float hsv[3];
	RGBtoHSV(Color4f(rgba), hsv);

	m_hue = hsv[0];

	if (updateCursor)
	{
		m_cursor.x = int(hsv[2] * 255.0f);
		m_cursor.y = int((1.0f - hsv[1]) * 255.0f);
	}

	updateGradientImage();
}

Color4ub ColorGradientControl::getColor() const
{
	Color4f clr;
	m_gradientImage->getPixelUnsafe(m_cursor.x, m_cursor.y, clr);
	return clr.toColor4ub();
}

void ColorGradientControl::updateGradientImage()
{
	Color4f color;
	float hsv[3];

	for (int y = 0; y < 256; ++y)
	{
		for (int x = 0; x < 256; ++x)
		{
			hsv[0] = m_hue;
			hsv[1] = 1.0f - float(y) / 255.0f;
			hsv[2] = float(x) / 255.0f;

			HSVtoRGB(hsv, color);

			m_gradientImage->setPixelUnsafe(x, y, color);
		}
	}

	m_gradientBitmap->copyImage(m_gradientImage);
}

void ColorGradientControl::eventButtonDown(MouseButtonDownEvent* event)
{
	m_cursor = event->getPosition();

	ColorEvent colorEvent(this, getColor());
	raiseEvent(&colorEvent);

	update();

	setCapture();
}

void ColorGradientControl::eventButtonUp(MouseButtonUpEvent* event)
{
	releaseCapture();
}

void ColorGradientControl::eventMouseMove(MouseMoveEvent* event)
{
	if (!hasCapture())
		return;

	m_cursor = event->getPosition();

	const int32_t size = dpi96(256);
	m_cursor.x = (m_cursor.x * 256) / size;
	m_cursor.y = (m_cursor.y * 256) / size;

	if (m_cursor.x < 0)
		m_cursor.x = 0;
	if (m_cursor.x > 255)
		m_cursor.x = 255;
	if (m_cursor.y < 0)
		m_cursor.y = 0;
	if (m_cursor.y > 255)
		m_cursor.y = 255;

	ColorEvent colorEvent(this, getColor());
	raiseEvent(&colorEvent);

	update();
}

void ColorGradientControl::eventPaint(PaintEvent* event)
{
	const int32_t size = dpi96(256);
	Canvas& canvas = event->getCanvas();

	canvas.drawBitmap(
		Point(0, 0),
		Size(size, size),
		Point(0, 0),
		Size(256, 256),
		m_gradientBitmap
	);

	Color4ub color = getColor();

	int average = (color.r + color.g + color.b) / 3;
	if (average < 128)
		canvas.setForeground(Color4ub(255, 255, 255));
	else
		canvas.setForeground(Color4ub(0, 0, 0));

	Point cursor(dpi96(m_cursor.x), dpi96(m_cursor.y));
	canvas.drawCircle(cursor, 5);

	event->consume();
}

	}
}

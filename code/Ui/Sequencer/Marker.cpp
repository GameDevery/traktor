#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/Sequencer/Marker.h"
#include "Ui/Sequencer/Sequence.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

const int c_sequenceHeight = 40;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Marker", Marker, Key)

Marker::Marker(int32_t time, bool movable)
:	m_time(time)
,	m_movable(movable)
{
}

void Marker::setTime(int32_t time)
{
	m_time = time;
}

int32_t Marker::getTime() const
{
	return m_time;
}

void Marker::move(int offset)
{
	if (m_movable)
		m_time += offset;
}

void Marker::getRect(const Sequence* sequence, const Rect& rcClient, Rect& outRect) const
{
	int32_t sequenceHeight = dpi96(c_sequenceHeight);
	int32_t x = sequence->clientFromTime(m_time);
	outRect.left = x - 3;
	outRect.top = rcClient.top + sequenceHeight + 2;
	outRect.right = x + 4;
	outRect.bottom = rcClient.bottom - 3;
}

void Marker::paint(SequencerControl* sequencer, ui::Canvas& canvas, const Sequence* sequence, const Rect& rcClient, int scrollOffset)
{
	int32_t sequenceHeight = dpi96(c_sequenceHeight);
	int32_t x = sequence->clientFromTime(m_time) - scrollOffset;

	Rect rc(rcClient.left + x - 3, rcClient.top + sequenceHeight + 2, rcClient.left + x + 4, rcClient.bottom - 3);

	if (sequence->getSelectedKey() != this)
	{
		canvas.setForeground(Color4ub(255, 255, 230));
		canvas.setBackground(Color4ub(230, 230, 180));
	}
	else
	{
		canvas.setForeground(Color4ub(255, 255, 180));
		canvas.setBackground(Color4ub(255, 255, 160));
	}

	canvas.fillGradientRect(rc);

	canvas.setForeground(Color4ub(0, 0, 0, 128));
	canvas.drawRect(rc);

	if (sequence->getSelectedKey() == this)
	{
		canvas.setForeground(Color4ub(255, 255, 255));
		canvas.drawRect(rc.inflate(2, 2));
	}
}

	}
}

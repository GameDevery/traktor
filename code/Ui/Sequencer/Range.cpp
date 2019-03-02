#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/Sequencer/Range.h"
#include "Ui/Sequencer/Sequence.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

const int c_sequenceHeight = 40;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Range", Range, Key)

Range::Range(int start, int end, bool movable)
:	m_start(start)
,	m_end(end)
,	m_movable(movable)
{
}

void Range::setStart(int start)
{
	m_start = start;
}

int Range::getStart() const
{
	return m_start;
}

void Range::setEnd(int end)
{
	m_end = end;
}

int Range::getEnd() const
{
	return m_end;
}

void Range::move(int offset)
{
	if (m_movable)
	{
		m_start += offset;
		m_end += offset;
	}
}

void Range::getRect(const Sequence* sequence, const Rect& rcClient, Rect& outRect) const
{
	int32_t sequenceHeight = dpi96(c_sequenceHeight);
	outRect.left = sequence->clientFromTime(m_start);
	outRect.top = rcClient.top + 2;
	outRect.right = sequence->clientFromTime(m_end);
	outRect.bottom = rcClient.top + sequenceHeight - 3;
}

void Range::paint(ui::Canvas& canvas, const Sequence* sequence, const Rect& rcClient, int scrollOffset)
{
	int32_t sequenceHeight = dpi96(c_sequenceHeight);

	int32_t x1 = sequence->clientFromTime(m_start) - scrollOffset;
	int32_t x2 = sequence->clientFromTime(m_end) - scrollOffset;

	Rect rc(rcClient.left + x1, rcClient.top + 2, rcClient.left + x2, rcClient.top + sequenceHeight - 3);

	canvas.setForeground(Color4ub(220, 255, 220));
	canvas.setBackground(Color4ub(180, 230, 180));
	canvas.fillGradientRect(rc);

	canvas.setForeground(Color4ub(0, 0, 0, 128));
	canvas.drawRect(rc);
}

	}
}

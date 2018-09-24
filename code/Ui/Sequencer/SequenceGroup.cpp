/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Drawing/Image.h"
#include "Ui/StyleBitmap.h"
#include "Ui/Sequencer/GroupVisibleEvent.h"
#include "Ui/Sequencer/SequenceGroup.h"
#include "Ui/Sequencer/SequencerControl.h"

// Resources
#include "Resources/SequenceExpand.h"
#include "Resources/SequenceCollapse.h"
#include "Resources/SequenceVisible.h"
#include "Resources/SequenceHidden.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

const int TIME_SCALE_DIVISOR = 8;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.SequenceGroup", SequenceGroup, SequenceItem)

SequenceGroup::SequenceGroup(const std::wstring& name)
:	SequenceItem(name)
,	m_expanded(true)
,	m_visible(true)
,	m_start(0)
,	m_end(100)
{
	m_imageExpand = new StyleBitmap(L"UI.SequenceExpand", c_ResourceSequenceExpand, sizeof(c_ResourceSequenceExpand));
	m_imageCollapse = new StyleBitmap(L"UI.SequenceCollapse", c_ResourceSequenceCollapse, sizeof(c_ResourceSequenceCollapse));
	m_imageVisible = new StyleBitmap(L"UI.SequenceVisible", c_ResourceSequenceVisible, sizeof(c_ResourceSequenceVisible));
	m_imageHidden = new StyleBitmap(L"UI.SequenceHidden", c_ResourceSequenceHidden, sizeof(c_ResourceSequenceHidden));
}

void SequenceGroup::expand()
{
	m_expanded = true;
}

void SequenceGroup::collapse()
{
	m_expanded = false;
}

bool SequenceGroup::isExpanded() const
{
	return m_expanded;
}

bool SequenceGroup::isCollapsed() const
{
	return !m_expanded;
}

void SequenceGroup::setVisible(bool visible)
{
	m_visible = visible;
}

bool SequenceGroup::isVisible() const
{
	return m_visible;
}

void SequenceGroup::setRange(int start, int end)
{
	m_start = start;
	m_end = end;
}

int SequenceGroup::getStart() const
{
	return m_start;
}

int SequenceGroup::getEnd() const
{
	return m_end;
}

void SequenceGroup::mouseDown(SequencerControl* sequencer, const Point& at, const Rect& rc, int button, int separator, int scrollOffset)
{
	// Select images based on the state of this group.
	IBitmap* imageExpand = m_expanded ? m_imageCollapse : m_imageExpand;
	IBitmap* imageVisible = m_visible ? m_imageVisible : m_imageHidden;

	// Calculate left edges.
	int expandLeft = rc.left + 4 + getDepth() * 16;
	int visibleLeft = expandLeft + imageExpand->getSize().cx + 4;

	// Check which icon user pressed, if any.
	if (at.x >= expandLeft && at.x <= expandLeft + int(imageExpand->getSize().cx))
	{
		int top = (rc.getHeight() - imageExpand->getSize().cy) / 2;
		if (at.y >= top && at.y <= top + int(imageExpand->getSize().cy))
			m_expanded = !m_expanded;
	}
	else if (at.x >= visibleLeft && at.x <= visibleLeft + int(imageVisible->getSize().cx))
	{
		int top = (rc.getHeight() - imageVisible->getSize().cy) / 2;
		if (at.y >= top && at.y <= top + int(imageVisible->getSize().cy))
			m_visible = !m_visible;

		GroupVisibleEvent groupVisibleEvent(sequencer, this, m_visible);
		sequencer->raiseEvent(&groupVisibleEvent);
	}
}

void SequenceGroup::mouseUp(SequencerControl* sequencer, const Point& at, const Rect& rc, int button, int separator, int scrollOffset)
{
}

void SequenceGroup::mouseMove(SequencerControl* sequencer, const Point& at, const Rect& rc, int button, int separator, int scrollOffset)
{
}

void SequenceGroup::paint(SequencerControl* sequencer, Canvas& canvas, const Rect& rc, int separator, int scrollOffset)
{
	// Select images based on the state of this group.
	IBitmap* imageExpand = m_expanded ? m_imageCollapse : m_imageExpand;
	IBitmap* imageVisible = m_visible ? m_imageVisible : m_imageHidden;

	// Calculate left edges.
	int expandLeft = rc.left + 4 + getDepth() * 16;
	int visibleLeft = expandLeft + imageExpand->getSize().cx + 4;
	int textLeft = visibleLeft + imageVisible->getSize().cx + 4;

	// Draw sequence group background.
	if (!isSelected())
	{
		canvas.setForeground(Color4ub(250, 249, 250));
		canvas.setBackground(Color4ub(238, 237, 240));
		canvas.fillGradientRect(rc);
	}
	else
	{
		canvas.setBackground(Color4ub(226, 229, 238));
		canvas.fillRect(rc);
	}

	canvas.setForeground(Color4ub(128, 128, 128));
	canvas.drawLine(rc.left, rc.bottom - 1, rc.right, rc.bottom - 1);
	canvas.drawLine(separator - 1, rc.top, separator - 1, rc.bottom);

	canvas.setClipRect(Rect(
		rc.left,
		rc.top,
		rc.left + separator - 2,
		rc.bottom
	));

	// Draw sequence group text.
	canvas.setForeground(Color4ub(0, 0, 0));
	Size ext = canvas.getFontMetric().getExtent(getName());
	canvas.drawText(
		Point(
			textLeft,
			rc.top + (rc.getHeight() - ext.cy) / 2
		),
		getName()
	);

	// Draw expand/fold icon.
	if (!getChildItems().empty())
	{
		canvas.drawBitmap(
			Point(
				expandLeft,
				rc.top + (rc.getHeight() - imageExpand->getSize().cy) / 2
			),
			Point(0, 0),
			imageExpand->getSize(),
			imageExpand
		);
	}

	// Draw visible icon.
	canvas.drawBitmap(
		Point(
			visibleLeft,
			rc.top + (rc.getHeight() - imageVisible->getSize().cy) / 2
		),
		Point(0, 0),
		imageVisible->getSize(),
		imageVisible
	);

	// Draw range.
	canvas.setClipRect(Rect(
		rc.left + separator,
		rc.top,
		rc.right,
		rc.bottom
	));

	int start = separator + m_start / TIME_SCALE_DIVISOR - scrollOffset;
	int end = separator + m_end / TIME_SCALE_DIVISOR - scrollOffset;
	int y = rc.getCenter().y;
	
	canvas.setBackground(Color4ub(80, 80, 80));
	canvas.drawLine(start, y - 2, start, y + 3);
	canvas.drawLine(start, y, end, y);
	canvas.drawLine(end, y - 2, end, y + 3);
}

	}
}

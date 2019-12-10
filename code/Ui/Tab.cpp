#include <algorithm>
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/StyleBitmap.h"
#include "Ui/Tab.h"
#include "Ui/TabPage.h"
#include "Ui/Font.h"

#include "Resources/TabClose.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

const int32_t c_tabHeight = 21;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Tab", Tab, Widget)

Tab::Tab()
:	m_imageWidth(0)
,	m_imageHeight(0)
,	m_closeButton(false)
,	m_drawBorder(false)
,	m_drawLine(false)
,	m_bottom(false)
{
}

bool Tab::create(Widget* parent, int32_t style)
{
	if (!Widget::create(parent, style & ~WsBorder))
		return false;

	addEventHandler< MouseTrackEvent >(this, &Tab::eventMouseTrack);
	addEventHandler< MouseMoveEvent >(this, &Tab::eventMouseMove);
	addEventHandler< MouseButtonDownEvent >(this, &Tab::eventButtonDown);
	addEventHandler< SizeEvent >(this, &Tab::eventSize);
	addEventHandler< PaintEvent >(this, &Tab::eventPaint);

	m_fontBold = getFont();
	m_fontBold.setBold(true);

	m_closeButton = bool((style & WsCloseButton) == WsCloseButton);
	m_drawBorder = bool((style & WsBorder) == WsBorder);
	m_drawLine = bool((style & WsLine) == WsLine);
	m_bottom = bool((style & WsBottom) == WsBottom);

	m_bitmapClose = new StyleBitmap(L"UI.TabClose", c_ResourceTabClose, sizeof(c_ResourceTabClose));

	return true;
}

Rect Tab::getInnerRect() const
{
	return m_innerRect;
}

int32_t Tab::addImage(IBitmap* image, int32_t imageCount)
{
	uint32_t width = 0, height = 0;

	// Resize existing image.
	if (m_bitmapImages)
	{
		width = m_bitmapImages->getSize().cx + image->getSize().cx;
		height = std::max(m_bitmapImages->getSize().cy, image->getSize().cy);

		Ref< ui::Bitmap > newImage = new ui::Bitmap(width, height);
		newImage->copyImage(m_bitmapImages->getImage());
		newImage->copySubImage(image->getImage(), Rect(Point(0, 0), image->getSize()), Point(m_bitmapImages->getSize().cx, 0));
		m_bitmapImages = newImage;
	}
	else
	{
		m_bitmapImages = image;
		m_imageWidth = std::max< uint32_t >(m_imageWidth, m_bitmapImages->getSize().cx / imageCount);
		m_imageHeight = std::max< uint32_t >(m_imageHeight, m_bitmapImages->getSize().cy);
	}

	return 0;
}

int32_t Tab::addPage(TabPage* page)
{
	if (page == 0)
		return -1;

	for (auto& ps : m_pages)
		ps.depth++;

	m_pages.push_back(PageState(page));

	if (m_selectedPage)
		m_selectedPage->setVisible(false);

	page->setRect(m_innerRect);
	page->setVisible(true);

	m_selectedPage = page;

	return int32_t(m_pages.size() - 1);
}

int32_t Tab::getPageCount() const
{
	return (int32_t)m_pages.size();
}

TabPage* Tab::getPage(int32_t index) const
{
	if (index >= 0 && index < (int32_t)m_pages.size())
		return m_pages[index].page;
	else
		return nullptr;
}

TabPage* Tab::getPageAt(const Point& position) const
{
	if (position.y >= dpi96(c_tabHeight))
		return nullptr;

	for (const auto& ps : m_pages)
	{
		if (position.x <= ps.right)
			return ps.page;
	}

	return nullptr;
}

void Tab::removePage(TabPage* page)
{
	auto it = std::find(m_pages.begin(), m_pages.end(), PageState(page));
	if (it == m_pages.end())
		return;

	m_pages.erase(it);

	if (page == m_selectedPage)
	{
		page->setVisible(false);

		// Decrement all depths, to one found with zero depth is the new active page.
		for (auto& ps : m_pages)
			ps.depth--;

		PageState* newPageState = findPageState(0);
		if (newPageState)
		{
			m_selectedPage = newPageState->page;
			m_selectedPage->setVisible(true);
		}
		else
			m_selectedPage = nullptr;
	}
}

void Tab::removeAllPages()
{
	m_pages.clear();
}

void Tab::setActivePage(TabPage* page)
{
	if (page == m_selectedPage)
		return;

	if (m_selectedPage)
		m_selectedPage->setVisible(false);

	if ((m_selectedPage = page) != nullptr)
	{
		PageState* state = findPageState(page);
		T_ASSERT(state);

		int32_t depth = state->depth;
		for (auto& ps : m_pages)
		{
			if (ps.depth <= depth)
				ps.depth++;
		}

		state->depth = 0;

		m_selectedPage->setVisible(true);
	}
}

TabPage* Tab::getActivePage()
{
	return m_selectedPage;
}

TabPage* Tab::cycleActivePage(bool forward)
{
	if (m_pages.size() < 2)
		return m_selectedPage;

	int32_t maxDepth = (int32_t)(m_pages.size() - 1);

	if (forward)
	{
		for (auto& ps : m_pages)
		{
			if (--ps.depth < 0)
				ps.depth = maxDepth;
		}
	}
	else
	{
		for (auto& ps : m_pages)
		{
			if (++ps.depth > maxDepth)
				ps.depth = 0;
		}
	}

	if (m_selectedPage)
		m_selectedPage->setVisible(false);

	PageState* pageState = findPageState(0);
	T_ASSERT(pageState);

	m_selectedPage = pageState->page;
	m_selectedPage->setVisible(true);

	update();

	return m_selectedPage;
}

Size Tab::getMinimumSize() const
{
	return Size(256, 256);
}

Size Tab::getPreferedSize() const
{
	return Size(256, 256);
}

void Tab::eventMouseTrack(MouseTrackEvent* event)
{
	if (!event->entered())
	{
		if (m_hoverPage)
		{
			m_hoverPage = nullptr;
			update();
		}
	}
}

void Tab::eventMouseMove(MouseMoveEvent* event)
{
	Point pnt = event->getPosition();
	Rect inner = Widget::getInnerRect();

	int32_t y0, y1;
	if (!m_bottom)
	{
		y0 = inner.top;
		y1 = inner.top + dpi96(c_tabHeight);
	}
	else
	{
		y0 = inner.bottom - dpi96(c_tabHeight);
		y1 = inner.bottom;
	}

	if (pnt.y >= y0 && pnt.y <= y1)
	{
		Ref< TabPage > hoverPage = getPageAt(pnt);
		if (hoverPage != m_hoverPage)
		{
			m_hoverPage = hoverPage;
			update();
		}
	}
}

void Tab::eventButtonDown(MouseButtonDownEvent* event)
{
	Point pnt = event->getPosition();
	Rect inner = Widget::getInnerRect();

	int32_t y0, y1;
	if (!m_bottom)
	{
		y0 = inner.top;
		y1 = inner.top + dpi96(c_tabHeight);
	}
	else
	{
		y0 = inner.bottom - dpi96(c_tabHeight);
		y1 = inner.bottom;
	}

	if (pnt.y >= y0 && pnt.y <= y1)
	{
		PageState* selectedPageState = nullptr;
		for (auto& ps : m_pages)
		{
			if (pnt.x <= ps.right)
			{
				selectedPageState = &ps;
				break;
			}
		}

		bool closed = false;
		if (m_closeButton && selectedPageState)
		{
			if (pnt.x >= selectedPageState->right - dpi96(16) && pnt.x < selectedPageState->right)
			{
				TabCloseEvent closeEvent(this, selectedPageState->page);
				raiseEvent(&closeEvent);
				closed = true;
			}
		}

		if (!closed && selectedPageState && selectedPageState->page != m_selectedPage)
		{
			if (m_selectedPage)
				m_selectedPage->setVisible(false);

			TabSelectionChangeEvent selectionChangeEvent(this, selectedPageState->page);
			raiseEvent(&selectionChangeEvent);

			if ((m_selectedPage = selectedPageState->page) != nullptr)
			{
				for (auto& ps : m_pages)
				{
					if (ps.depth < selectedPageState->depth)
						ps.depth++;
				}
				selectedPageState->depth = 0;
				m_selectedPage->setVisible(true);
			}
		}

		setFocus();
		update();
	}
}

void Tab::eventSize(SizeEvent* event)
{
	m_innerRect = Widget::getInnerRect();
	if (!m_bottom)
		m_innerRect.top += dpi96(c_tabHeight);
	else
		m_innerRect.bottom -= dpi96(c_tabHeight);

	if (m_drawBorder)
		m_innerRect = m_innerRect.inflate(-1, -1);

	for (Widget* child = getFirstChild(); child != 0; child = child->getNextSibling())
		child->setRect(m_innerRect);

	update();

	event->consume();
}

void Tab::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	Rect rcPaint = event->getUpdateRect();
	Rect rcInner = Widget::getInnerRect();

	const StyleSheet* ss = Application::getInstance()->getStyleSheet();

	int32_t y0, y1;
	if (!m_bottom)
	{
		y0 = rcInner.top;
		y1 = rcInner.top + dpi96(c_tabHeight);
	}
	else
	{
		y0 = rcInner.bottom - dpi96(c_tabHeight);
		y1 = rcInner.bottom;
	}

	Rect rcTabs(rcInner.left, y0, rcInner.right, y1);

	// Fill tab background.
	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(rcTabs);

	// Draw tab pages.
	if (!m_pages.empty())
	{
		int32_t left = rcTabs.left;
		for (auto& ps : m_pages)
		{
			const TabPage* page = ps.page;
			const std::wstring text = page->getText();

			Size sizText = canvas.getFontMetric().getExtent(text);

			int32_t tabWidthNoMargin = sizText.cx;
			if (m_closeButton)
			{
				Size closeSize = m_bitmapClose->getSize();
				tabWidthNoMargin += closeSize.cx + dpi96(4);
			}

			int32_t tabWidth = tabWidthNoMargin + dpi96(4 * 2);

			// Save right separator position in vector.
			ps.right = left + tabWidth;

			// Draw only those tabs that are visible.
			if (ps.right < rcTabs.right)
			{
				// Calculate tab item rectangle.
				Rect rcTab(
					left,
					rcTabs.top,
					left + tabWidth,
					rcTabs.bottom
				);
				if (m_drawLine)
				{
					if (!m_bottom)
						rcTab.bottom -= 2;
					else
						rcTab.top += 2;
				}

				// Highlight selected tab.
				if (page == m_selectedPage)
				{
					canvas.setBackground(ss->getColor(this, L"tab-background-color"));
					canvas.fillRect(rcTab);
				}
				else if (page == m_hoverPage)
				{
					canvas.setBackground(ss->getColor(this, L"tab-background-color-hover"));
					canvas.fillRect(rcTab);
				}

				// Draw close button.
				if (m_closeButton && (page == m_selectedPage || page == m_hoverPage))
				{
					Size closeSize = m_bitmapClose->getSize();
					canvas.drawBitmap(
						Point(rcTab.right - closeSize.cx - dpi96(4), rcTab.getCenter().y - closeSize.cy / 2 + dpi96(1)),
						Point(0, 0),
						closeSize,
						m_bitmapClose,
						BmAlpha
					);
				}

				// Draw text.
				Rect rcTabText(
					left + dpi96(4),
					rcTab.top,
					left + dpi96(4) + sizText.cx,
					rcTab.bottom
				);
				if (isEnable())
				{
					canvas.setForeground(ss->getColor(this, (page == m_selectedPage || page == m_hoverPage) ? L"tab-color-active" : L"tab-color-inactive"));
					canvas.drawText(rcTabText, text, AnLeft, AnCenter);
				}
				else
				{
					canvas.setForeground(Color4ub(250, 250, 250));
					canvas.drawText(rcTabText, text, AnLeft, AnCenter);

					rcTabText.left -= 1;
					rcTabText.top -= 1;
					canvas.setForeground(Color4ub(120, 120, 120));
					canvas.drawText(rcTabText, text, AnLeft, AnCenter);
				}
			}

			left += tabWidth;
		}
	}
	else
	{
		// No tab pages, fill solid background.
		Rect rcTabItem(
			rcInner.left,
			m_bottom ? rcInner.top : rcInner.top + dpi96(c_tabHeight),
			rcInner.right,
			m_bottom ? rcInner.bottom - dpi96(c_tabHeight) : rcInner.bottom
		);
		canvas.setBackground(ss->getColor(this, L"background-color"));
		canvas.fillRect(rcTabItem);
	}

	// Draw line.
	if (m_drawLine)
	{
		canvas.setForeground(ss->getColor(this, L"tab-line-color"));
		if (!m_bottom)
		{
			canvas.drawLine(rcTabs.left, rcTabs.bottom - 2, rcTabs.right, rcTabs.bottom - 2);
			canvas.drawLine(rcTabs.left, rcTabs.bottom - 1, rcTabs.right, rcTabs.bottom - 1);
		}
		else
		{
			canvas.drawLine(rcTabs.left, rcTabs.top, rcTabs.right, rcTabs.top);
			canvas.drawLine(rcTabs.left, rcTabs.top + 1, rcTabs.right, rcTabs.top + 1);
		}
	}

	// Draw surrounding gray border.
	if (m_drawBorder)
	{
		canvas.setForeground(ss->getColor(this, L"border-color"));
		canvas.drawRect(rcInner);
	}

	event->consume();
}

Tab::PageState* Tab::findPageState(const TabPage* page)
{
	for (auto& ps : m_pages)
	{
		if (ps.page == page)
			return &ps;
	}
	return nullptr;
}

Tab::PageState* Tab::findPageState(int32_t depth)
{
	for (auto& ps : m_pages)
	{
		if (ps.depth == depth)
			return &ps;
	}
	return nullptr;
}

Tab::PageState::PageState(TabPage* page_, int32_t right_)
:	page(page_)
,	right(right_)
,	depth(0)
{
}

bool Tab::PageState::operator == (const Tab::PageState& rh) const
{
	return page == rh.page;
}

	}
}


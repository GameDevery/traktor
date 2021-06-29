#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/StaticMap.h"
#include "Core/Thread/Semaphore.h"
#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Guid;

	namespace ui
	{

class IBitmap;
class ScrollBar;

/*! Log list control.
 * \ingroup UI
 */
class T_DLLCLASS LogList : public Widget
{
	T_RTTI_CLASS;

public:
	enum LogLevel : uint8_t
	{
		LvInfo		= 1 << 0,
		LvWarning	= 1 << 1,
		LvError		= 1 << 2,
		LvDebug		= 1 << 3
	};

	struct ISymbolLookup
	{
		virtual bool lookupLogSymbol(const Guid& symbolId, std::wstring& outSymbol) const = 0;
	};

	bool create(Widget* parent, int style, const ISymbolLookup* lookup);

	void add(uint32_t threadId, LogLevel level, const std::wstring& text);

	void removeAll();

	void setFilter(uint8_t filter);

	uint8_t getFilter() const;

	bool copyLog(uint8_t filter = ~0);

	virtual Size getPreferedSize() const override;

private:
	struct Entry
	{
		uint32_t threadId;
		LogLevel level;
		std::wstring text;
	};

	typedef AlignedVector< Entry > log_list_t;

	const ISymbolLookup* m_lookup = nullptr;
	Ref< ScrollBar > m_scrollBar;
	Ref< IBitmap > m_icons;
	log_list_t m_pending;
	Semaphore m_pendingLock;
	log_list_t m_logFull;
	log_list_t m_logFiltered;
	uint32_t m_logCount[3] = { 0, 0, 0 };
	StaticMap< uint32_t, uint32_t, 128 > m_threadIndices;
	int32_t m_itemHeight = 0;
	uint8_t m_filter = LvInfo | LvWarning | LvError;
	uint32_t m_nextThreadIndex = 0;
	int32_t m_selectedEntry = -1;

	void updateScrollBar();

	void eventPaint(PaintEvent* event);

	void eventSize(SizeEvent* event);

	void eventMouseButtonDown(MouseButtonDownEvent* event);

	void eventMouseWheel(MouseWheelEvent* event);

	void eventScroll(ScrollEvent* event);
};

	}
}


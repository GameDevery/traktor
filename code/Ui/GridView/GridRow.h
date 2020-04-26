#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Ui/Auto/AutoWidgetCell.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class GridItem;

/*! Grid row.
 * \ingroup UI
 */
class T_DLLCLASS GridRow : public AutoWidgetCell
{
	T_RTTI_CLASS;

public:
	enum RowStates
	{
		RsExpanded = 1,
		RsSelected = 2
	};

	GridRow(uint32_t initialState = RsExpanded);

	virtual ~GridRow();

	void setEditable(bool editable);

	bool getEditable() const { return m_editable; }

	void setState(uint32_t state);

	uint32_t getState() const { return m_state; }

	void setBackground(const Color4ub& background);

	const Color4ub& getBackground() const { return m_background; }

	void setMinimumHeight(int32_t minimumHeight);

	const int32_t getMinimumHeight() const { return m_minimumHeight; }

	int32_t getHeight() const;

	uint32_t add(GridItem* item);

	void set(uint32_t index, GridItem* item);

	Ref< GridItem > get(uint32_t index) const;

	const RefArray< GridItem >& get() const { return m_items; }

	uint32_t getIndex(const GridItem* item) const;

	void addChild(GridRow* row);

	void insertChildBefore(GridRow* insertBefore, GridRow* row);

	void insertChildAfter(GridRow* insertAfter, GridRow* row);

	void removeChild(GridRow* row);

	void removeAllChildren();

	GridRow* getParent() { return m_parent; }

	const RefArray< GridRow >& getChildren() const { return m_children; }

private:
	friend class GridView;

	bool m_editable;
	uint32_t m_state;
	Color4ub m_background;
	int32_t m_minimumHeight;
	RefArray< GridItem > m_items;
	GridRow* m_parent;
	RefArray< GridRow > m_children;
	Point m_mouseDownPosition;
	int32_t m_editMode;

	int32_t getDepth() const;

	virtual void placeCells(AutoWidget* widget, const Rect& rect) override final;

	virtual void interval() override final;

	virtual void mouseDown(MouseButtonDownEvent* event, const Point& position) override final;

	virtual void mouseUp(MouseButtonUpEvent* event, const Point& position) override final;

	virtual void mouseDoubleClick(MouseDoubleClickEvent* event, const Point& position) override final;

	virtual void mouseMove(MouseMoveEvent* event, const Point& position) override final;

	virtual void paint(Canvas& canvas, const Rect& rect) override final;
};

	}
}


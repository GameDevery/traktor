#pragma once

#include <string>
#include "Core/Object.h"
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
	namespace ui
	{

class Canvas;
class Rect;
class Command;
class MiniButton;
class MouseEvent;
class PropertyList;

/*! Property item.
 * \ingroup UI
 */
class T_DLLCLASS PropertyItem : public Object
{
	T_RTTI_CLASS;

public:
	PropertyItem(const std::wstring& text);

	void setText(const std::wstring& text);

	const std::wstring& getText() const;

	void setVisible(bool visible);

	bool isVisible() const;

	void expand();

	void collapse();

	bool isExpanded() const;

	bool isCollapsed() const;

	void setSelected(bool selected);

	bool isSelected() const;

	int getDepth() const;

	PropertyItem* getParentItem() const;

	RefArray< PropertyItem >& getChildItems();

	const RefArray< PropertyItem >& getChildItems() const;

protected:
	friend class AutoPropertyList;
	friend class PropertyList;

	void setPropertyList(PropertyList* propertyList);

	PropertyList* getPropertyList() const;

	void notifyUpdate();

	void notifyCommand(const Command& command);

	void notifyChange();

	void addChildItem(PropertyItem* childItem);

	void removeChildItem(PropertyItem* childItem);

	virtual bool needRemoveChildButton() const;

	virtual void createInPlaceControls(PropertyList* parent);

	virtual void destroyInPlaceControls();

	virtual void resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects);

	virtual void mouseButtonDown(MouseButtonDownEvent* event);

	virtual void mouseButtonUp(MouseButtonUpEvent* event);

	virtual void doubleClick(MouseDoubleClickEvent* event);

	virtual void mouseMove(MouseMoveEvent* event);

	virtual void paintBackground(Canvas& canvas, const Rect& rc);

	virtual void paintText(Canvas& canvas, const Rect& rc);

	virtual void paintValue(Canvas& canvas, const Rect& rc);

	virtual bool copy();

	virtual bool paste();

private:
	PropertyList* m_propertyList;
	std::wstring m_text;
	bool m_visible;
	bool m_expanded;
	bool m_selected;
	PropertyItem* m_parent;
	RefArray< PropertyItem > m_childItems;
	Ref< MiniButton > m_buttonRemove;

	void updateChildrenInPlaceControls();

	void eventClick(ButtonClickEvent* event);
};

	}
}


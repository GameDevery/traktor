#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Color4ub.h"
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

class TreeView;

/*! \brief Tree view item.
 * \ingroup UI
 */
class T_DLLCLASS TreeViewItem : public AutoWidgetCell
{
	T_RTTI_CLASS;

public:
	void setText(const std::wstring& text);

	std::wstring getText() const;

	void setBold(bool bold);

	bool isBold() const;

	void setTextOutlineColor(const Color4ub& outlineColor);

	const Color4ub& getTextOutlineColor() const;

	void removeAllImages();

	int32_t getImageCount() const;

	void setImage(int32_t index, int32_t image, int32_t expandedImage = -1, int32_t overlayImage = -1);

	int32_t getImage(int32_t index) const;

	int32_t getExpandedImage(int32_t index) const;

	int32_t getOverlayImage(int32_t index) const;

	bool isExpanded() const;

	void expand(bool recursive = false);

	bool isCollapsed() const;

	void collapse(bool recursive = false);

	bool isEnabled() const;

	void enable();

	void disable();

	bool isSelected() const;

	void select();

	void unselect();

	bool isVisible() const;

	void show();

	void setEditable(bool editable);

	bool isEditable() const;

	bool edit();

	void sort(bool recursive);

	TreeViewItem* getParent() const;

	TreeViewItem* getPreviousSibling(TreeViewItem* child) const;

	TreeViewItem* getNextSibling(TreeViewItem* child) const;

	bool hasChildren() const;

	const RefArray< TreeViewItem >& getChildren() const;

	Ref< TreeViewItem > findChild(const std::wstring& childPath);

	/*! \brief Get path to this item.
	 *
	 * Path is separated with / for each level.
	 */
	std::wstring getPath() const;

private:
	friend class TreeView;

	struct Image
	{
		int32_t image;
		int32_t expanded;
		int32_t overlay;

		Image()
		:	image(-1)
		,	expanded(-1)
		,	overlay(-1)
		{
		}
	};

	TreeView* m_view;
	TreeViewItem* m_parent;
	std::wstring m_text;
	Color4ub m_outlineColor;
	AlignedVector< Image > m_images;
	bool m_bold;
	bool m_expanded;
	bool m_enabled;
	bool m_selected;
	bool m_editable;
	Point m_mouseDownPosition;
	int32_t m_editMode;
	int32_t m_dragMode;
	RefArray< TreeViewItem > m_children;

	TreeViewItem(TreeView* view, TreeViewItem* parent, const std::wstring& text, int32_t image, int32_t expandedImage = -1, int32_t overlayImage = -1);

	TreeViewItem(TreeView* view, TreeViewItem* parent, const std::wstring& text);

	int32_t calculateDepth() const;

	Rect calculateExpandRect() const;

	Rect calculateImageRect() const;

	Rect calculateLabelRect() const;

	int32_t calculateWidth() const;

	virtual void interval() override final;

	virtual void mouseDown(MouseButtonDownEvent* event, const Point& position) override final;

	virtual void mouseUp(MouseButtonUpEvent* event, const Point& position) override final;

	virtual void mouseDoubleClick(MouseDoubleClickEvent* event, const Point& position) override final;

	virtual void mouseMove(MouseMoveEvent* event, const Point& position) override final;

	virtual void paint(Canvas& canvas, const Rect& rect) override final;
};

	}
}


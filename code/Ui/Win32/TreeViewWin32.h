#ifndef traktor_ui_TreeViewWin32_H
#define traktor_ui_TreeViewWin32_H

#include "Ui/Win32/WidgetWin32Impl.h"
#include "Ui/Itf/ITreeView.h"

namespace traktor
{
	namespace ui
	{

class TreeViewItemWin32;

class TreeViewWin32 : public WidgetWin32Impl< ITreeView >
{
public:
	TreeViewWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, int style);

	virtual void destroy();

	virtual int addImage(IBitmap* image, int imageCount);

	virtual Ref< TreeViewItem > createItem(TreeViewItem* parent, const std::wstring& text, int image, int expandedImage);

	virtual void removeItem(TreeViewItem* item);

	virtual void removeAllItems();

	virtual Ref< TreeViewItem > getRootItem() const;

	virtual Ref< TreeViewItem > getSelectedItem() const;

private:
	friend class TreeViewItemWin32;

	HIMAGELIST m_hImageList;
	std::map< HTREEITEM, Ref< TreeViewItemWin32 > > m_items;
	Ref< TreeViewItemWin32 > m_dragItem;

	Ref< TreeViewItemWin32 > getFromHandle(HTREEITEM hItem) const;

	LRESULT eventButtonUp(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);

	LRESULT eventNotify(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);
};

	}
}

#endif	// traktor_ui_TreeViewWin32_H

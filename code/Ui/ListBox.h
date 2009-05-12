#ifndef traktor_ui_ListBox_H
#define traktor_ui_ListBox_H

#include <map>
#include "Core/Heap/Ref.h"
#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

/*! \brief List box.
 * \ingroup UI
 */
class T_DLLCLASS ListBox : public Widget
{
	T_RTTI_CLASS(ListBox)

public:
	enum Styles
	{
		WsSingle = 0,
		WsMultiple = WsUser,
		WsExtended = (WsUser << 1),
		WsSort = (WsUser << 2),
		WsDefault = WsClientBorder | WsSingle
	};

	bool create(Widget* parent, const std::wstring& text = L"", int style = WsDefault);
	
	virtual Size getPreferedSize() const;

	int add(const std::wstring& item, Object* data = 0);

	bool remove(int index);

	void removeAll();

	int count() const;

	void setItem(int index, const std::wstring& item);

	void setData(int index, Object* data);

	std::wstring getItem(int index) const;

	Object* getData(int index) const;

	void select(int index);

	bool selected(int index) const;

	int getSelected(std::vector< int >& selected) const;

	int getSelected() const;

	std::wstring getSelectedItem() const;

	Object* getSelectedData() const;

	int getItemHeight() const;

	Rect getItemRect(int index) const;

	void addSelectEventHandler(EventHandler* eventHandler);

private:
	std::map< int, Ref< Object > > m_data;
};

	}
}

#endif	// traktor_ui_ListBox_H

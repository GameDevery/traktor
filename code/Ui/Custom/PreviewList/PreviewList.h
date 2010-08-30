#ifndef traktor_ui_custom_PreviewList_H
#define traktor_ui_custom_PreviewList_H

#include "Core/Ref.h"
#include "Ui/Custom/Auto/AutoWidget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

class PreviewItems;

class T_DLLCLASS PreviewList : public ui::custom::AutoWidget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, uint32_t style);

	void setItems(PreviewItems* items);

	Ref< PreviewItems > getItems() const;

	void addSelectEventHandler(EventHandler* eventHandler);

private:
	Ref< PreviewItems > m_items;

	virtual void layoutCells(const Rect& rc);

	void eventButtonDown(Event* event);
};

		}
	}
}

#endif	// traktor_ui_custom_PreviewList_H

#ifndef traktor_ui_custom_LayerControl_H
#define traktor_ui_custom_LayerControl_H

#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Bitmap;
class ScrollBar;

		namespace custom
		{

class LayerItem;

/*! \brief Layer control.
 * \ingroup UIC
 */
class T_DLLCLASS LayerControl : public Widget
{
	T_RTTI_CLASS(LayerControl)

public:
	enum GetSequenceFlags
	{
		GfDefault = 0,
		GfDescendants = 1,
		GfSelectedOnly = 2
	};

	bool create(Widget* parent, int style = WsNone);

	void addLayerItem(LayerItem* layerItem);

	void removeLayerItem(LayerItem* layerItem);

	void removeAllLayerItems();

	RefArray< LayerItem >& getLayerItems();

	int getItems(RefArray< LayerItem >& outItems, int flags);

	LayerItem* getLayerItem(int index, bool includeChildren = true);

	void addSelectEventHandler(EventHandler* eventHandler);

	void addChangeEventHandler(EventHandler* eventHandler);

	virtual Size getPreferedSize() const;

private:
	Ref< ScrollBar > m_scrollBar;
	RefArray< LayerItem > m_layers;
	Ref< Bitmap > m_imageVisible;
	Ref< Bitmap > m_imageHidden;

	void updateScrollBar();

	void paintItem(Canvas& canvas, Rect& rcItem, LayerItem* item, int childLevel);

	void eventScroll(Event* event);

	void eventSize(Event* event);

	void eventButtonDown(Event* event);

	void eventPaint(Event* event);
};

		}
	}
}

#endif	// traktor_ui_custom_LayerControl_H

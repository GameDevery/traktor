#include "Ui/Layer/LayerContentChangeEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.LayerContentChangeEvent", LayerContentChangeEvent, ContentChangeEvent)

LayerContentChangeEvent::LayerContentChangeEvent(EventSubject* sender, LayerItem* item)
:	ContentChangeEvent(sender)
,	m_item(item)
{
}

LayerItem* LayerContentChangeEvent::getItem() const
{
	return m_item;
}

	}
}

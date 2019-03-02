#pragma once

#include "Ui/Events/ContentChangeEvent.h"

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

/*! \brief
 * \ingroup UI
 */
class T_DLLCLASS GridItemContentChangeEvent : public ContentChangeEvent
{
	T_RTTI_CLASS;

public:
	GridItemContentChangeEvent(EventSubject* sender, GridItem* item);

	GridItem* getItem() const;

private:
	Ref< GridItem > m_item;
};

	}
}


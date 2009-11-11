#ifndef traktor_ui_Container_H
#define traktor_ui_Container_H

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
	
class Layout;
	
/*! \brief Layout container.
 * \ingroup UI
 */
class T_DLLCLASS Container : public Widget
{
	T_RTTI_CLASS(Container)

public:
	bool create(Widget* parent, int style = WsNone, Layout* layout = 0);

	virtual void fit();

	virtual void update(const Rect* rc = 0, bool immediate = false);
	
	virtual Size getMinimumSize() const;
	
	virtual Size getPreferedSize() const;
	
	virtual Size getMaximumSize() const;

	Ref< Layout > getLayout() const;
	
	void setLayout(Layout* layout);
	
private:
	Ref< Layout > m_layout;
	
	void eventSize(Event* event);
};
	
	}
}

#endif	// traktor_ui_Container_H

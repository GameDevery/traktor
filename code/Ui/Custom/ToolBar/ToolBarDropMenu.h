#ifndef traktor_ui_custom_ToolBarDropMenu_H
#define traktor_ui_custom_ToolBarDropMenu_H

#include "Core/RefArray.h"
#include "Ui/Command.h"
#include "Ui/Point.h"
#include "Ui/Custom/ToolBar/ToolBarItem.h"

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

class MenuItem;

		namespace custom
		{

/*! \brief Tool bar dropdown.
 * \ingroup UIC
 */
class T_DLLCLASS ToolBarDropMenu : public ToolBarItem
{
	T_RTTI_CLASS;

public:
	ToolBarDropMenu(const Command& command, int32_t width, const std::wstring& text, const std::wstring& toolTip);

	int32_t add(MenuItem* item);

	bool remove(int32_t index);

	void removeAll();

	int32_t count() const;

	MenuItem* get(int32_t index) const;

protected:
	virtual bool getToolTip(std::wstring& outToolTip) const;

	virtual Size getSize(const ToolBar* toolBar, int imageWidth, int imageHeight) const;

	virtual void paint(ToolBar* toolBar, Canvas& canvas, const Point& at, Bitmap* images, int imageWidth, int imageHeight);

	virtual bool mouseEnter(ToolBar* toolBar, MouseEvent* mouseEvent);

	virtual void mouseLeave(ToolBar* toolBar, MouseEvent* mouseEvent);

	virtual void buttonDown(ToolBar* toolBar, MouseEvent* mouseEvent);

	virtual void buttonUp(ToolBar* toolBar, MouseEvent* mouseEvent);

private:
	Command m_command;
	int32_t m_width;
	std::wstring m_text;
	std::wstring m_toolTip;
	RefArray< ui::MenuItem > m_items;
	bool m_hover;
	int32_t m_dropPosition;
	Point m_menuPosition;
};

		}
	}
}

#endif	// traktor_ui_custom_ToolBarDropMenu_H

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_ToolBarDropMenu_H
#define traktor_ui_ToolBarDropMenu_H

#include "Core/RefArray.h"
#include "Ui/Command.h"
#include "Ui/Point.h"
#include "Ui/ToolBar/ToolBarItem.h"

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

class MenuItem;

/*! \brief Tool bar dropdown.
 * \ingroup UI
 */
class T_DLLCLASS ToolBarDropMenu : public ToolBarItem
{
	T_RTTI_CLASS;

public:
	ToolBarDropMenu(int32_t width, const std::wstring& text, bool arrow, const std::wstring& toolTip);

	int32_t add(MenuItem* item);

	bool remove(int32_t index);

	void removeAll();

	int32_t count() const;

	MenuItem* get(int32_t index) const;

protected:
	virtual bool getToolTip(std::wstring& outToolTip) const override final;

	virtual Size getSize(const ToolBar* toolBar, int imageWidth, int imageHeight) const override final;

	virtual void paint(ToolBar* toolBar, Canvas& canvas, const Point& at, IBitmap* images, int imageWidth, int imageHeight) override final;

	virtual bool mouseEnter(ToolBar* toolBar) override final;

	virtual void mouseLeave(ToolBar* toolBar) override final;

	virtual void buttonDown(ToolBar* toolBar, MouseButtonDownEvent* mouseEvent) override final;

	virtual void buttonUp(ToolBar* toolBar, MouseButtonUpEvent* mouseEvent) override final;

private:
	Command m_command;
	int32_t m_width;
	std::wstring m_text;
	bool m_arrow;
	std::wstring m_toolTip;
	RefArray< ui::MenuItem > m_items;
	bool m_hover;
	int32_t m_dropPosition;
	Point m_menuPosition;
};

	}
}

#endif	// traktor_ui_ToolBarDropMenu_H

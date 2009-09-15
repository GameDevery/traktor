#ifndef traktor_ui_custom_ToolBarButton_H
#define traktor_ui_custom_ToolBarButton_H

#include "Core/Heap/Ref.h"
#include "Ui/Custom/ToolBar/ToolBarItem.h"
#include "Ui/Command.h"

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
		namespace custom
		{

/*! \brief Tool bar button.
 * \ingroup UIC
 */
class T_DLLCLASS ToolBarButton : public ToolBarItem
{
	T_RTTI_CLASS(ToolBarButton)

public:
	enum ButtonStyles
	{
		BsIcon = 1 << 0,
		BsText = 1 << 1,
		BsToggle = 1 << 2,
		BsToggled = BsToggle | (1 << 3),
		BsDefault = BsIcon,
		BsDefaultToggle = BsDefault | BsToggle,
		BsDefaultToggled = BsDefault | BsToggled
	};

	ToolBarButton(const std::wstring& text, const Command& command, uint32_t imageIndex, int style = BsDefault);

	void setText(const std::wstring& text);

	const std::wstring& getText() const;

	void setToggled(bool toggled);

	bool isToggled() const;

protected:
	virtual bool getToolTip(std::wstring& outToolTip) const;

	virtual Size getSize(const ToolBar* toolBar, int imageWidth, int imageHeight) const;

	virtual void paint(ToolBar* toolBar, Canvas& canvas, const Point& at, Bitmap* images, int imageWidth, int imageHeight);

	virtual void mouseEnter(ToolBar* toolBar, MouseEvent* mouseEvent);

	virtual void mouseLeave(ToolBar* toolBar, MouseEvent* mouseEvent);

	virtual void buttonDown(ToolBar* toolBar, MouseEvent* mouseEvent);

	virtual void buttonUp(ToolBar* toolBar, MouseEvent* mouseEvent);

private:
	enum ButtonStates
	{
		BstNormal = 0,
		BstHover = 1 << 0,
		BstPushed = 1 << 1,
		BstToggled = 1 << 2
	};

	std::wstring m_text;
	Command m_command;
	uint32_t m_imageIndex;
	int m_style;
	int m_state;
};

		}
	}
}

#endif	// traktor_ui_custom_ToolBarButton_H

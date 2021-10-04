#pragma once

#include "Ui/Widget.h"

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

/*! Shortcut edit control.
 * \ingroup UI
 */
class T_DLLCLASS ShortcutEdit : public Widget
{
	T_RTTI_CLASS;

public:
	ShortcutEdit();

	bool create(Widget* parent, int32_t keyState, VirtualKey virtualKey, int style = WsNone);

	virtual std::wstring getText() const override;

	virtual Size getPreferredSize(const Size& hint) const override;

	void set(int32_t keyState, VirtualKey virtualKey);

	int32_t getKeyState() const;

	VirtualKey getVirtualKey() const;

private:
	int32_t m_keyState;
	VirtualKey m_virtualKey;

	void eventKeyDown(KeyDownEvent* event);

	void eventPaint(PaintEvent* event);

	void eventFocus(FocusEvent* event);
};

	}
}


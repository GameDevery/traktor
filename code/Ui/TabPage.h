#pragma once

#include "Ui/Container.h"

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

class Tab;

/*! Tab page.
 * \ingroup UI
 */
class T_DLLCLASS TabPage : public Container
{
	T_RTTI_CLASS;

public:
	bool create(Tab* tab, const std::wstring& text, int32_t imageIndex, Layout* layout);

	bool create(Tab* tab, const std::wstring& text, Layout* layout);

	void setActive();

	bool isActive() const;

	Tab* getTab() const;

	int32_t getImageIndex() const;

private:
	Tab* m_tab;
	int32_t m_imageIndex;
};

	}
}


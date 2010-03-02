#ifndef traktor_render_QuickMenuTool_H
#define traktor_render_QuickMenuTool_H

#include "Ui/Dialog.h"

namespace traktor
{
	namespace ui
	{

class Edit;
class ListBox;

	}

	namespace render
	{

class QuickMenuTool : public ui::Dialog
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	const TypeInfo* showMenu();

private:
	Ref< ui::Edit > m_editFilter;
	Ref< ui::ListBox > m_listBoxSuggestions;

	void updateSuggestions(const std::wstring& filter);

	void eventFilterChange(ui::Event* event);

	void eventFilterKey(ui::Event* event);

	void eventSuggestionSelect(ui::Event* event);
};

	}
}

#endif	// traktor_render_QuickMenuTool_H

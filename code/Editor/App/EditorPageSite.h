#ifndef traktor_editor_EditorPageSite_H
#define traktor_editor_EditorPageSite_H

#include <map>
#include "Editor/IEditorPageSite.h"

namespace traktor
{
	namespace editor
	{

class EditorForm;

/*! \brief Editor page site implementation.
 * \ingroup Editor
 *
 * Keep information about editor pages and their
 * current state.
 *
 * This class just dispatch method calls to editor
 * form if page is currently active; otherwise
 * it will just modify it's internal state
 * and wait until page becomes active again.
 */
class EditorPageSite : public IEditorPageSite
{
	T_RTTI_CLASS;

public:
	EditorPageSite(EditorForm* editor, bool active);

	void show();

	void hide();

	virtual void setPropertyObject(Object* properties);

	virtual void createAdditionalPanel(ui::Widget* widget, int size, bool south);

	virtual void destroyAdditionalPanel(ui::Widget* widget);

	virtual void showAdditionalPanel(ui::Widget* widget);

	virtual void hideAdditionalPanel(ui::Widget* widget);

	const std::map< Ref< ui::Widget >, bool >& getPanelWidgets() const { return m_panelWidgets; }

private:
	EditorForm* m_editor;
	Ref< Object > m_properties;
	std::map< Ref< ui::Widget >, bool > m_panelWidgets;
	bool m_active;
};

	}
}

#endif	// traktor_editor_EditorPageSite_H

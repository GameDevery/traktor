#ifndef traktor_world_PostProcessEditorFactory_H
#define traktor_world_PostProcessEditorFactory_H

#include "Editor/IObjectEditorFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class T_DLLCLASS PostProcessEditorFactory : public editor::IObjectEditorFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getEditableTypes() const;

	virtual Ref< editor::IObjectEditor > createObjectEditor(editor::IEditor* editor) const;

	virtual void getCommands(std::list< ui::Command >& outCommands) const;
};

	}
}

#endif	// traktor_world_PostProcessEditorFactory_H

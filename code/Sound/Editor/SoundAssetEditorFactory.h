#ifndef traktor_sound_SoundAssetEditorFactory_H
#define traktor_sound_SoundAssetEditorFactory_H

#include "Editor/IObjectEditorFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class T_DLLCLASS SoundAssetEditorFactory : public editor::IObjectEditorFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getEditableTypes() const;

	virtual Ref< editor::IObjectEditor > createObjectEditor(editor::IEditor* editor) const;
};

	}
}

#endif	// traktor_sound_SoundAssetEditorFactory_H

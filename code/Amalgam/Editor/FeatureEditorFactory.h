/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_FeatureEditorFactory_H
#define traktor_amalgam_FeatureEditorFactory_H

#include "Editor/IObjectEditorFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

class T_DLLCLASS FeatureEditorFactory : public editor::IObjectEditorFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getEditableTypes() const override final;

	virtual bool needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const override final;

	virtual Ref< editor::IObjectEditor > createObjectEditor(editor::IEditor* editor) const override final;

	virtual void getCommands(std::list< ui::Command >& outCommands) const override final;
};

	}
}

#endif	// traktor_amalgam_FeatureEditorFactory_H

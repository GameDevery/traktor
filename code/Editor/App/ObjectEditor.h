/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_ObjectEditor_H
#define traktor_editor_ObjectEditor_H

#include "Core/Object.h"
#include "Editor/IEditor.h"

namespace traktor
{
	namespace ui
	{

class Dialog;

	}

	namespace editor
	{

class ObjectEditor
:	public IEditor
,	public Object
{
	T_RTTI_CLASS;

public:
	ObjectEditor(IEditor* editor, ui::Dialog* parent);

	virtual Ref< const PropertyGroup > getSettings() const override final;

	virtual Ref< PropertyGroup > checkoutGlobalSettings() override final;

	virtual void commitGlobalSettings() override final;

	virtual void revertGlobalSettings() override final;

	virtual Ref< PropertyGroup > checkoutWorkspaceSettings() override final;

	virtual void commitWorkspaceSettings() override final;

	virtual void revertWorkspaceSettings() override final;

	virtual Ref< ILogTarget > createLogTarget(const std::wstring& title) override final;

	virtual Ref< db::Database > getSourceDatabase() const override final;

	virtual Ref< db::Database > getOutputDatabase() const override final;

	virtual void updateDatabaseView() override final;

	virtual bool highlightInstance(const db::Instance* instance) override final;

	virtual const TypeInfo* browseType() override final;

	virtual const TypeInfo* browseType(const TypeInfoSet& base) override final;

	virtual Ref< db::Instance > browseInstance(const TypeInfo& filterType) override final;

	virtual Ref< db::Instance > browseInstance(const IBrowseFilter* filter) override final;

	virtual bool openEditor(db::Instance* instance) override final;

	virtual bool openDefaultEditor(db::Instance* instance) override final;

	virtual bool openTool(const std::wstring& toolType, const std::wstring& param) override final;

	virtual bool openBrowser(const net::Url& url) override final;

	virtual Ref< IEditorPage > getActiveEditorPage() override final;

	virtual void setActiveEditorPage(IEditorPage* editorPage) override final;

	virtual void buildAssets(const std::vector< Guid >& assetGuids, bool rebuild) override final;

	virtual void buildAsset(const Guid& assetGuid, bool rebuild) override final;

	virtual void buildAssets(bool rebuild) override final;

	virtual Ref< IPipelineDependencySet > buildAssetDependencies(const ISerializable* asset, uint32_t recursionDepth) override final;

	virtual void setStoreObject(const std::wstring& name, Object* object) override final;

	virtual Object* getStoreObject(const std::wstring& name) const override final;

private:
	IEditor* m_editor;
	ui::Dialog* m_parent;
};

	}
}

#endif	// traktor_editor_ObjectEditor_H

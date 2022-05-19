#pragma once

#include "Core/Ref.h"
#include "Editor/IEditorPage.h"

namespace traktor
{
	namespace db
	{

class Instance;

	}

	namespace editor
	{

class IDocument;
class IEditor;
class IEditorPageSite;
class PropertiesView;

	}

	namespace ui
	{

class ContentChangeEvent;

	}

	namespace render
	{

class TextureAsset;
class TextureControl;

class TextureAssetEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	explicit TextureAssetEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent) override final;

	virtual void destroy() override final;

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;
	Ref< TextureAsset > m_asset;
	Ref< TextureControl > m_textureControl;
	Ref< editor::PropertiesView > m_propertiesView;

	void updatePreview();

	void eventPropertiesChanged(ui::ContentChangeEvent* event);
};

	}
}

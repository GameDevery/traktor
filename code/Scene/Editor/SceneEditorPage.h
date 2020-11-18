#pragma once

#include "Editor/IEditorPage.h"
#include "Ui/Events/AllEvents.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;

	namespace editor
	{

class IDocument;
class IEditorPageSite;

	}

	namespace ui
	{

class Container;
class Font;
class IBitmap;
class Menu;
class StatusBar;
class Tab;
class ToolBar;
class ToolBarButton;
class ToolBarButtonClickEvent;
class GridColumnClickEvent;
class GridItemContentChangeEvent;
class GridRow;
class GridRowStateChangeEvent;
class GridView;

	}

	namespace world
	{

class EntityData;

	}

	namespace scene
	{

class CameraMovedEvent;
class EntityAdapter;
class EntityDependencyInvestigator;
class MeasurementEvent;
class PreModifyEvent;
class PostBuildEvent;
class PostFrameEvent;
class PostModifyEvent;
class SceneAsset;
class SceneEditorContext;
class ScenePreviewControl;

class T_DLLCLASS SceneEditorPage : public editor::IEditorPage
{
	T_RTTI_CLASS;

public:
	SceneEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document);

	virtual bool create(ui::Container* parent) override final;

	virtual void destroy() override final;

	virtual bool dropInstance(db::Instance* instance, const ui::Point& position) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

private:
	editor::IEditor* m_editor;
	editor::IEditorPageSite* m_site;
	editor::IDocument* m_document;
	Ref< SceneEditorContext > m_context;
	Ref< ui::Container > m_editPanel;
	Ref< ScenePreviewControl > m_editControl;
	Ref< ui::StatusBar > m_statusBar;
	Ref< ui::Container > m_entityPanel;
	Ref< ui::Tab > m_tabMisc;
	Ref< EntityDependencyInvestigator > m_entityDependencyPanel;
	Ref< ui::GridView > m_gridGuides;
	Ref< ui::GridView > m_gridMeasurements;
	Ref< ui::Menu > m_entityMenuDefault;
	Ref< ui::Menu > m_entityMenuGroup;
	Ref< ui::Menu > m_entityMenuExternal;
	Ref< ui::Container > m_controllerPanel;
	Ref< ui::ToolBar > m_entityToolBar;
	Ref< ui::ToolBarButton > m_buttonFilterEntity;
	Ref< ui::IBitmap > m_imageHidden;
	Ref< ui::IBitmap > m_imageVisible;
	Ref< ui::IBitmap > m_imageLocked;
	Ref< ui::IBitmap > m_imageUnlocked;
	Ref< ui::GridView > m_instanceGrid;
	Ref< ui::Font > m_instanceGridFontBold;
	Ref< ui::Font > m_instanceGridFontHuge;
	const TypeInfo* m_entityFilterType;
	uint32_t m_currentHash;

	bool createSceneAsset();

	void createControllerEditor();

	void updateScene();

	Ref< ui::GridRow > createInstanceGridRow(EntityAdapter* entityAdapter);

	void createInstanceGrid();

	void updateInstanceGridRow(ui::GridRow* row);

	void updateInstanceGrid();

	void updatePropertyObject();

	void updateStatusBar();

	bool addEntity(const TypeInfo* entityType);

	bool addComponent();

	bool createExternal();

	bool resolveExternal();

	bool moveToEntity();

	bool moveUp();

	bool moveDown();

	void eventEntityToolClick(ui::ToolBarButtonClickEvent* event);

	void eventGuideClick(ui::GridColumnClickEvent* event);

	void eventInstanceSelect(ui::SelectionChangeEvent* event);

	void eventInstanceExpand(ui::GridRowStateChangeEvent* event);

	void eventInstanceButtonDown(ui::MouseButtonDownEvent* event);

	void eventInstanceClick(ui::GridColumnClickEvent* event);

	void eventInstanceRename(ui::GridItemContentChangeEvent* event);

	void eventContextPostBuild(PostBuildEvent* event);

	void eventContextSelect(ui::SelectionChangeEvent* event);

	void eventContextPreModify(PreModifyEvent* event);

	void eventContextPostModify(PostModifyEvent* event);

	void eventContextCameraMoved(CameraMovedEvent* event);

	void eventContextPostFrame(PostFrameEvent* event);

	void eventContextMeasurement(MeasurementEvent* event);
};

	}
}


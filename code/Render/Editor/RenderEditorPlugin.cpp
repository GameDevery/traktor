#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IEditor.h"
#include "Render/IRenderSystem.h"
#include "Render/Vrfy/RenderSystemVrfy.h"
#include "Render/Editor/RenderEditorPlugin.h"
#include "Render/Editor/Shader/ShaderDependencyTracker.h"
#include "Ui/MessageBox.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderEditorPlugin", RenderEditorPlugin, editor::IEditorPlugin)

RenderEditorPlugin::RenderEditorPlugin(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool RenderEditorPlugin::create(ui::Widget* parent, editor::IEditorPageSite* site)
{
	if (!createRenderSystem())
		return false;

	return true;
}

void RenderEditorPlugin::destroy()
{
	Ref< IRenderSystem > renderSystem = m_editor->getStoreObject< IRenderSystem >(L"RenderSystem");
	if (renderSystem)
	{
		safeDestroy(renderSystem);
		m_editor->setStoreObject(L"RenderSystem", nullptr);
	}
}

bool RenderEditorPlugin::handleCommand(const ui::Command& command, bool result)
{
	if (command == L"Editor.SettingsChanged")
	{
		createRenderSystem();
		return true;
	}
	else if (command == L"Render.PrintMemoryUsage")
	{
		Ref< IRenderSystem > renderSystem = m_editor->getStoreObject< IRenderSystem >(L"RenderSystem");
		if (renderSystem)
		{
			RenderSystemStatistics rss;
			renderSystem->getStatistics(rss);

			log::info << L"Render system statistics ==================" << Endl;
			log::info << IncreaseIndent;
			log::info << L"Memory available: " << formatByteSize(rss.memoryAvailable) << Endl;
			log::info << L"Memory usage: " << formatByteSize(rss.memoryUsage) << Endl;
			log::info << L"Allocation count: " << rss.allocationCount << Endl;
			log::info << L"Buffers: " << rss.buffers << Endl;
			log::info << L"Simple textures: " << rss.simpleTextures << Endl;
			log::info << L"Cube textures: " << rss.cubeTextures << Endl;
			log::info << L"Volume textures: " << rss.volumeTextures << Endl;
			log::info << L"Render target sets: " << rss.renderTargetSets << Endl;
			log::info << L"Programs: " << rss.programs << Endl;
			log::info << DecreaseIndent;
		}
		else
			log::info << L"No render system created." << Endl;

		return true;
	}
	else
		return false;
}

void RenderEditorPlugin::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
	if (m_tracker && database == m_editor->getSourceDatabase())
		m_tracker->scan(database, eventId);
}

void RenderEditorPlugin::handleWorkspaceOpened()
{
	m_tracker = new ShaderDependencyTracker();
	m_tracker->scan(m_editor->getSourceDatabase());
	m_editor->setStoreObject(L"ShaderDependencyTracker", m_tracker);
}

void RenderEditorPlugin::handleWorkspaceClosed()
{
	m_editor->setStoreObject(L"ShaderDependencyTracker", nullptr);
	safeDestroy(m_tracker);
}

bool RenderEditorPlugin::createRenderSystem()
{
	auto settings = m_editor->getSettings();

	// Create render system.
	std::wstring renderSystemTypeName = settings->getProperty< std::wstring >(L"Editor.RenderSystem");

	// Check if render system is already instantiated.
	auto renderSystemUnsafe = m_editor->getStoreObject< IRenderSystem >(L"RenderSystemUnsafe");
	if (renderSystemUnsafe != nullptr)
	{
		if (type_name(renderSystemUnsafe) == renderSystemTypeName)
			return true;
	}

	const TypeInfo* renderSystemType = TypeInfo::find(renderSystemTypeName.c_str());
	if (!renderSystemType)
	{
		ui::MessageBox::show(str(L"Unable to instantiate render system \"%ls\",\nNo such type.", renderSystemTypeName.c_str()), L"Error", ui::MbIconError | ui::MbOk);
		return false;
	}

	Ref< IRenderSystem > renderSystem = dynamic_type_cast< IRenderSystem* >(renderSystemType->createInstance());
	T_ASSERT(renderSystem);

	Ref< RenderSystemVrfy > renderSystemVrfy = new RenderSystemVrfy(settings->getProperty< bool >(L"Editor.UseRenderDoc", false));

	RenderSystemDesc desc;
	desc.capture = renderSystem;
	desc.mipBias = settings->getProperty< float >(L"Editor.MipBias", 0.0f);
	desc.maxAnisotropy = settings->getProperty< int32_t >(L"Editor.MaxAnisotropy", 1);
	desc.maxAnisotropy = std::max(desc.maxAnisotropy, 1);
	desc.validation = settings->getProperty< bool >(L"Editor.RenderValidation", true);
	desc.programCache = settings->getProperty< bool >(L"Editor.UseProgramCache", false);
	if (!renderSystemVrfy->create(desc))
	{
		ui::MessageBox::show(str(L"Unable to instantiate render system \"%ls\",\nFailed to initialize render system.", renderSystemTypeName.c_str()), L"Error", ui::MbIconError | ui::MbOk);
		return false;
	}

	m_editor->setStoreObject(L"RenderSystem", renderSystemVrfy);
	m_editor->setStoreObject(L"RenderSystemUnsafe", renderSystem);
	return true;
}

	}
}

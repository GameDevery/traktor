#include "Core/Misc/String.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/Settings.h"
#include "I18N/Text.h"
#include "Render/IProgramCompiler.h"
#include "Render/IRenderSystem.h"
#include "Render/Editor/RenderSettingsPage.h"
#include "Ui/DropDown.h"
#include "Ui/Container.h"
#include "Ui/Edit.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderSettingsPage", 0, RenderSettingsPage, editor::ISettingsPage)

bool RenderSettingsPage::create(ui::Container* parent, Settings* settings, const std::list< ui::Command >& shortcutCommands)
{
	Ref< ui::Container > container = new ui::Container();
	if (!container->create(parent, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 0, 4)))
		return false;

	Ref< ui::Static > staticRenderer = new ui::Static();
	staticRenderer->create(container, i18n::Text(L"EDITOR_SETTINGS_RENDERER_TYPE"));

	m_dropRenderSystem = new ui::DropDown();
	m_dropRenderSystem->create(container, L"");
	
	Ref< ui::Static > staticCompiler = new ui::Static();
	staticCompiler->create(container, i18n::Text(L"EDITOR_SETTINGS_COMPILER_TYPE"));
	
	m_dropCompiler = new ui::DropDown();
	m_dropCompiler->create(container, L"");

	Ref< ui::Static > staticMipBias = new ui::Static();
	staticMipBias->create(container, i18n::Text(L"EDITOR_SETTINGS_RENDERER_MIPBIAS"));

	m_editMipBias = new ui::Edit();
	m_editMipBias->create(container, L"", ui::WsClientBorder, new ui::NumericEditValidator(true, -100.0f, 100.0f));

	Ref< ui::Static > staticMaxAnisotropy = new ui::Static();
	staticMaxAnisotropy->create(container, i18n::Text(L"EDITOR_SETTINGS_RENDERER_MAX_ANISOTROPY"));

	m_editMaxAnisotropy = new ui::Edit();
	m_editMaxAnisotropy->create(container, L"", ui::WsClientBorder, new ui::NumericEditValidator(false, 0, 16));

	std::wstring renderSystemType = settings->getProperty< PropertyString >(L"Editor.RenderSystem");

	std::vector< const TypeInfo* > renderSystemTypes;
	type_of< render::IRenderSystem >().findAllOf(renderSystemTypes, false);

	for (std::vector< const TypeInfo* >::const_iterator i = renderSystemTypes.begin(); i != renderSystemTypes.end(); ++i)
	{
		std::wstring name = (*i)->getName();
		int32_t index = m_dropRenderSystem->add(name);
		if (name == renderSystemType)
			m_dropRenderSystem->select(index);
	}
	
	std::wstring compilerType = settings->getProperty< PropertyString >(L"ShaderPipeline.ProgramCompiler");
	
	std::vector< const TypeInfo* > compilerTypes;
	type_of< render::IProgramCompiler >().findAllOf(compilerTypes, false);

	for (std::vector< const TypeInfo* >::const_iterator i = compilerTypes.begin(); i != compilerTypes.end(); ++i)
	{
		std::wstring name = (*i)->getName();
		int32_t index = m_dropCompiler->add(name);
		if (name == compilerType)
			m_dropCompiler->select(index);
	}

	m_editMipBias->setText(toString(settings->getProperty< PropertyFloat >(L"Editor.MipBias")));
	m_editMaxAnisotropy->setText(toString(settings->getProperty< PropertyInteger >(L"Editor.MaxAnisotropy")));

	parent->setText(i18n::Text(L"EDITOR_SETTINGS_RENDERER"));
	return true;
}

void RenderSettingsPage::destroy()
{
}

bool RenderSettingsPage::apply(Settings* settings)
{
	settings->setProperty< PropertyString >(L"Editor.RenderSystem", m_dropRenderSystem->getSelectedItem());
	settings->setProperty< PropertyString >(L"ShaderPipeline.ProgramCompiler", m_dropCompiler->getSelectedItem());
	settings->setProperty< PropertyFloat >(L"Editor.MipBias", parseString< float >(m_editMipBias->getText()));
	settings->setProperty< PropertyInteger >(L"Editor.MaxAnisotropy", parseString< int32_t >(m_editMaxAnisotropy->getText()));
	return true;
}

	}
}

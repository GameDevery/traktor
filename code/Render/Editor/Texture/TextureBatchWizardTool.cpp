/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Render/Editor/Texture/TextureAsset.h"
#include "Render/Editor/Texture/TextureBatchDialog.h"
#include "Render/Editor/Texture/TextureBatchWizardTool.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.TextureBatchWizardTool", 0, TextureBatchWizardTool, editor::IWizardTool)

std::wstring TextureBatchWizardTool::getDescription() const
{
	return i18n::Text(L"TEXTURE_BATCH_WIZARDTOOL_DESCRIPTION");
}

const TypeInfoSet TextureBatchWizardTool::getSupportedTypes() const
{
	return TypeInfoSet();
}

uint32_t TextureBatchWizardTool::getFlags() const
{
	return WfGroup;
}

bool TextureBatchWizardTool::launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance)
{
	TextureBatchDialog textureDialog(editor);

	if (!textureDialog.create(parent))
		return false;

	RefArray< TextureAsset > textureAssets;
	if (textureDialog.showModal(textureAssets))
	{
		for (RefArray< TextureAsset >::iterator i = textureAssets.begin(); i != textureAssets.end(); ++i)
		{
			std::wstring instanceName = Path((*i)->getFileName()).getFileNameNoExtension();
			Ref< db::Instance > textureAssetInstance = group->createInstance(instanceName);
			if (textureAssetInstance)
			{
				textureAssetInstance->setObject(*i);
				textureAssetInstance->commit();
			}
			else
				log::error << L"Unable to create instance \"" << instanceName << L"\"; asset not imported" << Endl;
		}
	}

	textureDialog.destroy();
	return true;
}

}

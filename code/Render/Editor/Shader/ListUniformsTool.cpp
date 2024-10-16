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
#include "Database/Traverse.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Render/Editor/Shader/External.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ListUniformsTool.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ListUniformsTool", 0, ListUniformsTool, IEditorTool)

std::wstring ListUniformsTool::getDescription() const
{
	return i18n::Text(L"SHADERGRAPH_LIST_UNIFORMS");
}

Ref< ui::IBitmap > ListUniformsTool::getIcon() const
{
	return nullptr;
}

bool ListUniformsTool::needOutputResources(std::set< Guid >& outDependencies) const
{
	return false;
}

bool ListUniformsTool::launch(ui::Widget* parent, editor::IEditor* editor, const PropertyGroup* param)
{
	Ref< db::Database > database = editor->getSourceDatabase();
	if (!database)
		return true;

	RefArray< db::Instance > instances;
	db::recursiveFindChildInstances(
		database->getRootGroup(),
		db::FindInstanceByType(type_of< ShaderGraph >()),
		instances
	);

    
    struct UniformInfo
    {
        Guid instance;
        ParameterType type;
        UpdateFrequency frequency;
        int32_t length;
    };
	SmallMap< std::wstring, AlignedVector< UniformInfo > > uniformInfos;

	for (auto instance : instances)
	{
		Ref< ShaderGraph > shaderGraph = instance->getObject< ShaderGraph >();
		if (!shaderGraph)
		{
			log::error << L"Unable to get shader graph from " << instance->getPath() << L"." << Endl;
			instance->revert();
			continue;
		}

		for (auto uniformNode : shaderGraph->findNodesOf< Uniform >())
		{
            auto& ui = uniformInfos[uniformNode->getParameterName()];
            ui.push_back({
                instance->getGuid(),
                uniformNode->getParameterType(),
                uniformNode->getFrequency(),
                1
            });
        }

		for (auto indexedUniformNode : shaderGraph->findNodesOf< IndexedUniform >())
		{
            auto& ui = uniformInfos[indexedUniformNode->getParameterName()];
            ui.push_back({
                instance->getGuid(),
                indexedUniformNode->getParameterType(),
                indexedUniformNode->getFrequency(),
                indexedUniformNode->getLength()
            });
        }
	}

    const wchar_t* parameterTypeNames[] =
    {
        L"scalar",
        L"vector",
        L"matrix",
        L"texture2d",
        L"texture3d",
        L"textureCube",
        L"structBuffer",
        L"image2d",
        L"image3d",
        L"imageCube"
    };

    const wchar_t* updateFrequencyNames[] =
    {
        L"once",
        L"frame",
        L"draw"
    };

    for (auto it : uniformInfos)
    {
        log::info << it.first << Endl;
        log::info << IncreaseIndent;

        for (const auto& ui : it.second)
            log::info << ui.instance.format() << L", " << parameterTypeNames[(int32_t)ui.type] << L"[" << ui.length << L"], (" << updateFrequencyNames[(int32_t)ui.frequency] << L")" << Endl;

        log::info << DecreaseIndent;
    }

	return true;
}

}

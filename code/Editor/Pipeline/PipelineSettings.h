/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Io/StringOutputStream.h"
#include "Editor/IPipelineSettings.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class PropertyGroup;

}

namespace traktor::editor
{

class T_DLLCLASS PipelineSettings : public IPipelineSettings
{
	T_RTTI_CLASS;

public:
	explicit PipelineSettings(const PropertyGroup* settings);

	virtual Ref< const IPropertyValue > getProperty(const std::wstring& propertyName, bool includeInHash, const IPropertyValue* defaultValue) const override final;

	uint32_t getHash() const;

	std::wstring getLog() const;

private:
	Ref< const PropertyGroup > m_settings;
	mutable uint32_t m_hash;
	mutable StringOutputStream m_log;
};

}

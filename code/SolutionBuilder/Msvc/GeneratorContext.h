/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Core/Io/Path.h"

#include <string>

namespace traktor::sb
{

/*! Generator context. */
class GeneratorContext
{
public:
	explicit GeneratorContext(const Path& solutionPathName, bool includeExternal);

	void set(const std::wstring& key, const std::wstring& value);

	std::wstring get(const std::wstring& key) const;

	std::wstring format(const std::wstring& option) const;

	std::wstring getVCProjectRelativePath(const std::wstring& path, bool resolve) const;

	std::wstring generateGUID(const std::wstring& key) const;

	const Path& getSolutionPathName() const { return m_solutionPathName; }

	bool getIncludeExternal() const { return m_includeExternal; }

private:
	Path m_solutionPathName;
	bool m_includeExternal;
	SmallMap< std::wstring, std::wstring > m_values;
};

}

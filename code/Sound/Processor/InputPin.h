/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"
#include "Sound/Processor/ProcessorTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class Node;

class T_DLLCLASS InputPin : public Object
{
	T_RTTI_CLASS;

public:
	InputPin(Node* node, const std::wstring& name, NodePinType type, bool optional);

	Node* getNode() const;

	const std::wstring& getName() const;

	NodePinType getPinType() const;

	bool isOptional() const;

private:
	Node* m_node;
	std::wstring m_name;
	NodePinType m_type;
	bool m_optional;
};

	}
}

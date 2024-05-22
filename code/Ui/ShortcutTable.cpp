/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Application.h"
#include "Ui/ShortcutTable.h"
#include "Ui/Events/ShortcutEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

uint32_t buildMapKey(int keyState, VirtualKey keyCode)
{
	return uint32_t(((keyState << 16) & 0xffff0000) | (uint32_t(keyCode) & 0x0000ffff));
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ShortcutTable", ShortcutTable, EventSubject)

bool ShortcutTable::create()
{
	m_keyDownEventHandler = ui::Application::getInstance()->addEventHandler< KeyDownEvent >(this, &ShortcutTable::eventKeyDown);
	return true;
}

void ShortcutTable::destroy()
{
	ui::Application::getInstance()->removeEventHandler(m_keyDownEventHandler);
	removeAllCommands();
}

void ShortcutTable::addCommand(int keyState, VirtualKey keyCode, const Command& command)
{
	m_commands[buildMapKey(keyState, keyCode)].push_back(command);
}

void ShortcutTable::removeCommand(int keyState, VirtualKey keyCode, const Command& command)
{
	auto it = m_commands.find(buildMapKey(keyState, keyCode));
	if (it != m_commands.end())
	{
		it->second.remove(command);
		if (it->second.empty())
			m_commands.erase(it);
	}
}

void ShortcutTable::removeAllCommands()
{
	m_commands.clear();
}

void ShortcutTable::eventKeyDown(KeyDownEvent* event)
{
	int keyState = event->getKeyState();
	VirtualKey keyCode = event->getVirtualKey();

	// Get command; ignore explicit KsControl as we should only
	// trigger on KsCommand which is defined as KsControl on Windows.
	std::map< uint32_t, std::list< Command > >::iterator it;
	if (keyState & KsCommand)
		it = m_commands.find(buildMapKey(keyState & ~KsControl, keyCode));
	else
		it = m_commands.find(buildMapKey(keyState, keyCode));

	if (it == m_commands.end())
		return;

	for (const auto& command : it->second)
	{
		// Raise command event to all listeners.
		ShortcutEvent shortcutEvent(this, command);
		raiseEvent(&shortcutEvent);

		// If event was consumed we also consume the key event.
		if (shortcutEvent.consumed())
		{
			event->consume();
			break;
		}
	}
}

	}
}

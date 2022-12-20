/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <cstring>
#include "Core/Io/OutputStream.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Script/Lua/ScriptUtilitiesLua.h"

namespace traktor::script
{

void dumpStack(lua_State* luaState, OutputStream& os, int32_t base)
{
	const int32_t top = lua_gettop(luaState);
	for (int32_t i = base + 1; i <= top; ++i)
	{
		const int32_t r = -1 - (top - i);
		const int32_t t = lua_type(luaState, i);
		switch (t)
		{
		case LUA_TSTRING:
			os << i << L" [" << r << L"] .\tstring: \"" << mbstows(lua_tostring(luaState, i)) << L"\"" << Endl;
			break;

		case LUA_TBOOLEAN:
			os << i << L" [" << r << L"] .\tboolean: " << (lua_toboolean(luaState, i) ? L"true" : L"false") << Endl;
			break;

		case LUA_TNUMBER:
			os << i << L" [" << r << L"] .\tnumber: " << lua_tonumber(luaState, i) << Endl;
			break;

		case LUA_TTABLE:
			os << i << L" [" << r << L"] .\ttable" << Endl;

			lua_pushstring(luaState, "__name");
			lua_rawget(luaState, i);
			if (lua_isstring(luaState, -1))
				os << L"\t\t\t.__name \"" << mbstows(lua_tostring(luaState, -1)) << L"\"" << Endl;
			lua_pop(luaState, 1);

			break;

		default:  /* other values */
			os << i << L" [" << r << L"] .\tother: " << mbstows(lua_typename(luaState, t)) << Endl;
			break;
		}
	}
}

int luaPrint(lua_State* L)
{
	int n = lua_gettop(L);
	int i;

	lua_getglobal(L, "tostring");
	for (i = 1; i <= n; ++i)
	{
		const char* s;
		lua_pushvalue(L, -1);
		lua_pushvalue(L, i);
		lua_call(L, 1, 1);
		s = lua_tostring(L, -1);
		if (s == NULL)
			return luaL_error(L, "\"tostring\" must return a string to \"print\"");
		if (i > 1)
			log::info << L"\t";
		log::info << mbstows(s);
		lua_pop(L, 1);
	}

	log::info << Endl;
	return 0;
}

int luaSleep(lua_State* L)
{
	const int32_t ms = (int32_t)lua_tointeger(L, -1);
	if (ms >= 0)
		ThreadManager::getInstance().getCurrentThread()->sleep(ms);
	return 0;
}

}

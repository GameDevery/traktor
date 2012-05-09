#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Script/Lua/ScriptUtilitiesLua.h"

namespace traktor
{
	namespace script
	{

void stackDump(lua_State* luaState)
{
	int32_t top = lua_gettop(luaState);

	T_DEBUG(L"Total in stack " << top);

	for (int32_t i = 1; i <= top; ++i)
	{
		int t = lua_type(luaState, i);
		switch (t)
		{
		case LUA_TSTRING:
			T_DEBUG(L"\tstring: \"" << mbstows(lua_tostring(luaState, i)) << L"\"");
			break;

		case LUA_TBOOLEAN:
			T_DEBUG(L"\tboolean: " << (lua_toboolean(luaState, i) ? L"true" : L"false"));
			break;

		case LUA_TNUMBER:
			T_DEBUG(L"\tnumber: " << lua_tonumber(luaState, i));
			break;

		default:  /* other values */
			T_DEBUG(L"\tother: " << mbstows(lua_typename(luaState, t)));
			break;
		}
	}
}

void* luaAlloc(void* ud, void* ptr, size_t osize, size_t nsize)
{
	size_t& allocTotal = *(size_t*)ud;
	if (nsize > 0)
	{
		if (osize >= nsize)
			return ptr;

		void* nptr = getAllocator()->alloc(nsize, 16, "LUA");
		if (!nptr)
			return 0;

		allocTotal += nsize;

		if (ptr && osize > 0)
		{
			std::memcpy(nptr, ptr, std::min(osize, nsize));
			getAllocator()->free(ptr);

			T_ASSERT (osize <= allocTotal);
			allocTotal -= osize;
		}

		return nptr;
	}
	else
	{
		getAllocator()->free(ptr);
		return 0;
	}
}

int luaPrint(lua_State *L)
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
			return luaL_error(L, LUA_QL("tostring") " must return a string to " LUA_QL("print"));
		if (i > 1)
			log::info << L"\t";
		log::info << mbstows(s);
		lua_pop(L, 1);
	}

	log::info << Endl;
	return 0;
}

	}
}

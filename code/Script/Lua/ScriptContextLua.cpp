/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/WildCompare.h"
#include "Core/Timer/Profiler.h"
#include "Script/Lua/ScriptBlobLua.h"
#include "Script/Lua/ScriptClassLua.h"
#include "Script/Lua/ScriptContextLua.h"
#include "Script/Lua/ScriptDelegateLua.h"
#include "Script/Lua/ScriptManagerLua.h"
#include "Script/Lua/ScriptObjectLua.h"
#include "Script/Lua/ScriptProfilerLua.h"
#include "Script/Lua/ScriptUtilitiesLua.h"

namespace traktor::script
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptContextLua", ScriptContextLua, IScriptContext)

ScriptContextLua::~ScriptContextLua()
{
	destroy();
}

void ScriptContextLua::destroy()
{
	if (m_scriptManager)
	{
		// Store reference locally as later the garbage
		// collect might recurse this call.
		Ref< ScriptManagerLua > scriptManager = m_scriptManager;
		m_scriptManager = nullptr;

		scriptManager->lock(this);
		{
			// Unpin our local environment reference.
			if (m_environmentRef != LUA_NOREF)
			{
				// Clear all global variables first.
				DO_0(m_luaState, lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_environmentRef));
				DO_1(m_luaState, lua_pushnil(m_luaState));

				while (lua_next(m_luaState, -2))
				{
					DO_1(m_luaState, lua_pop(m_luaState, 1));
					DO_1(m_luaState, lua_pushvalue(m_luaState, -1));
					DO_1(m_luaState, lua_pushnil(m_luaState));
					DO_1(m_luaState, lua_rawset(m_luaState, -4));
				}

				DO_1(m_luaState, luaL_unref(m_luaState, LUA_REGISTRYINDEX, m_environmentRef));
				m_environmentRef = LUA_NOREF;
				m_luaState = nullptr;
			}

			// Perform a full garbage collect; don't want
			// lingering objects.
			scriptManager->collectGarbageFullNoLock();
			scriptManager->destroyContext(this);
		}
		scriptManager->unlock();
	}
}

bool ScriptContextLua::load(const IScriptBlob* scriptBlob)
{
	m_scriptManager->lock(this);
	{
		CHECK_LUA_STACK(m_luaState, 0);

		// Unlock environment.
		if (m_strict)
		{
			CHECK_LUA_STACK(m_luaState, 0);
			lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_environmentRef);
			lua_getmetatable(m_luaState, -1);

			lua_pushlightuserdata(m_luaState, (void*)this);
			lua_pushcclosure(m_luaState, permitGlobalWrite, 1);
			lua_setfield(m_luaState, -2, "__newindex");

			lua_getglobal(m_luaState, "_G");
			lua_setfield(m_luaState, -2, "__index");

			lua_pop(m_luaState, 2);
		}

		const ScriptBlobLua* scriptBlobLua = mandatory_non_null_type_cast< const ScriptBlobLua* >(scriptBlob);
		const int32_t result = luaL_loadbuffer(
			m_luaState,
			(const char*)scriptBlobLua->m_script.c_str(),
			scriptBlobLua->m_script.length(),
			scriptBlobLua->m_fileName.c_str()
		);
		if (result != 0)
		{
			log::error << L"Script context load resource failed; \"" << mbstows(lua_tostring(m_luaState, -1)) << L"\"" << Endl;
			lua_pop(m_luaState, 1);
			m_scriptManager->unlock();
			return false;
		}

		lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_environmentRef);
		lua_setupvalue(m_luaState, -2, 1);
		lua_call(m_luaState, 0, 0);

		// Lock environment.
		if (m_strict)
		{
			CHECK_LUA_STACK(m_luaState, 0);
			lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_environmentRef);
			lua_getmetatable(m_luaState, -1);

			lua_pushlightuserdata(m_luaState, (void*)this);
			lua_pushcclosure(m_luaState, restrictedAccessWrite, 1);
			lua_setfield(m_luaState, -2, "__newindex");

			lua_pushlightuserdata(m_luaState, (void*)this);
			lua_pushcclosure(m_luaState, restrictedAccessRead, 1);
			lua_setfield(m_luaState, -2, "__index");

			lua_pop(m_luaState, 2);
		}
	}
	m_scriptManager->unlock();
	return true;
}

void ScriptContextLua::setGlobal(const std::string& globalName, const Any& globalValue)
{
	m_scriptManager->lock(this);
	{
		CHECK_LUA_STACK(m_luaState, 0);
		lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_environmentRef);
		lua_pushstring(m_luaState, globalName.c_str());
		m_scriptManager->pushAny(globalValue);
		lua_rawset(m_luaState, -3);
		lua_pop(m_luaState, 1);
	}
	m_scriptManager->unlock();
}

Any ScriptContextLua::getGlobal(const std::string& globalName)
{
	Any value;
	m_scriptManager->lock(this);
	{
		CHECK_LUA_STACK(m_luaState, 0);
		lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_environmentRef);
		lua_getfield(m_luaState, -1, globalName.c_str());
		value = m_scriptManager->toAny(-1);
	}
	m_scriptManager->unlock();
	return value;
}

Ref< const IRuntimeClass > ScriptContextLua::findClass(const std::string& className)
{
	Ref< ScriptClassLua > scriptClass;
	m_scriptManager->lock(this);
	{
		CHECK_LUA_STACK(m_luaState, 0);
		lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_environmentRef);
		lua_getfield(m_luaState, -1, className.c_str());
		if (lua_istable(m_luaState, -1))
		{
			scriptClass = ScriptClassLua::createFromStack(m_scriptManager, this, m_luaState);
		}
		lua_pop(m_luaState, 2);
	}
	m_scriptManager->unlock();
	return scriptClass;
}

bool ScriptContextLua::haveFunction(const std::string& functionName) const
{
	bool result;
	m_scriptManager->lock((ScriptContextLua*)this);
	{
		CHECK_LUA_STACK(m_luaState, 0);
		lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_environmentRef);
		lua_pushstring(m_luaState, functionName.c_str());
		lua_rawget(m_luaState, -2);
		result = (lua_isfunction(m_luaState, -1) != 0);
		lua_pop(m_luaState, 2);
	}
	m_scriptManager->unlock();
	return result;
}

Any ScriptContextLua::executeFunction(const std::string& functionName, uint32_t argc, const Any* argv)
{
	T_PROFILER_SCOPE(L"Script executeFunction");

	Any returnValue;
	m_scriptManager->lock(this);
	{
		CHECK_LUA_STACK(m_luaState, 0);

		lua_pushlightuserdata(m_luaState, (void*)this);
		lua_pushcclosure(m_luaState, runtimeError, 1);
		const int32_t errfunc = lua_gettop(m_luaState);

		lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, m_environmentRef);
		lua_pushstring(m_luaState, functionName.c_str());
		lua_rawget(m_luaState, -2);

		if (lua_isfunction(m_luaState, -1))
		{
			// Push arguments.
			{
				CHECK_LUA_STACK(m_luaState, argc);
				for (uint32_t i = 0; i < argc; ++i)
				{
					const Any& any = argv[i];
					if (any.isVoid())
						lua_pushnil(m_luaState);
					else if (any.isBoolean())
						lua_pushboolean(m_luaState, any.getBooleanUnsafe() ? 1 : 0);
					else if (any.isInt32())
						lua_pushinteger(m_luaState, any.getInt32Unsafe());
					else if (any.isInt64())
						lua_pushinteger(m_luaState, any.getInt64Unsafe());
					else if (any.isFloat())
						lua_pushnumber(m_luaState, any.getFloatUnsafe());
					else if (any.isString())
						lua_pushstring(m_luaState, any.getStringUnsafe().c_str());
					else if (any.isObject())
						m_scriptManager->pushObject(any.getObjectUnsafe());
					else
						lua_pushnil(m_luaState);
				}
			}

			if (m_scriptManager->m_profiler)
				m_scriptManager->m_profiler->notifyCallEnter();

			const int32_t err = lua_pcall(m_luaState, argc, 1, errfunc);
			if (err == 0)
				returnValue = m_scriptManager->toAny(-1);

			if (m_scriptManager->m_profiler)
				m_scriptManager->m_profiler->notifyCallLeave();
		}
		else
			log::error << L"Unable to call " << mbstows(functionName) << L"; no such function" << Endl;

		lua_pop(m_luaState, 3);
	}
	m_scriptManager->unlock();
	return returnValue;
}

Any ScriptContextLua::executeDelegate(ScriptDelegateLua* delegate, uint32_t argc, const Any* argv)
{
	T_PROFILER_SCOPE(L"Script executeDelegate");

	Any returnValue;
	m_scriptManager->lock(this);
	{
		CHECK_LUA_STACK(m_luaState, 0);

		// Push error function.
		lua_pushlightuserdata(m_luaState, (void*)this);
		lua_pushcclosure(m_luaState, runtimeError, 1);
		const int32_t errfunc = lua_gettop(m_luaState);

		delegate->push();

		// Push arguments.
		{
			CHECK_LUA_STACK(m_luaState, argc);
			for (uint32_t i = 0; i < argc; ++i)
			{
				const Any& any = argv[i];
				if (any.isVoid())
					lua_pushnil(m_luaState);
				else if (any.isBoolean())
					lua_pushboolean(m_luaState, any.getBooleanUnsafe() ? 1 : 0);
				else if (any.isInt32())
					lua_pushinteger(m_luaState, any.getInt32Unsafe());
				else if (any.isInt64())
					lua_pushinteger(m_luaState, any.getInt64Unsafe());
				else if (any.isFloat())
					lua_pushnumber(m_luaState, any.getFloatUnsafe());
				else if (any.isString())
					lua_pushstring(m_luaState, any.getStringUnsafe().c_str());
				else if (any.isObject())
					m_scriptManager->pushObject(any.getObjectUnsafe());
				else
					lua_pushnil(m_luaState);
			}
		}

		if (m_scriptManager->m_profiler)
			m_scriptManager->m_profiler->notifyCallEnter();

		// Call script method.
		const int32_t err = lua_pcall(m_luaState, argc, 1, errfunc);
		if (err == 0)
			returnValue = m_scriptManager->toAny(-1);

		if (m_scriptManager->m_profiler)
			m_scriptManager->m_profiler->notifyCallLeave();

		lua_pop(m_luaState, 2);
	}
	m_scriptManager->unlock();
	return returnValue;
}

Any ScriptContextLua::executeMethod(ScriptObjectLua* self, int32_t methodRef, uint32_t argc, const Any* argv)
{
	T_PROFILER_SCOPE(L"Script executeMethod");

	Any returnValue;
	m_scriptManager->lock(this);
	{
		CHECK_LUA_STACK(m_luaState, 0);

		// Push error function.
		lua_pushlightuserdata(m_luaState, (void*)this);
		lua_pushcclosure(m_luaState, runtimeError, 1);
		const int32_t errfunc = lua_gettop(m_luaState);

		// Push LUA function to call.
		lua_rawgeti(m_luaState, LUA_REGISTRYINDEX, methodRef);

		// Push wrapped LUA object.
		if (self)
			self->push();

		// Push arguments.
		{
			CHECK_LUA_STACK(m_luaState, argc);
			for (uint32_t i = 0; i < argc; ++i)
			{
				const Any& any = argv[i];
				if (any.isVoid())
					lua_pushnil(m_luaState);
				else if (any.isBoolean())
					lua_pushboolean(m_luaState, any.getBooleanUnsafe() ? 1 : 0);
				else if (any.isInt32())
					lua_pushinteger(m_luaState, any.getInt32Unsafe());
				else if (any.isInt64())
					lua_pushinteger(m_luaState, any.getInt64Unsafe());
				else if (any.isFloat())
					lua_pushnumber(m_luaState, any.getFloatUnsafe());
				else if (any.isString())
					lua_pushstring(m_luaState, any.getStringUnsafe().c_str());
				else if (any.isObject())
					m_scriptManager->pushObject(any.getObjectUnsafe());
				else
					lua_pushnil(m_luaState);
			}
		}

		if (m_scriptManager->m_profiler)
			m_scriptManager->m_profiler->notifyCallEnter();

		// Call script function.
		int32_t err;
		{
			T_PROFILER_SCOPE(L"lua_pcall");
			err = lua_pcall(m_luaState, argc + (self ? 1 : 0), 1, errfunc);
		}
		if (err == 0)
			returnValue = m_scriptManager->toAny(-1);

		if (m_scriptManager->m_profiler)
			m_scriptManager->m_profiler->notifyCallLeave();

		lua_pop(m_luaState, 2);
	}
	m_scriptManager->unlock();
	return returnValue;
}

ScriptContextLua::ScriptContextLua(ScriptManagerLua* scriptManager, lua_State* luaState, int32_t environmentRef, bool strict)
:	m_scriptManager(scriptManager)
,	m_luaState(luaState)
,	m_environmentRef(environmentRef)
,	m_strict(strict)
,	m_lastSelf(nullptr)
{
}

int32_t ScriptContextLua::runtimeError(lua_State* luaState)
{
	ScriptContextLua* this_ = reinterpret_cast< ScriptContextLua* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT(this_);
	T_ASSERT(this_->m_scriptManager);

	log::error << L"LUA RUNTIME ERROR; Debugger halted if attached." << Endl;

	const std::wstring error = mbstows(lua_tostring(luaState, -1));
	if (!error.empty())
		log::error << error << Endl;

	this_->m_scriptManager->breakDebugger(luaState);
	return 0;
}

// __newindex
int32_t ScriptContextLua::permitGlobalWrite(lua_State* luaState)
{
	ScriptContextLua* this_ = reinterpret_cast< ScriptContextLua* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT(this_);
	T_ASSERT(this_->m_scriptManager);

	const char* key = lua_tostring(luaState, -2);
	if (!key)
	{
		log::error << L"LUA RUNTIME ERROR; Debugger halted if attached." << Endl;
		log::error << L"GLOBAL access is restricted; cannot define new globals with non-literal keys." << Endl;
		this_->m_scriptManager->breakDebugger(luaState);
		return 0;
	}

	// Track all defined globals manually to ensure they are valid to read.
	this_->m_globals.insert(key);

	lua_rawset(luaState, -3);
	lua_pop(luaState, 1);
	return 0;
}

// __newindex
int32_t ScriptContextLua::restrictedAccessWrite(lua_State* luaState)
{
	ScriptContextLua* this_ = reinterpret_cast< ScriptContextLua* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT(this_);
	T_ASSERT(this_->m_scriptManager);

	// Check if global has been defined, thus allow write.
	const char* key = lua_tostring(luaState, -2);
	if (key != 0 && this_->m_globals.find(key) != this_->m_globals.end())
	{
		lua_rawset(luaState, -3);
		lua_pop(luaState, 1);
		return 1;
	}

	// No such global has been defined, trap as error.
	log::error << L"LUA RUNTIME ERROR; Debugger halted if attached." << Endl;
	if (key)
		log::error << L"GLOBAL access is restricted; cannot define new global \"" << mbstows(key) << L"\"." << Endl;
	else
		log::error << L"GLOBAL access is restricted; cannot define new globals." << Endl;

	this_->m_scriptManager->breakDebugger(luaState);
	return 0;
}

// __index
int32_t ScriptContextLua::restrictedAccessRead(lua_State* luaState)
{
	ScriptContextLua* this_ = reinterpret_cast< ScriptContextLua* >(lua_touserdata(luaState, lua_upvalueindex(1)));
	T_ASSERT(this_);
	T_ASSERT(this_->m_scriptManager);

	// Read from this context's environment table first.
	lua_pushvalue(luaState, -1);
	lua_rawget(luaState, -3);
	if (!lua_isnil(luaState, -1))
		return 1;
	lua_pop(luaState, 1);

	// Read from global table second.
	lua_getglobal(luaState, "_G");
	lua_pushvalue(luaState, -2);
	lua_rawget(luaState, -2);
	if (!lua_isnil(luaState, -1))
		return 1;
	lua_pop(luaState, 2);

	// Either no such variable exist or it's "nil", check if
	// it's been declared when this context was loaded.
	const char* key = lua_tostring(luaState, -1);
	if (key != 0 && this_->m_globals.find(key) != this_->m_globals.end())
	{
		lua_pushnil(luaState);
		return 1;
	}

	// No such variable exist thus issue a runtime error.
	log::error << L"LUA RUNTIME ERROR; Debugger halted if attached." << Endl;
	log::error << L"GLOBAL access is restricted; cannot read undefined global \"" << mbstows(key) << L"\"." << Endl;
	this_->m_scriptManager->breakDebugger(luaState);
	return 0;
}

}

/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/CoreClassFactory2.h"

#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes/BoxedStdVector.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Class/IRuntimeDelegate.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/Base64.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/HashStream.h"
#include "Core/Misc/MD5.h"
#include "Core/Misc/SHA1.h"
#include "Core/Thread/Result.h"
#include "Core/Timer/Timer.h"

namespace traktor
{
namespace
{

class ResultDeferred : public Result::IDeferred
{
public:
	explicit ResultDeferred(IRuntimeDelegate* delegate)
		: m_delegate(delegate)
	{
	}

	virtual void dispatch(const Result& result) const
	{
		const Any argv[] = {
			Any::fromObject(const_cast< Result* >(&result))
		};
		m_delegate->call(sizeof_array(argv), argv);
	}

private:
	Ref< IRuntimeDelegate > m_delegate;
};

class BoxedOption : public Boxed
{
	T_RTTI_CLASS;

public:
	explicit BoxedOption(const CommandLine::Option& option)
		: m_option(option)
	{
	}

	const std::wstring& getLongName() const { return m_option.getLongName(); }

	int32_t getInteger() const { return m_option.getInteger(); }

	const std::wstring& getString() const { return m_option.getString(); }

	virtual std::wstring toString() const
	{
		return L"CommandLine::Option";
	}

private:
	const CommandLine::Option& m_option;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.CommandLine.Option", BoxedOption, Boxed)

bool CommandLine_hasOption(CommandLine* self, const std::wstring& longName)
{
	return self->hasOption(longName);
}

Ref< BoxedOption > CommandLine_getOption(CommandLine* self, const std::wstring& longName)
{
	return new BoxedOption(self->getOption(longName));
}

void Result_defer(Result* self, IRuntimeDelegate* delegate)
{
	self->defer(new ResultDeferred(delegate));
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.CoreClassFactory2", 0, CoreClassFactory2, IRuntimeClassFactory)

void CoreClassFactory2::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classTimer = new AutoRuntimeClass< Timer >();
	classTimer->addConstructor();
	classTimer->addProperty("elapsedTime", &Timer::getElapsedTime);
	classTimer->addProperty("deltaTime", &Timer::getDeltaTime);
	classTimer->addMethod("reset", &Timer::reset);
	registrar->registerClass(classTimer);

	auto classIHash = new AutoRuntimeClass< IHash >();
	classIHash->addMethod("begin", &IHash::begin);
	classIHash->addMethod("end", &IHash::end);
	registrar->registerClass(classIHash);

	auto classAdler32 = new AutoRuntimeClass< Adler32 >();
	classAdler32->addConstructor();
	classAdler32->addMethod("get", &Adler32::get);
	registrar->registerClass(classAdler32);

	auto classBase64 = new AutoRuntimeClass< Base64 >();
	registrar->registerClass(classBase64);

	auto classCommandLine = new AutoRuntimeClass< CommandLine >();
	classCommandLine->addConstructor< const std::wstring&, const std::wstring& >();
	classCommandLine->addProperty("file", &CommandLine::getFile);
	classCommandLine->addProperty("count", &CommandLine::getCount);
	classCommandLine->addMethod("hasOption", CommandLine_hasOption);
	classCommandLine->addMethod("getOption", CommandLine_getOption);
	classCommandLine->addMethod("getInteger", &CommandLine::getInteger);
	classCommandLine->addMethod("getString", &CommandLine::getString);
	registrar->registerClass(classCommandLine);

	auto classCommandLineOption = new AutoRuntimeClass< BoxedOption >();
	classCommandLineOption->addMethod("getLongName", &BoxedOption::getLongName);
	classCommandLineOption->addMethod("getInteger", &BoxedOption::getInteger);
	classCommandLineOption->addMethod("getString", &BoxedOption::getString);
	registrar->registerClass(classCommandLineOption);

	auto classMD5 = new AutoRuntimeClass< MD5 >();
	classMD5->addConstructor();
	classMD5->addMethod("create", &MD5::create);
	classMD5->addMethod("createFromString", &MD5::createFromString);
	classMD5->addMethod("format", &MD5::format);
	registrar->registerClass(classMD5);

	auto classSHA1 = new AutoRuntimeClass< SHA1 >();
	classSHA1->addConstructor();
	classSHA1->addMethod("createFromString", &SHA1::createFromString);
	classSHA1->addMethod("format", &SHA1::format);
	registrar->registerClass(classSHA1);

	auto classHashStream = new AutoRuntimeClass< HashStream >();
	classHashStream->addConstructor< IHash* >();
	registrar->registerClass(classHashStream);

	auto classResult = new AutoRuntimeClass< Result >();
	classResult->addProperty("ready", &Result::ready);
	classResult->addProperty("succeeded", &Result::succeeded);
	classResult->addMethod("succeed", &Result::succeed);
	classResult->addMethod("fail", &Result::fail);
	classResult->addMethod("defer", &Result_defer);
	classResult->addMethod("wait", &Result::succeeded); // Mapping wait to succeeded, will block until result is available.
	registrar->registerClass(classResult);
}

}

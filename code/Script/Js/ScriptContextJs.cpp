#include "Script/Js/ScriptContextJs.h"
#include "Script/IScriptClass.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/TString.h"

namespace traktor
{
	namespace script
	{
		namespace
		{

struct ConstructorData
{
	Ref< ScriptContextJs > scriptContext;
	Ref< IScriptClass > scriptClass;
};

struct FunctionData
{
	Ref< ScriptContextJs > scriptContext;
	Ref< IScriptClass > scriptClass;
	uint32_t methodId;
};

// The callback that is invoked by v8 whenever the JavaScript 'print'
// function is called.  Prints its arguments on stdout separated by
// spaces and ending with a newline.
v8::Handle< v8::Value > printCallback(const v8::Arguments& args)
{
	for (int i = 0; i < args.Length(); i++)
	{
		v8::HandleScope handleScope;
		if (i > 0)
			log::info << L" ";
		v8::String::Utf8Value str(args[i]);
		log::info << mbstows(*str);
	}
	log::info << Endl;
	return v8::Undefined();
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptContextJs", ScriptContextJs, IScriptContext)

ScriptContextJs::ScriptContextJs()
{
}

ScriptContextJs::~ScriptContextJs()
{
	destroy();
}

bool ScriptContextJs::create(const RefArray< IScriptClass >& registeredClasses)
{
	v8::HandleScope handleScope;

	// Register common functions.
	v8::Local< v8::ObjectTemplate > globalObjectTemplate = v8::ObjectTemplate::New();
	globalObjectTemplate->Set(v8::String::New("print"), v8::FunctionTemplate::New(printCallback));

	m_context = v8::Context::New(0, globalObjectTemplate);
	if (m_context.IsEmpty())
		return false;

	v8::Context::Scope contextScope(m_context);

	// Setup all registered classes.
	for (RefArray< IScriptClass >::const_iterator i = registeredClasses.begin(); i != registeredClasses.end(); ++i)
	{
		IScriptClass* scriptClass = *i;
		T_ASSERT (scriptClass);

		const TypeInfo& exportType = scriptClass->getExportType();
		std::vector< std::wstring > exportPath;
		Split< std::wstring >::any(exportType.getName(), L".", exportPath);

		// Create function template for class.
		ConstructorData* constructorData = new ConstructorData();
		constructorData->scriptContext = this;
		constructorData->scriptClass = scriptClass;

		v8::Local< v8::FunctionTemplate > classTemplate = v8::FunctionTemplate::New(
			&ScriptContextJs::invokeConstructor,
			v8::External::New(constructorData)
		);
		classTemplate->SetClassName(createString(
			exportPath.back()
		));

		// Create function for each script method.
		for (uint32_t j = 0; j < scriptClass->getMethodCount(); ++j)
		{
			FunctionData* functionData = new FunctionData();
			functionData->scriptContext = this;
			functionData->scriptClass = scriptClass;
			functionData->methodId = j;

			v8::Local< v8::FunctionTemplate > methodTemplate = v8::FunctionTemplate::New(
				&ScriptContextJs::invokeMethod,
				v8::External::New(functionData)
			);

			classTemplate->PrototypeTemplate()->Set(
				createString(scriptClass->getMethodName(j)),
				methodTemplate
			);
		}

		// Setup inheritance.
		const TypeInfo* superType = scriptClass->getExportType().getSuper();
		T_ASSERT (superType);

		for (std::vector< RegisteredClass >::const_iterator j = m_classRegistry.begin(); j != m_classRegistry.end(); ++j)
		{
			if (superType == &j->scriptClass->getExportType())
			{
				classTemplate->Inherit(j->functionTemplate);
				break;
			}
		}

		// Ensure we have an internal field available for C++ object pointer.
		v8::Local< v8::ObjectTemplate > objectTemplate = classTemplate->InstanceTemplate();
		objectTemplate->SetInternalFieldCount(1);

		// Export class function.
		v8::Local< v8::Object > classNsObject = m_context->Global();
		for (std::vector< std::wstring >::const_iterator j = exportPath.begin(); j != exportPath.end() - 1; ++j)
		{
			v8::Handle< v8::String > exportPathName = createString(*j);

			v8::Local< v8::Value > exportPathObject = classNsObject->Get(exportPathName);
			if (exportPathObject->IsUndefined())
			{
				v8::Local< v8::Object > childNsObject = v8::Object::New();
				classNsObject->Set(exportPathName, childNsObject);
				classNsObject = childNsObject;
			}
			else
				classNsObject = exportPathObject->ToObject();
		}
		classNsObject->Set(
			createString(exportPath.back()),
			classTemplate->GetFunction()
		);

		RegisteredClass registeredClass =
		{ 
			scriptClass,
			v8::Persistent< v8::FunctionTemplate >::New(classTemplate),
			v8::Persistent< v8::Function >::New(classTemplate->GetFunction())
		};
		m_classRegistry.push_back(registeredClass);
	}

	return true;
}

void ScriptContextJs::destroy()
{
	m_context.Dispose();
}

void ScriptContextJs::setGlobal(const std::wstring& globalName, const Any& globalValue)
{
	v8::Context::Scope contextScope(m_context);
	v8::HandleScope handleScope;

	m_context->Global()->Set(
		createString(globalName),
		toValue(globalValue)
	);
}

Any ScriptContextJs::getGlobal(const std::wstring& globalName)
{
	v8::Context::Scope contextScope(m_context);
	v8::HandleScope handleScope;

	v8::Local< v8::Value > value = m_context->Global()->Get(createString(globalName));
	return fromValue(value);
}

bool ScriptContextJs::executeScript(const std::wstring& script, bool compileOnly, IErrorCallback* errorCallback)
{
	v8::Context::Scope contextScope(m_context);
	v8::HandleScope handleScope;
	v8::TryCatch trycatch;

	v8::Local< v8::String > str = v8::Local< v8::String >::New(createString(script));
	
	v8::Local< v8::Script > obj = v8::Script::Compile(str);
	if (obj.IsEmpty())
	{
		v8::Handle< v8::Value > exception = trycatch.Exception();
		v8::String::AsciiValue xs(exception);
		if (errorCallback)
			errorCallback->syntaxError(0, mbstows(*xs));
		else
			log::error << L"Unhandled JavaScript exception occurred \"" << mbstows(*xs) << L"\"" << Endl;
		return false;
	}

	if (compileOnly)
		return true;

	v8::Local< v8::Value > result = obj->Run();
	if (result.IsEmpty())
	{
		v8::Handle< v8::Value > exception = trycatch.Exception();
		v8::String::AsciiValue xs(exception);
		if (errorCallback)
			errorCallback->otherError(mbstows(*xs));
		else
			log::error << L"Unhandled JavaScript exception occurred \"" << mbstows(*xs) << L"\"" << Endl;
		return false;
	}

	return true;
}

bool ScriptContextJs::haveFunction(const std::wstring& functionName) const
{
	v8::Context::Scope contextScope(m_context);
	v8::HandleScope handleScope;

	return m_context->Global()->Has(createString(functionName));
}

Any ScriptContextJs::executeFunction(const std::wstring& functionName, uint32_t argc, const Any* argv)
{
	v8::Context::Scope contextScope(m_context);
	v8::HandleScope handleScope;
	v8::TryCatch trycatch;

	v8::Local< v8::Value > value = m_context->Global()->Get(createString(functionName));
	if (value.IsEmpty())
		return Any();

	v8::Local< v8::Function > function = v8::Local< v8::Function >::Cast(value);
	if (function.IsEmpty())
		return Any();

	std::vector< v8::Local< v8::Value > > av(argc);
	for (uint32_t i = 0; i < argc; ++i)
		av[i] = v8::Local< v8::Value >::New(toValue(argv[i]));

	v8::Local< v8::Value > result = function->Call(
		m_context->Global(),
		av.size(),
		av.size() > 0 ? &av[0] : 0
	);

	if (result.IsEmpty())
	{
		v8::Handle< v8::Value > exception = trycatch.Exception();
		v8::String::AsciiValue xs(exception);
		log::error << L"Unhandled JavaScript exception occurred \"" << mbstows(*xs) << L"\"" << Endl;
		return Any();
	}

	return fromValue(result);
}

Any ScriptContextJs::executeMethod(Object* self, const std::wstring& methodName, uint32_t argc, const Any* argv)
{
	v8::Context::Scope contextScope(m_context);
	v8::HandleScope handleScope;
	v8::TryCatch trycatch;

	v8::Local< v8::Value > value = m_context->Global()->Get(createString(methodName));
	if (value.IsEmpty())
		return Any();

	v8::Local< v8::Function > function = v8::Local< v8::Function >::Cast(value);
	if (function.IsEmpty())
		return Any();

	std::vector< v8::Local< v8::Value > > av(argc);
	for (uint32_t i = 0; i < argc; ++i)
		av[i] = v8::Local< v8::Value >::New(toValue(argv[i]));

	v8::Handle< v8::Value > recv = createObject(self);

	v8::Local< v8::Value > result = function->Call(
		recv->ToObject(),
		av.size(),
		&av[0]
	);

	if (result.IsEmpty())
	{
		v8::Handle< v8::Value > exception = trycatch.Exception();
		v8::String::AsciiValue xs(exception);
		log::error << L"Unhandled JavaScript exception occurred \"" << mbstows(*xs) << L"\"" << Endl;
		return Any();
	}

	return fromValue(result);
}

v8::Handle< v8::Value > ScriptContextJs::invokeConstructor(const v8::Arguments& arguments)
{
	v8::Local< v8::External > constructorDataX = v8::Local< v8::External >::Cast(arguments.Data());
	ConstructorData* constructorData = static_cast< ConstructorData* >(constructorDataX->Value());

	v8::Local< v8::External > objectRefExternal;
	if (arguments[0]->IsExternal())
	{
		objectRefExternal = v8::Local< v8::External >::Cast(arguments[0]);
	}
	else
	{
		if (!constructorData->scriptClass->haveConstructor())
			return v8::Undefined();

		Any argv[16];
		for (int i = 0; i < arguments.Length(); ++i)
			argv[i] = constructorData->scriptContext->fromValue(arguments[i]);

		Ref< Object > object = constructorData->scriptClass->construct(arguments.Length(), argv);
		Ref< Object >* objectRef = new Ref< Object >(object);

		objectRefExternal = v8::External::New(objectRef);
	}

	arguments.This()->SetInternalField(0, objectRefExternal);
	return arguments.This();
}

v8::Handle< v8::Value > ScriptContextJs::invokeMethod(const v8::Arguments& arguments)
{
	v8::Local< v8::Object > vthis = arguments.This();
	if (vthis->InternalFieldCount() <= 0)
		return v8::Undefined();

	v8::Local< v8::External > objectX = v8::Local< v8::External >::Cast(vthis->GetInternalField(0));
	Ref< Object >* objectRef = static_cast< Ref< Object >* >(objectX->Value());
	Ref< Object > object = *objectRef;

	v8::Local< v8::External > functionDataX = v8::Local< v8::External >::Cast(arguments.Data());
	FunctionData* functionData = static_cast< FunctionData* >(functionDataX->Value());

	Any argv[16];
	for (int i = 0; i < arguments.Length(); ++i)
		argv[i] = functionData->scriptContext->fromValue(arguments[i]);

	Any result = functionData->scriptClass->invoke(
		object,
		functionData->methodId,
		arguments.Length(),
		argv
	);

	return functionData->scriptContext->toValue(result);
}

void ScriptContextJs::weakHandleCallback(v8::Persistent< v8::Value > object, void* parameter)
{
	Ref< Object >* objectRef = reinterpret_cast< Ref< Object >* >(parameter);
	T_ASSERT (objectRef);

	delete objectRef;
}

v8::Handle< v8::String > ScriptContextJs::createString(const std::wstring& s) const
{
	return v8::String::New((const uint16_t*)s.c_str());
}

v8::Handle< v8::Value > ScriptContextJs::createObject(Object* object) const
{
	if (object)
	{
		const TypeInfo* objectType = &type_of(object);
		T_ASSERT (objectType);

		v8::Handle< v8::Function > minFunction;
		uint32_t minScriptClassDiff = ~0UL;

		for (std::vector< RegisteredClass >::const_iterator i = m_classRegistry.begin(); i != m_classRegistry.end(); ++i)
		{
			uint32_t scriptClassDiff = type_difference(i->scriptClass->getExportType(), *objectType);
			if (scriptClassDiff < minScriptClassDiff)
			{
				minFunction = i->function;
				minScriptClassDiff = scriptClassDiff;
			}
		}

		if (!minFunction.IsEmpty())
		{
			Ref< Object >* objectRef = new Ref< Object >(object);

			v8::Local< v8::External > objectRefExternal = v8::External::New(objectRef);

			v8::Handle< v8::Value > args(objectRefExternal);
			v8::Persistent< v8::Object > instanceHandle(minFunction->NewInstance(1, &args));

			instanceHandle.MakeWeak(objectRef, &ScriptContextJs::weakHandleCallback);
			return instanceHandle;
		}
	}
	return v8::Null();
}

v8::Handle< v8::Value > ScriptContextJs::toValue(const Any& value) const
{
	if (value.isBoolean())
	{
		if (value.getBoolean())
			return v8::True();
		else
			return v8::False();
	}
	if (value.isInteger())
		return v8::Int32::New(value.getInteger());
	if (value.isFloat())
		return v8::Number::New(value.getFloat());
	if (value.isString())
		return createString(value.getString());
	if (value.isObject())
		return createObject(value.getObject());	
	
	return v8::Undefined();
}

Any ScriptContextJs::fromValue(v8::Handle< v8::Value > value) const
{
	if (value.IsEmpty())
		return Any();
	if (value->IsTrue())
		return Any(true);
	if (value->IsFalse())
		return Any(false);
	if (value->IsString())
	{
		v8::String::Utf8Value str(value);
		return Any(mbstows(*str));
	}
	if (value->IsInt32())
		return Any(int32_t(value->ToInt32()->Value()));
	if (value->IsNumber())
		return Any(float(value->ToNumber()->Value()));
	if (value->IsObject())
	{
		v8::Local< v8::Object > objectWrapper = value->ToObject();
		v8::Local< v8::External > objectExternal = v8::Local< v8::External >::Cast(objectWrapper->GetInternalField(0));
		if (objectExternal->IsExternal())
		{
			Ref< Object >* objectRef = static_cast< Ref< Object >* >(objectExternal->Value());
			Ref< Object > object = *objectRef;
			return Any(object);
		}
	}
	
	return Any();
}

	}
}

#include "Core/Class/CastAny.h"
#include "Core/Class/IRuntimeClass.h"
#include "Core/Class/IRuntimeDispatch.h"
#include "Model/Model.h"
#include "Model/Operations/ExecuteScript.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.ExecuteScript", ExecuteScript, IModelOperation)

ExecuteScript::ExecuteScript(const IRuntimeClass* scriptClass)
:   m_scriptClass(scriptClass)
{
}

bool ExecuteScript::apply(Model& model) const
{
    auto scriptDispatch = findRuntimeClassMethod(m_scriptClass, "apply");
    if (!scriptDispatch)
        return false;

    Ref< ITypedObject > scriptObject = createRuntimeClassInstance(
        m_scriptClass,
        const_cast< ExecuteScript* >(this),
        0,
        nullptr
    );
    if (!scriptObject)
        return false;

    Any argv[] = { CastAny< Model* >::set(&model) };
    Any result = scriptDispatch->invoke(
        scriptObject,
        sizeof_array(argv),
        argv
    );

    return CastAny< bool >::get(result);
}

	}
}

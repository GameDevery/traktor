#pragma once

#include "Resource/Id.h"
#include "Spark/IComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IRuntimeClass;

	namespace spark
	{

/*! \brief Script component.
 * \ingroup Spark
 */
class T_DLLCLASS ScriptComponentData : public IComponentData
{
	T_RTTI_CLASS;

public:
	virtual Ref< IComponent > createInstance(const Context* context, Sprite* owner) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	friend class CharacterPipeline;

	resource::Id< IRuntimeClass > m_class;
};

	}
}


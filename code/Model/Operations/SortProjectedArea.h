#pragma once

#include "Model/IModelOperation.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MODEL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace model
	{

/*!
 * \ingroup Model
 */
class T_DLLCLASS SortProjectedArea : public IModelOperation
{
	T_RTTI_CLASS;

public:
	explicit SortProjectedArea(bool insideOut);

	virtual bool apply(Model& model) const override final;

private:
	bool m_insideOut;
};

	}
}


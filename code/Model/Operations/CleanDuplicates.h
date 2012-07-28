#ifndef traktor_model_CleanDuplicates_H
#define traktor_model_CleanDuplicates_H

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

/*! \brief
 * \ingroup Model
 */
class T_DLLCLASS CleanDuplicates : public IModelOperation
{
	T_RTTI_CLASS;

public:
	virtual bool apply(Model& model) const;
};

	}
}

#endif	// traktor_model_CleanDuplicates_H

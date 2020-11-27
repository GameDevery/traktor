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
class T_DLLCLASS Quantize : public IModelOperation
{
	T_RTTI_CLASS;

public:
	explicit Quantize(float step);

	explicit Quantize(const Vector4& step);

	virtual bool apply(Model& model) const override final;

private:
	Vector4 m_step;
};

	}
}


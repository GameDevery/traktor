#pragma once

#include "Core/Math/Matrix44.h"
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
class T_DLLCLASS Transform : public IModelOperation
{
	T_RTTI_CLASS;

public:
	explicit Transform(const Matrix44& tf);

	virtual bool apply(Model& model) const override final;

private:
	Matrix44 m_transform;
};

	}
}


#pragma once

#include "Core/Math/Transform.h"
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
class T_DLLCLASS Boolean : public IModelOperation
{
	T_RTTI_CLASS;

public:
	enum BooleanOperation
	{
		BoUnion,
		BoIntersection,
		BoDifference
	};

	Boolean(
		const Model& modelA, const Transform& modelTransformA,
		const Model& modelB, const Transform& modelTransformB,
		BooleanOperation operation
	);

	virtual bool apply(Model& model) const override final;

private:
	const Model& m_modelA;
	const Model& m_modelB;
	Transform m_modelTransformA;
	Transform m_modelTransformB;
	BooleanOperation m_operation;
};

	}
}


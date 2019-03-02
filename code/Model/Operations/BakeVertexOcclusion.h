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

/*! \brief
 * \ingroup Model
 */
class T_DLLCLASS BakeVertexOcclusion : public IModelOperation
{
	T_RTTI_CLASS;

public:
	BakeVertexOcclusion(
		int32_t rayCount = 64,
		float raySpread = 0.75f,
		float rayBias = 0.1f
	);

	virtual bool apply(Model& model) const override final;

private:
	int32_t m_rayCount;
	float m_raySpread;
	float m_rayBias;
};

	}
}


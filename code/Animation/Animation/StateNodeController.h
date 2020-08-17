#pragma once

#include "Animation/IPoseController.h"
#include "Animation/Animation/StateContext.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

class StateNode;

/*! \brief
 * \ingroup Animation
 */
class T_DLLCLASS StateNodeController : public IPoseController
{
	T_RTTI_CLASS;

public:
	StateNodeController(StateNode* node);

	virtual void destroy() override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual bool evaluate(
		float time,
		float deltaTime,
		const Transform& worldTransform,
		const Skeleton* skeleton,
		const AlignedVector< Transform >& jointTransforms,
		AlignedVector< Transform >& outPoseTransforms,
		bool& outUpdateController
	) override final;

	virtual void estimateVelocities(
		const Skeleton* skeleton,
		AlignedVector< Velocity >& outVelocities
	) override final;

private:
	Ref< StateNode > m_node;
	StateContext m_context;
	bool m_initialized;
};

	}
}


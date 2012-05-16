#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonUtils.h"
#include "Animation/Animation/StateNode.h"
#include "Animation/Animation/StateGraph.h"
#include "Animation/Animation/StatePoseController.h"
#include "Animation/Animation/Transition.h"
#include "Core/Math/Const.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.StatePoseController", StatePoseController, IPoseController)

StatePoseController::StatePoseController(const resource::Proxy< StateGraph >& stateGraph)
:	m_stateGraph(stateGraph)
,	m_blendState(0.0f)
,	m_blendDuration(0.0f)
,	m_timeFactor(1.0f)
{
}

void StatePoseController::setCondition(const std::wstring& condition, bool enabled)
{
	m_conditions[condition] = enabled;
}

void StatePoseController::setTimeFactor(float timeFactor)
{
	m_timeFactor = timeFactor;
}

void StatePoseController::destroy()
{
}

void StatePoseController::setTransform(const Transform& transform)
{
}

void StatePoseController::evaluate(
	float deltaTime,
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& jointTransforms,
	AlignedVector< Transform >& outPoseTransforms,
	bool& outUpdateController
)
{
	Pose currentPose;

	// Prepare graph evaluation context.
	if (m_stateGraph.changed() || !m_currentState)
	{
		m_currentState = m_stateGraph->getRootState();
		if (m_currentState)
			m_currentState->prepareContext(m_currentStateContext);
		m_nextState = 0;
		m_blendState = 0.0f;
		m_blendDuration = 0.0f;
		m_stateGraph.consume();
	}

	if (!m_currentState)
		return;

	// Evaluate current state.
	m_currentState->evaluate(
		m_currentStateContext,
		currentPose
	);
	m_currentStateContext.setTime(m_currentStateContext.getTime() + deltaTime * m_timeFactor);

	// Build final pose transforms.
	if (m_nextState)
	{
		// Only blend between states if there is a transition time.
		if (m_blendDuration > 0.0f)
		{
			Pose nextPose, blendPose;

			m_nextState->evaluate(
				m_nextStateContext,
				nextPose
			);
			m_nextStateContext.setTime(m_nextStateContext.getTime() + deltaTime * m_timeFactor);

			Scalar blend = Scalar(sinf((m_blendState / m_blendDuration) * PI / 2.0f));

			blendPoses(
				&currentPose,
				&nextPose,
				blend,
				&blendPose
			);

			calculatePoseTransforms(
				skeleton,
				&blendPose,
				outPoseTransforms
			);
		}
		else
		{
			calculatePoseTransforms(
				skeleton,
				&currentPose,
				outPoseTransforms
			);
		}

		// Swap in next state when we've completely blended into it.
		m_blendState += deltaTime;
		if (m_blendState >= m_blendDuration)
		{
			m_currentState = m_nextState;
			m_currentStateContext = m_nextStateContext;
			m_nextState = 0;
			m_blendState = 0.0f;
			m_blendDuration = 0.0f;
		}
	}
	else
	{
		calculatePoseTransforms(
			skeleton,
			&currentPose,
			outPoseTransforms
		);
	}

	// Execute transition to another state.
	if (!m_nextState)
	{
		const RefArray< Transition >& transitions = m_stateGraph->getTransitions();

		// First try all transitions with explicit condition.
		for (RefArray< Transition >::const_iterator i = transitions.begin(); i != transitions.end(); ++i)
		{
			if ((*i)->from() != m_currentState || (*i)->getCondition().empty())
				continue;

			// Is transition permitted?
			bool transitionPermitted = false;
			switch ((*i)->getMoment())
			{
			case Transition::TmImmediatly:
				transitionPermitted = true;
				break;

			case Transition::TmEnd:
				{
					float timeLeft = max(m_currentStateContext.getDuration() - m_currentStateContext.getTime(), 0.0f);
					if (timeLeft <= (*i)->getDuration())
						transitionPermitted = true;
				}
				break;
			}
			if (!transitionPermitted)
				continue;

			// Is condition satisfied?
			bool value = false;

			std::wstring condition = (*i)->getCondition();
			if (condition[0] == L'!')
				value = !m_conditions[condition.substr(1)];
			else
				value = m_conditions[condition];

			if (!value)
				continue;

			// Begin transition to found state.
			m_nextState = (*i)->to();
			m_nextState->prepareContext(m_nextStateContext);
			m_blendState = 0.0f;
			m_blendDuration = (*i)->getDuration();
			break;
		}

		// Still no transition state found, we try all transitions without explicit condition.
		for (RefArray< Transition >::const_iterator i = transitions.begin(); i != transitions.end(); ++i)
		{
			if ((*i)->from() != m_currentState || !(*i)->getCondition().empty())
				continue;

			// Is transition permitted?
			bool transitionPermitted = false;
			switch ((*i)->getMoment())
			{
			case Transition::TmImmediatly:
				transitionPermitted = true;
				break;

			case Transition::TmEnd:
				{
					float timeLeft = max(m_currentStateContext.getDuration() - m_currentStateContext.getTime(), 0.0f);
					if (timeLeft <= (*i)->getDuration())
						transitionPermitted = true;
				}
				break;
			}
			if (!transitionPermitted)
				continue;

			// Begin transition to found state.
			m_nextState = (*i)->to();
			m_nextState->prepareContext(m_nextStateContext);
			m_blendState = 0.0f;
			m_blendDuration = (*i)->getDuration();
			break;
		}
	}
}

void StatePoseController::estimateVelocities(
	const Skeleton* skeleton,
	AlignedVector< Velocity >& outVelocities
)
{
}

	}
}

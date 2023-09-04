/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Containers/SmallMap.h"
#include "Animation/IPoseController.h"
#include "Animation/Pose.h"
#include "Animation/Animation/StateContext.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::animation
{

class AnimationGraph;
class ITransformTime;
class StateNode;
class StateContext;

/*! Animation pose evaluation controller.
 * \ingroup Animation
 */
class T_DLLCLASS AnimationGraphPoseController : public IPoseController
{
	T_RTTI_CLASS;

public:
	explicit AnimationGraphPoseController(const resource::Proxy< AnimationGraph >& stateGraph, ITransformTime* transformTime);

	bool setState(const std::wstring& stateName);

	void setCondition(const std::wstring& condition, bool enabled, bool reset);

	void setTime(float time);

	float getTime() const;

	void setTimeFactor(float timeFactor);

	float getTimeFactor() const;

	virtual void destroy() override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual bool evaluate(
		float time,
		float deltaTime,
		const Transform& worldTransform,
		const Skeleton* skeleton,
		const AlignedVector< Transform >& jointTransforms,
		AlignedVector< Transform >& outPoseTransforms
	) override final;

private:
	resource::Proxy< AnimationGraph > m_animationGraph;
	Ref< ITransformTime > m_transformTime;
	Ref< StateNode > m_currentState;
	StateContext m_currentStateContext;
	Ref< StateNode > m_nextState;
	StateContext m_nextStateContext;
	Pose m_evaluatePose;
	float m_blendState;
	float m_blendDuration;
	SmallMap< std::wstring, std::pair< bool, bool > > m_conditions;
	float m_timeFactor;
};

}

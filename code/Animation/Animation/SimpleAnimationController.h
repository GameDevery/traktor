/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include "Animation/IPoseController.h"
#include "Animation/Pose.h"
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

class Animation;
class ITransformTime;

/*!
 * \ingroup Animation
 */
class T_DLLCLASS SimpleAnimationController : public IPoseController
{
	T_RTTI_CLASS;

public:
	explicit SimpleAnimationController(
		const resource::Proxy< Animation >& animation,
		ITransformTime* transformTime
	);

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
	resource::Proxy< Animation > m_animation;
	Ref< ITransformTime > m_transformTime;
	float m_timeOffset;
	float m_lastTime;
	Pose m_evaluationPose;
};

}
